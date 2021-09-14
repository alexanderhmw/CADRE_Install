/**********************************************************
  Simple Comms
  Copyright (c) 2010-2013 Carnegie Mellon University,
  All rights reserved.

  This source code was developed in part with support from 
  sponsors including General Motors, the National Science 
  Foundation and the US Department of Transportation.

  Use of this software is strictly governed by agreements 
  between Carnegie Mellon University and various sponsors.

  Users of this software must also be fully aware and agree 
  that Carnegie Mellon does not guarantee the correct 
  functioning of this software in any system. Carnegie 
  Mellon or any of its affiliates will not be liable for any 
  damage and/or penalties resulting from the use of this 
  software. Any user of this software takes complete 
  responsibility for the use of any software or design.
 **********************************************************/

/**
 * @file      UdpDiscovery.cc
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "UdpDiscovery.h"

#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <limits.h>

#include "ControlMessages.h"
#include "NetworkIntfUtils.h"

using namespace std;

namespace SimpleComms
{

UdpDiscovery::UdpDiscovery(const std::string &advertisedIntf, const uint16_t port)
    : advertisedIntf_(advertisedIntf),
      port_(port),
      broadcastAddress_(INADDR_BROADCAST),
      fd_(-1),
      rollingMessageID_(0),
      threadRunning_(0)
{
}

UdpDiscovery::~UdpDiscovery()
{
}

Status::States UdpDiscovery::initialize()
{
    if (threadRunning_)
    {
        return Status::AlreadyConnected;
    }

    {
        char hostNameBuf[HOST_NAME_MAX];

        if (gethostname(hostNameBuf, HOST_NAME_MAX) == -1)
        {
            return Status::Error;
        }

        hostName_ = hostNameBuf;
    }

    if (!NetworkIntfUtils::GetInterfaceParameter(advertisedIntf_, NetworkIntfUtils::Broadcast, broadcastAddress_))
    {
        logger_.log_warn("Error retrieving broadcast address for %s", advertisedIntf_.c_str());

        return Status::Error;
    }

    if ((fd_ = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        logger_.log_warn("Error creating socket: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

        return Status::ErrorCreatingSocket;
    }

    {
        int broadcast = 1;
        if (setsockopt(fd_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(int))) 
        {
            logger_.log_warn("Error setting SO_BROADCAST: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

            terminate();

            return Status::Error;
        }
    }

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port_);
    servAddr.sin_addr.s_addr = htonl(broadcastAddress_);
    
    if (bind(fd_, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) == -1) 
    {
        terminate();

        return Status::ErrorBindingSocket;
    }

    if (fcntl(fd_, F_SETFL, O_NONBLOCK) == -1)
    {
        terminate();

        return Status::Error;
    }

    // Spawn a thread that sits on fd_ and queues the incoming data
    if (pthread_create(&discoveryThreadHandle_, NULL, discoveryThreadLauncher, this) == 0)
    {
        // Flag that we successfully launched a thread for later joining
        threadRunning_++;
    }
    else
    {
        terminate();

        return Status::Error;
    }

    instanceEpoch_.setNow();

    return Status::Ok;
}

Status::States UdpDiscovery::isActive()
{
    return (fd_ == -1) ? Status::Error : Status::Ok;
}

Status::States UdpDiscovery::terminate()
{
    return terminate(false);
}

Status::States UdpDiscovery::registerLocalPeeringMethod(const PeerPtr localPeer)
{
    localPeeringMethods_[localPeer->getTransport()] = localPeer;

    return Status::Ok;
}

Status::States UdpDiscovery::unregisterLocalPeeringMethod(const PeerPtr localPeer)
{
    PeeringMethodsMap::iterator i = localPeeringMethods_.find(localPeer->getTransport());

    if (i != localPeeringMethods_.end())
    {
        localPeeringMethods_.erase(i);
    }

    return Status::Ok;
}

Status::States UdpDiscovery::registerRemotePeeringMethod(const PeerPtr remotePeer)
{
    remotePeeringMethods_[remotePeer->getTransport()] = remotePeer;

    return Status::Ok;
}

Status::States UdpDiscovery::unregisterRemotePeeringMethod(const PeerPtr remotePeer)
{
    PeeringMethodsMap::iterator i = remotePeeringMethods_.find(remotePeer->getTransport());

    if (i != remotePeeringMethods_.end())
    {
        remotePeeringMethods_.erase(i);
    }

    return Status::Ok;
}

Status::States UdpDiscovery::terminate(const bool discoveryThread)
{
    close(fd_);
    fd_ = -1;

    if (!discoveryThread && threadRunning_)
    {
        threadRunning_--;
        pthread_join(discoveryThreadHandle_, NULL);
    }

    return Status::Ok;
}

void *UdpDiscovery::discoveryThreadLauncher(void *arg)
{
    ((UdpDiscovery *)arg)->discoveryThreadMain();

    return NULL;
}

void UdpDiscovery::discoveryThreadMain()
{
    TimeStamp nextCycle;
    nextCycle.setNow();
    nextCycle = nextCycle + 1.0;

    while (fd_ != -1)
    {
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(fd_, &rfds);

        TimeStamp now;
        now.setNow();

        struct timeval waitLimit = max(nextCycle - now, TimeStamp(0, 0));

        const int selectStatus = select(fd_ + 1, &rfds, NULL, NULL, &waitLimit);

        if (selectStatus == -1)
        {
            switch (errno)
            {
            case EINTR:
                // Ignore
                break;
            default:
                logger_.log_warn("Error during select: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

                terminate(true);

                return;
            }
        }

        if (fd_ == -1)
        {
            break;
        }

        if (FD_ISSET(fd_, &rfds))
        {
            handleIncomingMsg();
        }

        now.setNow();
        if (now > nextCycle)
        {
            nextCycle = now + 1.0;

            sendSubscriptions();
            sendRegisteredRemotePeeringMethods();
        }
    }
}

Status::States UdpDiscovery::sendMsg(Message &msg, const uint32_t dataLength)
{
    memcpy(msg.origin, hostName_.c_str(), hostName_.length() + 1);
    memcpy(msg.channel, SIMPLECOMMS_CONTROL_CHANNEL, sizeof(SIMPLECOMMS_CONTROL_CHANNEL));
    gettimeofday(&msg.sendMsgTime, NULL);
    msg.length = dataLength;
    msg.rollingMessageID = rollingMessageID_++;
    msg.totalPages = 1;

    msg.pageNumber = 1;
    msg.dataLength = msg.length;

    messageSetChecksum(msg);

    struct sockaddr_in servAddr;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port_);
    servAddr.sin_addr.s_addr = htonl(broadcastAddress_);

    const int sendStatus = sendto(fd_, 
                                  reinterpret_cast<const void *>(&msg), 
                                  msg.MessageLength(), 
                                  MSG_NOSIGNAL, 
                                  reinterpret_cast<sockaddr *>(&servAddr), 
                                  sizeof(servAddr)
                                 );

    if (sendStatus == -1)
    {
        logger_.log_warn("Error sending message: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");
    }

    return (sendStatus == -1) ? Status::Error : Status::Ok;
}

Status::States UdpDiscovery::sendSubscriptions()
{
    vector<string> locallySubscribedChannels;

    for (PeeringMethodsMap::iterator i(localPeeringMethods_.begin());
         i != localPeeringMethods_.end();
         ++i
        )
    {
        (*i).second->getSubscriptions(locallySubscribedChannels);
    }

    {
        Message msg;
        SubscriptionListMessage *subListMsg = reinterpret_cast<SubscriptionListMessage *>(msg.data);

        subListMsg->type = ControlMessages::SubscriptionList;
        subListMsg->numChannels = 0;
        
        for (vector<string>::iterator i = locallySubscribedChannels.begin(); 
             i != locallySubscribedChannels.end(); 
             ++i
            )
        {
            const string &channelName(*i);

            if (channelName.length() < sizeof(subListMsg->channel[0]))
            {
                memcpy(subListMsg->channel[subListMsg->numChannels], channelName.c_str(), channelName.length() + 1);
                subListMsg->numChannels++;

                if (subListMsg->numChannels == 999)
                {
                    break;
                }
            }
        }

        return sendMsg(msg, sizeof(subListMsg->type) 
                            + sizeof(subListMsg->numChannels) 
                            + sizeof(subListMsg->channel[0]) * subListMsg->numChannels
                      );
    }

    return Status::Ok;
}

Status::States UdpDiscovery::sendRegisteredRemotePeeringMethods()
{
    Status::States returnStatus = Status::Ok;

    if (!remotePeeringMethods_.empty())
    {
        for (PeeringMethodsMap::iterator i = remotePeeringMethods_.begin(); 
             i != remotePeeringMethods_.end(); 
             ++i
            )
        {
            logger_.log_debug("Sending connection method %s", transportName(i->first));

            Message msg;
            const PeerPtr &remotePeer(i->second);
            const Status::States getConnInfoStatus = remotePeer->getConnectionInfo(msg);

            if (getConnInfoStatus == Status::Ok)
            {
                ConnectionMessage *connMsg(reinterpret_cast<ConnectionMessage *>(&msg.data));
                connMsg->offerEpoch = instanceEpoch_;

                const Status::States sendStatus = sendMsg(msg, msg.dataLength);

                if (sendStatus != Status::Ok)
                {
                    returnStatus = sendStatus;
                }
            }
            else
            {
                returnStatus = getConnInfoStatus;
            }
        }
    }

    return returnStatus;
}

Status::States UdpDiscovery::handleIncomingMsg()
{
    Status::States returnStatus = Status::Ok;
    Message msg;
    const int recvStatus = recv(fd_, reinterpret_cast<void *>(&msg), sizeof(msg), MSG_DONTWAIT | MSG_NOSIGNAL); 

    if (recvStatus == -1)
    {
        if (errno != EAGAIN)
        {
            logger_.log_warn("Error receiving message: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

            terminate(true);

            returnStatus = Status::Error;
        }
    }
    else
    {
        returnStatus = Status::NewData;
    }

    if (returnStatus == Status::NewData)
    {
        if (messageValidateChecksum(msg))
        {
            if (string(msg.channel) == SIMPLECOMMS_CONTROL_CHANNEL)
            {
                returnStatus = handleControlMsg(msg);
            }
            else
            {
                returnStatus = Status::Error;
                logger_.log_warn("Non-control message on discovery broadcast");
            }
        }
        else
        {
            returnStatus = Status::Error;
            logger_.log_warn("Checksum error on discovery broadcast");
        }
    }

    return returnStatus;
}

Status::States UdpDiscovery::handleControlMsg(const Message &msg)
{
    const string origin(msg.origin);

    // Ignore messages that originated here
    if (hostName_ != origin)
    {
        if (msg.data[0] == ControlMessages::SubscriptionList)
        {
            const SubscriptionListMessage *subListMsg = reinterpret_cast<const SubscriptionListMessage *>(msg.data);
            vector<string> subscribes, unsubscribes;

            uint32_t i = 0;
            ChannelSubscriberMap::iterator j = channelSubscribers_.begin();
            while ((i < subListMsg->numChannels) && (j != channelSubscribers_.end()))
            {
                const string channel(subListMsg->channel[i]);
                SubscriberSet &subscribers(j->second);
                const int channelCompareResult = channel.compare(j->first);

                if (channelCompareResult > 0)
                {
                    if (subscribers.erase(origin) > 0)
                    {
                        unsubscribes.push_back(j->first);
                    }

                    ++j;
                }
                else if (channelCompareResult == 0)
                {
                    const pair<SubscriberSet::iterator, bool> result = subscribers.insert(origin);

                    if (result.second)
                    {
                        subscribes.push_back(channel);
                    }

                    ++j;
                    ++i;
                }
                else if (channelCompareResult < 0)
                {
                    SubscriberSet tmpSubscriberSet;
                    tmpSubscriberSet.insert(origin);

                    channelSubscribers_.insert(j, make_pair(subListMsg->channel[i], tmpSubscriberSet));

                    subscribes.push_back(channel);

                    ++i;
                }
            }

            while (i < subListMsg->numChannels)
            {
                SubscriberSet tmpSubscriberSet;
                tmpSubscriberSet.insert(origin);

                channelSubscribers_.insert(j, make_pair(subListMsg->channel[i], tmpSubscriberSet));

                subscribes.push_back(subListMsg->channel[i]);

                ++i;
            }

            while (j != channelSubscribers_.end())
            {
                if (j->second.erase(origin) > 0)
                {
                    unsubscribes.push_back(j->first);
                }

                ++j;
            }

            for (PeeringMethodsMap::iterator i = remotePeeringMethods_.begin();
                 i != remotePeeringMethods_.end();
                 ++i
                )
            {
                updateSubscriptions(i->second, origin, subscribes, unsubscribes);
            }

            j = channelSubscribers_.end();
            for (ChannelSubscriberMap::iterator i = channelSubscribers_.begin(); 
                 i != channelSubscribers_.end(); 
                 j = i++
                )
            {
                if (channelSubscribers_.end() != j)
                {
                    if (j->second.empty())
                    {
                        channelSubscribers_.erase(j);
                    }
                }
            }
        }
        else if (msg.data[0] == ControlMessages::Connect)
        {
            const ConnectionMessage *connMsg = reinterpret_cast<const ConnectionMessage *>(msg.data);
            MessagePtr msgPtr(new(msg.dataLength) MessageStub(msg));
            bool newOffer = false;

            logger_.log_debug("Got connection offer from %s over %s", origin.c_str(), transportName(connMsg->transport));

            {
                const PeerTransportPair connInfoKey(origin, connMsg->transport);
                ConnectionInfoMap::iterator connInfoIter(connectionOffers_.find(connInfoKey));
                if (connInfoIter == connectionOffers_.end())
                {
                    newOffer = true;
                    connectionOffers_[PeerTransportPair(origin, connMsg->transport)] = msgPtr;
                }
                else
                {
                    const MessagePtr &prevMsg((*connInfoIter).second);
                    const ConnectionMessage *prevConnMsg(reinterpret_cast<ConnectionMessage *>(prevMsg->data));
                    if (TimeStamp(prevConnMsg->offerEpoch) != TimeStamp(connMsg->offerEpoch))
                    {
                        newOffer = true;
                    }

                    (*connInfoIter).second = msgPtr;
                }
            }

            PeeringMethodsMap::iterator i = remotePeeringMethods_.find(connMsg->transport);
            if (i != remotePeeringMethods_.end())
            {
                const PeerPtr &remotePeer(i->second);

                if (newOffer || !remotePeer->isConnectedTo(origin))
                {
                    if (remotePeer->handleControlMsg(msgPtr) == Status::NewData)
                    {
                        vector<string> subscribes;

                        for (ChannelSubscriberMap::iterator j = channelSubscribers_.begin(); 
                             j != channelSubscribers_.end(); 
                             ++j
                            )
                        {
                            if (j->second.find(origin) != j->second.end())
                            {
                                subscribes.push_back(j->first);
                            }
                        }

                        updateSubscriptions(remotePeer, origin, subscribes, vector<string>());
                    }
                }
            }
        }
    }

    return Status::Ok;
}

Status::States UdpDiscovery::updateSubscriptions(const PeerPtr remotePeer, 
                                                 const string &origin, 
                                                 const vector<string> &subscribes, 
                                                 const vector<string> &unsubscribes
                                                )
{
    MessagePtr tmp(new(sizeof(SubscriptionMessage)) MessageStub());

    memcpy(tmp->origin, origin.c_str(), origin.length() + 1);
    memcpy(tmp->channel, SIMPLECOMMS_CONTROL_CHANNEL, sizeof(SIMPLECOMMS_CONTROL_CHANNEL));
    tmp->length = tmp->dataLength = sizeof(SubscriptionMessage);

    for (vector<string>::const_iterator i = subscribes.begin(); i != subscribes.end(); ++i)
    {
        SubscriptionMessage *subscriptionMessage(reinterpret_cast<SubscriptionMessage *>(tmp->data));

        subscriptionMessage->type = ControlMessages::Subscribe;
        memcpy(&subscriptionMessage->channel, i->c_str(), i->length() + 1);
        messageSetChecksum(*tmp);

        remotePeer->handleControlMsg(tmp);
    }

    for (vector<string>::const_iterator i = unsubscribes.begin(); i != unsubscribes.end(); ++i)
    {
        SubscriptionMessage *subscriptionMessage(reinterpret_cast<SubscriptionMessage *>(tmp->data));

        subscriptionMessage->type = ControlMessages::Unsubscribe;
        memcpy(&subscriptionMessage->channel, i->c_str(), i->length() + 1);
        messageSetChecksum(*tmp);

        remotePeer->handleControlMsg(tmp);
    }

    return Status::Ok;
}

}

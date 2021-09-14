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
 * @file      BasePeer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "BasePeer.h"
#include "ControlMessages.h"
#include "PThreadLocker.h"

#include <cerrno>
#include <cstdio>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "TimeStamp.h"

using namespace std;

namespace SimpleComms
{

BasePeer::BasePeer(const Transports::TransportTypes &transport, 
                   const bool supportsPartialWrites, 
                   const TimeStamp &staleFragmentPruningThreshold
                  )
    : transport_(transport),
      supportsPartialWrites_(supportsPartialWrites),
      staleFragmentPruningThreshold_(staleFragmentPruningThreshold),
      staleThresholdValid_(staleFragmentPruningThreshold != TimeStamp()),
      fd_(-1),
      serverData_(new ServerData(this))
{
    pthread_mutex_init(&clientsMutex_, NULL);
}

BasePeer::~BasePeer()
{
    pthread_mutex_destroy(&clientsMutex_);
}

bool BasePeer::isSubscribedTo(const string &channel)
{
    PThreadLocker clientsLock(&clientsMutex_);

    ChannelData tmp;
    tmp.name = channel;

    return binary_search(channels_.begin(),
                         channels_.end(),
                         tmp,
                         ChannelDataSortByName()
                        );
}

void BasePeer::getSubscriptions(std::vector<std::string> &channels)
{
    PThreadLocker clientsLock(&clientsMutex_);

    for (vector<ChannelData>::iterator i = channels_.begin(); i != channels_.end(); ++i)
    {
        channels.push_back(i->name);
    }
}

bool BasePeer::isConnectedTo(const std::string &clientName)
{
    PThreadLocker clientsLock_(&clientsMutex_);
    bool isConnected = false;

    ClientList::iterator clientIter = clients_.end();
    if (clientLookup(clientName, clientIter))
    {
        if ((*clientIter)->fd != -1)
        {
            isConnected = true;
        }
    }

    return isConnected;
}

Transports::TransportTypes BasePeer::getTransport()
{
    return transport_;
}

Status::States BasePeer::queueMsg(const MessagePtr &msg, BaseClientList &newPendingClients)
{
    if (fd_ == -1)
    {
        return Status::NotConnected;
    }

    Status::States queueMsgStatus = Status::Ok;

    const string channel(msg->channel);
    logger_.log_debug("%s to %s", msg->origin, channel.c_str());

    if (channel == SIMPLECOMMS_CONTROL_CHANNEL)
    {
        PThreadLocker clientsLock(&clientsMutex_);
        ClientList::iterator clientIter(clients_.end());

        if (clientLookup(msg->origin, clientIter))
        {
            if (!(*clientIter)->hasDataToWrite())
            {
                newPendingClients.push_back(*clientIter);
            }

            (*clientIter)->outBox.push_back(msg);
            (*clientIter)->dataPending = true;
        }
    }
    else
    {
        PThreadLocker clientsLock(&clientsMutex_);
        ChannelData tmp;
        tmp.name = channel;

        vector<ChannelData>::iterator i = lower_bound(channels_.begin(),
                                                      channels_.end(),
                                                      tmp,
                                                      ChannelDataSortByName()
                                                     );
        if (i == channels_.end())
        {
            return Status::Ok;
        }

        ChannelData &currentChannel(*i);
        if (currentChannel.name != channel)
        {
            return Status::Ok;
        }

        TimeStamp now; now.setNow();
        for (ClientList::iterator i = currentChannel.subscribers.begin(); 
             i != currentChannel.subscribers.end(); 
             ++i
            )
        {
            ClientData &currentClient(**i);

            if (currentClient.fd != -1)
            {
                logger_.log_debug("%s to %s over %s", msg->origin, currentClient.name.c_str(), channel.c_str());

                const bool hadDataToWrite = currentClient.hasDataToWrite();

                currentClient.outBox.push_back(msg);

                while (staleThresholdValid_ && !currentClient.outBox.empty())
                {
                    if (now - currentClient.outBox.front()->sendMsgTime > staleFragmentPruningThreshold_)
                    {
                        currentClient.outBox.pop_front();
                    }
                    else
                    {
                        break;
                    }
                }

                currentClient.dataPending = !currentClient.outBox.empty() || (currentClient.currentOutboundMessage.get() != NULL);

                if (!hadDataToWrite && currentClient.hasDataToWrite())
                {
                    newPendingClients.push_back(*i);
                }
            }
        }
    }
        
    return queueMsgStatus;
}

Status::States BasePeer::receiveMsg(const BaseClientPtr &clientHandle, MessagePtr &msg)
{
    if (fd_ == -1)
    {
        return Status::NotConnected;
    }

    if (clientHandle != serverData_)
    {
        return Status::InvalidHandle;
    }

    int msgLength;
    const int fionreadStatus = ioctl(fd_, FIONREAD, &msgLength);

    if (fionreadStatus == -1)
    {
        return Status::Error;
    }
    else if (msgLength < static_cast<int>(sizeof(MessageHeader)))
    {
        return Status::Error;
    }

    msg.reset(new(msgLength - sizeof(MessageHeader)) MessageStub());
    const int recvStatus = recv(fd_, reinterpret_cast<void *>(msg.get()), msgLength, MSG_DONTWAIT | MSG_NOSIGNAL); 

    if ((recvStatus == -1) && (errno == EAGAIN))
    {
        return Status::Ok;
    }
    else if (recvStatus == -1)
    {
        return Status::Error;
    }

    if (string(msg->channel) == SIMPLECOMMS_CONTROL_CHANNEL)
    {
        return handleControlMsg(msg);
    }

    return Status::NewData;
}

Status::States BasePeer::getNamedReadHandles(BaseClientList &readHandles)
{
    readHandles.push_back(serverData_);

    return Status::Ok;
}

Status::States BasePeer::getNamedMonitorErrorHandles(BaseClientList &monitorHandles)
{
    PThreadLocker clientsLock(&clientsMutex_);

    for (ClientList::iterator i = clients_.begin(); i != clients_.end(); ++i)
    {
        ClientData &currentClient(**i);

        if (currentClient.outBox.empty() && currentClient.fd != -1)
        {
            monitorHandles.push_back(*i);
        }
    }

    return Status::Ok;
}

Status::States BasePeer::getNamedWriteHandles(BaseClientList &writeHandles)
{
    PThreadLocker clientsLock(&clientsMutex_);

    for (ClientList::iterator i = clients_.begin(); i != clients_.end(); ++i)
    {
        ClientData &currentClient(**i);

        if (!currentClient.outBox.empty() && currentClient.fd != -1)
        {
            writeHandles.push_back(*i);
        }
    }

    return Status::Ok;
}

Status::States BasePeer::doWrites(const BaseClientList &writeHandles)
{
    Status::States doWritesStatus = Status::Ok;

    for (BaseClientList::const_iterator i = writeHandles.begin(); i != writeHandles.end(); ++i)
    {
        const ClientDataPtr namedClient(boost::dynamic_pointer_cast<ClientData>(*i));

        if (namedClient.get() == NULL)
        {
            doWritesStatus = Status::Error;
            continue;
        }

        pthread_mutex_lock(&clientsMutex_);

        ClientData &currentClient(*namedClient);

        logger_.log_debug("doWrites(%s) handling %s", transportName(transport_), currentClient.name.c_str());

        if ((currentClient.fd == -1) || !currentClient.hasDataToWrite())
        {
            pthread_mutex_unlock(&clientsMutex_);
        }
        else
        {
            const int fd = currentClient.fd;
            const TimeStamp fdStamp(currentClient.fdStamp);
            const int bytesWrittenSoFar = currentClient.bytesWrittenSoFar;

            if (currentClient.currentOutboundMessage.get() == NULL)
            {
                currentClient.currentOutboundMessage = currentClient.outBox.front();
            }

            const MessagePtr msg(currentClient.currentOutboundMessage);

            logger_.log_debug("%s to %s over %s (%s)", msg->origin, currentClient.name.c_str(), msg->channel, transportName(transport_));

            pthread_mutex_unlock(&clientsMutex_);

            uint32_t sendOffset = 0, sendLength = msg->MessageLength();
            if (supportsPartialWrites_ && bytesWrittenSoFar)
            {
                sendOffset = bytesWrittenSoFar;
                sendLength = sendLength - bytesWrittenSoFar;
            }

            const int sendStatus = send(fd, 
                                        reinterpret_cast<char *>(msg.get()) + sendOffset, 
                                        sendLength,
                                        MSG_NOSIGNAL
                                       );
            if ((sendStatus == -1) && (errno == EAGAIN))
            {
                PThreadLocker clientsLock(&clientsMutex_);

                if (fdStamp == currentClient.fdStamp)
                {
                    currentClient.readyToWrite = false;
                    currentClient.currentOutboundMessage.reset();
                }
            }
            else if (sendStatus == -1)
            {
                if (errno < sys_nerr)
                {
                    logger_.log_warn("Error in send(): %s", sys_errlist[errno]);
                }
                else
                {
                    logger_.log_warn("Unknown error in send()");
                }

                {
                    PThreadLocker clientsLock(&clientsMutex_);

                    if (fdStamp == currentClient.fdStamp)
                    {
                        clientReset(namedClient, true);
                    }
                    else
                    {
                        logger_.log_debug("Error in send() is stale");
                    }
                }

                doWritesStatus = Status::Error;
            }
            else if (static_cast<unsigned>(sendStatus) < sendLength)
            {
                if (supportsPartialWrites_)
                {
                    PThreadLocker clientsLock(&clientsMutex_);

                    if (fdStamp == currentClient.fdStamp)
                    {
                        currentClient.readyToWrite = false;
                        currentClient.bytesWrittenSoFar += sendStatus;
                    }
                }
                else
                {
                    logger_.log_warn("Partial write (%d < %d) on %s", 
                                     sendStatus, 
                                     msg->MessageLength(), 
                                     transportName(transport_)
                                    );

                    doWritesStatus = Status::Error;
                }
            }
            else
            {
                PThreadLocker clientsLock(&clientsMutex_);

                if (fdStamp == currentClient.fdStamp)
                {
                    if (!currentClient.outBox.empty() && (currentClient.outBox.front().get() == currentClient.currentOutboundMessage.get()))
                    {
                        currentClient.outBox.pop_front();
                    }

                    currentClient.currentOutboundMessage.reset();
                    currentClient.bytesWrittenSoFar = 0;

                    currentClient.dataPending = !currentClient.outBox.empty();
                }
            }
        }
    }

    return doWritesStatus;
}

Status::States BasePeer::handleErrors(BaseClientList &errorHandles)
{
    Status::States status = Status::Ok;
    bool serverError = false;

    for (BaseClientList::const_iterator i(errorHandles.begin());
         i != errorHandles.end();
         ++i
        )
    {
        PThreadLocker clientsLock(&clientsMutex_);
        const BaseClientPtr &baseClientPtr(*i);

        if (baseClientPtr == serverData_)
        {
            serverError = true;
        }
        else
        {
            const ClientDataPtr currentClientPtr(boost::dynamic_pointer_cast<ClientData>(baseClientPtr));

            if (currentClientPtr.get())
            {
                int err = -1;
                socklen_t len = sizeof(err);

                if (getsockopt(currentClientPtr->fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
                {
                    err = -1;
                }

                if (err != 0)
                {
                    clientReset(currentClientPtr, true);
                }
            }
            else
            {
                status = Status::InvalidHandle;
            }
        }
    }

    if (serverError)
    {
        disconnect();
        status = connect();
    }

    return status;
}

Status::States BasePeer::handleControlMsg(const MessagePtr &msg)
{
    Status::States status = Status::Ok;

    if (peerName_ == msg->origin)
    {
        return Status::Ok;
    }

    // Make a single connection attempt upon receiving a Connect message.
    if (msg->data[0] == ControlMessages::Connect)
    {
        const ConnectionMessage *connMsg = reinterpret_cast<ConnectionMessage *>(msg->data);

        if (connMsg->transport == transport_)
        {
            PThreadLocker clientsLock(&clientsMutex_);
            ClientList::iterator clientIter(clients_.end());

            if (clientLookup(msg->origin, clientIter, true))
            {
                const ConnectionMessage *connMsg = reinterpret_cast<const ConnectionMessage *>(msg->data);
                if (clientConnect(*clientIter, connMsg))
                {
                    (*clientIter)->fdStamp.setNow();
                    status = Status::NewData;
                }
                else
                {
                    clients_.erase(clientIter);
                }
            }
        }
        else
        {
            logger_.log_warn("Unexpected transport specified in connection message from %s", msg->origin);
        }
    } 
    else if (ControlMessages::Disconnect == msg->data[0])
    {
        const DisconnectionMessage *connMsg = reinterpret_cast<DisconnectionMessage *>(msg->data);

        if (connMsg->transport == transport_)
        {
            PThreadLocker clientsLock(&clientsMutex_);
            ClientList::iterator clientIter(clients_.end());

            if (clientLookup(msg->origin, clientIter))
            {
                ClientDataPtr namedClient(*clientIter);

                clients_.erase(clientIter);

                clientReset(namedClient);
            }
        }
        else
        {
            logger_.log_warn("Unexpected transport specified in disconnection message from %s", msg->origin);
        }
    }
    else if ((ControlMessages::Subscribe == msg->data[0]) || (ControlMessages::Unsubscribe == msg->data[0]))
    {
        PThreadLocker clientsLock(&clientsMutex_);
        const bool subscribe = ControlMessages::Subscribe == msg->data[0];
        const SubscriptionMessage *subscriptionMessage = reinterpret_cast<const SubscriptionMessage *>(msg->data);
        ClientList::iterator clientIter = clients_.end();

        if (clientLookup(msg->origin, clientIter))
        {
            const ClientDataPtr namedClient(*clientIter);
            bool subscribed(false), unsubscribed(false);
            ChannelData tmp;
            tmp.name = subscriptionMessage->channel;

            vector<ChannelData>::iterator i = lower_bound(channels_.begin(),
                                                          channels_.end(),
                                                          tmp,
                                                          ChannelDataSortByName()
                                                         );

            bool newChannel = false;
            if (channels_.end() == i)
            {
                newChannel = true;
            }
            else
            {
                ChannelData &currentChannel(*i);
                if (currentChannel.name != tmp.name)
                {
                    newChannel = true;
                }
                else
                {
                    ClientList::iterator j = lower_bound(currentChannel.subscribers.begin(),
                                                         currentChannel.subscribers.end(),
                                                         *clientIter
                                                        );

                    if (currentChannel.subscribers.end() == j)
                    {
                        if (subscribe)
                        {
                            subscribed = true;
                            currentChannel.subscribers.push_back(*clientIter);
                        }
                    }
                    else if (*j != *clientIter)
                    {
                        if (subscribe)
                        {
                            subscribed = true;
                            currentChannel.subscribers.insert(j, *clientIter);
                        }
                    }
                    else
                    {
                        if (!subscribe)
                        {
                            unsubscribed = true;
                            currentChannel.subscribers.erase(j);
                        }
                    }
                }
            }

            if (newChannel && subscribe)
            {
                tmp.subscribers.push_back(*clientIter);
                channels_.insert(i, tmp);

                subscribed = true;
            }

            if (subscribed || unsubscribed)
            {
                logger_.log_debug("%s %s %s over %s", 
                                  namedClient->name.c_str(), 
                                  subscribed ? "subscribed to" : "unsubscribed from",
                                  tmp.name.c_str(), 
                                  transportName(transport_)
                                 );
            }

            status = Status::NewData;
        }
    }

    return status;
}

BasePeer::ClientDataPtr BasePeer::clientCreate(const string &name)
{
    ClientDataPtr tmp(new ClientData(this, name));

    return tmp;
}

/**
 * @brief Find or create the desired client entry in the clientsList_
 *
 * @param name String identifier of the client
 * @param namedClient Iterator to return entry in
 *
 * @return True if an existing entry was found or a blank one was created 
 *
 * Assumes clientsMutex_ is held by calling function
 */
bool BasePeer::clientLookup(const string &name, ClientList::iterator &namedClient, const bool create)
{
    ClientDataPtr tmp(clientCreate(name));

    namedClient = lower_bound(clients_.begin(), 
                              clients_.end(),
                              tmp,
                              ClientDataSortByName()
                             );

    bool exists = true;
    if (namedClient == clients_.end())
    {
        exists = false;
    }
    else if ((*namedClient)->name != name)
    {
        exists = false;
    }

    if (!exists)
    {
        if (create)
        {
            tmp->fd = -1;

            logger_.log_debug("Adding new client %s", name.c_str());

            namedClient = clients_.insert(namedClient, tmp);
        }
        else
        {
            logger_.log_debug("Client %s not found", name.c_str());
            namedClient = clients_.end();
        }
    }

    return (namedClient != clients_.end());
}

/**
 * @brief Deinitialize and remove traces of a client
 *
 * @param namedClient A pointer to the client to reset
 *
 * Assumes clientsMutex_ is held by calling function
 * The system call close may block with clientsMutex_ held
 */
void BasePeer::clientReset(const ClientDataPtr &namedClient, const bool retireEntry)
{
    ClientData &currentClient(*namedClient);

    logger_.log_debug("Reseting client %s", currentClient.name.c_str());

    close(currentClient.fd);
    currentClient.fd = -1;

    currentClient.fdStamp = TimeStamp();

    currentClient.outBox.clear();
    currentClient.currentOutboundMessage.reset();

    currentClient.dataPending = false;

    if (retireEntry) 
    {
        ClientList::iterator clientIter(clients_.end());

        if (clientLookup(currentClient.name, clientIter))
        {
            clients_.erase(clientIter);
        }
    }

    vector<ChannelData> oldChannels(channels_);
    channels_.clear();
    for (vector<ChannelData>::iterator i = oldChannels.begin(); i != oldChannels.end(); ++i)
    {
        ChannelData &currentChannel(*i);
        ClientList::iterator j = lower_bound(currentChannel.subscribers.begin(),
                                             currentChannel.subscribers.end(),
                                             namedClient
                                            );

        if (j != currentChannel.subscribers.end())
        {
            if (*j == namedClient)
            {
                currentChannel.subscribers.erase(j);
            }
        }

        if (!currentChannel.subscribers.empty())
        {
            channels_.push_back(currentChannel);
        }
    }
}

}

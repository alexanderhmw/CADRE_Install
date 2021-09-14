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
 * @file      UnixClient.cc
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "UnixClient.h"
#include "ControlMessages.h"
#include "Monitor.h"
#include "PThreadLocker.h"

#include <cerrno>

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

using namespace std;

namespace SimpleComms
{

/**
 * @brief UnixClient constructor with client and local server path information
 *
 * @param serverPath The path of the local server
 * @param clientPath The path this client will register itself at
 */
UnixClient::UnixClient(const string &serverPath, 
                       const string &clientPath,
                       const double &staleBufferPruningThreshold_s
                      )
    : serverPath_(serverPath),
      clientPath_(clientPath),
      fd_(-1),
      serverfd_(-1),
      rollingMessageID_(0),
      staleBufferPruningThreshold_s_(staleBufferPruningThreshold_s),
      threadRunning_(0)
{
    // fdMutex_ is marked recursive as we may call disconnect() from connect() to tear down a partial connection
    {
        pthread_mutexattr_t mutexAttr;

        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE_NP);

        pthread_mutex_init(&fdMutex_, &mutexAttr);
        pthread_mutex_init(&channelsMutex_, &mutexAttr);

        pthread_mutexattr_destroy(&mutexAttr);
    }
}

/**
 * @brief UnixClient class destructor
 */
UnixClient::~UnixClient()
{
    disconnect();

    pthread_mutex_destroy(&channelsMutex_);
    pthread_mutex_destroy(&fdMutex_);
}

/**
 * @brief Establish presence at clientPath and connect to serverPath
 *
 * @return Status::Ok if the connection and handshake was successful
 */
Status::States UnixClient::connect()
{
    {
        // We plan on working with the file descriptors
        PThreadLocker fdLock(&fdMutex_);

        if (fd_ != -1)
        {
            logger_.log_warn("Already connected");

            return Status::AlreadyConnected;
        }
        // If the receiverThread encountered an error and terminated the connection, we may have to join it now
        else if (threadRunning_)
        {
            // Temporarily release the file descriptors so any active threads can spot the -1 and terminate
            pthread_mutex_unlock(&fdMutex_);

            threadRunning_--;
            pthread_join(receiverThreadHandle_, NULL);

            pthread_mutex_lock(&fdMutex_);
        }

        // Tie fd_ to clientPath_ to listen for messages from the server
        {
            struct sockaddr_un servAddr;

            const int maxPathLength = min(MAX_NULL_TERM_CLIENT_NAME, static_cast<int>(sizeof(servAddr.sun_path)) - 1);
            if (static_cast<int>(clientPath_.length()) + 1 > maxPathLength)
            {
                logger_.log_warn("Specified path exceeds maximum length (%d bytes)", maxPathLength);

                return Status::SocketPathTooLong;
            }

            if ((fd_ = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
            {
                logger_.log_warn("Error during socket(PF_UNIX, SOCK_DGRAM, 0): %s", 
                                 (errno < sys_nerr) ? sys_errlist[errno] : "Unknown"
                                );

                return Status::ErrorCreatingSocket;
            }

            memset(reinterpret_cast<void *>(&servAddr), 0, sizeof(servAddr));
            servAddr.sun_family = AF_UNIX;

            // Abstract namespace triggered via leading null
            strncpy(&servAddr.sun_path[1], clientPath_.c_str(), sizeof(servAddr.sun_path) - 1);
            const int servLen = 1 + clientPath_.length() + sizeof(servAddr.sun_family);

            if (bind(fd_, reinterpret_cast<struct sockaddr *>(&servAddr), servLen) < 0)
            {
                logger_.log_warn("Error binding socket: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

                disconnect();

                return Status::ErrorBindingSocket;
            }

            if (fcntl(fd_, F_SETFD, FD_CLOEXEC) < 0)
            {
                logger_.log_warn("Error setting FD_CLOEXEC: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

                disconnect();

                return Status::Error;
            }
        }

        // Tie serverfd_ to serverPath_ to send messages to the server
        {
            struct sockaddr_un servAddr;
            if (serverPath_.length() + 1 > sizeof(servAddr.sun_path) - 1)
            {
                logger_.log_warn("Specified server path exceeds maximum length (%d bytes)", 
                                 sizeof(servAddr.sun_path) - 1
                                );

                disconnect();

                return Status::SocketPathTooLong; 
            }

            memset(reinterpret_cast<void *>(&servAddr), 0, sizeof(servAddr));
            servAddr.sun_family = AF_UNIX;

            // Abstract path_space triggered via leading null
            strncpy(&servAddr.sun_path[1], serverPath_.c_str(), sizeof(servAddr.sun_path) - 1);
            const int servLen = 1 + serverPath_.length() + sizeof(servAddr.sun_family);

            if ((serverfd_ = socket(PF_UNIX, SOCK_DGRAM,0)) < 0)
            {
                logger_.log_warn("Error during socket(PF_UNIX, SOCK_DGRAM, 0): %s", 
                                 (errno < sys_nerr) ? sys_errlist[errno] : "Unknown"
                                );

                disconnect();

                return Status::ErrorCreatingSocket;
            }

            if (::connect(serverfd_, reinterpret_cast<struct sockaddr *>(&servAddr), servLen) < 0)
            {
                logger_.log_warn("Error during connect: %s", 
                                 (errno < sys_nerr) ? sys_errlist[errno] : "Unknown"
                                );

                disconnect();

                return Status::ErrorConnectingSocket;
            }

            if (fcntl(serverfd_, F_SETFD, FD_CLOEXEC) < 0)
            {
                logger_.log_warn("Error setting FD_CLOEXEC: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

                disconnect();

                return Status::Error;
            }
        }

        // Initialize the connection request acknowledged condition
        if (!connectionAcknowledgedCondition.init())
        {
            logger_.log_warn("Error initializing connectionAcknowledgedCondition: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

            disconnect();

            return Status::Error;
        }

        // Initialize the new message condition
        if (!messagesAvailableCondition.init())
        {
            logger_.log_warn("Error initializing messagesAvailableCondition: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

            disconnect();

            return Status::Error;
        }

        // Spawn a thread that sits on fd_ and queues the incoming data
        if (pthread_create(&receiverThreadHandle_, NULL, receiverThreadLauncher, this) == 0)
        {
            // Flag that we successfully launched a thread for later joining
            threadRunning_++;
        }
        else
        {
            logger_.log_warn("Error initializing receiver thread: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

            disconnect();

            return Status::Error;
        }

    } // We are done messing around with the file descriptors

    // Simple exchange to make the the connection is established
    {
        // (1) Send the local server a notice that we want to connect
        ConnectionMessage connectionMessage;

        connectionMessage.type = ControlMessages::Connect;
        connectionMessage.transport = Transports::UnixDomainSocket;

        const Status::States sendMsgStatus = sendMsg(SIMPLECOMMS_CONTROL_CHANNEL, 
                                                     string(reinterpret_cast<const char *>(&connectionMessage), 
                                                            sizeof(ConnectionMessage)
                                                           )
                                                    );

        if (sendMsgStatus != Status::Ok)
        {
            logger_.log_warn("Error sending connection message: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");

            return sendMsgStatus;
        }

        // (2) Wait for local server to echo our notice
        {
            Monitor responseMonitor;

            responseMonitor.add(connectionAcknowledgedCondition);

            responseMonitor.wait(TimeStamp(10.0));
        }
        
        // (3) If we've received the echo, clear the condition and proceed
        if (connectionAcknowledgedCondition.isSet())
        {
            connectionAcknowledgedCondition.unset();
        }
        else
        {
            logger_.log_warn("Connection not acknowledged");

            disconnect();

            return Status::Error;
        }
    }

    {
        PThreadLocker channelsLock(&channelsMutex_);

        for (ChannelMap::iterator i = channels_.begin();
             i != channels_.end();
             ++i
            )
        {
            subscribe((*i).first, (*i).second.maxReceiveDepth);
        }
    }

    return Status::Ok;
}

/**
 * @brief Disconnect from the local server
 *
 * The internal function closeConnection() does all the work.
 *
 * @return Refer to closeConnection()
 */
Status::States UnixClient::disconnect()
{
    return closeConnection(false);
}

/**
 * @brief Subscribe to a channel
 *
 * @param channel Name of the channel to subscribe to
 *
 * @return Status::Ok if the request was successfully sent. See sendMsg(const Message &msg) for further return values.
 *
 * Success here only means the request was made. The server may still deny the request. Verify status later with
 * isSubscribed().
 */
Status::States UnixClient::subscribe(const string &channel, const uint32_t maxReceiveDepth)
{
    Status::States status = Status::Ok;

    if (channel.length() < MAX_NULL_TERM_CHANNEL_NAME)
    {
        bool sendSubscribeRequest = false;

        {
            PThreadLocker channelsLock(&channelsMutex_);
            const ChannelMap::iterator i(channels_.find(channel));

            if (i == channels_.end())
            {
                sendSubscribeRequest = true;
                channels_.insert(make_pair(channel, ChannelData(maxReceiveDepth)));
            }
            else
            {
                ChannelData &channelData((*i).second);

                channelData.maxReceiveDepth = maxReceiveDepth;

                if (!channelData.acknowledged)
                {
                    sendSubscribeRequest = true;
                }
            }
        }

        if (sendSubscribeRequest)
        {
            SubscriptionMessage subcriptionMessage;

            subcriptionMessage.type = ControlMessages::Subscribe;
            memcpy(&subcriptionMessage.channel, channel.c_str(), channel.length() + 1);

            status = sendMsg(SIMPLECOMMS_CONTROL_CHANNEL, 
                             string(reinterpret_cast<const char *>(&subcriptionMessage), sizeof(SubscriptionMessage))
                            );
        }
    }
    else
    {
        status = Status::ChannelNameTooLong;
    }

    return status;
}

/**
 * @brief Unsubscribe from a channel
 *
 * @param channel Name of the channel to unsubscribe from
 *
 * @return Status::Ok if the request was successfully sent. See sendMsg(const Message &msg) for further return values.
 *
 * See notes concerning the return value under subscribe().
 */
Status::States UnixClient::unsubscribe(const string &channel)
{
    Status::States status = Status::Ok;
    bool sendUnsubscribeRequest = false;

    {
        PThreadLocker channelsLock(&channelsMutex_);
        const ChannelMap::iterator i(channels_.find(channel));

        if (i != channels_.end())
        {
            sendUnsubscribeRequest = true;
            channels_.erase(i);
        }
    }
        
    if (sendUnsubscribeRequest)
    {
        SubscriptionMessage subcriptionMessage;

        subcriptionMessage.type = ControlMessages::Unsubscribe;
        memcpy(&subcriptionMessage.channel, channel.c_str(), channel.length() + 1);

        status = sendMsg(SIMPLECOMMS_CONTROL_CHANNEL, 
                         string(reinterpret_cast<const char *>(&subcriptionMessage), sizeof(SubscriptionMessage))
                        );
    }

    return status;
}

/**
 * @brief Retrieve totals for channel/origin
 *
 * @param statsKey channel/origin to fetch totals for
 *
 * @return ChannelOriginStatsMap::iterator to either the existing entry or newly created entry for channel/origin
 */
UnixClient::ChannelOriginStatsMap::iterator UnixClient::fetchTotals(const ChannelOriginKey &statsKey)
{
    ChannelOriginStatsMap::iterator statsIter(channelOriginStats_.find(statsKey));

    if (statsIter == channelOriginStats_.end())
    {
        const pair<ChannelOriginKey, ChannelOriginTotals> newStats(statsKey, ChannelOriginTotals());
        const pair<ChannelOriginStatsMap::iterator, bool> insertionResult(channelOriginStats_.insert(newStats));

        statsIter = insertionResult.first;

        memcpy((*statsIter).second.channel, statsKey.first.c_str(), statsKey.first.length() + 1);
        memcpy((*statsIter).second.origin, statsKey.second.c_str(), statsKey.second.length() + 1);
    }

    return statsIter;
}

/**
 * @brief Send a message
 *
 * @param channel Name of the channel to send the message to
 * @param data The data to send to named channel
 *
 * @return Status::Ok if the request was successfully sent. See sendMsg(const Message &msg) for further return values.
 */
Status::States UnixClient::sendMsg(const string &channel, const string &data)
{
    Status::States status = Status::Ok;
    MessageBase msg;

    if (channel.length() + 1 > sizeof(msg.channel))
    {
        return Status::ChannelNameTooLong;
    }
    
    memcpy(&msg.origin, clientPath_.c_str(), clientPath_.length() + 1);
    memcpy(&msg.channel, channel.c_str(), channel.length() + 1);
    gettimeofday(&msg.sendMsgTime, NULL);
    msg.length = data.length();
    msg.rollingMessageID = rollingMessageID_++;
    msg.totalPages = msg.length / MAX_FRAGMENT_PAYLOAD + ((msg.length % MAX_FRAGMENT_PAYLOAD) > 0);

    for (uint32_t i = 0; (i < msg.totalPages) & (status == Status::Ok); ++i)
    {
        msg.pageNumber = i + 1;
        msg.dataLength = min(msg.length - MAX_FRAGMENT_PAYLOAD * i, MAX_FRAGMENT_PAYLOAD);
        msg.data = reinterpret_cast<unsigned char *>(const_cast<char *>(data.c_str())) + MAX_FRAGMENT_PAYLOAD * i;
        messageSetChecksum(msg);

        status = sendMsg(msg);
    }

    return status;
}

/**
 * @brief Receive a message
 *
 * @param channel The name of the channel to poll for data
 * @param data Destination for new data
 * @param msgInfo Auxiliary information about the data
 *
 * @return Status::NewData if new data was available and delivered
 *         Status::Ok if new data wasn't available but the request did not fail
 *         Status::NotConnected if the client isn't connected and no new data is left in the queue
 */
Status::States UnixClient::receiveMsg(const string &channel, string &data, MessageInfo &msgInfo)
{
    PThreadLocker channelsLock(&channelsMutex_);
    Status::States returnStatus = Status::Ok;

    const ChannelMap::iterator channelIter(channels_.find(channel));
    if (channelIter != channels_.end())
    {
        ChannelData &channelData((*channelIter).second);

        if (!channelData.completeBuffers.empty())
        {
            {
                FragmentBuffer &fragBuf(*channelData.completeBuffers.front());

                msgInfo.origin = fragBuf.lastHeader.origin;
                msgInfo.sendMsgTime = fragBuf.lastHeader.sendMsgTime;
                msgInfo.msgReadyTime = fragBuf.lastMessageAdded;

                data.swap(fragBuf.data);
            }

            channelData.completeBuffers.pop();

            if (channelData.completeBuffers.empty())
            {
                messagesAvailableCondition.unset();
            }

            returnStatus = Status::NewData;
        }
        else
        {
            if (fd_ == -1)
            {
                returnStatus = Status::NotConnected;
            }
        }
    }

    return returnStatus;
}

/**
 * @brief Receive a message without auxiliary data
 *
 * @param channel The name of the channel to poll for data
 * @param data Destination for new data
 *
 * @return See receiveMsg(const string &channel, string &data, MessageInfo &msgInfo)
 */
Status::States UnixClient::receiveMsg(const string &channel, string &data)
{
    MessageInfo msgInfo;

    return receiveMsg(channel, data, msgInfo);
}

/**
 * @brief Get a condition instance that reflects whether new messages are available
 *
 * @return Requested condition instance
 */
Condition &UnixClient::getMessagesAvailableCondition()
{
    return messagesAvailableCondition;
}

/**
 * @brief Internal function to send a message fragment
 *
 * @param msg Fragment to send
 *
 * @return XXX
 */
Status::States UnixClient::sendMsg(const Message &msg)
{
    const int sendStatus = send(serverfd_, reinterpret_cast<const void *>(&msg), msg.MessageLength(), MSG_NOSIGNAL);

    if (sendStatus == -1)
    {
        // XXX
        return Status::Error;
    }

    return Status::Ok;
}

Status::States UnixClient::sendMsg(const MessageBase &msg)
{
    struct iovec messageIovec[2];

    messageIovec[0].iov_base = const_cast<MessageBase *>(&msg);
    messageIovec[0].iov_len = sizeof(MessageHeader);
    messageIovec[1].iov_base = const_cast<unsigned char *>(msg.data);
    messageIovec[1].iov_len = msg.dataLength;

    struct msghdr messageHeader;

    messageHeader.msg_name = NULL;
    messageHeader.msg_namelen = 0;
    messageHeader.msg_iov = messageIovec;
    messageHeader.msg_iovlen = 2;
    messageHeader.msg_control = NULL;
    messageHeader.msg_controllen = 0;
    messageHeader.msg_flags = 0;

    const int sendStatus = sendmsg(serverfd_, &messageHeader, MSG_NOSIGNAL);

    if (sendStatus == -1)
    {
        // XXX
        return Status::Error;
    }

    return Status::Ok;
}

Status::States UnixClient::handleIncomingMsg()
{
    Status::States status(Status::Ok);
    MessageBase msgBase;
    const int recvStatus = recv(fd_, 
                                reinterpret_cast<void *>(&msgBase), 
                                sizeof(MessageHeader), 
                                MSG_DONTWAIT | MSG_PEEK
                               ); 

    if ((recvStatus == -1) && (errno == EAGAIN))
    {
        return Status::Ok;
    }
    else if (recvStatus == -1)
    {
        closeConnection(true);

        return Status::Error;
    }

    totalFragmentsReceived_++;

    if (messageValidateHeaderChecksum(msgBase))
    {
        if (string(msgBase.channel) == SIMPLECOMMS_CONTROL_CHANNEL)
        {
            Message fullMsg;
            recv(fd_, reinterpret_cast<void *>(&fullMsg), sizeof(Message), 0); 

            if (messageValidateChecksum(fullMsg))
            {
                pthread_mutex_unlock(&fdMutex_);
                status = handleControlMsg(fullMsg);
                pthread_mutex_lock(&fdMutex_);
            } 
            else
            {
                checksumErrors_++;
            }
        }
        else
        {
            const MessageKey fragmentBufferKey(msgBase.origin, msgBase.rollingMessageID);
            FragmentBufferMap::iterator fbIter(fragmentBuffers_.find(fragmentBufferKey));

            if (fbIter == fragmentBuffers_.end())
            {
                const pair<MessageKey, FragmentBufferPtr> 
                    newBuffer(fragmentBufferKey, FragmentBufferPtr(new FragmentBuffer(fragmentBuffersLRU_.end())));
                const pair<FragmentBufferMap::iterator, bool> insertionResult(fragmentBuffers_.insert(newBuffer));

                fbIter = insertionResult.first;
            }

            FragmentBuffer &fragBuf(*(*fbIter).second);

            // If we are on the recently used list, pull us off for now
            if (fragBuf.lruEntry != fragmentBuffersLRU_.end())
            {
                fragmentBuffersLRU_.erase(fragBuf.lruEntry);
            }

            if (fragBuf.receivedCount == 0)
            {
                fragBuf.data.resize(msgBase.length, 0);
                fragBuf.received.resize(msgBase.totalPages);
            }

            {
                msgBase.data = reinterpret_cast<unsigned char *>(
                                   const_cast<char *>(&fragBuf.data[(msgBase.pageNumber - 1) * MAX_FRAGMENT_PAYLOAD])
                               );

                struct iovec messageIovec[2];

                messageIovec[0].iov_base = &fragBuf.lastHeader;
                messageIovec[0].iov_len = sizeof(MessageHeader);
                messageIovec[1].iov_base = msgBase.data;
                messageIovec[1].iov_len = msgBase.dataLength;

                struct msghdr messageHeader;

                messageHeader.msg_name = NULL;
                messageHeader.msg_namelen = 0;
                messageHeader.msg_iov = messageIovec;
                messageHeader.msg_iovlen = 2;
                messageHeader.msg_control = NULL;
                messageHeader.msg_controllen = 0;
                messageHeader.msg_flags = 0;

                recvmsg(fd_, &messageHeader, 0); 
            }

            if (messageValidateChecksum(msgBase))
            {
                pthread_mutex_unlock(&fdMutex_);

                fragBuf.lastMessageAdded.setNow();
                fragBuf.received[msgBase.pageNumber - 1] = true;
                fragBuf.receivedCount++;
                fragBuf.receivedBytes += msgBase.dataLength;

                // Update stats
                ChannelOriginTotals &stats((*fetchTotals(ChannelOriginKey(msgBase.channel, msgBase.origin))).second);

                stats.totalFragments++;
                stats.totalBytes += msgBase.dataLength;
                stats.totalDelay += static_cast<double>(fragBuf.lastMessageAdded - msgBase.sendMsgTime);

                // Handle message completion
                if (fragBuf.receivedCount == msgBase.totalPages)
                {
                    totalMessagesReceived_++;

                    stats.completedMessages++;
                    stats.completedFragments += fragBuf.receivedCount;
                    stats.completedBytes += msgBase.length;

                    {
                        PThreadLocker channelsLock(&channelsMutex_);
                        ChannelMap::iterator channelIter(channels_.find(msgBase.channel));
                    
                        if (channelIter != channels_.end())
                        {
                            ChannelData &channelData((*channelIter).second);
                            
                            channelData.completeBuffers.push((*fbIter).second);

                            if ((channelData.maxReceiveDepth > 0)
                                && (channelData.completeBuffers.size() > channelData.maxReceiveDepth)
                               )
                            {
                                channelData.completeBuffers.pop();
                            }
                        }
                    }

                    messagesAvailableCondition.set();

                    fragmentBuffers_.erase(fbIter);
                }
                else
                {
                    // Incomplete fragBuf still in play, reintroduce to LRU
                    fragBuf.lruEntry = fragmentBuffersLRU_.insert(fragmentBuffersLRU_.end(), fbIter);
                }

                pthread_mutex_lock(&fdMutex_);
            }
            else
            {
                checksumErrors_++;
            }
        }
    }
    else
    {
        checksumErrors_++;
    }

    return status;
}

Status::States UnixClient::handleControlMsg(const Message &msg)
{
    logger_.log_debug("UnixClient::handleControlMsg (%d)", msg.data[0]);

    switch (msg.data[0])
    {
        case ControlMessages::Connect:
        {
            connectionAcknowledgedCondition.set();
            break;
        }

        case ControlMessages::Subscribe:
        {
            const SubscriptionMessage *subscriptionMessage = reinterpret_cast<const SubscriptionMessage *>(msg.data);
            const ChannelMap::iterator i(channels_.find(subscriptionMessage->channel));

            if (i != channels_.end())
            {
                (*i).second.acknowledged = true;
            }

            break;
        }
    }

    return Status::Ok;
}

Status::States UnixClient::closeConnection(const bool receiverThread)
{
    {
        PThreadLocker fdLock(&fdMutex_);

        if (fd_ != -1)
        {
            close(fd_);
            fd_ = -1;
        }

        if (serverfd_ != -1)
        {
            {
                DisconnectionMessage disconnectionMessage;

                disconnectionMessage.type = ControlMessages::Disconnect;
                disconnectionMessage.transport = Transports::UnixDomainSocket;

                sendMsg(SIMPLECOMMS_CONTROL_CHANNEL, 
                        string(reinterpret_cast<const char *>(&disconnectionMessage), 
                               sizeof(disconnectionMessage)
                              )
                       );
            }

            close(serverfd_);
            serverfd_ = -1;
        }
    }

    if (!receiverThread && threadRunning_)
    {
        threadRunning_--;
        pthread_join(receiverThreadHandle_, NULL);
    }

    {
        PThreadLocker channelsLock(&channelsMutex_);

        for (ChannelMap::iterator i = channels_.begin();
             i != channels_.end();
             ++i
            )
        {
            (*i).second.acknowledged = false;
        }
    }

    return Status::Ok;
}

/**
 * @brief Internal function to cull partial fragment buffers that have been around past a threshold
 */
void UnixClient::pruneStalePartialBuffers()
{
    TimeStamp now;
    now.setNow();

    while (!fragmentBuffersLRU_.empty())
    {
        FragmentBuffer &fragBuf(*(*fragmentBuffersLRU_.front()).second);

        if (static_cast<double>(now - fragBuf.lastMessageAdded) > staleBufferPruningThreshold_s_)
        {
            logger_.log_debug("Pruning stale message from %s on channel %s with %d out of %d parts",
                              fragBuf.lastHeader.origin, 
                              fragBuf.lastHeader.channel, 
                              fragBuf.receivedCount, 
                              fragBuf.lastHeader.totalPages
                             );

            { // Update stats
                const ChannelOriginKey statsKey(fragBuf.lastHeader.channel, fragBuf.lastHeader.origin);
                ChannelOriginTotals &stats((*fetchTotals(statsKey)).second);

                stats.incompleteMessages++;
                stats.expectedFragments += fragBuf.lastHeader.totalPages;
                stats.receivedFragments += fragBuf.receivedCount;
                stats.receivedBytes += fragBuf.receivedBytes;
                stats.expectedBytes += fragBuf.lastHeader.length;
            }

            fragmentBuffers_.erase(fragmentBuffersLRU_.front());
            fragmentBuffersLRU_.pop_front();
        }
        else
        {
            break;
        }
    }
}

void *UnixClient::receiverThreadLauncher(void *arg)
{
    ((UnixClient *)arg)->receiverThread();

    return NULL;
}

void UnixClient::receiverThread()
{
    lastReport_.setNow();
    totalFragmentsReceived_ = 0;
    totalMessagesReceived_ = 0;
    checksumErrors_ = 0;

    pthread_mutex_lock(&fdMutex_);
    while (fd_ != -1)
    {
        fd_set rfds;

        FD_ZERO(&rfds);
        FD_SET(fd_, &rfds);

        pthread_mutex_unlock(&fdMutex_);

        struct timeval waitLimit;
        waitLimit.tv_sec = 1;
        waitLimit.tv_usec = 0;

        const int selectStatus = select(fd_ + 1, &rfds, NULL, NULL, &waitLimit);

        if (selectStatus == -1)
        {
            switch (errno)
            {
            case EINTR:
                // Ignore
                break;
            default:
                if (errno < sys_nerr)
                {
                    logger_.log_warn("Error during select: %s", sys_errlist[errno]);
                }
                else
                {
                    logger_.log_warn("Unknown error during select");
                }

                closeConnection(true);

                return;
            }
        }

        pruneStalePartialBuffers();

        pthread_mutex_lock(&fdMutex_);

        if (fd_ == -1)
        {
            break;
        }

        if (FD_ISSET(fd_, &rfds)) 
        {
            if (handleIncomingMsg() != Status::Ok)
            {
                break;
            }
        }

        {
            TimeStamp now;
            now.setNow();
            const double delta = (double)(now - lastReport_);

            if (delta > 1)
            {
                logger_.log_info("Total Fragments: %lf; Total Messages: %lf; Checksum Errors: %lf",
                                 totalFragmentsReceived_ / delta, 
                                 totalMessagesReceived_ / delta, 
                                 checksumErrors_ / delta
                                );

                totalFragmentsReceived_ = totalMessagesReceived_ = checksumErrors_ = 0;

                { // Publish stats
                    string statsBuffer;
                    for (ChannelOriginStatsMap::iterator i(channelOriginStats_.begin()); 
                         i != channelOriginStats_.end(); 
                         ++i
                        )
                    {
                        const ChannelOriginTotals &stats((*i).second);

                        statsBuffer.append(reinterpret_cast<const char *>(&stats), sizeof(stats));
                    }

                    sendMsg(SIMPLECOMMS_STATS_CHANNEL, statsBuffer);

                    channelOriginStats_.clear();
                }

                lastReport_ = now;
            }
        }
    }

    pthread_mutex_unlock(&fdMutex_);
}

}

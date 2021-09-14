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
 * @file      UnixClient.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_UNIXCLIENT_H_
#define _SIMPLECOMMS_UNIXCLIENT_H_

#include <map>
#include <list>
#include <queue>
#include <string>
#include <vector>

#include <boost/utility.hpp>

#include <log/Logger.h>

#include "Client.h"
#include "Message.h"
#include "StatsMessages.h"

namespace SimpleComms
{

class UnixClient : public Client, public boost::noncopyable
{
    public:
        UnixClient(const std::string &serverPath, 
                   const std::string &clientPath, 
                   const double &staleBufferPruningThreshold_s = 1.0
                  );
        virtual ~UnixClient();

        virtual Status::States connect();
        virtual Status::States disconnect();

        virtual Status::States subscribe(const std::string &channel, const uint32_t maxReceiveDepth);
        virtual Status::States unsubscribe(const std::string &channel);

        virtual Status::States sendMsg(const std::string &channel, const std::string &data);
        virtual Status::States receiveMsg(const std::string &channel, std::string &data, MessageInfo &msgInfo);
        virtual Status::States receiveMsg(const std::string &channel, std::string &data);

        virtual Condition &getMessagesAvailableCondition();

    private:
        const std::string serverPath_;  ///< Path to local server
        const std::string clientPath_;  ///< Path this client will register at
        pthread_mutex_t fdMutex_;       ///< Mutex to protect operations to fd_/serverfd_/threadRunning_
        int fd_;                        ///< The socket this client is listening on, protected by fdMutex_
        int serverfd_;                  ///< The socket the server is listening on, protected by fdMutex_

        //! A condition variable reflecting whether a connection request was acknowledged
        Condition connectionAcknowledgedCondition;

        uint32_t rollingMessageID_;     ///< A count that increments on every sendMsg

        //! Break circular reference between FragmentBuffer and leastRecentlyUpdatedFragmentBuffers_
        struct FragmentBuffer;
        typedef boost::shared_ptr<FragmentBuffer> FragmentBufferPtr;

        //! Origin and rollingMessageID are unique over significant periods making it a suitable message key
        typedef std::pair<std::string, uint32_t> MessageKey;
        //! MessageKey to FragmentBuffer map
        typedef std::map<MessageKey, FragmentBufferPtr> FragmentBufferMap;
        //! Least recently used FragmentBuffers list
        typedef std::list<FragmentBufferMap::iterator> FragmentBufferLRU;

        /**
         * @brief A workspace for reassembly of message fragments
         */
        struct FragmentBuffer
        {
            FragmentBuffer(const FragmentBufferLRU::iterator &i)
                : receivedCount(0),
                  receivedBytes(0),
                  lruEntry(i)
            {
            }

            TimeStamp lastMessageAdded;     ///< Time stamp of the last message added, for tracking stale buffers
            MessageHeader lastHeader;       ///< Header of the latest page
            std::string data;               ///< Assembly area of received fragments
            std::vector<bool> received;     ///< Flag to indicate whether a fragment was received
            uint32_t receivedCount;         ///< Number of received fragments
            uint32_t receivedBytes;         ///< Number of received bytes
            FragmentBufferLRU::iterator lruEntry; //< Least recently used list entry
        };

        //! Fragment buffers associated to messages to provide for reassembly
        FragmentBufferMap fragmentBuffers_;
        //! Least recently updated fragment buffer list
        FragmentBufferLRU fragmentBuffersLRU_;
        //! Pruning threshold
        double staleBufferPruningThreshold_s_;
        void pruneStalePartialBuffers();

        struct ChannelData
        {
            ChannelData(const uint32_t maxReceiveDepth) 
                : acknowledged(false),
                  maxReceiveDepth(maxReceiveDepth)
            {
            }

            bool acknowledged;
            uint32_t maxReceiveDepth;
            std::queue<FragmentBufferPtr> completeBuffers;
        };

        typedef std::map<std::string, ChannelData> ChannelMap;

        //! A mapping of channels to lists of completed fragment buffers, protected by completeBuffersPerChannelMutex_
        ChannelMap channels_;
        //! Mutex to protect completeBuffersPerChannel_ between the receiver thread and the client
        pthread_mutex_t channelsMutex_;
        //! A condition variable reflecting whether messages are available for receiving
        Condition messagesAvailableCondition;

        typedef std::pair<std::string, std::string> ChannelOriginKey;
        typedef std::map<ChannelOriginKey, ChannelOriginTotals> ChannelOriginStatsMap;
        ChannelOriginStatsMap channelOriginStats_;
        ChannelOriginStatsMap::iterator fetchTotals(const ChannelOriginKey &statsKey);

        Status::States sendMsg(const Message &msg);
        Status::States sendMsg(const MessageBase &msg);
        Status::States handleIncomingMsg();
        Status::States handleControlMsg(const Message &msg);
        Status::States closeConnection(const bool receiverThread);

        uint32_t threadRunning_;            ///< A counter tracking pthread_(create|join), protected by fdMutex_
        pthread_t receiverThreadHandle_;    ///< Handle for the receiver thread
        static void *receiverThreadLauncher(void *arg);
        void receiverThread();

        TimeStamp lastReport_;
        uint32_t totalFragmentsReceived_, totalMessagesReceived_, checksumErrors_;

        logger::Logger logger_;             ///< Client's status logging facility
};

}

#endif

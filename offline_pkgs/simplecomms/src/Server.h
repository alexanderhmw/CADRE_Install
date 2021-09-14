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
 * @file      Server.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_SERVER_H_
#define _SIMPLECOMMS_SERVER_H_

#include <pthread.h>
#include <sys/poll.h>

#include <deque>
#include <list>

#include <boost/noncopyable.hpp>
#include <boost/scoped_array.hpp>

#include "Peer.h"

#include <log/Logger.h>

namespace SimpleComms
{

class Server : public boost::noncopyable
{
    public:
        Server(const PeerPtr local, const PeerPtr remote);
        ~Server();

        bool start(const uint32_t numberOfOutputWorkers);
        void stop();

    private:
        const PeerPtr local_;
        const PeerPtr remote_;

        pthread_t inThread_;
        uint32_t numberOfOutputWorkers_;
        boost::scoped_array<pthread_t> outThreads_;
        int inToOutPipe_[2];
        volatile bool running_;

        pthread_mutex_t writeQueuesMutex_;
        std::list<Peer::BaseClientPtr> writeWaiting_;
        std::deque<Peer::BaseClientPtr> writeReady_;
        pthread_cond_t writeQueuesNotEmpty_;

        pthread_mutex_t writeWaitingMutex_;
        uint32_t maxWriteHandles_;
        boost::scoped_array<pollfd> writefds_;

        volatile unsigned reads_;
        volatile unsigned localReadBytes_, remoteReadBytes_;
        volatile unsigned writes_;

        static void *inThreadMainLauncher(void *arg);
        static void *outThreadMainLauncher(void *arg);

        void inThreadMain();
        void outThreadMain();

        void doPoll();

        logger::Logger logger_;
};

}

#endif

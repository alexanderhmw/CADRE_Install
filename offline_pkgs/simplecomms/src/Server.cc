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
 * @file      Server.cc
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "Server.h"

#include <errno.h>
#include <cstdio>

using namespace std;

namespace SimpleComms
{

Server::Server(const PeerPtr local, const PeerPtr remote)
    : local_(local),
      remote_(remote),
      numberOfOutputWorkers_(0),
      running_(false),
      maxWriteHandles_(0),
      writefds_(new pollfd[1]),
      reads_(0),
      localReadBytes_(0),
      remoteReadBytes_(0),
      writes_(0)
{
    pthread_mutex_init(&writeQueuesMutex_, NULL);
    pthread_mutex_init(&writeWaitingMutex_, NULL);
    pthread_cond_init(&writeQueuesNotEmpty_, NULL);
    inToOutPipe_[0] = -1;
    inToOutPipe_[1] = -1;
}

Server::~Server()
{
    pthread_cond_destroy(&writeQueuesNotEmpty_);
    pthread_mutex_destroy(&writeQueuesMutex_);
    pthread_mutex_destroy(&writeWaitingMutex_);
}

bool Server::start(const uint32_t numberOfOutputWorkers)
{
    numberOfOutputWorkers_ = numberOfOutputWorkers;

    if (!running_)
    {
        bool inToOutPipeActive = false, inThreadActive = false;
        uint32_t createdThreadsCount = 0;
        running_ = true;

        if (pipe(inToOutPipe_) != 0)
        {
            logger_.log_warn("Error creating in to out signalling pipe: %s", 
                             (errno < sys_nerr) ? sys_errlist[errno] : "Unknown"
                            );

            running_ = false;
        }
        else
        {
            inToOutPipeActive = true;
        }

        if (running_)
        {
            const int result = pthread_create(&inThread_, NULL, inThreadMainLauncher, this);

            if (result != 0)
            {
                logger_.log_warn("Error creating input thread: %s", 
                                 (result < sys_nerr) ? sys_errlist[result] : "Unknown"
                                );

                running_ = false;
            }
            else
            {
                inThreadActive = true;
            }
        }

        if (running_)
        {
            outThreads_.reset(new pthread_t[numberOfOutputWorkers_]);

            for (uint32_t i = 0; (i < numberOfOutputWorkers_) && running_; ++i)
            {
                const int result = pthread_create(&outThreads_[i], NULL, outThreadMainLauncher, this);

                if (result != 0)
                {
                    logger_.log_warn("Error creating output thread: %s", 
                                     (result < sys_nerr) ? sys_errlist[result] : "Unknown"
                                    );

                    running_ = false;
                }
                else
                {
                    ++createdThreadsCount;
                }
            }
        }

        // Clean up if the process failed. The last item doesn't need cleaning up as either it wasn't attempted at all, 
        // or was the reason for the failure, not requiring clean up.
        if (!running_)
        {
            if (inToOutPipeActive)
            {
                close(inToOutPipe_[0]);
                close(inToOutPipe_[1]);

                inToOutPipe_[0] = -1;
                inToOutPipe_[1] = -1;
            }

            if (inThreadActive)
            {
                pthread_join(inThread_, NULL);
            }

            pthread_cond_broadcast(&writeQueuesNotEmpty_);

            for (uint32_t i = 0; i < createdThreadsCount; ++i)
            {
                pthread_join(outThreads_[i], NULL);
            }
        }
    }

    return running_;
}

void Server::stop()
{
    if (running_)
    {
        running_ = false;

        pthread_cond_broadcast(&writeQueuesNotEmpty_);

        pthread_join(inThread_, NULL);

        for (uint32_t i = 0; i < numberOfOutputWorkers_; i++)
        {
            pthread_join(outThreads_[i], NULL);
        }

        close(inToOutPipe_[0]);
        close(inToOutPipe_[1]);

        inToOutPipe_[0] = -1;
        inToOutPipe_[1] = -1;
    }
}

void *Server::inThreadMainLauncher(void *arg)
{
    reinterpret_cast<Server *>(arg)->inThreadMain();

    return NULL;
}

void *Server::outThreadMainLauncher(void *arg)
{
    reinterpret_cast<Server *>(arg)->outThreadMain();

    return NULL;
}

void Server::inThreadMain()
{
    TimeStamp then, now;
    Peer::BaseClientList readHandles;
    boost::scoped_array<pollfd> ufds;
    uint32_t maxHandles = 0;
    while (running_)
    {
        readHandles.clear();
    
        MessagePtr msg;

        local_->getNamedReadHandles(readHandles);
        if (remote_)
        {
            remote_->getNamedReadHandles(readHandles);
        }

        const unsigned int nfds = readHandles.size();

        if (nfds > maxHandles)
        {
            maxHandles = nfds;
            readHandles.reserve(maxHandles);
            ufds.reset(new pollfd[maxHandles]);
        }

        for (unsigned int i = 0; i < nfds; i++)
        {
            ufds[i].fd = readHandles[i]->getReadHandle();
            ufds[i].events = POLLIN;
        }

        const int pollResult = poll(ufds.get(), nfds, 1000);

        if (pollResult > 0)
        {
            for (unsigned int i = 0; i < nfds; i++)
            {
                Peer * const currentHandlePeer(readHandles[i]->getParentPeer());
                if (ufds[i].revents & POLLIN)
                {
                    if (currentHandlePeer->receiveMsg(readHandles[i], msg) == Status::NewData)
                    {
                        Peer::BaseClientList newPendingClients;

                        local_->queueMsg(msg, newPendingClients);

                        if (remote_ && (currentHandlePeer != remote_.get()))
                        {
                            remote_->queueMsg(msg, newPendingClients);
                        }

                        pthread_mutex_lock(&writeQueuesMutex_);

                        for (Peer::BaseClientList::iterator i = newPendingClients.begin(); 
                             i != newPendingClients.end(); 
                             i++
                            )
                        {
                            if (!(*i)->isOnWriteQueue())
                            {
                                logger_.log_debug("WRITEON %s", (*i)->getName().c_str());
                                (*i)->setOnWriteQueue();

                                if ((*i)->isReadyToWrite())
                                {
                                    writeReady_.push_back(*i);
                                }
                                else
                                {
                                    writeWaiting_.push_back(*i);
                                }

                                pthread_cond_signal(&writeQueuesNotEmpty_);
                            }
                            else
                            {
                                logger_.log_debug("WRITEDUP %s", (*i)->getName().c_str());
                            } 
                        }

                        // If there are clients requiring polling...
                        if (!writeWaiting_.empty())
                        {
                            // and I can't acquire a lock to the poll routine, wake it up
                            if (pthread_mutex_trylock(&writeWaitingMutex_) != 0)
                            {
                                const char buf = 0;
                                write(inToOutPipe_[1], &buf, 1);
                            }
                            // otherwise, never mind
                            else
                            {
                                pthread_mutex_unlock(&writeWaitingMutex_);
                            }
                        }

                        pthread_mutex_unlock(&writeQueuesMutex_);

                        reads_++;

                        if (currentHandlePeer == local_.get())
                        {
                            localReadBytes_ += msg->MessageLength();
                        }
                        else
                        {
                            remoteReadBytes_ += msg->MessageLength();
                        }
                    }
                }

                if (ufds[i].revents & (POLLERR | POLLHUP | POLLNVAL))
                {
                    Peer::BaseClientList errorHandles(1, readHandles[i]);

                    currentHandlePeer->handleErrors(errorHandles);
                }
            }
        }

        now.setNow();
        if ((double)(now - then) > 1)
        {
            fprintf(stderr, 
                    "reads %7.1lf (local %8.1f kB/s, remote %8.1lf kB/s); writes %7.1lf\n", 
                    reads_ / (double)(now - then), 
                    (localReadBytes_ / 1024.0) / (double)(now - then), 
                    (remoteReadBytes_ / 1024.0) / (double)(now - then), 
                    writes_ / (double)(now - then)
                   );

            reads_ = 0;
            localReadBytes_ = 0;
            remoteReadBytes_ = 0;

            writes_ = 0;
            then = now;
        }
    }
}

void Server::outThreadMain()
{
    pthread_mutex_lock(&writeQueuesMutex_);

    while (running_)
    {
        if (!writeWaiting_.empty() && (pthread_mutex_trylock(&writeWaitingMutex_) == 0))
        {
            doPoll();

            pthread_mutex_unlock(&writeWaitingMutex_);
        }
        else if (!writeReady_.empty())
        {
            Peer::BaseClientPtr currentClientPtr = writeReady_.front();
            writeReady_.pop_front();

            pthread_mutex_unlock(&writeQueuesMutex_);

            {
                Peer::BaseClientList writeHandles(1, currentClientPtr);
                currentClientPtr->getParentPeer()->doWrites(writeHandles);
                writes_++;
            }

            pthread_mutex_lock(&writeQueuesMutex_);

            if (currentClientPtr->hasDataToWrite())
            {
                if (currentClientPtr->isReadyToWrite())
                {
                    logger_.log_debug("WRITEREADY %s", currentClientPtr->getName().c_str());
                    writeReady_.push_back(currentClientPtr);
                }
                else
                {
                    logger_.log_debug("WRITEWAIT %s", currentClientPtr->getName().c_str());
                    writeWaiting_.push_back(currentClientPtr);
                }
            }
            else
            {
                logger_.log_debug("WRITEOFF %s", currentClientPtr->getName().c_str());
                currentClientPtr->setOffWriteQueue();
            }
        }
        else
        {
            pthread_cond_wait(&writeQueuesNotEmpty_, &writeQueuesMutex_);
        }
    }

    pthread_mutex_unlock(&writeQueuesMutex_);
}

void Server::doPoll()
{
    const uint32_t writeWaitingCount = writeWaiting_.size();

    if (writeWaitingCount > maxWriteHandles_)
    {
        maxWriteHandles_ = writeWaitingCount;
        writefds_.reset(new pollfd[maxWriteHandles_ + 1]);
    }

    {
        uint32_t j = 0;
        for (std::list<Peer::BaseClientPtr>::iterator i = writeWaiting_.begin();
             i != writeWaiting_.end();
             ++i, ++j)
        {
            logger_.log_debug("DOPOLL %s", (*i)->getName().c_str());
            writefds_[j].fd = (*i)->getWriteHandle();
            writefds_[j].events = POLLOUT;
        }
    }

    pthread_mutex_unlock(&writeQueuesMutex_);

    writefds_[writeWaitingCount].fd = inToOutPipe_[0];
    writefds_[writeWaitingCount].events = POLLIN;

    const int pollResult = poll(writefds_.get(), writeWaitingCount + 1, 1000);

    // Clear the wake up from the input thread, the write polling will be updated when this loop turns over
    if ((pollResult > 0) && (writefds_[writeWaitingCount].revents & POLLIN))
    {
        char buf = 0;
        read(inToOutPipe_[0], &buf, sizeof(buf));
    }

    pthread_mutex_lock(&writeQueuesMutex_);

    if (pollResult > 0) 
    {
        std::list<Peer::BaseClientPtr> newWaiting_;

        std::list<Peer::BaseClientPtr>::iterator i(writeWaiting_.begin());
        for (uint32_t j = 0;
             (j < writeWaitingCount) && (i != writeWaiting_.end());
             ++i, ++j)
        {
            if (writefds_[j].revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                logger_.log_debug("POLLERR %s", (*i)->getName().c_str());
                Peer::BaseClientList errorHandles(1, *i);

                (*i)->getParentPeer()->handleErrors(errorHandles);
            }
            else if (writefds_[j].revents & POLLOUT)
            {
                logger_.log_debug("POLL+++ %s", (*i)->getName().c_str());
                (*i)->setReadyToWrite();
                writeReady_.push_back(*i);
            }
            else
            {
                logger_.log_debug("POLL--- %s", (*i)->getName().c_str());
                newWaiting_.push_back(*i);
            }
        }

        if (i == writeWaiting_.end())
        {
            writeWaiting_.swap(newWaiting_);
        }
        else
        {
            writeWaiting_.erase(writeWaiting_.begin(), i);
            writeWaiting_.insert(writeWaiting_.end(), newWaiting_.begin(), newWaiting_.end());
        }
    }
}

}


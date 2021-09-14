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
#ifndef _SIMPLECOMMS_BASEPEER_H_
#define _SIMPLECOMMS_BASEPEER_H_

#include <pthread.h>
#include <list>

#include "Peer.h"
#include "ControlMessages.h"

#include <log/Logger.h>

namespace SimpleComms
{

class BasePeer : public Peer
{
    public:
        BasePeer(const Transports::TransportTypes &transport, 
                 const bool supportsPartialWrites, 
                 const TimeStamp &staleFragmentPruningThreshold
                );
        virtual ~BasePeer();

        virtual bool isSubscribedTo(const std::string &channel);
        virtual void getSubscriptions(std::vector<std::string> &channels);

        virtual Status::States connect() = 0;
        virtual bool isConnectedTo(const std::string &clientName);
        virtual Status::States disconnect() = 0;

        virtual Transports::TransportTypes getTransport();

        virtual Status::States queueMsg(const MessagePtr &msg, BaseClientList &newPendingClients);
        virtual Status::States receiveMsg(const BaseClientPtr &clientHandle, MessagePtr &msg);

        virtual Status::States getNamedReadHandles(BaseClientList &readHandles);
        virtual Status::States getNamedMonitorErrorHandles(BaseClientList &monitorHandles);
        virtual Status::States getNamedWriteHandles(BaseClientList &writeHandles);

        virtual Status::States doWrites(const BaseClientList &writeHandles);

        virtual Status::States handleErrors(BaseClientList &errorHandles);

        virtual Status::States handleControlMsg(const MessagePtr &msg);

    protected:
        const Transports::TransportTypes transport_;
        const bool supportsPartialWrites_;
        const TimeStamp staleFragmentPruningThreshold_;
        const bool staleThresholdValid_;
        std::string peerName_;
        int fd_;

        class ServerData : public BaseClientType
        {
            public:
                ServerData(BasePeer *bp) : bp_(bp) { }
                virtual ~ServerData() { }

                virtual Peer *getParentPeer() { return bp_; }
                virtual std::string getName() { return bp_->peerName_; }
                virtual int getReadHandle() { return bp_->fd_; }
                virtual int getWriteHandle() { return -1; }
                virtual bool isReadyToWrite() { return false; }
                virtual void setReadyToWrite() { }
                virtual bool hasDataToWrite() { return false; }
                virtual void setOnWriteQueue() { }
                virtual void setOffWriteQueue() { }
                virtual bool isOnWriteQueue() { return false; }

            private:
                BasePeer *bp_;
        };

        boost::shared_ptr<ServerData> serverData_;

        class ClientData : public BaseClientType
        {
            public:
                ClientData(BasePeer *bp, const std::string &name) 
                    : name(name), 
                      fd(-1),
                      readyToWrite(false),
                      bytesWrittenSoFar(0),
                      dataPending(false),
                      onWriteQueue(false),
                      bp_(bp) 
                { 
                }
                virtual ~ClientData() { }

                const std::string name;
                int fd;
                volatile bool readyToWrite;
                TimeStamp fdStamp;
                std::list<MessagePtr> outBox;
                MessagePtr currentOutboundMessage;
                uint32_t bytesWrittenSoFar;
                volatile bool dataPending;
                volatile bool onWriteQueue;

                virtual Peer *getParentPeer() { return bp_; }
                virtual std::string getName() { return name; }
                virtual int getReadHandle() { return -1; }
                virtual int getWriteHandle() { return fd; }
                virtual bool isReadyToWrite() { return readyToWrite; }
                virtual void setReadyToWrite() { readyToWrite = true; }
                virtual bool hasDataToWrite() { return dataPending; }
                virtual void setOnWriteQueue() { onWriteQueue = true; }
                virtual void setOffWriteQueue() { onWriteQueue = false; }
                virtual bool isOnWriteQueue() { return onWriteQueue; }

            private:
                BasePeer *bp_;
        };

        typedef boost::shared_ptr<ClientData> ClientDataPtr;
        typedef std::vector<ClientDataPtr> ClientList;

        struct ClientDataSortByName : public std::binary_function<ClientDataPtr, 
                                                                  ClientDataPtr, 
                                                                  bool
                                                                 > 
        {
            bool operator()(const ClientDataPtr a, const ClientDataPtr b) 
            { 
                return a->name < b->name; 
            }
        };

        ClientList clients_;
        pthread_mutex_t clientsMutex_;

        struct ChannelData
        {
            std::string name;
            ClientList subscribers;
        };

        struct ChannelDataSortByName : public std::binary_function<ChannelData, ChannelData, bool> 
        {
            bool operator()(const ChannelData &a, const ChannelData &b) 
            { 
                return a.name < b.name; 
            }
        };

        std::vector<ChannelData> channels_;

        virtual ClientDataPtr clientCreate(const std::string &name);
        virtual bool clientLookup(const std::string &name, ClientList::iterator &namedClient, const bool create = false);
        virtual bool clientConnect(const ClientDataPtr &namedClient, const ConnectionMessage *connMsg) = 0;
        virtual void clientReset(const ClientDataPtr &namedClient, const bool retireEntry = false);

        logger::Logger logger_;
};

}

#endif

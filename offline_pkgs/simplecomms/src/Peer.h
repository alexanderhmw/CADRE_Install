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
 * @file      Peer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_PEER_H_
#define _SIMPLECOMMS_PEER_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "ControlMessages.h"
#include "Message.h"
#include "Status.h"

namespace SimpleComms
{

class Peer
{
    public:
        virtual ~Peer()
        {
        }

        virtual bool isSubscribedTo(const std::string &channel) = 0;
        virtual void getSubscriptions(std::vector<std::string> &channels) = 0;

        virtual Status::States connect() = 0;
        virtual bool isConnectedTo(const std::string &clientName) = 0;
        virtual Status::States disconnect() = 0;

        virtual Transports::TransportTypes getTransport() = 0;
        virtual Status::States getConnectionInfo(Message &msg) = 0;

        class BaseClientType
        {
            public:
                virtual ~BaseClientType() { };

                virtual Peer *getParentPeer() = 0;
                virtual std::string getName() = 0;
                virtual int getReadHandle() = 0;
                virtual int getWriteHandle() = 0;
                virtual bool isReadyToWrite() = 0;
                virtual void setReadyToWrite() = 0;
                virtual bool hasDataToWrite() = 0;
                virtual void setOnWriteQueue() = 0;
                virtual void setOffWriteQueue() = 0;
                virtual bool isOnWriteQueue() = 0;
        };

        typedef boost::shared_ptr<BaseClientType> BaseClientPtr;
        typedef std::vector<BaseClientPtr> BaseClientList;

        virtual Status::States queueMsg(const MessagePtr &msg, BaseClientList &newPendingClients) = 0;
        virtual Status::States receiveMsg(const BaseClientPtr &clientHandle, MessagePtr &msg) = 0;

        virtual Status::States getNamedReadHandles(BaseClientList &readHandles) = 0;
        virtual Status::States getNamedMonitorErrorHandles(BaseClientList &monitorHandles) = 0;
        virtual Status::States getNamedWriteHandles(BaseClientList &writeHandles) = 0;

        virtual Status::States doWrites(const BaseClientList &writeHandles) = 0;

        virtual Status::States handleErrors(BaseClientList &errorHandles) = 0;

        virtual Status::States handleControlMsg(const MessagePtr &msg) = 0;
};

typedef boost::shared_ptr<Peer> PeerPtr;

}

#endif

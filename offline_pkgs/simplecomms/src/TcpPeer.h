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
 * @file      TcpPeer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_TCPPEER_H_
#define _SIMPLECOMMS_TCPPEER_H_

#include "BasePeer.h"

namespace SimpleComms
{

class TcpPeer : public BasePeer
{
    public:
        TcpPeer(const std::string &advertisedIntf, 
                const uint16_t port, 
                const int backLog, 
                const TimeStamp &staleFragmentPruningThreshold
               );
        virtual ~TcpPeer();

        virtual Status::States connect();
        virtual Status::States disconnect();

        virtual Status::States getConnectionInfo(Message &msg);

        virtual Status::States receiveMsg(const BaseClientPtr &clientHandle, MessagePtr &msg);

        virtual Status::States getNamedReadHandles(BaseClientList &readHandles);

        virtual Status::States handleErrors(BaseClientList &errorHandles);

    protected:
        const std::string &advertisedIntf_;
        uint16_t port_;
        int backLog_;

        class InboundClientDataTCP : public BaseClientType
        {
            public:
                InboundClientDataTCP(BasePeer *bp, const int fd) : fd(fd), msgBufIndex(0), bp_(bp) { }
                virtual ~InboundClientDataTCP() { }

                virtual Peer *getParentPeer() { return bp_; }
                virtual std::string getName() { return ""; }
                virtual int getReadHandle() { return fd; }
                virtual int getWriteHandle() { return -1; }
                virtual bool isReadyToWrite() { return false; }
                virtual void setReadyToWrite() { }
                virtual bool hasDataToWrite() { return false; }
                virtual void setOnWriteQueue() { }
                virtual void setOffWriteQueue() { }
                virtual bool isOnWriteQueue() { return false; }

                int fd;
                MessageStub msgHeaderBuf;
                MessagePtr msg;
                uint32_t msgBufIndex;

            private:
                BasePeer *bp_;
        };

        typedef boost::shared_ptr<InboundClientDataTCP> InboundClientDataTCPPtr;
        typedef std::vector<InboundClientDataTCPPtr> InboundClientList;

        InboundClientList inboundClients_;
        void inboundClientReset(const InboundClientDataTCPPtr &inboundClient);

        virtual bool clientConnect(const ClientDataPtr &namedClient, const ConnectionMessage *connMsg);
};

}

#endif

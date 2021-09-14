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
 * @file      UdpPeer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_UDPPEER_H_
#define _SIMPLECOMMS_UDPPEER_H_

#include "BasePeer.h"

namespace SimpleComms
{

class UdpPeer : public BasePeer
{
    public:
        UdpPeer(const std::string &advertisedIntf, 
                const uint16_t port,
                const TimeStamp &staleFragmentPruningThreshold
               );
        virtual ~UdpPeer();

        virtual Status::States connect();
        virtual Status::States disconnect();

        virtual Status::States getConnectionInfo(Message &msg);

    protected:
        const std::string &advertisedIntf_;
        const uint16_t port_;

        bool clientConnect(const ClientDataPtr &namedClient, const ConnectionMessage *connMsg);
};

}

#endif

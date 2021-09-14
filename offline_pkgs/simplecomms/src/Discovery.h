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
 * @file      Discovery.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_DISCOVERY_H_
#define _SIMPLECOMMS_DISCOVERY_H_

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "Peer.h"
#include "Status.h"

namespace SimpleComms
{

class Discovery
{
    public:
        virtual ~Discovery()
        {
        }

        virtual Status::States initialize() = 0;
        virtual Status::States isActive() = 0;
        virtual Status::States terminate() = 0;

        virtual Status::States registerLocalPeeringMethod(const PeerPtr localPeer) = 0;
        virtual Status::States unregisterLocalPeeringMethod(const PeerPtr localPeer) = 0;
        virtual Status::States registerRemotePeeringMethod(const PeerPtr remotePeer) = 0;
        virtual Status::States unregisterRemotePeeringMethod(const PeerPtr remotePeer) = 0;
};

typedef boost::shared_ptr<Discovery> DiscoveryPtr;

}

#endif

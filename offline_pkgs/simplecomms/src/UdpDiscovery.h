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
 * @file      UdpDiscovery.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_UDPDISCOVERY_H_
#define _SIMPLECOMMS_UDPDISCOVERY_H_

#include <map>

#include <log/Logger.h>

#include "Discovery.h"
#include "TimeStamp.h"

namespace SimpleComms
{

class UdpDiscovery : public Discovery
{
    public:
        UdpDiscovery(const std::string &advertisedIntf, const uint16_t port);
        virtual ~UdpDiscovery();

        virtual Status::States initialize();
        virtual Status::States isActive();
        virtual Status::States terminate();

        virtual Status::States registerLocalPeeringMethod(const PeerPtr localPeer);
        virtual Status::States unregisterLocalPeeringMethod(const PeerPtr localPeer);
        virtual Status::States registerRemotePeeringMethod(const PeerPtr remotePeer);
        virtual Status::States unregisterRemotePeeringMethod(const PeerPtr remotePeer);

    private:
        const std::string &advertisedIntf_;
        const uint16_t port_;
        std::string hostName_;
        unsigned int broadcastAddress_;
        TimeStamp instanceEpoch_;
        int fd_;
        int rollingMessageID_;

        Status::States terminate(const bool discoveryThread);

        typedef std::set<std::string> SubscriberSet;
        typedef std::map<std::string, SubscriberSet> ChannelSubscriberMap;
        ChannelSubscriberMap channelSubscribers_;

        typedef std::map<Transports::TransportTypes, PeerPtr> PeeringMethodsMap;
        PeeringMethodsMap remotePeeringMethods_, localPeeringMethods_;

        typedef std::pair<std::string, Transports::TransportTypes> PeerTransportPair;
        typedef std::map<PeerTransportPair, MessagePtr> ConnectionInfoMap;
        ConnectionInfoMap connectionOffers_;

        uint32_t threadRunning_;            
        pthread_t discoveryThreadHandle_;    
        static void *discoveryThreadLauncher(void *arg);
        void discoveryThreadMain();

        Status::States sendMsg(Message &msg, const uint32_t dataLength);
        Status::States sendSubscriptions();
        Status::States sendRegisteredRemotePeeringMethods();
        Status::States handleIncomingMsg();
        Status::States handleControlMsg(const Message &msg);

        Status::States updateSubscriptions(const PeerPtr remotePeer, 
                                           const std::string &origin,
                                           const std::vector<std::string> &subscribes, 
                                           const std::vector<std::string> &unsubscribes
                                          );

        logger::Logger logger_;
};

}

#endif

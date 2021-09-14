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
 * @file      ControlMessages.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_CONTROLMESSAGES_H_
#define _SIMPLECOMMS_CONTROLMESSAGES_H_

#include <stdint.h>

#include "Constants.h"
#include "TimeStamp.h"

namespace SimpleComms
{

namespace ControlMessages
{
    enum ControlMessageTypes
    {
        Connect,
        Disconnect,
        Subscribe,
        Unsubscribe,
        SubscriptionList
    };
}

namespace Transports
{
    enum TransportTypes
    {
        UnixDomainSocket,
        UDP,
        TCP
    };
}

const char *transportName(const Transports::TransportTypes &transport);

struct ConnectionMessage 
{
    ControlMessages::ControlMessageTypes type;
    Transports::TransportTypes transport;
    struct timeval offerEpoch;
} __attribute__((packed));

struct ConnectionMessageIPV4
{
    ControlMessages::ControlMessageTypes type;
    Transports::TransportTypes transport;
    struct timeval offerEpoch;
    uint32_t inetAddr;
    uint16_t inetPort;
} __attribute__((packed));

struct DisconnectionMessage 
{
    ControlMessages::ControlMessageTypes type;
    Transports::TransportTypes transport;
} __attribute__((packed));

struct SubscriptionMessage 
{
    ControlMessages::ControlMessageTypes type;
    char channel[MAX_NULL_TERM_CHANNEL_NAME];
} __attribute__((packed));

struct SubscriptionListMessage 
{
    ControlMessages::ControlMessageTypes type;
    uint16_t numChannels;
    char channel[999][MAX_NULL_TERM_CHANNEL_NAME];
} __attribute__((packed));

}

#endif

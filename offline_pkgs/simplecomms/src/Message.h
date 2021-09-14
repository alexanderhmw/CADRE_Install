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
 * @file      Message.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_MESSAGE_H_
#define _SIMPLECOMMS_MESSAGE_H_

#include <stdint.h>
#include <zlib.h>
#include <string.h>

#include <boost/shared_ptr.hpp>

#include "Constants.h"
#include "TimeStamp.h"

namespace SimpleComms
{

struct MessageHeader
{
    MessageHeader() { }

    char origin[MAX_NULL_TERM_CLIENT_NAME];
    char channel[MAX_NULL_TERM_CHANNEL_NAME];
    struct timeval sendMsgTime;
    uint32_t length;
    uint32_t rollingMessageID;
    uint32_t totalPages;

    uint32_t pageNumber;
    uint32_t dataLength;
    uint32_t headerChecksum;
    uint32_t dataChecksum;

    uint32_t MessageLength() const
    {
        return sizeof(MessageHeader) + dataLength;
    }
} __attribute__((packed));

struct Message : MessageHeader
{
    Message() { }

    unsigned char data[MAX_FRAGMENT_PAYLOAD];
} __attribute__((packed));

struct MessageBase : MessageHeader
{
    MessageBase() { }

    unsigned char *data;
};

struct MessageStub : MessageHeader
{
    MessageStub() { }
    MessageStub(const Message &source)
    {
        memcpy(this, &source, source.MessageLength());
    }

    unsigned char data[0];

    void *operator new(const long unsigned int size)
    {
        return ::operator new(size);
    }

    void *operator new(const long unsigned int size, const unsigned int dataLength)
    {
        return ::operator new(size + dataLength);
    }

    void operator delete(void *retired)
    {
        ::operator delete(retired);
    }
};

typedef boost::shared_ptr<MessageStub> MessagePtr;

#include "Message.def.h"

}

#endif

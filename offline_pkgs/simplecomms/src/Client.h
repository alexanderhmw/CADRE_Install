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
 * @file      Client.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_CLIENT_H_
#define _SIMPLECOMMS_CLIENT_H_

#include <string>
#include <vector>

#include "Condition.h"
#include "MessageInfo.h"
#include "Status.h"

namespace SimpleComms
{

class Client
{
    public:
        virtual ~Client()
        {
        }

        virtual Status::States connect() = 0;
        virtual Status::States disconnect() = 0;

        virtual Status::States subscribe(const std::string &channel, const uint32_t maxReceiveDepth = 0) = 0;
        virtual Status::States unsubscribe(const std::string &channel) = 0;

        virtual Status::States sendMsg(const std::string &channel, const std::string &data) = 0;
        virtual Status::States receiveMsg(const std::string &channel, std::string &data, MessageInfo &msgInfo) = 0;
        virtual Status::States receiveMsg(const std::string &channel, std::string &data) = 0;

        virtual Condition &getMessagesAvailableCondition() = 0;
};

}

#endif

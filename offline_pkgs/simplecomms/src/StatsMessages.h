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
 * @file      UnixClient.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */

#include "Constants.h"

namespace SimpleComms
{

struct ChannelOriginTotals
{
    ChannelOriginTotals()
        : origin(),
          channel(),
          totalFragments(0),
          totalBytes(0),
          totalDelay(0),
          completedMessages(0),
          completedFragments(0),
          completedBytes(0),
          incompleteMessages(0),
          expectedFragments(0),
          receivedFragments(0),
          expectedBytes(0),
          receivedBytes(0)
    {
    }

    char origin[MAX_NULL_TERM_CLIENT_NAME];
    char channel[MAX_NULL_TERM_CHANNEL_NAME];
    uint32_t totalFragments, totalBytes;
    double totalDelay;
    uint32_t completedMessages, completedFragments, completedBytes;
    uint32_t incompleteMessages, expectedFragments, receivedFragments, expectedBytes, receivedBytes;
};

}


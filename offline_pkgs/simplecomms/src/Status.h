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
 * @file      Status.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#ifndef _SIMPLECOMMS_STATUS_H_
#define _SIMPLECOMMS_STATUS_H_

namespace SimpleComms
{
    namespace Status
    {
        enum States
        {
            Ok,
            Error,
            AlreadyConnected,
            SocketPathTooLong,
            ErrorCreatingSocket,
            ErrorBindingSocket,
            ErrorConnectingSocket,
            ErrorListeningSocket,
            NotConnected,
            NewData,
            ChannelNameTooLong,
            InvalidHandle
        };
    }
}

#endif

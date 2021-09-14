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
 * @file    Monitor.h
 * @author  Tugrul Galatali
 * @date    10/16/2006
 *
 * @attention Copyright (c) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <map>
#include <vector>

#include "TimeStamp.h"
#include "Condition.h"

class Monitor {
    public:
        Monitor();
        ~Monitor();

        void add(Condition &c);
        void remove(Condition &c);

        std::vector<Condition *> wait(const TimeStamp &timeout);

    private:
        std::map<int, Condition *> conditions;
};

#endif

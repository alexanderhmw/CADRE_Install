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
 * @file    Condition.h
 * @author  Tugrul Galatali
 * @date    10/16/2006
 *
 * @attention Copyright (c) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#ifndef _CONDITION_H_
#define _CONDITION_H_

#include <pthread.h>
#include <boost/noncopyable.hpp>

class Condition : boost::noncopyable {
    public:
        Condition();
        ~Condition();

        bool init();

        void set();
        void unset();

        bool isSet();

        friend bool operator==(const Condition &a, const Condition &b);

        friend class Monitor;
    private:
        pthread_mutex_t pipeMutex_;
        volatile bool set_;
        int pipe_[2];
};

#endif

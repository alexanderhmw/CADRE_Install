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
 * @file    PThreadLocker.h
 * @author  Tugrul Galatali
 * @date    08/24/2006
 *
 * @attention Copyright (c) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#ifndef _PTHREADLOCKER_H_
#define _PTHREADLOCKER_H_

#include <pthread.h>
#include <boost/noncopyable.hpp>

/**
 * @brief PThreadLocker provides "automatic" unlocking on loss of scope.
 *
 * Modelled on QT's QMutexLocker, declare it in a scope where exclusive access of a resource is neeeded. Upon exit from
 * that scope, the variable is descoped, the destructor is called and the mutex is unlocked. Useful for blocks with
 * multiple exit points.
 *
 */
class PThreadLocker : public boost::noncopyable
{
    public:
        PThreadLocker(pthread_mutex_t *m) : m(m) 
        { 
            pthread_mutex_lock(m); 
        }

        ~PThreadLocker() 
        {
            pthread_mutex_unlock(m); 
        }

    private:
        pthread_mutex_t *m;
};

#endif

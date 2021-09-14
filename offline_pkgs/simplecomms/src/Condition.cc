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
 * @file    Condition.cc
 * @author  Tugrul Galatali
 * @date    10/16/2006
 *
 * @attention Copyright (c) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#include "Condition.h"

#include <unistd.h>

Condition::Condition()
    : set_(false)
{
    pthread_mutex_init(&pipeMutex_, NULL);

    pipe_[0] = -1;
    pipe_[1] = -1;
}

Condition::~Condition()
{
    close(pipe_[0]);
    close(pipe_[1]);
}

/**
 * @brief Initialize the condition
 *
 * @return true if the initialization was successful
 */
bool Condition::init()
{
    if ((pipe_[0] == -1) && (pipe_[1] == -1))
    {
        if (pipe(pipe_) == -1)
        {
            pipe_[0] = -1;
            pipe_[1] = -1;

            return false;
        }
    }
    
    return true;
}

/**
 * @brief Set the condition
 *
 * Monitor::wait should not block if this condition is included after this call.
 */
void Condition::set()
{
    pthread_mutex_lock(&pipeMutex_);

    if (!set_)
    {
        set_ = true;

        {
            const char buf = 0;
            write(pipe_[1], &buf, 1);
        }
    }

    pthread_mutex_unlock(&pipeMutex_);
}

/**
 * @brief Unset the condition
 *
 * Monitor::wait should block if this condition is included after this call.
 */
void Condition::unset()
{
    pthread_mutex_lock(&pipeMutex_);

    if (set_)
    {
        set_ = false;

        {
            char buf = 0;
            read(pipe_[0], &buf, 1);
        }
    }

    pthread_mutex_unlock(&pipeMutex_);
}

/**
 * @brief Query if the condition is set
 *
 * @return true if the condition is set
 */
bool Condition::isSet()
{
    return set_;
}

/**
 * @brief Test if conditions are the same
 *
 * @return true if the conditions are the same
 *
 * The file descriptors from pipe should be unique.
 */
bool operator==(const Condition &a, const Condition &b)
{
    return (a.pipe_[0] == b.pipe_[0]) && (a.pipe_[1] == b.pipe_[1]);
}

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
 * @file    Monitor.cc
 * @author  Tugrul Galatali
 * @date    10/16/2006
 *
 * @attention Copyright (c) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#include "Monitor.h"

using namespace std;

Monitor::Monitor()
{
}

Monitor::~Monitor()
{
}

/**
 * @brief Add a condition to the monitor.
 */
void Monitor::add(Condition &c)
{
    const map<int, Condition *>::iterator dupCheck = conditions.find(c.pipe_[0]);

    if (dupCheck == conditions.end())
    {
        conditions[c.pipe_[0]] = &c;
    }
}

/**
 * @brief Remove a condition from the monitor.
 */
void Monitor::remove(Condition &c)
{
    const map<int, Condition *>::iterator dupCheck = conditions.find(c.pipe_[0]);

    if (dupCheck != conditions.end())
    {
        conditions.erase(dupCheck);
    }
}

/**
 * @brief Block until one of the contained conditions is set, or timeout expires.
 */
vector<Condition *> Monitor::wait(const TimeStamp &timeout)
{
    fd_set rfds;

    FD_ZERO(&rfds);

    int maxfd = 0;
    for (map<int, Condition *>::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
    {
        FD_SET((*i).second->pipe_[0], &rfds);

        maxfd = (*i).second->pipe_[0];
    }

    int retVal = 0;
    if (timeout == TimeStamp())
    {
        retVal = select(maxfd + 1, &rfds, NULL, NULL, NULL);
    }
    else
    {
        struct timeval tv;

        tv.tv_sec = timeout.tv_sec;
        tv.tv_usec = timeout.tv_usec;

        retVal = select(maxfd + 1, &rfds, NULL, NULL, &tv);
    }

    vector<Condition *> setConditions;
    if (retVal > 0)
    {
        for (map<int, Condition *>::const_iterator i = conditions.begin(); i != conditions.end(); ++i)
        {
            if (FD_ISSET((*i).second->pipe_[0], &rfds))
            {
                setConditions.push_back((*i).second);
            }
        }
    }
    
    return setConditions;
}

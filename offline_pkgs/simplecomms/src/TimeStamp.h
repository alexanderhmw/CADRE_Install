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
 * @file      TimeStamp.h
 * @author    Tugrul Galatali (tugrul@galatali.com)
 * @date      09/10/2006
 *
 * @attention Copyright (C) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#ifndef _TIME_H_TIMESTAMP_H_
#define _TIME_H_TIMESTAMP_H_

#include <sys/time.h>
#include <time.h>

#include <stdint.h>

/**
 * @brief Basic time stamp class
 *
 * A simple time stamp class founded on the veteran struct timeval plus some convenience functions.
 *
 * The issue of abstract time hasn't really been discussed yet, but the stamp itself hopefully doesn't need to be 
 * abstract.
 */
struct TimeStamp : public timeval 
{
    TimeStamp();
    TimeStamp(const time_t &s, const suseconds_t &us);
    TimeStamp(const double &time_s);
    TimeStamp(const struct timeval &tv);

    operator double () const;

    TimeStamp operator+ (const TimeStamp &m) const;
    TimeStamp operator- (const TimeStamp &m) const;
    TimeStamp operator+ (const double &time_s) const;
    TimeStamp operator- (const double &time_s) const;

    bool operator== (const TimeStamp &m) const;
    bool operator!= (const TimeStamp &m) const;
    bool operator< (const TimeStamp &m) const;
    bool operator<= (const TimeStamp &m) const;
    bool operator> (const TimeStamp &m) const;
    bool operator>= (const TimeStamp &m) const;

    bool setNow();

    void normalize();

    template<class Archive>
    void serialize(Archive &ar, const unsigned int /* file_version */);
};

#include "TimeStamp.def.h"

#endif //ifndef _TIMESTAMP_H_

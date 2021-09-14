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
 * @file      TimeStamp.def.h
 * @author    Tugrul Galatali (tugrul@galatali.com)
 * @date      09/10/2006
 *
 * @attention Copyright (C) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 *
 * @todo The code below is fairly trivial/inline worthy, but perhaps we should find a library for it to call home.
 *
 * Implementation of the TimeStamp class.
 */

#ifndef _TIME_H_TIMESTAMP_H_
#error Should only be included from TimeStamp.h
#endif

/**
 * @brief Default TimeStamp constructor
 *
 * Initialize the time stamp to 0. Can't use an initialization list as timeval doesn't come with a handy constructor.
 */
inline TimeStamp::TimeStamp()
{
    tv_sec = 0;
    tv_usec = 0;
}

/**
 * @brief TimeStamp sec/usec constructor
 *
 * @param s Initialization time, whole second component
 * @param us Initialization time, microsecond component
 *
 * Initialize the time stamp to given seconds and microseconds.
 */
inline TimeStamp::TimeStamp(const time_t &s, const suseconds_t &us)
{
    tv_sec = s;
    tv_usec = us;
}

/**
 * @brief TimeStamp double constructor
 *
 * @param time_s Initialization time
 *
 * Initialize the time stamp to time in seconds.
 */
inline TimeStamp::TimeStamp(const double &time_s)
{
    tv_sec = int(time_s);
    tv_usec = int((time_s - tv_sec) * 1000000);
}

inline TimeStamp::TimeStamp(const struct timeval &tv)
{
    tv_sec = tv.tv_sec;
    tv_usec = tv.tv_usec;
}

/**
 * @brief Cast as double
 *
 * @return Stored time in seconds in double format
 *
 * A double has just enough precision to store 32-bit second count and 20-bit microsecond count, and is sometimes more 
 * convenient to work with.
 */
inline TimeStamp::operator double () const
{
    return tv_sec + tv_usec / 1000000.0f;
}

/**
 * @brief Add two TimeStamps
 *
 * @param m The addend.
 *
 * @return The total of this and the argument.
 */
inline TimeStamp TimeStamp::operator+ (const TimeStamp &m) const 
{
    TimeStamp tmp;

    tmp.tv_sec = tv_sec + m.tv_sec;
    tmp.tv_usec = tv_usec + m.tv_usec;

    tmp.normalize();

    return tmp;
}

/**
 * @brief Subtract two TimeStamps
 *
 * @param m The subtrahend.
 *
 * @return The difference of this and the argument.
 */
inline TimeStamp TimeStamp::operator- (const TimeStamp &m) const 
{
    TimeStamp tmp;

    tmp.tv_sec = tv_sec - m.tv_sec;
    tmp.tv_usec = tv_usec - m.tv_usec;

    tmp.normalize();

    return tmp;
}

/**
 * @brief Add a double in seconds to a TimeStamp
 *
 * @param time_s The addend in seconds as a double.
 *
 * @return The total of this and the argument.
 */
inline TimeStamp TimeStamp::operator+ (const double &time_s) const 
{
    TimeStamp tmp;

    tmp.tv_sec = tv_sec + int(time_s);
    tmp.tv_usec = tv_usec + int((time_s - int(time_s)) * 1000000);

    tmp.normalize();

    return tmp;
}

/**
 * @brief Subtract a double in seconds from a TimeStamp
 *
 * @param time_s The subtrahend in seconds as a double.
 *
 * @return The difference of this and the argument.
 */
inline TimeStamp TimeStamp::operator- (const double &time_s) const 
{
    TimeStamp tmp;

    tmp.tv_sec = tv_sec - int(time_s);
    tmp.tv_usec = tv_usec - int((time_s - int(time_s)) * 1000000);

    tmp.normalize();

    return tmp;
}

/**
 * @brief Test for equality of this and another TimeStamp
 */
inline bool TimeStamp::operator== (const TimeStamp &m) const 
{
    return (tv_sec == m.tv_sec) && (tv_usec == m.tv_usec);
}

/**
 * @brief Test for inequality of this and another TimeStamp
 */
inline bool TimeStamp::operator!= (const TimeStamp &m) const 
{
    return (tv_sec != m.tv_sec) || (tv_usec != m.tv_usec);
}

/**
 * @brief Test if this TimeStamp is less than another
 */
inline bool TimeStamp::operator< (const TimeStamp &m) const 
{
    return ((tv_sec < m.tv_sec) || ((tv_sec == m.tv_sec) && (tv_usec < m.tv_usec)));
}

/**
 * @brief Test if this TimeStamp is less than or equal to another
 */
inline bool TimeStamp::operator<= (const TimeStamp &m) const 
{
    return ((tv_sec < m.tv_sec) || ((tv_sec == m.tv_sec) && (tv_usec <= m.tv_usec)));
}

/**
 * @brief Test if this TimeStamp is greater than to another
 */
inline bool TimeStamp::operator> (const TimeStamp &m) const 
{
    return ((tv_sec > m.tv_sec) || ((tv_sec == m.tv_sec) && (tv_usec > m.tv_usec)));
}

/**
 * @brief Test if this TimeStamp is greater than or equal to another
 */
inline bool TimeStamp::operator>= (const TimeStamp &m) const 
{
    return ((tv_sec > m.tv_sec) || ((tv_sec == m.tv_sec) && (tv_usec >= m.tv_usec)));
}

/**
 * @brief Set to current time with gettimeofday
 */
inline bool TimeStamp::setNow()
{
    return (gettimeofday(this, NULL) == 0);
}

/**
 * @brief Move whole seconds from tv_usec to tv_sec, leaving 0 <= tv_usec < 1000000
 */
inline void TimeStamp::normalize() 
{
    if (tv_usec < 0) 
    {
        uint32_t negative_seconds = -tv_usec / 1000000 + 1;

        tv_sec -= negative_seconds;
        tv_usec += negative_seconds * 1000000;
    }

    tv_sec += tv_usec / 1000000;
    tv_usec %= 1000000;
}

/**
 * @brief boost::serialization compliant marshalling method
 *
 * TimeStamp is archive version invariant
 */
template<class Archive>
void TimeStamp::serialize(Archive &ar, const unsigned int /* file_version */) {
    ar & tv_sec;
    ar & tv_usec;
}


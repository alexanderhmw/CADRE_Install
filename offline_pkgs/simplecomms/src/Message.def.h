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
 * @file      Message.def.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */

template <typename T>
bool messageComputeChecksum(T &msg, uint32_t &newHeaderChecksum, uint32_t &newDataChecksum, const bool headerOnly = false)
{
    if (msg.dataLength <= MAX_FRAGMENT_PAYLOAD)
    {
        const uint32_t oldHeaderChecksum = msg.headerChecksum;
        const uint32_t oldDataChecksum = msg.dataChecksum;

        msg.headerChecksum = 0;

        if (!headerOnly)
        {
            // Adler is faster and good enough for our purposes with enough data
            // http://www.zlib.net/zlib_tech.html
            if (msg.dataLength > 1024)
            {
                newDataChecksum = adler32(0, reinterpret_cast<const Bytef *>(msg.data), msg.dataLength);
            }
            else
            {
                newDataChecksum = crc32(0, reinterpret_cast<const Bytef *>(msg.data), msg.dataLength);
            }

            msg.dataChecksum = newDataChecksum;
        }

        newHeaderChecksum = crc32(0, 
                                  reinterpret_cast<const Bytef *>(&msg), 
                                  sizeof(MessageHeader)
                                 );

        msg.headerChecksum = oldHeaderChecksum;
        msg.dataChecksum = oldDataChecksum;

        return true;
    }

    return false;
}

template <typename T>
bool messageSetChecksum(T &msg)
{
    uint32_t newHeaderChecksum = 0, newDataChecksum = 0;
    const bool status = messageComputeChecksum(msg, newHeaderChecksum, newDataChecksum);

    if (status)
    {
        msg.headerChecksum = newHeaderChecksum;
        msg.dataChecksum = newDataChecksum;
    }

    return status;
}

template <typename T>
bool messageValidateHeaderChecksum(T &msg)
{
    uint32_t newHeaderChecksum = 0, newDataChecksum = 0;
    const bool status = messageComputeChecksum(msg, newHeaderChecksum, newDataChecksum, true);
    assert(newDataChecksum == 0);

    return status && (msg.headerChecksum == newHeaderChecksum);
}

template <typename T>
bool messageValidateChecksum(T &msg)
{
    uint32_t newHeaderChecksum = 0, newDataChecksum = 0;
    const bool status = messageComputeChecksum(msg, newHeaderChecksum, newDataChecksum);

    return status && (msg.headerChecksum == newHeaderChecksum) && (msg.dataChecksum == newDataChecksum);
}


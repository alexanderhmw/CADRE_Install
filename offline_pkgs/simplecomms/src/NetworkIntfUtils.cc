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
 * @file    NetworkIntfUtils.h
 * @author  Tugrul Galatali
 * @date    03/10/2007
 *
 * @attention Copyright (c) 2006
 * @attention Carnegie Mellon University
 * @attention All rights reserved.
 */
#include "NetworkIntfUtils.h"

#include <stdint.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include <boost/scoped_array.hpp>

namespace NetworkIntfUtils
{

bool GetInterfaceAddresses(IntfNameAddrMap &intfs)
{
    int sockfd;

    if (0 > (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP))) {
        return false;
    }

    struct ifconf ifConf;

    ifConf.ifc_req = NULL;
    ifConf.ifc_len = 0;

    if (ioctl(sockfd, SIOCGIFCONF, &ifConf)) {
        close(sockfd);
        return false;
    }

    boost::scoped_array<unsigned char> ifcReqBuf(new unsigned char[ifConf.ifc_len]);
    ifConf.ifc_req = reinterpret_cast<ifreq *>(ifcReqBuf.get());

    if (ioctl(sockfd, SIOCGIFCONF, &ifConf)) {
        close(sockfd);
        return false;
    }

    for (uint32_t i = 0; i < ifConf.ifc_len / sizeof(ifreq); i++)
    {
        struct ifreq &ifr(ifConf.ifc_req[i]);

        intfs[ifr.ifr_name] = ntohl(reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_addr)->sin_addr.s_addr);
    }

    close(sockfd);

    return true;
}

bool GetInterfaceParameter(const std::string &name, const ParameterTypes &type, unsigned int &value)
{
    int ioctlNumber;

    switch (type)
    {
        case Address:
            ioctlNumber = SIOCGIFADDR;
            break;

        case Netmask:
            ioctlNumber = SIOCGIFNETMASK;
            break;

        case Broadcast:
            ioctlNumber = SIOCGIFBRDADDR;
            break;

        default:
            return false;
    }

    int sockfd;

    if (0 > (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP))) {
        return false;
    }

    struct ifreq ifr; 

    strncpy(ifr.ifr_name, name.c_str(), IFNAMSIZ);

    if (ioctl(sockfd, ioctlNumber, &ifr)) {
        close(sockfd);
        return false;
    }

    value = ntohl(reinterpret_cast<struct sockaddr_in *>(&ifr.ifr_addr)->sin_addr.s_addr);

    close(sockfd);

    return true;
}

}

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
 * @file      UdpPeer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "UdpPeer.h"
#include "ControlMessages.h"
#include "NetworkIntfUtils.h"

#include <cstdio>
#include <cerrno>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

using namespace std;

namespace SimpleComms
{

UdpPeer::UdpPeer(const string &advertisedIntf, 
                 const uint16_t port,
                 const TimeStamp &staleFragmentPruningThreshold
                )
    : BasePeer(Transports::UDP, false, staleFragmentPruningThreshold),
      advertisedIntf_(advertisedIntf),
      port_(port)
{
}

UdpPeer::~UdpPeer()
{
}

Status::States UdpPeer::connect()
{
    if (fd_ != -1)
    {
        return Status::AlreadyConnected;
    }

    {
        char hostNameBuf[HOST_NAME_MAX];

        if (gethostname(hostNameBuf, HOST_NAME_MAX) == -1)
        {
            return Status::Error;
        }

        peerName_ = hostNameBuf;
    }

    struct sockaddr_in servAddr;

    if ((fd_ = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        return Status::ErrorCreatingSocket;
    }

    memset(reinterpret_cast<void *>(&servAddr), 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port_);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd_, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) < 0)
    {
        close(fd_);
        fd_ = -1;

        return Status::ErrorBindingSocket;
    }

    return Status::Ok;
}

Status::States UdpPeer::disconnect()
{
    close(fd_);
    fd_ = -1;

    return Status::Ok;
}

Status::States UdpPeer::getConnectionInfo(Message &msg)
{
    Status::States returnStatus = Status::Ok;

    NetworkIntfUtils::IntfNameAddrMap intfs;
    if (NetworkIntfUtils::GetInterfaceAddresses(intfs))
    {
        ConnectionMessageIPV4 *connMsg = reinterpret_cast<ConnectionMessageIPV4 *>(msg.data);

        connMsg->type = ControlMessages::Connect;
        connMsg->transport = Transports::UDP;
        connMsg->inetAddr = htonl(intfs[advertisedIntf_]);
        connMsg->inetPort = htons(port_);

        msg.dataLength = sizeof(ConnectionMessageIPV4);
    }
    else
    {
        returnStatus = Status::Error;
    }

    return returnStatus;
}

/**
 * @brief Connect to the given client entry using the given connection information
 *
 * @param namedClient Pointer to the client entry to connect to
 * @param connMsg Pointer to the ConnectionMessage supplying the ip address/port
 *
 * Assumes clientsMutex_ is held by calling function
 * System calls are made while holding clientsMutex_
 */
bool UdpPeer::clientConnect(const ClientDataPtr &namedClient, const ConnectionMessage *connMsg)
{
    const ConnectionMessageIPV4 *connMsgIPV4 = reinterpret_cast<const ConnectionMessageIPV4 *>(connMsg);
    ClientData &currentClient(*namedClient);

    if (currentClient.fd != -1)
    {
        clientReset(namedClient);
    }

    logger_.log_debug("Connecting to client %s (%X %d)", currentClient.name.c_str(), connMsgIPV4->inetAddr, connMsgIPV4->inetPort);

    struct sockaddr_in servAddr;
    memset(reinterpret_cast<void *>(&servAddr), 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = connMsgIPV4->inetPort;
    servAddr.sin_addr.s_addr = connMsgIPV4->inetAddr;

    if ((currentClient.fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
    {
        if (errno < sys_nerr)
        {
            logger_.log_warn("Error creating socket: %s", sys_errlist[errno]);
        }
        else
        {
            logger_.log_warn("Unknown error creating socket");
        }

        return false;
    }

    if (::connect(currentClient.fd, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) < 0)
    {
        if (errno < sys_nerr)
        {
            logger_.log_warn("Error connecting socket: %s", sys_errlist[errno]);
        }
        else
        {
            logger_.log_warn("Unknown error connecting socket");
        }

        clientReset(namedClient);

        return false;
    }

    return true;
}

}

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
 * @file      UnixPeer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "UnixPeer.h"
#include "ControlMessages.h"

#include <cstdio>
#include <cerrno>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

using namespace std;

namespace SimpleComms
{

UnixPeer::UnixPeer(const string &path,
                   const TimeStamp &staleFragmentPruningThreshold
                  )
    : BasePeer(Transports::UnixDomainSocket, false, staleFragmentPruningThreshold)
{
    peerName_ = path;
}

UnixPeer::~UnixPeer()
{
}

Status::States UnixPeer::connect()
{
    if (fd_ != -1)
    {
        return Status::AlreadyConnected;
    }

    struct sockaddr_un servAddr;

    if (peerName_.length() > sizeof(servAddr.sun_path) - 2)
    {
        return Status::SocketPathTooLong;
    }

    if ((fd_ = socket(PF_UNIX, SOCK_DGRAM, 0)) < 0)
    {
        return Status::ErrorCreatingSocket;
    }

    memset(reinterpret_cast<void *>(&servAddr), 0, sizeof(servAddr));
    servAddr.sun_family = AF_UNIX;

    // Abstract namespace triggered via leading null
    strncpy(&servAddr.sun_path[1], peerName_.c_str(), sizeof(servAddr.sun_path) - 1);
    const int servLen = 1 + peerName_.length() + sizeof(servAddr.sun_family);

    if (bind(fd_, reinterpret_cast<struct sockaddr *>(&servAddr), servLen) < 0)
    {
        close(fd_);
        fd_ = -1;

        return Status::ErrorBindingSocket;
    }

    return Status::Ok;
}

Status::States UnixPeer::disconnect()
{
    close(fd_);
    fd_ = -1;

    return Status::Ok;
}

Status::States UnixPeer::getConnectionInfo(Message &msg)
{
    ConnectionMessage *connMsg = reinterpret_cast<ConnectionMessage *>(msg.data);

    connMsg->type = ControlMessages::Connect;
    connMsg->transport = Transports::UnixDomainSocket;

    return Status::Ok;
}

/**
 * @brief Connect to the given client entry using the given connection information
 *
 * @param namedClient Pointer to the client entry to connect to
 *
 * Assumes clientsMutex_ is held by calling function
 * System calls are made while holding clientsMutex_
 */
bool UnixPeer::clientConnect(const ClientDataPtr &namedClient, const ConnectionMessage *)
{
    ClientData &currentClient(*namedClient);

    if (currentClient.fd != -1)
    {
        clientReset(namedClient);
    }

    logger_.log_debug("Connecting to client %s", currentClient.name.c_str());

    struct sockaddr_un servAddr;
    if (currentClient.name.length() > sizeof(servAddr.sun_path) - 2)
    {
        return false;
    }

    memset(reinterpret_cast<void *>(&servAddr), 0, sizeof(servAddr));
    servAddr.sun_family = AF_UNIX;

    // Abstract namespace triggered via leading null
    strncpy(&servAddr.sun_path[1], currentClient.name.c_str(), sizeof(servAddr.sun_path) - 1);
    const int servLen = 1 + currentClient.name.length() + sizeof(servAddr.sun_family);

    if ((currentClient.fd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
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

    if (::connect(currentClient.fd, reinterpret_cast<struct sockaddr *>(&servAddr), servLen) < 0)
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

    currentClient.readyToWrite = true;

    return true;
}

}

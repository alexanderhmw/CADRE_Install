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
 * @file      TcpPeer.h
 * @author    Tugrul Galatali
 * @date      02/10/2007
 *
 * @attention Copyright (C) 2007
 * @attention Carnegie Mellon University
 * @attention All rights reserved
 */
#include "TcpPeer.h"
#include "ControlMessages.h"
#include "PThreadLocker.h"
#include "NetworkIntfUtils.h"

#include <cstdio>
#include <cerrno>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <limits.h>

#include "TimeStamp.h"

using namespace std;

namespace SimpleComms
{

TcpPeer::TcpPeer(const string &advertisedIntf, 
                 const uint16_t port, 
                 const int backLog, 
                 const TimeStamp &staleFragmentPruningThreshold
                )
    : BasePeer(Transports::TCP, true, staleFragmentPruningThreshold),
      advertisedIntf_(advertisedIntf),
      port_(port),
      backLog_(backLog)
{
}

TcpPeer::~TcpPeer()
{
}

Status::States TcpPeer::connect()
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

    if ((fd_ = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        return Status::ErrorCreatingSocket;
    }

    {
        const int flags = fcntl(fd_, F_GETFL);

        if ((flags < 0) || (fcntl(fd_, F_SETFL, flags | O_NONBLOCK) < 0))
        {
            close(fd_);
            fd_ = -1;

            return Status::ErrorCreatingSocket;
        }
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

    {
        socklen_t sockLen = sizeof(servAddr);
        if (getsockname(fd_, reinterpret_cast<struct sockaddr *>(&servAddr), &sockLen) < 0)
        {
            close(fd_);
            fd_ = -1;

            return Status::ErrorBindingSocket;
        }
    }

    port_ = ntohs(servAddr.sin_port);

    if (listen(fd_, backLog_) < 0)
    {
        close(fd_);
        fd_ = -1;

        return Status::ErrorListeningSocket;
    }

    return Status::Ok;
}

Status::States TcpPeer::disconnect()
{
    close(fd_);
    fd_ = -1;

    return Status::Ok;
}

Status::States TcpPeer::getConnectionInfo(Message &msg)
{
    Status::States returnStatus = Status::Ok;

    NetworkIntfUtils::IntfNameAddrMap intfs;
    if (NetworkIntfUtils::GetInterfaceAddresses(intfs))
    {
        ConnectionMessageIPV4 *connMsg = reinterpret_cast<ConnectionMessageIPV4 *>(msg.data);

        connMsg->type = ControlMessages::Connect;
        connMsg->transport = Transports::TCP;
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

Status::States TcpPeer::receiveMsg(const BaseClientPtr &clientHandle, MessagePtr &msg)
{
    Status::States status = Status::Ok;
    const InboundClientDataTCPPtr inboundClientDataTCP(boost::dynamic_pointer_cast<InboundClientDataTCP>(clientHandle));

    if (clientHandle == serverData_)
    {
        struct sockaddr_in remoteAddr;
        socklen_t remoteAddrLen(sizeof(remoteAddr));
        int newfd = accept(fd_, reinterpret_cast<struct sockaddr *>(&remoteAddr), &remoteAddrLen);

        if (newfd >= 0)
        {
            const int flags = fcntl(newfd, F_GETFL);

            if ((flags < 0) || (fcntl(newfd, F_SETFL, flags | O_NONBLOCK) < 0))
            {
                close(newfd);
                newfd = -1;
            }
        }

        if (newfd >= 0)
        {
            const int flag = 1;

            if (setsockopt(newfd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) != 0)
            {
                close(newfd);
                newfd = -1;
            }
        }

        if (newfd >= 0)
        {
            const InboundClientDataTCPPtr newInboundClient(new InboundClientDataTCP(this, newfd));
            const InboundClientList::iterator i(lower_bound(inboundClients_.begin(), 
                                                            inboundClients_.end(), 
                                                            newInboundClient
                                                           ));

            inboundClients_.insert(i, newInboundClient);
        }
        else
        {
            status = Status::Error;
        }
    }
    else if (inboundClientDataTCP.get() != NULL)
    {
        InboundClientDataTCP &currentClient(*inboundClientDataTCP);

        if (currentClient.msgBufIndex < sizeof(MessageHeader))
        {
            const int recvStatus = recv(currentClient.fd, 
                                        reinterpret_cast<uint8_t *>(&currentClient.msgHeaderBuf) 
                                            + currentClient.msgBufIndex,
                                        sizeof(MessageHeader) - currentClient.msgBufIndex,
                                        MSG_DONTWAIT
                                       ); 

            if (recvStatus > 0)
            {
                currentClient.msgBufIndex += recvStatus;

                if (currentClient.msgBufIndex == sizeof(MessageHeader))
                {
                    if (!messageValidateHeaderChecksum(currentClient.msgHeaderBuf))
                    {
                        logger_.log_warn("Header checksum error");
                        status = Status::Error;
                    }
                    else
                    {
                        currentClient.msg.reset(new(currentClient.msgHeaderBuf.dataLength) MessageStub());
                        memcpy(currentClient.msg.get(), &currentClient.msgHeaderBuf, sizeof(MessageHeader));
                    }
                }
            }
            else
            {
                logger_.log_warn("Error in recv: %s", (errno < sys_nerr) ? sys_errlist[errno] : "Unknown");
                status = Status::Error;
            }
        }
        else
        {
            const uint32_t dataIndex = currentClient.msgBufIndex - sizeof(MessageHeader);
            const int recvStatus = recv(currentClient.fd, 
                                        reinterpret_cast<uint8_t *>(&currentClient.msg->data) + dataIndex,
                                        currentClient.msgHeaderBuf.dataLength - dataIndex,
                                        MSG_DONTWAIT
                                       ); 

            if (recvStatus > 0)
            {
                currentClient.msgBufIndex += recvStatus;

                if (currentClient.msgBufIndex == currentClient.msgHeaderBuf.MessageLength())
                {
                    // We could validateChecksum() here for the entire message, but the client already does this. 
                    // Synchronization errors will show up shortly in the next validateHeaderChecksum(), so no real
                    // need for the redundancy.
                    msg = currentClient.msg;
                    currentClient.msg.reset();

                    currentClient.msgBufIndex = 0;

                    status = Status::NewData;
                }
            }
            else
            {
                status = Status::Error;
            }
        }

        if (status == Status::Error)
        {
            inboundClientReset(inboundClientDataTCP);
        }
    }
    else
    {
        status = Status::InvalidHandle;
    }

    return status;
}

Status::States TcpPeer::getNamedReadHandles(BaseClientList &readHandles)
{
    readHandles.push_back(serverData_);
    readHandles.insert(readHandles.end(), inboundClients_.begin(), inboundClients_.end());

    return Status::Ok;
}

Status::States TcpPeer::handleErrors(BaseClientList &errorHandles)
{
    Status::States status = Status::Ok;

    for (BaseClientList::iterator i(errorHandles.begin());
         i != errorHandles.end();
        )
    {
        PThreadLocker clientsLock(&clientsMutex_);
        const BaseClientPtr &baseClientPtr(*i);
        const InboundClientDataTCPPtr currentClientPtr(boost::dynamic_pointer_cast<InboundClientDataTCP>(baseClientPtr));

        if (currentClientPtr.get())
        {
            int err = -1;
            socklen_t len = sizeof(err);

            if (getsockopt(currentClientPtr->fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0)
            {
                err = -1;
            }

            if (err != 0)
            {
                inboundClientReset(currentClientPtr);
            }

            i = errorHandles.erase(i);
        }
        else
        {
            ++i;
        }
    }

    if (!errorHandles.empty())
    {
        BasePeer::handleErrors(errorHandles);
    }

    return status;
}

void TcpPeer::inboundClientReset(const InboundClientDataTCPPtr &inboundClient)
{
    close(inboundClient->fd);

    const InboundClientList::iterator i(lower_bound(inboundClients_.begin(), 
                                                    inboundClients_.end(), 
                                                    inboundClient
                                                   ));

    if (inboundClient == *i)
    {
        inboundClients_.erase(i);
    }
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
bool TcpPeer::clientConnect(const ClientDataPtr &namedClient, const ConnectionMessage *connMsg)
{
    const ConnectionMessageIPV4 *connMsgIPV4 = reinterpret_cast<const ConnectionMessageIPV4 *>(connMsg);
    ClientData &currentClient(*boost::dynamic_pointer_cast<ClientData>(namedClient));

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

    if ((currentClient.fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
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

    {
        const int flags = fcntl(currentClient.fd, F_GETFL);

        if ((flags < 0) || (fcntl(currentClient.fd, F_SETFL, flags | O_NONBLOCK) < 0))
        {
            clientReset(namedClient);

            return false;
        }
    }

    if (::connect(currentClient.fd, reinterpret_cast<struct sockaddr *>(&servAddr), sizeof(servAddr)) < 0)
    {
        if (errno != EINPROGRESS)
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
    }

    {
        const int flag = 1;

        if (setsockopt(currentClient.fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) != 0)
        {
            clientReset(namedClient);

            return false;
        }
    }

    return true;
}

}

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

#include "Monitor.h"
#include "UnixClient.h"
#include <cstdio>

using namespace std;
using namespace SimpleComms;

void sendmsg(Client *test, const char *channel, const string &msg)
{
    while (test->sendMsg(channel, msg) != Status::Ok)
    {
        printf("disconnect\n");
        test->disconnect();

        Status::States connectResult;
        while ((connectResult = test->connect()) != Status::Ok)
        {
            perror("Failed to connect");
            sleep(1);
        }
    }
}

int main(int argc, char **argv)
{
    logger::Logger::setGlobalPriorityThreshold(logger::Priority::notice);

    if ((argc < 4) || (argc > 6))
    {
        printf("%s msgSize taskName inChannelName [outChannelName] [partner]\n", argv[0]);
        exit(1);
    }

    const char *server = "scs";
    const int msgSize = atoi(argv[1]);
    const char *client = argv[2];
    const char *inChannel = argv[3]; 
    const char *outChannel = (argc > 4) ? argv[4] : inChannel; 
    const char *partner = (argc > 5) ? argv[5] : client;

    Client *test = new UnixClient(server, client);

    while (!(test->connect() == Status::Ok))
    {
        perror("Failed to connect");
        sleep(1);
    }

    test->subscribe(inChannel);

    string msgBuf(msgSize, 0);

    TimeStamp lastReport;
    lastReport.setNow();
    uint32_t receivedCount = 0;
    double latencySum = 0, maxLatency = 0;
    double roundTripLatencySum = 0, roundTripMaxLatency = 0;
    bool sendMessage = true;
    while (1)
    {
        Monitor messagesAvailableMonitor;

        messagesAvailableMonitor.add(test->getMessagesAvailableCondition());

        messagesAvailableMonitor.wait(TimeStamp(1.0));

        if (test->getMessagesAvailableCondition().isSet())
        {
            MessageInfo msgInfo;
            if (test->receiveMsg(inChannel, msgBuf, msgInfo) == Status::NewData)
            {
                if (msgInfo.origin == partner) 
                {
                    const double latency = (double)(msgInfo.msgReadyTime - msgInfo.sendMsgTime);
                    receivedCount++;
                    latencySum += latency;
                    maxLatency = max(latency, maxLatency);

                    TimeStamp roundTripStartTime;
                    memcpy(&roundTripStartTime, msgBuf.c_str() + sizeof(TimeStamp), sizeof(TimeStamp));

                    const double roundTripLatency = (double)(msgInfo.msgReadyTime - roundTripStartTime);
                    roundTripLatencySum += roundTripLatency;
                    roundTripMaxLatency = max(roundTripLatency, roundTripMaxLatency);

                    if (msgInfo.msgReadyTime - lastReport > TimeStamp(1.0))
                    {
                        printf("received: %3d ptp avg: %6.1fms max: %6.1fms rtt avg: %6.1fms max: %6.1fms\n", 
                               receivedCount, 
                               latencySum / receivedCount * 1000,
                               maxLatency * 1000,
                               roundTripLatencySum / receivedCount * 1000, 
                               roundTripMaxLatency * 1000
                              );
                        latencySum = maxLatency = roundTripLatencySum = roundTripMaxLatency = receivedCount = 0;
                        lastReport = msgInfo.msgReadyTime;
                    }

                    sendMessage = true;
                }
            }
        }
        else
        {
            printf("Nothing received for one second, sending message anyway!\n");
            sendMessage = true;
        }

        if (sendMessage)
        {
            { 
                TimeStamp tmp;
                tmp.setNow();

                memcpy(const_cast<char *>(msgBuf.c_str()) + sizeof(TimeStamp), 
                       const_cast<char *>(msgBuf.c_str()), 
                       sizeof(TimeStamp)
                      );
                memcpy(const_cast<char *>(msgBuf.c_str()), &tmp, sizeof(TimeStamp));
            }

            sendmsg(test, outChannel, msgBuf);

            sendMessage = false;
        }
    }
}

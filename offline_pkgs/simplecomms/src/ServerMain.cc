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

#include <errno.h>
#include <signal.h>
#include <cstdio>


#include "Server.h"
#include "UdpDiscovery.h"
#include "UnixPeer.h"
#include "TcpPeer.h"
#include "UdpPeer.h"

#include <log/Logger.h>

#define MAX_EXIT_SIGNALS_INTERVAL 1.0

using namespace std;
using namespace SimpleComms;
using namespace logger;


int main(int argc, char **argv)
{
    Logger::setGlobalPriorityThreshold(logger::Priority::notice);

////////////////////////////////////////////////////////////////////////////////
// Parse command line options
  std::map<std::string, std::string> vm;

  vm["advertisedIntf"] = "eth0";
  vm["maxExitSignalsToCatch"] = "3";
  vm["staleFragmentPruningThreshold"]= "1.0";
  vm["numberOfOutputWorkers"]= "2";
  vm["tcpPort"] = "0";
  vm["tcpListenBackLog"] = "8";
  vm["udpPort"] = "11500";
  vm["unixPath"] = "scs";
  vm["udpDiscoveryPort"] = "11501";

  typedef std::pair<std::string, std::string> StringPair;
  std::vector< StringPair > help_info;
  help_info.push_back(
      StringPair("advertisedIntf",
                 "Details of this interface will be advertised for remote connections")
      );
  help_info.push_back(
      StringPair("help","Produce this help message")
      );
  help_info.push_back(
      StringPair("maxExitSignalsToCatch","Number of exit signals to catch during one second before terminating")
      );
  help_info.push_back(
      StringPair("staleFragmentPruningThreshold", "Fragments sent more than specified seconds ago will be discarded")
      );
  help_info.push_back(
      StringPair("numberOfOutputWorkers","Number of threads use to dispatch messages, minimum 2")
      );
  help_info.push_back(
      StringPair("tcp","Use TCP for remote peering")
      );
  help_info.push_back(
      StringPair("tcpPort", "TCP port for peering, 0 for automatic")
      );
  help_info.push_back(
      StringPair("tcpListenBackLog", "listen(2) backlog for TCP peer")
      );
  help_info.push_back(
      StringPair("udp","Use UDP for remote peering")
      );
  help_info.push_back(
      StringPair("udpPort","UDP port for peering")
      );
  help_info.push_back(
      StringPair("unixPath", "Local unix domain socket path")
      );
  help_info.push_back(
      StringPair("udpDiscoveryPort","UDP port for autodiscovery of peers")
      );

  std::vector<std::string> args;
  for (int ii=1; ii < argc; ++ii)
  {
    args.push_back(argv[ii]);
  }

  for (unsigned int ii=0; ii < args.size();/**/)
  {
    if (args[ii].length() < 3)
    {
      // we've encountered something that isn't a parameter specification, print
      // the help message and exit
      vm["help"] = "true";
      break;
    }
    if (args[ii][0]!='-' || args[ii][1]!='-')
    {
      // ditto.
      vm["help"] = "true";
      break;
    }

    if (args[ii] == "--help")
    {
      vm["help"] = "true";
      break;
    }
    else if (args[ii] == "--tcp")
    {
      vm["tcp"] = "true";
      ii++;
    }
    else if (args[ii] == "--udp")
    {
      vm["udp"] = "true";
      ii++;
    }
    else
    {
      // every other argument should have a default value in the table
      // if it's not in the table....
      if (vm.count( args[ii].substr(2)) == 0)
      {
        fprintf(stderr, "unsupported parameter: %s\n", args[ii].c_str());
        vm["help"] = "true";
        break;
      }
      else
      {
        if (ii < args.size()-1 )
        {
          vm[ args[ii].substr(2)] = args[ii+1];
          ii+=2;
        }
        else
        {
          fprintf(stderr, "parameter %s requires an argument\n", args[ii].c_str());
          vm["help"] = "true";
          break;
        }
      }
    }


  }


  if (vm.count("help"))
  {
    fprintf(stdout, "usage:\n" );
    for (unsigned int ii=0; ii < help_info.size(); ++ii)
    {
      fprintf(stdout," --%s\n     %s\n",help_info[ii].first.c_str(),
              help_info[ii].second.c_str());

    }
            exit(0);
    }

////////////////////////////////////////////////////////////////////////////////
// Move server into its own session if not already
    if (getpgid(0) != getpid())
    {
        const int setsidResult = setsid();

        if (setsidResult == -1)
        {
            fprintf(stderr, 
                    "Error creating session setsid(): %s\n", 
                    (errno < sys_nerr) ? sys_errlist[errno] : "Unknown"
                   );

            exit(1);
        }
    }

////////////////////////////////////////////////////////////////////////////////
// Block harmful signals to permit handling
    sigset_t signalSet;

    sigemptyset(&signalSet);

    sigaddset(&signalSet, SIGHUP);
    sigaddset(&signalSet, SIGINT);
    sigaddset(&signalSet, SIGTERM);
    sigaddset(&signalSet, SIGPIPE);
    sigaddset(&signalSet, SIGQUIT);

    {
        const int pthread_sigmaskResult = pthread_sigmask(SIG_BLOCK, &signalSet, NULL);
        if (pthread_sigmaskResult != 0)
        {
            fprintf(stderr, 
                    "Error blocking signals pthread_sigmask(): %s\n", 
                    (pthread_sigmaskResult < sys_nerr) ? sys_errlist[pthread_sigmaskResult] : "Unknown"
               );

            exit(2);
        }
    }

////////////////////////////////////////////////////////////////////////////////
// Instantiate the local peering method
    const PeerPtr localPeer(new UnixPeer(vm["unixPath"],
                                         TimeStamp(atof(vm["staleFragmentPruningThreshold"].c_str()))
                                        ));

    if (localPeer->connect() != Status::Ok)
    {
        fprintf(stderr, "Failed to initialize local peer!\n");
        exit(10);
    }
    
////////////////////////////////////////////////////////////////////////////////
// Instantiate the remote peering method
    PeerPtr remotePeer;
    
    if (vm.count("udp") && vm.count("tcp"))
    {
        fprintf(stderr, "Both --udp and --tcp may not be specified at once\n");
        exit(20);
    }
    else if (vm.count("udp"))
    {
        remotePeer.reset(new UdpPeer(vm["advertisedIntf"],
                                     atoi(vm["udpPort"].c_str()),
                                     TimeStamp(atof(vm["staleFragmentPruningThreshold"].c_str()))
                                    ));
    }
    else if (vm.count("tcp"))
    {
        remotePeer.reset(new TcpPeer(vm["advertisedIntf"],
                                     atoi(vm["tcpPort"].c_str()),
                                     atoi(vm["tcpListenBackLog"].c_str()),
                                     TimeStamp(atof(vm["staleFragmentPruningThreshold"].c_str()))
                                    ));
    }

    if (remotePeer)
    {
        if (remotePeer->connect() != Status::Ok)
        {
            fprintf(stderr, "Failed to initialize remote peer!\n");
            exit(21);
        }
    }

////////////////////////////////////////////////////////////////////////////////
// Instantiate the server thread
    Server server(localPeer, remotePeer);

    const unsigned int numberOfOutputWorkers(atoi(vm["numberOfOutputWorkers"].c_str()));

    if (numberOfOutputWorkers < 2)
    {
        fprintf(stderr, "At least two workers are required, %d specified\n", numberOfOutputWorkers);
        exit(30);
    }

    if (!server.start(numberOfOutputWorkers))
    {
        fprintf(stderr, "Failed to initialize server!\n");
        exit(31);
    }

////////////////////////////////////////////////////////////////////////////////
// Instantiate the discovery agent if remote peering is enabled
    DiscoveryPtr discovery;
    
    if (remotePeer)
    {
        discovery.reset(new UdpDiscovery(vm["advertisedIntf"], atoi(vm["udpDiscoveryPort"].c_str())));

        if (discovery->initialize() != Status::Ok)
        {
            fprintf(stderr, "Failed to initialize discovery agent!\n");
            exit(40);
        }

        discovery->registerLocalPeeringMethod(localPeer);
        discovery->registerRemotePeeringMethod(remotePeer);
    }

////////////////////////////////////////////////////////////////////////////////
// Monitor signals
    TimeStamp lastExitSignal; lastExitSignal.setNow();
    const int maxExitSignalsToCatch = atoi(vm["maxExitSignalsToCatch"].c_str());
    for (int numSignalsCaught = 0; numSignalsCaught < maxExitSignalsToCatch;)
    {
        int sigNum;

        sigwait(&signalSet, &sigNum);

        if ((sigNum == SIGINT) 
            || (sigNum == SIGTERM)
            || (sigNum == SIGQUIT)
           )
        {
            TimeStamp now; now.setNow();

            if (now - lastExitSignal > TimeStamp(MAX_EXIT_SIGNALS_INTERVAL))
            {
                numSignalsCaught = 1;
            }
            else
            {
                numSignalsCaught++;
            }

            fprintf(stderr, 
                    "signal %s caught, %lf since last signal, %d more to terminate\n", 
                    strsignal(sigNum), 
                    static_cast<double>(now - lastExitSignal), 
                    maxExitSignalsToCatch - numSignalsCaught
                   );

            lastExitSignal = now;
        }
    }

////////////////////////////////////////////////////////////////////////////////
// Shutdown

    if (discovery)
    {
        discovery->terminate();
    }

    server.stop();

    return 0;
}


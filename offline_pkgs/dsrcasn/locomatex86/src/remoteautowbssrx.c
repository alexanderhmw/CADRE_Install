/*

* Copyright (c) 2005-2007 Arada Syatems, Inc. All rights reserved.

* Proprietary and Confidential Material.

*

*/

#include "wave.h"
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>


//static USTEntry entry;
//static WMEApplicationRequest wreq;
static WMEApplicationRequest entry;
//static WSMIndication wsmrxind;
static int devicemode = WAVEDEVICE_REMOTE;
static WMEApplicationRequest aregreq;
static int pid;
static uint64_t count = 0;
void 	receiveWME_NotifIndication(WMENotificationIndication *wmeindication);
void 	receiveWSMIndication(WSMIndication *wsmindication);
void sig_int(void);
void sig_term(void);

int buildUSTEntry();
int rxWSMPPkts(int);
int	confirmBeforeJoin(WMEApplicationIndication *);

/*This program demonstrates how to recive WSMP packets on a HOST, from a USER registered on a TARGET device*/
int main (int argc, char *argv[]) 
{
	int ret;
	char ifname[30]="eth0";
	int blockflag = 0 ;
	//struct ifreq ifr;
    //	WSMIndication rxpkt;

	

#ifdef	WIN32
		
	WIN_SOCK_DLL_INVOKE  //intializing windows sockets
#endif
	pid = getpid();

	if(argc < 5) {
		printf("usage: remoterx <TARGETIP> [user-req type<1-auto> <2-unconditional> <3-none> ] [imm access ] [extended access ] [ channel <optional>] [Interface Name] \n");
		printf("--> Mention Interface name is \"lo\" for running remoterx in Locomate Board and Default interface is eth0\n");	
		printf("--> Mention Interface name for running remoterx in Windows Machine\n");	
		exit(-1);
	}
	
	setRemoteDeviceIP(argv[1]);
	if(argc == 6)
	strcpy(ifname,argv[5]);
	/*Set the Remote Device IP */
	buildUSTEntry();
	if( ( atoi( argv[2] ) > USER_REQ_SCH_ACCESS_NONE ) || ( atoi(argv[2]) < USER_REQ_SCH_ACCESS_AUTO ) ) {
		printf("User request type invalid: setting default to auto\n");
		entry.userreqtype = USER_REQ_SCH_ACCESS_AUTO;
	}
	else
	{
		entry.userreqtype = atoi(argv[2]);
	}
	if(entry.userreqtype == USER_REQ_SCH_ACCESS_AUTO_UNCONDITIONAL)
	{
		if(argc < 6)
		{
			printf("\n\tchannel needed for unconditional access\n");
			exit(-1);
		}
		else
		{
			entry.channel = atoi(argv[5]);
			if(argc == 7)
			strcpy(ifname,argv[6]);
		}
	}
	
	entry.schaccess = atoi(argv[2]);
	entry.schextaccess = atoi(argv[3]);
    	devicemode = WAVEDEVICE_REMOTE;
	ret =  invokeWAVEDevice(devicemode, blockflag ); /*blockflag is ignored in this case*/
	
	if (ret < 0 ) {
	} else {
		printf("Driver invoked\n");
	}
#if 0
	/*Get the IP of eth0*/	
	sfd = socket(AF_INET6, SOCK_STREAM, 0);
	if(sfd >= 0) {
		memset(&ifaddr, 0, sizeof(ifaddr));

#ifdef WIN32
		if((host_entry=gethostbyname(szHostName))!=NULL){
				szLocalIP = inet_ntoa (*(struct in_addr *)*host_entry->h_addr_list);
				sin->sin_addr.s_addr = inet_addr(szLocalIP);
				aregreq.ipv6addr = sin->sin_addr;
				entry.ipaddr = sin->sin_addr;
		}
#else
		if( getifaddrs(&ifaddr) == 0 )
                {
                	for( ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next )
                        {
                        	if( ifa->ifa_addr == NULL ) continue;
                                if( ( ifa->ifa_flags & IFF_UP ) == NULL ) continue;

                                if(ifa->ifa_addr->sa_family == AF_INET )
                                {
                                	sin4 = (struct sockaddr_in *)(ifa->ifa_addr);
                                        if(inet_ntop(ifa->ifa_addr->sa_family,(void *)&(sin4->sin_addr), str, sizeof(str)) == NULL )
                                        {
                                        	printf("IPV4 Interface = %s: inet_ntop failed\n", ifa->ifa_name );
                                        }
                                        else
                                        {
                				inet_pton( AF_INET, argv[1], &inaddr );
                                        	printf("IPV4 Interface =%s      %s\n",ifa->ifa_name, str );
                                        }
					inet_pton(AF_INET6, str, &sin6->sin6_addr );
					
                                }
                                else if( ifa->ifa_addr->sa_family = AF_INET6 )
                                {
                                	sin6 = (struct sockaddr_in6 *)(ifa->ifa_addr);
                                        if(inet_ntop(ifa->ifa_addr->sa_family,(void *)&(sin6->sin6_addr), str, sizeof(str)) == NULL )
                                        {
                                        	printf("IPV6 Interface = %s: inet_ntop failed\n", ifa->ifa_name );
                                        }
                                        else
                                        {
                				inet_pton( AF_INET6, argv[1], &inaddr );
                                        	printf("IPV6 Interface =%s      %s\n",ifa->ifa_name, str );
                                        }
					inet_pton(AF_INET6, str, &sin6->sin6_addr );
                                }
                       	}
			
			aregreq.ipv6addr = sin6->sin6_addr;
			entry.ipv6addr = sin6->sin6_addr;
		}
		else {
                	ret = inet_pton( AF_INET, argv[1], &inaddr );
			if( ret != 1 )
			{
                		ret = inet_pton( AF_INET6, argv[1], &inaddr );
				if( ret != 1 )
					perror("inet_pton() failed");
			}

           		aregreq.ipv6addr = inaddr;
           		entry.ipv6addr = inaddr;
               }
#endif
	}
#endif
	
	getUSTIpv6Addr(&entry.ipv6addr,ifname);
	
	aregreq.ipv6addr = entry.ipv6addr;

	/*Register a call back function with LIBWAVE to receive WME Notifications and WSMIndications from TARGET*/
//	registerWMENotifIndication( receiveWME_NotifIndication );
	registerWSMIndication(receiveWSMIndication);
//	registerLinkConfirm(confirmBeforeJoin);

	/*Set the notification and service PORTS*/
	aregreq.notif_port = 9999;
	entry.serviceport = 8888;
	
	/*Tell LIBWAVE where to listen for notifications*/ 	
	setWMEApplRegNotifParams(&aregreq);
	
	/*Start recieving packets*/
		
	printf("Registering User\n");
	
	if (registerUser(pid, &entry) < 0)
	{
		printf("Register User Failed \n");
		printf("Removing user if already present  %d\n", !removeUser(pid, &entry));
		printf("USER Registered %d with PSID =%u \n", registerUser(pid, &entry), entry.psid);
	}
	signal(SIGINT,(void *)sig_int);
	signal(SIGTERM,(void *)sig_term);
	while(1);
	/*On exit its better to call removeUser(pid, &entry)*/  
#ifdef WIN32
	system("PAUSE");
#endif
	return 0;
}

int buildUSTEntry() 
{
	entry.psid = 5;
	entry.userreqtype = USER_REQ_SCH_ACCESS_AUTO;
	return 0;
}

void receiveWME_NotifIndication(WMENotificationIndication *wmeindication)
{
	//printf("WME Notification-Indication Received");
}

/*LIBWAVE calls this function when it receives a WSMP packet (aka WSMIndication) from TARGET*/
void receiveWSMIndication(WSMIndication *wsmindication) 
{
	printf("WSMP Packet received, len %u, packet number=%llu\n", wsmindication->data.length , ++count);
	/*Process your packet here*/
}
int confirmBeforeJoin(WMEApplicationIndication *appind)
{
	printf("\nJoin\n");
	return 1; /*Return 0 to stop Joining the WBSS*/
}

void sig_int(void)
{
//	int ret;

	removeUser(pid, &entry);
	closeWAVEDevice();
	signal(SIGINT,SIG_DFL);
	printf("\n\nPackets received = %llu\n", count); 
	printf("remoterx killed by kill signal\n");
	exit(0);

}

void sig_term(void)
{
//	int ret;

	removeUser(pid, &entry);
	closeWAVEDevice();
	signal(SIGINT,SIG_DFL);
	printf("\n\nPackets received = %llu\n", count); 
	printf("remoterx killed by kill signal\n");
	exit(0);
}

	

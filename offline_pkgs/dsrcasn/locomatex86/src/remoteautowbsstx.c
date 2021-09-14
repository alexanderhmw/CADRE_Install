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

static WMEApplicationRequest wreq;
static WMEApplicationRequest entry;
//static WSMIndication wsmrxind;
static WSMRequest wsmreq;
//static WMECancelTxRequest cancelReq;
static int devicemode = WAVEDEVICE_REMOTE;
static WMEApplicationRequest aregreq;
static int pid; 
static WMETARequest tareq;
 
void 	receiveWME_NotifIndication(WMENotificationIndication *wmeindication);
void 	receiveWSMIndication(WSMIndication *wsmindication);

int buildWMETARequest();
int buildPSTEntry();
int buildWSMRequestPacket();
int buildWMETARequest();
int buildWMEApplicationRequest();
int txWSMPPkts(int);
static uint64_t packets = 0;
static uint64_t drops = 0;
static char Data[30]="LOCOMATE-ARADA SYSTEMS";
static uint16_t len=500;
void sig_int(void);
void sig_term(void);
/*This program demonstrates how to start a WBSS on a TARGET device*/

struct ta_argument {
	uint8_t channel;
	uint8_t channelinterval;
} taarg;

int main (int argc, char *argv[]) {

	int ret;
	int blockflag = 0 ;
	char ifname[30]="eth0";
#ifdef	WIN32
		WIN_SOCK_DLL_INVOKE

//    This is required to invoke DLLs for Sockets in WIN32		//

#endif

	pid = getpid();

	if(argc < 5) {
		printf("usage: remotetx <TARGETIP> [sch-channel access <1-alternating> <0-continuous> ] [TA Channel ] [TA Channel Interval <1-cch> <2-sch> ] [interface name]\n");
		printf("--> Mention interface name is  \"lo\" for running remotetx in Locomate Board and Default interface is eth0\n");
		printf("--> Mention interface name  for running remotetx in Windows Machine. Ex:Local Area Connection.\n");
		exit(-1);
	}

	taarg.channel = atoi(argv[3]);
	taarg.channelinterval = atoi(argv[4]);
	if(argc == 6)
	strcpy(ifname,argv[5]);	
	/*Initialize the data structures*/
	
	printf("Filling Provider Service Table entry %d\n", buildPSTEntry() );
	printf("Building a WSM Request Packet %d\n", buildWSMRequestPacket() );
	printf("Building a WME Application  Request %d\n", buildWMEApplicationRequest() );
	printf("Building TA Request %d\n", buildWMETARequest() );

	/*Provide the IP address of the TARGET WAVE-device*/
	setRemoteDeviceIP(argv[1]);
	devicemode = WAVEDEVICE_REMOTE;
	ret = invokeWAVEDevice(devicemode, blockflag); /*blockflag is ignored in this case*/
	if (ret < 0 ) {
		/*Error*/
	} else {
		printf("Driver invoked\n");
	}
#if 0	
	/*Get the IP address of eth0*/
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
                                }
                        }
                        aregreq.ipv6addr = sin6->sin6_addr;
                        entry.ipv6addr = sin6->sin6_addr;
		}
		
		else {
				ret = inet_pton(AF_INET, argv[1], &inaddr );
				if( ret != 1 )
				{
					ret = inet_pton(AF_INET6, argv[1], &inaddr );
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
	registerWMENotifIndication(receiveWME_NotifIndication);
	registerWSMIndication(receiveWSMIndication);
	
	/*Set the notification IP and PORT*/
	aregreq.notif_port = 6666;
	
	/*Tell LIBWAVE where to listen for notifications*/ 
	setWMEApplRegNotifParams(&aregreq);


	
	printf("Registering provider\n ");

	/*NOTE:If the TARGET device is not up or the link is down the libwave calls will wait indefinetly*/
	removeProvider(pid, &entry );
	
	/*Register a  Provider on the TARGET, Note: Most of the libawave functions are identical whether the device is local or remote*/
	/*the only difference being that remote calls may hang (when no reply comes from TARGET) and the way WSMIndications are received*/
	if (registerProvider(pid, &entry ) < 0 ) {
		printf("Register Provider failed\n");
		exit(-1);
	} else {
		printf("Provider registered with PSID = %u \n", entry.psid);
	}
        if (transmitTA(&tareq) < 0)  {
            printf("send TA failed\n ");
        } else { 
            printf("send TA successful\n") ;
        }
	
	/*Transmit some packets*/
	ret = txWSMPPkts(pid);
	if (ret == 0 )
		printf("All Packets transmitted\n");
	else 
		printf("%d Packets dropped\n", ret);
#ifdef WIN32
	system("PAUSE");
#endif
	return 0;
}

int buildWMETARequest()
{
    tareq.action = TA_ADD;
    tareq.repeatrate = 100;
    tareq.channel = taarg.channel;
    tareq.channelinterval = taarg.channelinterval;
    tareq.servicepriority = 1;
	return 0;
}


/*Fill up the data structure  to register a PROVIDER application*/	
int buildPSTEntry(){
	entry.psid = 5;
	entry.priority = 1;
	entry.channel = 172;
	/*This is the Port where WSMIndications will be received*/
	entry.serviceport = 8888;
	entry.repeatrate = 10;
	entry.repeatrate = 50; // repeatrate =50 per 5seconds = 1Hz
    entry.channelaccess = CHACCESS_ALTERNATIVE;
	return 0;
}

/*Build a request to transmit a WSM packet*/	
int buildWSMRequestPacket() {
	wsmreq.chaninfo.channel = 172 ;
	wsmreq.chaninfo.rate = 3 ;
	wsmreq.chaninfo.txpower = 15 ;
	wsmreq.version = 1 ;
	wsmreq.security = 1 ;
	wsmreq.psid = 5 ;
	wsmreq.txpriority = 1;
	memset ( &wsmreq.data, 0, sizeof( WSMData ));
	memcpy ( &wsmreq.data.contents, &Data, sizeof( Data ));
	memcpy ( &wsmreq.data.length, &len, sizeof( len ));
	return 0;
}

/*Build a request to start a WBSS*/
int  buildWMEApplicationRequest() {	
	wreq.psid = 5 ;
	wreq.repeats = 1;
	wreq.persistence = 1;
	/*WRSS Request channel should be same as that of PROVIDER*/
	wreq.channel = 172;
	return 1;
}

/*Transmit the packets here*/
int txWSMPPkts(int pid) {
		//int pwrvalues;
	//	int ratecount;
	//	int txprio;
		int ret = 0,count = 0;// pktcount, count = 0;
//		unsigned char mac[6] =	{ 0x00, 0x03, 0x7f, 0x07, 0x81, 0x8b};
		printf("Transmiting...\n");

	/* catch control-c and kill signal*/
	signal(SIGINT,(void *)sig_int);
	signal(SIGTERM,(void *)sig_term);

	while (1) {
		#ifndef WIN32  
		usleep(2000);	
		#else        
		sleep(0.002);
		#endif
			ret = txWSMPacket(pid, &wsmreq);
                if( ret < 0) {
                        drops++;
                }
                else {
                        packets++;
                        count++;
                }
                printf("Transmitted #%llu#                                      Dropped #%llu# len %u\n", packets, drops,wsmreq.data.length);
	}
	return drops;
}

void receiveWME_NotifIndication(WMENotificationIndication *wmeindication)
{
}


void receiveWSMIndication(WSMIndication *wsmindication) 
{

}

void sig_int(void)
{
	int ret;

	ret = stopWBSS(pid, &wreq);
	removeProvider(pid, &entry);
	closeWAVEDevice();
	printf("\n\nPACKTES SENT = %llu\n",packets); 
	printf("PACKTES DROPPED = %llu\n",drops); 
	printf("remotetx killed by control-C\n");
	signal(SIGINT,SIG_DFL);
	exit(0);

}

void sig_term(void)
{
	int ret;

	ret = stopWBSS(pid, &wreq);
	removeProvider(pid, &entry);
	closeWAVEDevice();
	printf("\n\nPACKTES SENT = %llu\n",packets); 
	printf("PACKTES DROPPED = %llu\n",drops); 
	printf("remotetx killed by control-C\n");
	signal(SIGINT,SIG_DFL);
	exit(0);
}

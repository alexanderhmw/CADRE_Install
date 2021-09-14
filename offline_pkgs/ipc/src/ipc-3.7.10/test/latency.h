/******************************************************************************
 * PROJECT: New Millennium, DS1
 *          IPC (Interprocess Communication) Package
 *
 * (c) Copyright 1996 Reid Simmons.  All rights reserved.
 *
 * FILE: latency.h
 *
 * ABSTRACT: Test the latency of IPC for various sized messages.
 *
 * $Revision: 2.2 $
 * $Date: 2000/01/27 20:47:11 $
 * $Author: reids $
 *
 * REVISION HISTORY
 *
 * $Log: latency.h,v $
 * Revision 2.2  2000/01/27 20:47:11  reids
 * Removed compiler warnings for RedHat Linux
 *
 * Revision 2.1.1.1  1999/11/23 19:07:37  reids
 * Putting IPC Version 2.9.0 under local (CMU) CVS control.
 *
 * Revision 1.1.2.1  1996/10/22 17:33:33  reids
 * Small mods to make it give more reasonable results.
 *
 * Revision 1.1  1996/06/17 18:43:32  rouquett
 * ipc test files
 *
 * Revision 1.1  1996/05/09 01:07:50  reids
 * Added latency and throughput tests.
 * Modified to work on vxworks (m68k).
 *
 *
 *****************************************************************************/
#ifndef LATENCY_H
#define LATENCY_H

#if !defined(__sgi) && !defined(WIN32) && !defined(REDHAT_52)
#define UNUSED_PRAGMA
#endif

#define NSEND (1000)

#define MSG_NAME    "LatencyMsg"
#define MSG_FORMAT  "{long, long}"

#endif /* LATENCY_H */

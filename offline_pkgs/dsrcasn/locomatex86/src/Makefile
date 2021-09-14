# Copyright (c) 2005-2007 Arada Syatems, Inc. All rights reserved.

# Proprietary and Confidential Material.

INCS+= -I ../incs

INCS+= -I .

BINDIR= ../bin

LIBDIR= ../lib

CC=gcc


ALL= remotetx remoterx 

all:	${ALL}
	mv -f ${ALL} ../bin

%.o: %.c
	$(CC) $(INCS) -c -DLOCOMATE -o $@ $<

LDFLAGS= -lm -lpthread -lwave -L $(LIBDIR) -Xlinker -rpath -Xlinker $(LIBDIR) -lwave-encdec -Lasn1c/


#wsmpdemo: wsmpdemo.o  wavelogger.o wavegps.o gpsc_probe.o asnlogger.o
#	${CC} -g -o wsmpdemo  wsmpdemo.o  wavelogger.o wavegps.o gpsc_probe.o asnlogger.o $(LDFLAGS) 

remotetx: remoteautowbsstx.o
	${CC} -g -o remotetx remoteautowbsstx.o ${LDFLAGS} 

remoterx: remoteautowbssrx.o
	${CC} -g -o remoterx remoteautowbssrx.o ${LDFLAGS} 

clean:
	rm -f *.o
	rm -f ${BINDIR}/remotetx ${BINDIR}/remoterx	

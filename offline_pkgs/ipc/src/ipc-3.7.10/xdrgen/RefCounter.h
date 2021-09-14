/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.1 $  $Author: reids $  $Date: 2001/03/16 17:56:01 $
 *
 * PROJECT:      Distributed Robotic Agents
 * DESCRIPTION:  
 *
 * (c) Copyright 2001 CMU. All rights reserved.
 ***************************************************************************/

#ifndef INCRefCounter_h
#define INCRefCounter_h

class RefCounter {
public:
  void ref(void) { refCount++; }
  void unref(void) {
    refCount--;
    if (refCount <= 0) delete this;
  }
  void checkRef(void) {
    if (refCount <= 0) delete this;
  }

protected:
  RefCounter(void) { refCount = 0; }
  // must override default copy constructor!
  RefCounter(RefCounter &r) { refCount = 0; }
  virtual ~RefCounter(void) { }
  int refCount;
};

#endif // INCRefCounter_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: RefCounter.h,v $
 * Revision 1.1  2001/03/16 17:56:01  reids
 * Release of Trey's code to generate IPC format strings from XDR definitions.
 *
 * Revision 1.1  2001/02/05 21:10:46  trey
 * initial check-in
 *
 *
 ***************************************************************************/

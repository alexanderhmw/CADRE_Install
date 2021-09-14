/********** tell emacs we use -*- c++ -*- style comments *******************
 * $Revision: 1.2 $  $Author: reids $  $Date: 2001/11/14 17:58:54 $
 *
 * PROJECT:      Distributed Robotic Agents
 * DESCRIPTION:  
 *
 * (c) Copyright 1999 CMU. All rights reserved.
 ***************************************************************************/

#ifndef INCXDRParser_h
#define INCXDRParser_h

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include "XDRTree.h"

/***************************************************************************
 * MACROS
 ***************************************************************************/

/***************************************************************************
 * CLASSES AND TYPEDEFS
 ***************************************************************************/
class XDRParser {
public:
  XDRParser(void);

  void setVerbosity(bool _verbose) { verbose = _verbose; }

  bool parseFile(const char *fileName, list<string> &cppArgs);
  XDRSpecification *getSyntaxTree(void) { return syntaxTree; }

protected:
  bool verbose;
  XDRSpecification *syntaxTree;
};

#endif // INCXDRParser_h

/***************************************************************************
 * REVISION HISTORY:
 * $Log: XDRParser.h,v $
 * Revision 1.2  2001/11/14 17:58:54  reids
 * Incorporating various changes that Trey made in the DIRA/FIRE versions.
 *
 * Revision 1.1  2001/03/16 17:56:05  reids
 * Release of Trey's code to generate IPC format strings from XDR definitions.
 *
 * Revision 1.2  2001/03/21 19:26:15  trey
 * re-enabled using runCPP call to pre-process input file before parsing
 *
 * Revision 1.1  2001/02/05 21:10:50  trey
 * initial check-in
 *
 *
 ***************************************************************************/

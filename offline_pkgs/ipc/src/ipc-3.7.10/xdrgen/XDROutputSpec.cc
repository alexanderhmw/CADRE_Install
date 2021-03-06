/****************************************************************************
 * $Revision: 1.2 $  $Author: trey $  $Date: 2004/04/06 15:06:08 $
 *
 * PROJECT:      Distributed Robotic Agents
 * DESCRIPTION:  
 *
 * (c) Copyright 2001 CMU. All rights reserved.
 ***************************************************************************/

/***************************************************************************
 * INCLUDES
 ***************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include "XDROutputSpec.h"

/***************************************************************************
 * GLOBAL VARIABLES AND STATIC CLASS MEMBERS
 ***************************************************************************/

/***************************************************************************
 * FUNCTIONS
 ***************************************************************************/

void
XDROutputSpec::setIndentLevel(int newIndentLevel) {
  indentLevel = newIndentLevel;
  indent = string(indentLevel,' ');
}

void
XDROutputSpec::beginSpec(XDRSpecification *spec) {
  out << "/* generated by XDROutputSpec */" << endl << endl;
  setIndentLevel(0);
}

void
XDROutputSpec::endSpec(XDRSpecification *spec) {
  /* do nothing */
}

void
XDROutputSpec::beginTypeDef(XDRDeclNode *decl, int index,
			    int numTypeDefs) {
  if (T_STRUCT != decl->typeSpec->type) {
    out << indent << "typedef ";
  }
}

void
XDROutputSpec::endTypeDef(XDRDeclNode *decl, int index, int numTypeDefs) {
  out << ";" << endl;
  if (index < numTypeDefs-1) out << endl;
}

void
XDROutputSpec::beginStructField(XDRDeclNode *decl, int index, int numDecls) {
  /* do nothing */
}

void
XDROutputSpec::endStructField(XDRDeclNode *decl, int index, int numDecls) {
  out << ";" << endl;
}

void
XDROutputSpec::beginTypeSpec(XDRTypeSpecNode *typeSpec) {
  out << indent << typeSpec->nameOfType();
}

void
XDROutputSpec::endTypeSpec(XDRTypeSpecNode *typeSpec) {
  /* do nothing */
}

void
XDROutputSpec::beginStruct(XDRTypeSpecStructNode *structNode) {
  out << " ";
  if (structFieldContextStack.empty()
      && T_STRUCT == getTypeDefContext()->typeSpec->type) {
    out << getDeclContext()->fieldName->val << " ";
  }
  out << "{" << endl;
  setIndentLevel(indentLevel+2);
}

void
XDROutputSpec::endStruct(XDRTypeSpecStructNode *structNode) {
  setIndentLevel(indentLevel-2);
  out << indent << "}";
}

void
XDROutputSpec::beforeArrayDims(XDRTypeSpecNode *typeSpec) {
  if (!(structFieldContextStack.empty()
	&& T_STRUCT == getTypeDefContext()->typeSpec->type)) {
    out << " " << getDeclContext()->fieldName->val;
  }
}

void
XDROutputSpec::processArrayDim(XDRArrayDimNode *arrayDim, int index,
			       int numArrayDims) {
  XDRArrayEnum arrayEnum = (index == numArrayDims)
    ? A_VAR_ARRAY : getTypeSpecContext()->arrayEnum;
  out << ((A_FIXED_ARRAY == arrayEnum) ? "[" : "<");
  if (0 != arrayDim->maxLength) {
    out << arrayDim->maxLength->val;
  }
  out << ((A_FIXED_ARRAY == arrayEnum) ? "]" : ">");
}


/***************************************************************************
 * REVISION HISTORY:
 * $Log: XDROutputSpec.cc,v $
 * Revision 1.2  2004/04/06 15:06:08  trey
 * updated for more recent bison and g++
 *
 * Revision 1.1  2001/03/16 17:56:04  reids
 * Release of Trey's code to generate IPC format strings from XDR definitions.
 *
 * Revision 1.1  2001/02/05 21:10:49  trey
 * initial check-in
 *
 *
 ***************************************************************************/

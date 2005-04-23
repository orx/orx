/**
 * @file orxString.h
 * 
 * String Module - Offers functions to manage Strings and CRC
 * 
 * @todo Add CRC generation
 * @todo Maybe add functionalities to have an easyer string management than standard C API
 */
 
 /***************************************************************************
 orxString.h
 String management Module
 
 begin                : 21/04/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxSTRING_H_
#define _orxSTRING_H_

#include "orxInclude.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "debug/orxDebug.h"

/** Initialize the string module
 */
extern orxDLLAPI orxSTATUS orxString_Init();

/** Uninitialize the string module
 */
extern orxDLLAPI orxVOID orxString_Exit();

/** Compare two strings. If the first one is smaller than the second, it returns -1,
 * If the second one is bigger than the first, and 0 if they are equals
 * @param _zString1   (IN) First String to compare
 * @param _zString2   (IN) Second string to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
orxSTATIC orxINLINE orxU32 orxString_Compare(orxSTRING _zString1, orxSTRING _zString2)
{
  return strcmp(_zString1, _zString2);
}

/** Compare N first character from two strings. If the first one is smaller 
 * than the second, it returns -1, If the second one is bigger than the first,
 * and 0 if they are equals.
 * @param _zString1   (IN) First String to compare
 * @param _zString2   (IN) Second string to compare
 * @param _u32NbChar  (IN) Number of character to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
orxSTATIC orxINLINE orxU32 orxString_NCompare(orxSTRING _zString1, orxSTRING _zString2, orxU32 _u32NbChar)
{
  return strncmp(_zString1, _zString2, _u32NbChar);
}

/** Print a message on STDIN
 * @param _zMessage (IN) Message to print (with optional parameters. Same syntax as printf
 */
orxSTATIC orxINLINE orxVOID orxString_Print(orxSTRING _zMessage, ...)
{
  /* Declare argument lists */
  va_list args;
  
  /* Parse arguments, print and end it */
  va_start(args, _zMessage);
  vprintf(_zMessage, args);
  va_end(args);
}

/** Print a message on STDIN and returns to line
 * @param _zMessage (IN) Message to print (with optional parameters. Same syntax as printf
 */
orxSTATIC orxINLINE orxVOID orxString_PrintLn(orxSTRING _zMessage, ...)
{
  /* Declare argument lists */
  va_list args;
  
  /* Print message */
  va_start(args, _zMessage);
  vprintf(_zMessage, args);
  va_end(args);

  /* return to line */
  printf("\n");
}

/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
extern orxDLLAPI orxSTATUS orxString_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxSTRING _zMessage);

/** Convert a String to a value
 * @param _ps32OutValue   (OUT) Converted value
 * @param _zString        (IN)  String To convert
 * @return  return the status of the conversion
 */
orxSTATIC orxINLINE orxSTATUS orxString_ToS32(orxS32 *_ps32OutValue, orxSTRING _zString)
{
  orxCHAR **ppcEndPtr; /* Address of the first invalid character */
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  
  /* Convert */
//  *_ps32OutValue = strtol(_zString, ppcEndPtr, 10);
  
  /* Valid conversion ? */
  if (ppcEndPtr != orxNULL)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILED;
  }
}
#endif /* _orxSTRING_H_ */

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
#include <stdlib.h>
#include <ctype.h>

#include "debug/orxDebug.h"

/** Initialize the string module
 */
extern orxDLLAPI orxSTATUS              orxString_Init();

/** Uninitialize the string module
 */
extern orxDLLAPI orxVOID                orxString_Exit();


/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxString_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxSTRING _zMessage);

/** Continues a CRC with a string one
 * @param _zString        (IN)  String used to continue the given CRC
 * @param _u32CRC         (IN)  Base CRC.
 * @return The resulting CRC.
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxString_ContinueCRC(orxSTRING _zString, orxU32 _u32CRC);


/* *** String inlined functions *** */


/** Copies a string.
 * @param _zDstString     (IN) Destination string
 * @param _zSrcString     (IN) Source string
 * @return Copied string.
 */
orxSTATIC orxINLINE orxSTRING           orxString_Copy(orxSTRING _zDstString, orxSTRING _zSrcString)
{
  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Done! */
  return(strcpy(_zDstString, _zSrcString));
}

/** Compare two strings. If the first one is smaller than the second, it returns -1,
 * If the second one is bigger than the first, and 0 if they are equals
 * @param _zString1   (IN) First String to compare
 * @param _zString2   (IN) Second string to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
orxSTATIC orxINLINE orxS32              orxString_Compare(orxSTRING _zString1, orxSTRING _zString2)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */  
  return(strcmp(_zString1, _zString2));
}

/** Compare N first character from two strings. If the first one is smaller 
 * than the second, it returns -1, If the second one is bigger than the first,
 * and 0 if they are equals.
 * @param _zString1   (IN) First String to compare
 * @param _zString2   (IN) Second string to compare
 * @param _u32NbChar  (IN) Number of character to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
orxSTATIC orxINLINE orxS32              orxString_NCompare(orxSTRING _zString1, orxSTRING _zString2, orxU32 _u32NbChar)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return strncmp(_zString1, _zString2, _u32NbChar);
}

/** Prints a formated string
 * @param _zDstString    (IN) Destination string
 * @param _zSrcString    (IN) Source formated string
 * @retrun The number of written characters
 */
orxSTATIC orxINLINE orxS32              orxString_Printf(orxSTRING _zDstString, orxSTRING _zSrcString, ...)
{
  va_list stArgs;
  orxS32 s32Result;

  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Parse arguments, print and end it */
  va_start(stArgs, _zSrcString);
  s32Result = vsprintf(_zDstString, _zSrcString, stArgs);
  va_end(stArgs);

  return s32Result;
}

/** Print a message on STDIN
 * @param _zMessage (IN) Message to print (with optional parameters. Same syntax as printf
 */
orxSTATIC orxINLINE orxVOID             orxString_Print(orxSTRING _zMessage, ...)
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
orxSTATIC orxINLINE orxVOID             orxString_PrintLn(orxSTRING _zMessage, ...)
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

/** Returns the number of character in the string
 * @param _zString (IN) String used for length computation
 * @return Length of the string (doesn't count final '\0')
 */
orxSTATIC orxINLINE orxU32              orxString_Length(orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Done! */
  return(strlen(_zString));
}

/** Convert a String to a value
 * @param _ps32OutValue   (OUT) Converted value
 * @param _zString        (IN)  String To convert
 * @param _u32Base        (IN)  BAse of the read value (generally 10, but can be 16 to read hexa)
 * @return  return the status of the conversion
 */
orxSTATIC orxINLINE orxSTATUS           orxString_ToS32(orxS32 *_ps32OutValue, orxSTRING _zString, orxU32 _u32Base)
{
  /* char instead of orxCHAR to avoid compilation warnings */
  orxCHAR *pcEndPtr; /* Address of the first invalid character */
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);
  
  /* Convert */
  *_ps32OutValue = strtol(_zString, &pcEndPtr, _u32Base);
  
  /* Valid conversion ? */
  if ((orxString_Length(_zString) > 0) && (isdigit((pcEndPtr - 1)[0])))
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILED;
  }
}

/** Lowercase a string
 * @param _zString        (IN)  String To convert
 * @return The converted string.
 */
orxSTATIC orxINLINE orxSTRING           orxString_LowerCase(orxSTRING _zString)
{
  orxCHAR *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Converts the whole string */
  for(pc = _zString; *pc != '\0'; pc++)
  {
    /* Needs to be converted? */
    if(*pc >= 'A' && *pc <= 'Z')
    {
      /* Lower case */
      *pc |= 0x20;
    }
  }

  return _zString;
}

/** Uppercase a string
 * @param _zString        (IN)  String To convert
 * @return The converted string.
 */
orxSTATIC orxINLINE orxSTRING           orxString_UpperCase(orxSTRING _zString)
{
  orxCHAR *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Converts the whole string */
  for(pc = _zString; *pc != '\0'; pc++)
  {
    /* Needs to be converted? */
    if(*pc >= 'a' && *pc <= 'z')
    {
      /* Upper case */
      *pc &= ~0x20;
    }
  }

  return _zString;
}

/** Converts a string to a CRC
 * @param _zString        (IN)  String To convert
 * @return The resulting CRC.
 */
orxSTATIC orxINLINE orxU32              orxString_ToCRC(orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);
  
  /* Computes the ID */
  return(orxString_ContinueCRC(_zString, 0));
}

#endif /* _orxSTRING_H_ */

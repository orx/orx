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


/** Continues a CRC with a string one
 * @param[in] _zString        String used to continue the given CRC
 * @param[in] _u32CRC         Base CRC.
 * @return The resulting CRC.
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxString_ContinueCRC(orxCONST orxSTRING _zString, orxU32 _u32CRC);

/** Continues a CRC with a string one
 * @param[in] _zString        String used to continue the given CRC
 * @param[in] _u32CRC         Base CRC.
 * @param[in] _u32CharNumber  Number of character to process
 * @return The resulting CRC.
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxString_NContinueCRC(orxCONST orxSTRING _zString, orxU32 _u32CRC, orxU32 _u32CharNumber);


/* *** String inlined functions *** */


/** Copies N characters from a string
 * @param[in] _zDstString       Destination string
 * @param[in] _zSrcString       Source string
 * @param[in] _u32CharNumber    Number of characters to copy
 * @return Copied string
 */
orxSTATIC orxINLINE orxSTRING orxString_NCopy(orxSTRING _zDstString, orxCONST orxSTRING _zSrcString, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Done! */
  return(strncpy(_zDstString, _zSrcString, _u32CharNumber));
}

/** Copies a string.
 * @param _zDstString     (IN) Destination string
 * @param _zSrcString     (IN) Source string
 * @return Copied string.
 */
orxSTATIC orxINLINE orxSTRING orxString_Copy(orxSTRING _zDstString, orxCONST orxSTRING _zSrcString)
{
  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Done! */
  return(strcpy(_zDstString, _zSrcString));
}

/** Duplicate a string.
 * @param _zSrcString	(IN) String to duplicate.
 * @return Duplicated string.
 */
orxSTATIC orxINLINE orxSTRING orxString_Duplicate(orxCONST orxSTRING _zSrcString)
{
	  /* Checks */
	  orxASSERT(_zSrcString != orxNULL);

	  /* Done! */
	  return(strdup(_zSrcString));
}

/** Compare two strings. If the first one is smaller than the second, it returns -1,
 * If the second one is bigger than the first, and 0 if they are equals
 * @param _zString1   (IN) First String to compare
 * @param _zString2   (IN) Second string to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
orxSTATIC orxINLINE orxS32 orxString_Compare(orxCONST orxSTRING _zString1, orxCONST orxSTRING _zString2)
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
orxSTATIC orxINLINE orxS32 orxString_NCompare(orxCONST orxSTRING _zString1, orxCONST orxSTRING _zString2, orxU32 _u32NbChar)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return strncmp(_zString1, _zString2, _u32NbChar);
}

/** Returns the number of character in the string
 * @param _zString (IN) String used for length computation
 * @return Length of the string (doesn't count final orxCHAR_NULL)
 */
orxSTATIC orxINLINE orxU32 orxString_GetLength(orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Done! */
  return(strlen(_zString));
}

/** Convert a String to a value
 * @param _ps32OutValue   (OUT) Converted value
 * @param _zString        (IN)  String To convert
 * @param _u32Base        (IN)  Base of the read value (generally 10, but can be 16 to read hexa)
 * @return  return the status of the conversion
 */
orxSTATIC orxINLINE orxSTATUS orxString_ToS32(orxS32 *_ps32OutValue, orxCONST orxSTRING _zString, orxU32 _u32Base)
{
  orxCHAR *pcEndPtr; /* Address of the first invalid character */
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);
  
  /* Convert */
  *_ps32OutValue = strtol(_zString, &pcEndPtr, _u32Base);
  
  /* Valid conversion ? */
  if((orxString_GetLength(_zString) > 0) && ((_zString[0] != orxCHAR_NULL && pcEndPtr[0] == orxCHAR_NULL)))
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILURE;
  }
}

/** Convert a string to a value
 * @param _pfOutValue     (OUT) Converted value
 * @param _zString        (IN)  String To convert
 * @return  return the status of the conversion
 */
orxSTATIC orxINLINE orxSTATUS orxString_ToFloat(orxFLOAT *_pfOutValue, orxCONST orxSTRING _zString)
{
  /* Correct parameters ? */
  orxASSERT(_pfOutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  /* Note : Here we should use strtot which detects errors.
   * This function is C99 compliant but it doesn't seems to be implemented in
   * the standard GNU lib C. We will use atof instead (which doesn't detect errors :( )
   */
  *_pfOutValue = (orxFLOAT)atof(_zString);

  return orxSTATUS_SUCCESS;
}

/** Lowercase a string
 * @param _zString        (IN)  String To convert
 * @return The converted string.
 */
orxSTATIC orxINLINE orxSTRING orxString_LowerCase(orxSTRING _zString)
{
  orxCHAR *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Converts the whole string */
  for(pc = _zString; *pc != orxCHAR_NULL; pc++)
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
orxSTATIC orxINLINE orxSTRING orxString_UpperCase(orxSTRING _zString)
{
  orxCHAR *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Converts the whole string */
  for(pc = _zString; *pc != orxCHAR_NULL; pc++)
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
orxSTATIC orxINLINE orxU32 orxString_ToCRC(orxCONST orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Computes the ID */
  return(orxString_ContinueCRC(_zString, 0));
}

/** Converts a string to a CRC
 * @param[in] _zString        String To convert
 * @param[in] _u32CharNumber  Number of characters to process
 * @return The resulting CRC.
 */
orxSTATIC orxINLINE orxU32 orxString_NToCRC(orxCONST orxSTRING _zString, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Computes the ID */
  return(orxString_NContinueCRC(_zString, 0, _u32CharNumber));
}

/** Returns the first occurence of _zString2 in _zString1
 * @param[in] _zString1 String to analyze
 * @param[in] _zString2 String that must be inside _zString1
 * @return The pointer of the first occurence of _zString2, or orxNULL if not found
 */
orxSTATIC orxINLINE orxSTRING orxString_SearchString(orxCONST orxSTRING _zString1, orxCONST orxSTRING _zString2)
{
  /* Correct parameters ? */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);
  
  /* Returns result */
  return(strstr(_zString1, _zString2));
}

/** Returns the first occurence of _cChar in _zString
 * @param[in] _zString String to analyze
 * @param[in] _cChar   The character to find
 * @return The pointer of the first occurence of _cChar, or orxNULL if not found
 */
orxSTATIC orxINLINE orxSTRING orxString_SearchChar(orxCONST orxSTRING _zString, orxCHAR _cChar)
{
  /* Correct parameters ? */
  orxASSERT(_zString != orxNULL);
  
  /* Returns result */
  return(strchr(_zString, _cChar));
}

/** Returns the first occurence of _cChar in _zString
 * @param[in] _zString      String to analyze
 * @param[in] _cChar        The character to find
 * @param[in] _u32Position  Search begin position
 * @return The index of the next occurence of requested character, starting at given position / -1 if not found
 */
orxSTATIC orxINLINE orxS32 orxString_SearchCharIndex(orxCONST orxSTRING _zString, orxCHAR _cChar, orxU32 _u32Position)
{
  orxREGISTER orxS32    s32Result = -1;
  orxREGISTER orxS32    s32Index;
  orxREGISTER orxCHAR  *pc;

  /* Correct parameters ? */
  orxASSERT(_zString != orxNULL);
  orxASSERT(_u32Position < orxString_GetLength(_zString));

  /* For all characters */
  for(s32Index = _u32Position, pc = _zString + s32Index; *pc != orxCHAR_NULL; pc++, s32Index++)
  {
    /* Found? */
    if(*pc == _cChar)
    {
      /* Updates result */
      s32Result = s32Index;
    }
  }

  /* Done! */
  return s32Result;
}

/** Prints a formated string to a memory buffer
 * @param[out] _zDstString  Destination string
 * @param[int] _zSrcString  Source formated string
 * @return The number of written characters
 */
orxSTATIC orxINLINE orxS32 orxString_Print(orxSTRING _zDstString, orxSTRING _zSrcString, ...)
{
  va_list stArgs;
  orxS32  s32Result;

  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Gets variable arguments & print the string */
  va_start(stArgs, _zSrcString);
  s32Result = vsprintf(_zDstString, _zSrcString, stArgs);
  va_end(stArgs);

  /* Done! */
  return s32Result;
}

#endif /* _orxSTRING_H_ */

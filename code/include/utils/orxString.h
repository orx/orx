/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxString.h
 * @date 21/04/2005
 * @author bestel@arcallians.org
 *
 * @todo
 * - Add autoindexing for ID generation
 */

/**
 * @addtogroup orxString
 *
 * String module
 * Module that handles strings
 *
 * @{
 */


#ifndef _orxSTRING_H_
#define _orxSTRING_H_


#include "orxInclude.h"
#include "memory/orxMemory.h"
#include "math/orxVector.h"

#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

  #define strtoll   _strtoi64
  #define strtoull  _strtoui64

#endif /* __orxMSVC__ */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include "debug/orxDebug.h"


#define orxSTRING_KC_VECTOR_START       '('
#define orxSTRING_KC_VECTOR_START_ALT   '{'
#define orxSTRING_KC_VECTOR_SEPARATOR   ','
#define orxSTRING_KC_VECTOR_END         ')'
#define orxSTRING_KC_VECTOR_END_ALT     '}'


extern orxDLLAPI const orxU32 sau32CRCTable[256];


/* *** String inlined functions *** */


/** Continues a CRC with a string one
 * @param[in] _zString        String used to continue the given CRC
 * @param[in] _u32CRC         Base CRC.
 * @return The resulting CRC.
 */
static orxINLINE orxU32                 orxString_ContinueCRC(const orxSTRING _zString, orxU32 _u32CRC)
{
  orxU32          u32CRC;
  const orxCHAR  *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Inits CRC */
  u32CRC = _u32CRC ^ 0xFFFFFFFFL;

  /* For the whole string */
  for(pc = _zString; *pc != orxCHAR_NULL; pc++)
  {
    /* Computes the CRC */
    u32CRC = sau32CRCTable[(u32CRC ^ *pc) & 0xFF] ^ (u32CRC >> 8);
  }

  /* Done! */
  return(u32CRC ^ 0xFFFFFFFFL);
}

/** Continues a CRC with a string one
 * @param[in] _zString        String used to continue the given CRC
 * @param[in] _u32CRC         Base CRC.
 * @param[in] _u32CharNumber  Number of character to process
 * @return The resulting CRC.
 */
static orxINLINE orxU32                 orxString_NContinueCRC(const orxSTRING _zString, orxU32 _u32CRC, orxU32 _u32CharNumber)
{
  orxU32          u32CRC;
  orxU32          u32Counter;
  const orxCHAR  *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Inits CRC */
  u32CRC = _u32CRC ^ 0xFFFFFFFFL;

  /* For the whole string */
  for(pc = _zString, u32Counter = 0; (*pc != orxCHAR_NULL) && (u32Counter < _u32CharNumber); pc++, u32Counter++)
  {
    /* Computes the CRC */
    u32CRC = sau32CRCTable[(u32CRC ^ *pc) & 0xFF] ^ (u32CRC >> 8);
  }

  /* Done! */
  return(u32CRC ^ 0xFFFFFFFFL);
}

/** Skips all white spaces
 * @param[in] _zString        Concerned string
 * @return    Sub string located after all leading white spaces
 */
static orxINLINE const orxSTRING        orxString_SkipWhiteSpaces(const orxSTRING _zString)
{
  const orxSTRING zResult;

  /* Non null? */
  if(_zString != orxNULL)
  {
    /* Skips all white spaces */
    for(zResult = _zString; (*zResult == ' ') || (*zResult == '\t'); zResult++);

    /* Empty? */
    if(*zResult == orxCHAR_NULL)
    {
      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }
  else
  {
    /* Updates result */
    zResult = orxNULL;
  }

  /* Done! */
  return zResult;
}

/** Skips path
 * @param[in] _zString        Concerned string
 * @return    Sub string located after all non-terminal directory separators
 */
static orxINLINE const orxSTRING        orxString_SkipPath(const orxSTRING _zString)
{
  const orxSTRING zResult;

  /* Non null? */
  if(_zString != orxNULL)
  {
    const orxCHAR *pc;

    /* Updates result */
    zResult = _zString;

    /* For all characters */
    for(pc = _zString; *pc != orxCHAR_NULL; pc++)
    {
      /* Is a directory separator? */
      if((*pc == orxCHAR_DIRECTORY_SEPARATOR_LINUX) || (*pc == orxCHAR_DIRECTORY_SEPARATOR_WINDOWS))
      {
        orxCHAR cNextChar = *(pc + 1);

        /* Non terminal and not a directory separator? */
        if((cNextChar != orxCHAR_NULL) && (cNextChar != orxCHAR_DIRECTORY_SEPARATOR_LINUX) && (cNextChar != orxCHAR_DIRECTORY_SEPARATOR_WINDOWS))
        {
          /* Updates result */
          zResult = pc + 1;
        }
      }
    }
  }
  else
  {
    /* Updates result */
    zResult = orxNULL;
  }

  /* Done! */
  return zResult;
}

/** Returns the number of orxCHAR in the string (for non-ASCII UTF-8 string, it won't be the actual number of unicode characters)
 * @param[in] _zString                  String used for length computation
 * @return                              Length of the string (doesn't count final orxCHAR_NULL)
 */
static orxINLINE orxU32                 orxString_GetLength(const orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Done! */
  return((orxU32)strlen(_zString));
}

/** Tells if a character is ASCII from its ID
 * @param[in] _u32CharacterCodePoint    Concerned character code
 * @return                              orxTRUE is it's a non-extended ASCII character, orxFALSE otherwise
 */
static orxINLINE orxBOOL                orxString_IsCharacterASCII(orxU32 _u32CharacterCodePoint)
{
  return((_u32CharacterCodePoint < 0x80) ? orxTRUE : orxFALSE);
}

/** Gets the UTF-8 encoding length of given character
 * @param[in] _u32CharacterCodePoint    Concerned character code
 * @return                              Encoding length in UTF-8 for given character if valid, orxU32_UNDEFINED otherwise
 */
static orxINLINE orxU32                 orxString_GetUTF8CharacterLength(orxU32 _u32CharacterCodePoint)
{
  orxU32 u32Result;

  /* 1-byte long? */
  if(_u32CharacterCodePoint < 0x80)
  {
    /* Updates result */
    u32Result = 1;
  }
  else if(_u32CharacterCodePoint < 0x0800)
  {
    /* Updates result */
    u32Result = 2;
  }
  else if(_u32CharacterCodePoint < 0x00010000)
  {
    /* Updates result */
    u32Result = 3;
  }
  else if(_u32CharacterCodePoint < 0x00110000)
  {
    /* Updates result */
    u32Result = 4;
  }
  else
  {
    /* Updates result */
    u32Result = orxU32_UNDEFINED;
  }

  /* Done! */
  return u32Result;
}

/** Prints a unicode character encoded with UTF-8 to an orxSTRING
 * @param[in] _zDstString               Destination string
 * @param[in] _u32Size                  Available size on the string
 * @param[in] _u32CharacterCodePoint    Unicode code point of the character to print
 * @return                              Length of the encoded UTF-8 character (1, 2, 3 or 4) if valid, orxU32_UNDEFINED otherwise
 */
static orxU32 orxFASTCALL               orxString_PrintUTF8Character(orxSTRING _zDstString, orxU32 _u32Size, orxU32 _u32CharacterCodePoint)
{
  orxU32 u32Result;

  /* Gets character's encoded length */
  u32Result = orxString_GetUTF8CharacterLength(_u32CharacterCodePoint);

  /* Enough room? */
  if(u32Result <= _u32Size)
  {
    /* Depending on character's length */
    switch(u32Result)
    {
      case 1:
      {
        /* Writes character */
        *_zDstString = (orxCHAR)_u32CharacterCodePoint;

        break;
      }

      case 2:
      {
        /* Writes first character */
        *_zDstString++ = (orxCHAR)(0xC0 | ((_u32CharacterCodePoint & 0x07C0) >> 6));

        /* Writes second character */
        *_zDstString = (orxCHAR)(0x80 | (_u32CharacterCodePoint & 0x3F));

        break;
      }

      case 3:
      {
        /* Writes first character */
        *_zDstString++ = (orxCHAR)(0xE0 | ((_u32CharacterCodePoint & 0xF000) >> 12));

        /* Writes second character */
        *_zDstString++ = (orxCHAR)(0x80 | ((_u32CharacterCodePoint & 0x0FC0) >> 6));

        /* Writes third character */
        *_zDstString = (orxCHAR)(0x80 | (_u32CharacterCodePoint & 0x3F));

        break;
      }

      case 4:
      {
        /* Writes first character */
        *_zDstString++ = (orxCHAR)(0xF0 | ((_u32CharacterCodePoint & 0x001C0000) >> 18));

        /* Writes second character */
        *_zDstString++ = (orxCHAR)(0x80 | ((_u32CharacterCodePoint & 0x0003F000) >> 12));

        /* Writes third character */
        *_zDstString++ = (orxCHAR)(0x80 | ((_u32CharacterCodePoint & 0x00000FC0) >> 6));

        /* Writes fourth character */
        *_zDstString = (orxCHAR)(0x80 | (_u32CharacterCodePoint & 0x3F));

        break;
      }

      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't print invalid unicode character <0x%X> to string.", _u32CharacterCodePoint);

        /* Updates result */
        u32Result = orxU32_UNDEFINED;

        break;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't print unicode character <0x%X> to string as there isn't enough space for it.", _u32CharacterCodePoint);

    /* Updates result */
    u32Result = orxU32_UNDEFINED;
  }

  /* Done! */
  return u32Result;
}

/** Returns the code of the first character of the UTF-8 string
 * @param[in] _zString                  Concerned string
 * @param[out] _pzRemaining             If non null, will contain the remaining string after the first UTF-8 character
 * @return                              Code of the first UTF-8 character of the string, orxU32_UNDEFINED if it's an invalid character
 */
static orxU32 orxFASTCALL               orxString_GetFirstCharacterCodePoint(const orxSTRING _zString, const orxSTRING *_pzRemaining)
{
  orxU8  *pu8Byte;
  orxU32  u32Result;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Gets the first byte */
  pu8Byte = (orxU8 *)_zString;

  /* ASCII? */
  if(*pu8Byte < 0x80)
  {
    /* Updates result */
    u32Result = *pu8Byte;
  }
  /* Invalid UTF-8 byte sequence */
  else if(*pu8Byte < 0xC0)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: multi-byte sequence non-leading byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

    /* Updates result */
    u32Result = orxU32_UNDEFINED;
  }
  /* Overlong UTF-8 2-byte sequence */
  else if(*pu8Byte < 0xC2)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: overlong 2-byte sequence starting with byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

    /* Updates result */
    u32Result = orxU32_UNDEFINED;
  }
  /* 2-byte sequence */
  else if(*pu8Byte < 0xE0)
  {
    /* Updates result with first character */
    u32Result = *pu8Byte++ & 0x1F;

    /* Valid second character? */
    if((*pu8Byte & 0xC0) == 0x80)
    {
      /* Updates result */
      u32Result = (u32Result << 6) | (*pu8Byte & 0x3F);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: 2-byte sequence non-trailing byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

      /* Updates result */
      u32Result = orxU32_UNDEFINED;
    }
  }
  /* 3-byte sequence */
  else if(*pu8Byte < 0xF0)
  {
    /* Updates result with first character */
    u32Result = *pu8Byte++ & 0x0F;

    /* Valid second character? */
    if((*pu8Byte & 0xC0) == 0x80)
    {
      /* Updates result */
      u32Result = (u32Result << 6) | (*pu8Byte++ & 0x3F);

      /* Valid third character? */
      if((*pu8Byte & 0xC0) == 0x80)
      {
        /* Updates result */
        u32Result = (u32Result << 6) | (*pu8Byte & 0x3F);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: 3-byte sequence non-trailing byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

        /* Updates result */
        u32Result = orxU32_UNDEFINED;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: 3-byte sequence non-trailing byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

      /* Updates result */
      u32Result = orxU32_UNDEFINED;
    }
  }
  /* 4-byte sequence */
  else if(*pu8Byte < 0xF5)
  {
    /* Updates result with first character */
    u32Result = *pu8Byte++ & 0x07;

    /* Valid second character? */
    if((*pu8Byte & 0xC0) == 0x80)
    {
      /* Updates result */
      u32Result = (u32Result << 6) | (*pu8Byte++ & 0x3F);

      /* Valid third character? */
      if((*pu8Byte & 0xC0) == 0x80)
      {
        /* Updates result */
        u32Result = (u32Result << 6) | (*pu8Byte++ & 0x3F);

        /* Valid fourth character? */
        if((*pu8Byte & 0xC0) == 0x80)
        {
          /* Updates result */
          u32Result = (u32Result << 6) | (*pu8Byte & 0x3F);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: 4-byte sequence non-trailing byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

          /* Updates result */
          u32Result = orxU32_UNDEFINED;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: 4-byte sequence non-trailing byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

        /* Updates result */
        u32Result = orxU32_UNDEFINED;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: 4-byte sequence non-trailing byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

      /* Updates result */
      u32Result = orxU32_UNDEFINED;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF-8 string at <0x%X>: invalid out-of-bound byte '%c' (0x%2X) at index %d.", _zString, *pu8Byte, *pu8Byte, pu8Byte - (orxU8 *)_zString);

    /* Updates result */
    u32Result = orxU32_UNDEFINED;
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = (orxSTRING)(pu8Byte + 1);
  }

  /* Done! */
  return u32Result;
}

/** Returns the number of valid unicode characters (UTF-8) in the string (for ASCII string, it will be the same result as orxString_GetLength())
 * @param[in] _zString                  Concerned string
 * @return                              Number of valid unicode characters contained in the string, orxU32_UNDEFINED for an invalid UTF-8 string
 */
static orxINLINE orxU32                 orxString_GetCharacterCounter(const orxSTRING _zString)
{
  const orxCHAR  *pc;
  orxU32          u32Result;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* For all characters */
  for(pc = _zString, u32Result = 0; *pc != orxCHAR_NULL; u32Result++)
  {
    /* Invalid current character ID */
    if(orxString_GetFirstCharacterCodePoint(pc, &pc) == orxU32_UNDEFINED)
    {
      /* Updates result */
      u32Result = orxU32_UNDEFINED;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Invalid or non-UTF8 string <%s>, can't count characters.", _zString);

      break;
    }
  }

  /* Done! */
  return u32Result;
}

/** Copies N characters from a string
 * @param[in] _zDstString       Destination string
 * @param[in] _zSrcString       Source string
 * @param[in] _u32CharNumber    Number of characters to copy
 * @return Copied string
 */
static orxINLINE orxSTRING              orxString_NCopy(orxSTRING _zDstString, const orxSTRING _zSrcString, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Done! */
  return(strncpy(_zDstString, _zSrcString, (size_t)_u32CharNumber));
}

/** Copies a string.
 * @param[in] _zDstString      Destination string
 * @param[in] _zSrcString      Source string
 * @return Copied string.
 */
static orxINLINE orxSTRING              orxString_Copy(orxSTRING _zDstString, const orxSTRING _zSrcString)
{
  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Done! */
  return(strcpy(_zDstString, _zSrcString));
}

/** Duplicate a string.
 * @param[in] _zSrcString  String to duplicate.
 * @return Duplicated string.
 */
static orxINLINE orxSTRING              orxString_Duplicate(const orxSTRING _zSrcString)
{
  orxU32    u32Size;
  orxSTRING zResult;

  /* Checks */
  orxASSERT(_zSrcString != orxNULL);

  /* Gets string size in bytes */
  u32Size = (orxString_GetLength(_zSrcString) + 1) * sizeof(orxCHAR);

  /* Allocates it */
  zResult = (orxSTRING)orxMemory_Allocate(u32Size, orxMEMORY_TYPE_TEXT);

  /* Valid? */
  if(zResult != orxNULL)
  {
    /* Copies source to it */
    orxMemory_Copy(zResult, _zSrcString, u32Size);
  }

  /* Done! */
  return zResult;
}

/** Deletes a string
 * @param[in] _zString                  String to delete
 */
static orxINLINE orxSTATUS              orxString_Delete(orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);
  orxASSERT(_zString != orxSTRING_EMPTY);

  /* Frees its memory */
  orxMemory_Free(_zString);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Compare two strings, case sensitive. If the first one is smaller than the second it returns -1,
 * 1 if the second one is bigger than the first, and 0 if they are equals
 * @param[in] _zString1    First String to compare
 * @param[in] _zString2    Second string to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
static orxINLINE orxS32                 orxString_Compare(const orxSTRING _zString1, const orxSTRING _zString2)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return(strcmp(_zString1, _zString2));
}

/** Compare N first character from two strings, case sensitive. If the first one is smaller
 * than the second it returns -1, 1 if the second one is bigger than the first
 * and 0 if they are equals.
 * @param[in] _zString1       First String to compare
 * @param[in] _zString2       Second string to compare
 * @param[in] _u32CharNumber  Number of character to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
static orxINLINE orxS32                 orxString_NCompare(const orxSTRING _zString1, const orxSTRING _zString2, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return strncmp(_zString1, _zString2, (size_t)_u32CharNumber);
}

/** Compare two strings, case insensitive. If the first one is smaller than the second, it returns -1,
 * If the second one is bigger than the first, and 0 if they are equals
 * @param[in] _zString1    First String to compare
 * @param[in] _zString2    Second string to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
static orxINLINE orxS32                 orxString_ICompare(const orxSTRING _zString1, const orxSTRING _zString2)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

#ifdef __orxWINDOWS__

  /* Done! */
  return(stricmp(_zString1, _zString2));

#else /* __orxWINDOWS__ */

  /* Done! */
  return strcasecmp(_zString1, _zString2);

#endif /* __orxWINDOWS__ */
}

/** Compare N first character from two strings, case insensitive. If the first one is smaller
 * than the second, it returns -1, If the second one is bigger than the first,
 * and 0 if they are equals.
 * @param[in] _zString1       First String to compare
 * @param[in] _zString2       Second string to compare
 * @param[in] _u32CharNumber  Number of character to compare
 * @return -1, 0 or 1 as indicated in the description.
 */
static orxINLINE orxS32                 orxString_NICompare(const orxSTRING _zString1, const orxSTRING _zString2, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

#ifdef __orxWINDOWS__

  /* Done! */
  return strnicmp(_zString1, _zString2, (size_t)_u32CharNumber);

#else /* __orxWINDOWS__ */

  /* Done! */
  return strncasecmp(_zString1, _zString2, _u32CharNumber);

#endif /* __orxWINDOWS__ */
}

/** Converts a String to a signed int value using the given base
 * @param[in]   _zString        String To convert
 * @param[in]   _u32Base        Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _ps32OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToS32Base(const orxSTRING _zString, orxU32 _u32Base, orxS32 *_ps32OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_ps32OutValue = strtol(_zString, &pcEnd, (size_t)_u32Base);

  /* Valid conversion ? */
  if((pcEnd != _zString) && (_zString[0] != orxCHAR_NULL))
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = pcEnd;
  }

  /* Done! */
  return eResult;
}

/** Converts a String to a signed int value, guessing the base
 * @param[in]   _zString        String To convert
 * @param[out]  _ps32OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToS32(const orxSTRING _zString, orxS32 *_ps32OutValue, const orxSTRING *_pzRemaining)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Hexadecimal? */
  if((_zString[0] != orxCHAR_EOL)
  && (_zString[0] == '0')
  && (_zString[1] != orxCHAR_EOL)
  && ((_zString[1] | 0x20) == 'x'))
  {
    /* Gets hexa value */
    eResult = orxString_ToS32Base(_zString + 2, 16, _ps32OutValue, _pzRemaining);
  }
  /* Binary? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && (_zString[1] != orxCHAR_EOL)
       && ((_zString[1] | 0x20) == 'b'))
  {
    /* Gets binary value */
    eResult = orxString_ToS32Base(_zString + 2, 2, _ps32OutValue, _pzRemaining);
  }
  /* Octal? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && ((_zString[1]) >= '0')
       && ((_zString[1]) <= '9'))
  {
    /* Gets octal value */
    eResult = orxString_ToS32Base(_zString + 1, 8, _ps32OutValue, _pzRemaining);
  }
  /* Decimal */
  else
  {
    /* Gets decimal value */
    eResult = orxString_ToS32Base(_zString, 10, _ps32OutValue, _pzRemaining);
  }

  /* Done! */
  return eResult;
}

/** Converts a String to an unsigned int value using the given base
 * @param[in]   _zString        String To convert
 * @param[in]   _u32Base        Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _pu32OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToU32Base(const orxSTRING _zString, orxU32 _u32Base, orxU32 *_pu32OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_pu32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_pu32OutValue = strtoul(_zString, &pcEnd, (size_t)_u32Base);

  /* Valid conversion ? */
  if((pcEnd != _zString) && (_zString[0] != orxCHAR_NULL))
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = pcEnd;
  }

  /* Done! */
  return eResult;
}

/** Converts a String to an unsigned int value, guessing the base
 * @param[in]   _zString        String To convert
 * @param[out]  _pu32OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToU32(const orxSTRING _zString, orxU32 *_pu32OutValue, const orxSTRING *_pzRemaining)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_pu32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Hexadecimal? */
  if((_zString[0] != orxCHAR_EOL)
  && (_zString[0] == '0')
  && (_zString[1] != orxCHAR_EOL)
  && ((_zString[1] | 0x20) == 'x'))
  {
    /* Gets hexa value */
    eResult = orxString_ToU32Base(_zString + 2, 16, _pu32OutValue, _pzRemaining);
  }
  /* Binary? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && (_zString[1] != orxCHAR_EOL)
       && ((_zString[1] | 0x20) == 'b'))
  {
    /* Gets binary value */
    eResult = orxString_ToU32Base(_zString + 2, 2, _pu32OutValue, _pzRemaining);
  }
  /* Octal? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && ((_zString[1]) >= '0')
       && ((_zString[1]) <= '9'))
  {
    /* Gets octal value */
    eResult = orxString_ToU32Base(_zString + 1, 8, _pu32OutValue, _pzRemaining);
  }
  /* Decimal */
  else
  {
    /* Gets decimal value */
    eResult = orxString_ToU32Base(_zString, 10, _pu32OutValue, _pzRemaining);
  }

  /* Done! */
  return eResult;
}

/** Converts a String to a signed int value using the given base
 * @param[in]   _zString        String To convert
 * @param[in]   _u32Base        Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _ps64OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToS64Base(const orxSTRING _zString, orxU32 _u32Base, orxS64 *_ps64OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_ps64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_ps64OutValue = strtoll(_zString, &pcEnd, (size_t)_u32Base);

  /* Valid conversion ? */
  if((pcEnd != _zString) && (_zString[0] != orxCHAR_NULL))
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = pcEnd;
  }

  /* Done! */
  return eResult;
}

/** Converts a String to a signed int value, guessing the base
 * @param[in]   _zString        String To convert
 * @param[out]  _ps64OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToS64(const orxSTRING _zString, orxS64 *_ps64OutValue, const orxSTRING *_pzRemaining)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_ps64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Hexadecimal? */
  if((_zString[0] != orxCHAR_EOL)
  && (_zString[0] == '0')
  && (_zString[1] != orxCHAR_EOL)
  && ((_zString[1] | 0x20) == 'x'))
  {
    /* Gets hexa value */
    eResult = orxString_ToS64Base(_zString + 2, 16, _ps64OutValue, _pzRemaining);
  }
  /* Binary? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && (_zString[1] != orxCHAR_EOL)
       && ((_zString[1] | 0x20) == 'b'))
  {
    /* Gets binary value */
    eResult = orxString_ToS64Base(_zString + 2, 2, _ps64OutValue, _pzRemaining);
  }
  /* Octal? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && ((_zString[1]) >= '0')
       && ((_zString[1]) <= '9'))
  {
    /* Gets octal value */
    eResult = orxString_ToS64Base(_zString + 1, 8, _ps64OutValue, _pzRemaining);
  }
  /* Decimal */
  else
  {
    /* Gets decimal value */
    eResult = orxString_ToS64Base(_zString, 10, _ps64OutValue, _pzRemaining);
  }

  /* Done! */
  return eResult;
}

/** Converts a String to an unsigned int value using the given base
 * @param[in]   _zString        String To convert
 * @param[in]   _u32Base        Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _pu64OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToU64Base(const orxSTRING _zString, orxU32 _u32Base, orxU64 *_pu64OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_pu64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_pu64OutValue = strtoull(_zString, &pcEnd, (size_t)_u32Base);

  /* Valid conversion ? */
  if((pcEnd != _zString) && (_zString[0] != orxCHAR_NULL))
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = pcEnd;
  }

  /* Done! */
  return eResult;
}

/** Converts a String to an unsigned int value, guessing the base
 * @param[in]   _zString        String To convert
 * @param[out]  _pu64OutValue   Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToU64(const orxSTRING _zString, orxU64 *_pu64OutValue, const orxSTRING *_pzRemaining)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_pu64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Hexadecimal? */
  if((_zString[0] != orxCHAR_EOL)
  && (_zString[0] == '0')
  && (_zString[1] != orxCHAR_EOL)
  && ((_zString[1] | 0x20) == 'x'))
  {
    /* Gets hexa value */
    eResult = orxString_ToU64Base(_zString + 2, 16, _pu64OutValue, _pzRemaining);
  }
  /* Binary? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && (_zString[1] != orxCHAR_EOL)
       && ((_zString[1] | 0x20) == 'b'))
  {
    /* Gets binary value */
    eResult = orxString_ToU64Base(_zString + 2, 2, _pu64OutValue, _pzRemaining);
  }
  /* Octal? */
  else if((_zString[0] != orxCHAR_EOL)
       && (_zString[0] == '0')
       && ((_zString[1]) >= '0')
       && ((_zString[1]) <= '9'))
  {
    /* Gets octal value */
    eResult = orxString_ToU64Base(_zString + 1, 8, _pu64OutValue, _pzRemaining);
  }
  /* Decimal */
  else
  {
    /* Gets decimal value */
    eResult = orxString_ToU64Base(_zString, 10, _pu64OutValue, _pzRemaining);
  }

  /* Done! */
  return eResult;
}

/** Convert a string to a value
 * @param[in]   _zString        String To convert
 * @param[out]  _pfOutValue     Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToFloat(const orxSTRING _zString, orxFLOAT *_pfOutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_pfOutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Linux / Mac / iOS / Android / MSVC? */
#if defined(__orxLINUX__) || defined(__orxMAC__) || defined (__orxIOS__) || defined(__orxMSVC__) || defined(__orxANDROID__)

  /* Converts it */
  *_pfOutValue = (orxFLOAT)strtod(_zString, &pcEnd);

#else /* __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxMSVC__ || __orxANDROID__ */

  /* Converts it */
  *_pfOutValue = strtof(_zString, &pcEnd);

#endif /* __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxMSVC__ || __orxANDROID__ */

  /* Valid conversion ? */
  if((pcEnd != _zString) && (_zString[0] != orxCHAR_NULL))
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = pcEnd;
  }

  /* Done! */
  return eResult;
}

/** Convert a string to a vector
 * @param[in]   _zString        String To convert
 * @param[out]  _pvOutValue     Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToVector(const orxSTRING _zString, orxVECTOR *_pvOutValue, const orxSTRING *_pzRemaining)
{
  orxVECTOR       stValue;
  const orxSTRING zString;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(_pvOutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Skips all white spaces */
  zString = orxString_SkipWhiteSpaces(_zString);

  /* Is a vector start character? */
  if((*zString == orxSTRING_KC_VECTOR_START)
  || (*zString == orxSTRING_KC_VECTOR_START_ALT))
  {
    /* Skips all white spaces */
    zString = orxString_SkipWhiteSpaces(zString + 1);

    /* Gets X value */
    eResult = orxString_ToFloat(zString, &(stValue.fX), &zString);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Skips all white spaces */
      zString = orxString_SkipWhiteSpaces(zString);

      /* Is a vector separator character? */
      if(*zString == orxSTRING_KC_VECTOR_SEPARATOR)
      {
        /* Skips all white spaces */
        zString = orxString_SkipWhiteSpaces(zString + 1);

        /* Gets Y value */
        eResult = orxString_ToFloat(zString, &(stValue.fY), &zString);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Skips all white spaces */
          zString = orxString_SkipWhiteSpaces(zString);

          /* Is a vector separator character? */
          if(*zString == orxSTRING_KC_VECTOR_SEPARATOR)
          {
            /* Skips all white spaces */
            zString = orxString_SkipWhiteSpaces(zString + 1);

            /* Gets Z value */
            eResult = orxString_ToFloat(zString, &(stValue.fZ), &zString);

            /* Success? */
            if(eResult != orxSTATUS_FAILURE)
            {
              /* Skips all white spaces */
              zString = orxString_SkipWhiteSpaces(zString);

              /* Is not a vector end character? */
              if((*zString != orxSTRING_KC_VECTOR_END)
              && (*zString != orxSTRING_KC_VECTOR_END_ALT))
              {
                /* Updates result */
                eResult = orxSTATUS_FAILURE;
              }
            }
          }
        }
      }
    }
  }

  /* Valid? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Updates vector */
    orxVector_Copy(_pvOutValue, &stValue);

    /* Asks for remaining string? */
    if(_pzRemaining != orxNULL)
    {
      /* Stores it */
      *_pzRemaining = zString + 1;
    }
  }

  /* Done! */
  return eResult;
}

/** Convert a string to a boolean
 * @param[in]   _zString        String To convert
 * @param[out]  _pbOutValue     Converted value
 * @param[out]  _pzRemaining    If non null, will contain the remaining string after the number conversion
 * @return  orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS              orxString_ToBool(const orxSTRING _zString, orxBOOL *_pbOutValue, const orxSTRING *_pzRemaining)
{
  orxS32    s32Value;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(_pbOutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Tries numeric value */
  eResult = orxString_ToS32Base(_zString, 10, &s32Value, _pzRemaining);

  /* Valid? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Updates boolean */
    *_pbOutValue = (s32Value != 0) ? orxTRUE : orxFALSE;
  }
  else
  {
    orxU32 u32Length;

    /* Gets length of false */
    u32Length = orxString_GetLength(orxSTRING_FALSE);

    /* Is false? */
    if(orxString_NICompare(_zString, orxSTRING_FALSE, u32Length) == 0)
    {
      /* Updates boolean */
      *_pbOutValue = orxFALSE;

      /* Has remaining? */
      if(_pzRemaining != orxNULL)
      {
        /* Updates it */
        *_pzRemaining += u32Length;
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Gets length of true */
      u32Length = orxString_GetLength(orxSTRING_TRUE);

      /* Is true? */
      if(orxString_NICompare(_zString, orxSTRING_TRUE, u32Length) == 0)
      {
        /* Updates boolean */
        *_pbOutValue = orxTRUE;

        /* Has remaining? */
        if(_pzRemaining != orxNULL)
        {
          /* Updates it */
          *_pzRemaining += u32Length;
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Lowercase a string
 * @param[in] _zString          String To convert
 * @return The converted string.
 */
static orxINLINE orxSTRING              orxString_LowerCase(orxSTRING _zString)
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
 * @param[in] _zString          String To convert
 * @return The converted string.
 */
static orxINLINE orxSTRING              orxString_UpperCase(orxSTRING _zString)
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
 * @param[in] _zString          String To convert
 * @return The resulting CRC.
 */
static orxINLINE orxU32                 orxString_ToCRC(const orxSTRING _zString)
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
static orxINLINE orxU32                 orxString_NToCRC(const orxSTRING _zString, orxU32 _u32CharNumber)
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
static orxINLINE const orxSTRING        orxString_SearchString(const orxSTRING _zString1, const orxSTRING _zString2)
{
  /* Checks */
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
static orxINLINE const orxSTRING        orxString_SearchChar(const orxSTRING _zString, orxCHAR _cChar)
{
  /* Checks */
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
static orxINLINE orxS32                 orxString_SearchCharIndex(const orxSTRING _zString, orxCHAR _cChar, orxU32 _u32Position)
{
  orxS32          s32Result = -1;
  orxS32          s32Index;
  const orxCHAR  *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);
  orxASSERT(_u32Position <= orxString_GetLength(_zString));

  /* For all characters */
  for(s32Index = _u32Position, pc = _zString + s32Index; *pc != orxCHAR_NULL; pc++, s32Index++)
  {
    /* Found? */
    if(*pc == _cChar)
    {
      /* Updates result */
      s32Result = s32Index;

      break;
    }
  }

  /* Done! */
  return s32Result;
}

/** Prints a formated string to a memory buffer
 * @param[out] _zDstString  Destination string
 * @param[in]  _zSrcString  Source formated string
 * @return The number of written characters
 */
static orxINLINE orxS32 orxCDECL        orxString_Print(orxSTRING _zDstString, const orxSTRING _zSrcString, ...)
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

/** Prints a formated string to a memory buffer using a max size
 * @param[out] _zDstString    Destination string
 * @param[in]  _zSrcString    Source formated string
 * @param[in]  _u32CharNumber Max number of character to print
 * @return The number of written characters
 */
static orxINLINE orxS32 orxCDECL        orxString_NPrint(orxSTRING _zDstString, orxU32 _u32CharNumber, const orxSTRING _zSrcString, ...)
{
  va_list stArgs;
  orxS32  s32Result;

  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Gets variable arguments & print the string */
  va_start(stArgs, _zSrcString);
  s32Result = vsnprintf(_zDstString, (size_t)_u32CharNumber, _zSrcString, stArgs);
  va_end(stArgs);

  /* Done! */
  return s32Result;
}

/** Gets the extension from a file name
 * @param[in]  _zFileName     Concerned file name
 * @return Extension if exists, orxSTRING_EMPTY otherwise
 */
static orxINLINE const orxSTRING        orxString_GetExtension(const orxSTRING _zFileName)
{
  orxS32          s32Index, s32NextIndex;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(_zFileName != orxNULL);

  /* Finds last '.' */
  for(s32Index = orxString_SearchCharIndex(_zFileName, '.', 0);
      (s32Index >= 0) && ((s32NextIndex = orxString_SearchCharIndex(_zFileName, '.', s32Index + 1)) > 0);
      s32Index = s32NextIndex);

  /* Updates result */
  zResult = (s32Index >= 0) ? _zFileName + s32Index + 1 : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */

#endif /* _orxSTRING_H_ */

/** @} */

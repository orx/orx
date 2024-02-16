/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @author iarwain@orx-project.org
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

#include "math/orxVector.h"
#include "memory/orxMemory.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#ifndef __orxWINDOWS__
  #include <strings.h>
#endif /* !__orxWINDOWS__ */

#include "debug/orxDebug.h"

#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

  #define strtoll   _strtoi64
  #define strtoull  _strtoui64

#endif /* __orxMSVC__ */

#define STRTO_CAST (int)


#define orxSTRING_KC_VECTOR_START                         '('
#define orxSTRING_KC_VECTOR_START_ALT                     '{'
#define orxSTRING_KC_VECTOR_SEPARATOR                     ','
#define orxSTRING_KC_VECTOR_END                           ')'
#define orxSTRING_KC_VECTOR_END_ALT                       '}'


/** Defines
 */
#define orxString_ToCRC                                   orxString_Hash


/* *** String inlined functions *** */


/** Skips all white spaces
 * @param[in]   _zString                Concerned string
 * @return      The sub-string located after all leading white spaces, including EOL characters
 */
static orxINLINE const orxSTRING                          orxString_SkipWhiteSpaces(const orxSTRING _zString)
{
  const orxSTRING zResult;

  /* Non null? */
  if(_zString != orxNULL)
  {
    /* Skips all white spaces */
    for(zResult = _zString; (*zResult == ' ') || (*zResult == '\t') || (*zResult == orxCHAR_CR) || (*zResult == orxCHAR_LF); zResult++)
      ;

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
 * @param[in]   _zString                Concerned string
 * @return      The sub-string located after all non-terminal directory separators
 */
static orxINLINE const orxSTRING                          orxString_SkipPath(const orxSTRING _zString)
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

/** Gets the number of orxCHAR in the string (for non-ASCII UTF-8 string, it won't be the actual number of unicode characters)
 * @param[in]   _zString                String used for length computation
 * @return      The length of the string (doesn't include the final orxCHAR_NULL character)
 */
static orxINLINE orxU32                                   orxString_GetLength(const orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Done! */
  return((orxU32)strlen(_zString));
}

/** Tells if a character is ASCII from its ID
* @param[in]    _u32CharacterCodePoint  Concerned character code
 * @return      orxTRUE is it's a non-extended ASCII character, orxFALSE otherwise
 */
static orxINLINE orxBOOL                                  orxString_IsCharacterASCII(orxU32 _u32CharacterCodePoint)
{
  /* Done! */
  return((_u32CharacterCodePoint < 0x80) ? orxTRUE : orxFALSE);
}

/** Tells if a character is alpha-numeric from its ID
 * @param[in]   _u32CharacterCodePoint  Concerned character code
 * @return      orxTRUE is it's a non-extended ASCII alpha-numerical character, orxFALSE otherwise
 */
static orxINLINE orxBOOL                                  orxString_IsCharacterAlphaNumeric(orxU32 _u32CharacterCodePoint)
{
  /* Done! */
  return(((_u32CharacterCodePoint >= 'a') && (_u32CharacterCodePoint <= 'z'))
      || ((_u32CharacterCodePoint >= 'A') && (_u32CharacterCodePoint <= 'Z'))
      || ((_u32CharacterCodePoint >= '0') && (_u32CharacterCodePoint <= '9'))) ? orxTRUE : orxFALSE;
}

/** Gets the UTF-8 encoding length of a given character
 * @param[in]   _u32CharacterCodePoint  Concerned character code
 * @return      The encoding length in UTF-8 for the given character if valid, orxU32_UNDEFINED otherwise
 */
static orxINLINE orxU32                                   orxString_GetUTF8CharacterLength(orxU32 _u32CharacterCodePoint)
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
 * @param[in]   _zDstString             Destination string
 * @param[in]   _u32Size                Available size on the string
 * @param[in]   _u32CharacterCodePoint  Unicode code point of the character to print
 * @return      The length of the encoded UTF-8 character (1, 2, 3 or 4) if valid, orxU32_UNDEFINED otherwise
 */
static orxU32 orxFASTCALL                                 orxString_PrintUTF8Character(orxSTRING _zDstString, orxU32 _u32Size, orxU32 _u32CharacterCodePoint)
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

/** Gets the unicode code point of the first character of the UTF-8 string
 * @param[in]   _zString                Concerned string
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the first UTF-8 character
 * @return      The code of the first UTF-8 character of the string, orxU32_UNDEFINED if it's an invalid character
 */
static orxU32 orxFASTCALL                                 orxString_GetFirstCharacterCodePoint(const orxSTRING _zString, const orxSTRING *_pzRemaining)
{
  const orxU8  *pu8Byte;
  orxU32        u32Result;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Gets the first byte */
  pu8Byte = (const orxU8 *)_zString;

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

/** Gets the number of valid unicode characters (UTF-8) in the string (for ASCII string, it will be the same result as orxString_GetLength())
 * @param[in]   _zString                Concerned string
 * @return      The number of valid unicode characters contained in the string, orxU32_UNDEFINED for an invalid UTF-8 string
 */
static orxINLINE orxU32                                   orxString_GetCharacterCount(const orxSTRING _zString)
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

/** Copies up to N characters from a string
 * @param[in]   _zDstString             Destination string
 * @param[in]   _zSrcString             Source string
 * @param[in]   _u32CharNumber          Number of characters to copy
 * @return      The copied string
 */
static orxINLINE orxSTRING                                orxString_NCopy(orxSTRING _zDstString, const orxSTRING _zSrcString, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);
  orxASSERT(_u32CharNumber > 0);

  /* Done! */
  return(strncpy(_zDstString, _zSrcString, (size_t)_u32CharNumber));
}

/** Duplicates a string.
 * @param[in]   _zSrcString             String to duplicate
 * @return      The duplicated string
 */
static orxINLINE orxSTRING                                orxString_Duplicate(const orxSTRING _zSrcString)
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
 * @param[in]   _zString                String to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_Delete(orxSTRING _zString)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);
  orxASSERT(_zString != orxSTRING_EMPTY);

  /* Frees its memory */
  orxMemory_Free(_zString);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Compares two strings (case sensitive)
 * @param[in]   _zString1               First String to compare
 * @param[in]   _zString2               Second string to compare
 * @return      -1 if _zString1 < _zString2, 0 if _zString1 == _zString2, 1 if _zString1 > _zString2
 */
static orxINLINE orxS32                                   orxString_Compare(const orxSTRING _zString1, const orxSTRING _zString2)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return(strcmp(_zString1, _zString2));
}

/** Compares the N first characters of two strings (case sensitive)
 * and 0 if they are equals.
 * @param[in]   _zString1               First String to compare
 * @param[in]   _zString2               Second string to compare
 * @param[in]   _u32CharNumber          Number of character to compare
 * @return      -1 if _zString1 < _zString2, 0 if _zString1 == _zString2, 1 if _zString1 > _zString2
 */
static orxINLINE orxS32                                   orxString_NCompare(const orxSTRING _zString1, const orxSTRING _zString2, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return(strncmp(_zString1, _zString2, (size_t)_u32CharNumber));
}

/** Compares two strings (case insensitive)
 * @param[in]   _zString1               First String to compare
 * @param[in]   _zString2               Second string to compare
 * @return      -1 if _zString1 < _zString2, 0 if _zString1 == _zString2, 1 if _zString1 > _zString2
 */
static orxINLINE orxS32                                   orxString_ICompare(const orxSTRING _zString1, const orxSTRING _zString2)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

#ifdef __orxWINDOWS__

  /* Done! */
  return(stricmp(_zString1, _zString2));

#else /* __orxWINDOWS__ */

  /* Done! */
  return(strcasecmp(_zString1, _zString2));

#endif /* __orxWINDOWS__ */
}

/** Compare the N first characters of two strings (case insensitive)
 * @param[in]   _zString1               First String to compare
 * @param[in]   _zString2               Second string to compare
 * @param[in]   _u32CharNumber          Number of character to compare
 * @return      -1 if _zString1 < _zString2, 0 if _zString1 == _zString2, 1 if _zString1 > _zString2
 */
static orxINLINE orxS32                                   orxString_NICompare(const orxSTRING _zString1, const orxSTRING _zString2, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

#ifdef __orxWINDOWS__

  /* Done! */
  return(strnicmp(_zString1, _zString2, (size_t)_u32CharNumber));

#else /* __orxWINDOWS__ */

  /* Done! */
  return(strncasecmp(_zString1, _zString2, _u32CharNumber));

#endif /* __orxWINDOWS__ */
}

/** Gets the edit distance (Damerau-Levenshtein) between two strings
 * @param[in]   _zString1               First string
 * @param[in]   _zString2               Second string
 * @return      The edit distance between both strings
 */
static orxINLINE orxU32                                   orxString_GetEditDistance(const orxSTRING _zString1, const orxSTRING _zString2)
{
  orxU32 u32Length1, u32Length2, u32Result;

  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Gets string lengths */
  u32Length1 = orxString_GetLength(_zString1);
  u32Length2 = orxString_GetLength(_zString2);

  /* Valid? */
  if((u32Length1 > 0) && (u32Length2 > 0))
  {
    orxU32 *au32PreviousRow = (orxU32 *)alloca((u32Length2 + 1) * sizeof(orxU32));
    orxU32 *au32CurrentRow = (orxU32 *)alloca((u32Length2 + 1) * sizeof(orxU32));
    orxU32 *au32NextRow = (orxU32 *)alloca((u32Length2 + 1) * sizeof(orxU32));
    orxU32 i, j;

    /* Initializes the previous and current rows */
    orxMemory_Zero(au32PreviousRow, (u32Length2 + 1) * sizeof(orxU32));
    for(i = 0; i < u32Length2; i++)
    {
      au32CurrentRow[i] = i;
    }

    /* For all characters in the first string */
    for(i = 0; i < u32Length1; i++)
    {
      orxU32 *pu32dummy;

      /* Inits the first value of the next row */
      au32NextRow[0] = i + 1;

      /* For all characters in the second string */
      for(j = 0; j < u32Length2; j++)
      {
        /* Substitution? */
        au32NextRow[j + 1] = au32CurrentRow[j] + ((_zString1[i] != _zString2[j]) ? 1 : 0);

        /* Deletion? */
        au32NextRow[j + 1] = orxMIN(au32NextRow[j + 1], au32CurrentRow[j + 1] + 1);

        /* Insertion? */
        au32NextRow[j + 1] = orxMIN(au32NextRow[j + 1], au32NextRow[j] + 1);

        /* Swap? */
        if((i > 0)
        && (j > 0)
        && (_zString1[i - 1] == _zString2[j])
        && (_zString1[i] == _zString2[j - 1]))
        {
          au32NextRow[j + 1] = orxMIN(au32NextRow[j + 1], au32PreviousRow[j - 1] + 1);
        }
      }

      /* Cycles the rows */
      pu32dummy       = au32PreviousRow;
      au32PreviousRow = au32CurrentRow;
      au32CurrentRow  = au32NextRow;
      au32NextRow     = pu32dummy;
    }

    /* Updates result */
    u32Result = au32CurrentRow[u32Length2];
  }
  else
  {
    /* Updates result */
    u32Result = u32Length1 + u32Length2;
  }

  /* Done! */
  return u32Result;
}

/** Extracts the base (2, 8, 10 or 16) from a literal number
 * @param[in]   _zString                String from which to extract the base
 * @param[out]  _pzRemaining            If non null, it will contain the remaining number literal, right after the base prefix (0x, 0b or 0) if any
 * @return      The base or the numerical value, defaults to 10 (decimal) when no prefix is found or the literal value couldn't be identified
 */
static orxINLINE orxU32                                   orxString_ExtractBase(const orxSTRING _zString, const orxSTRING *_pzRemaining)
{
  const orxSTRING zString;
  orxU32          u32Result, u32Offset;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Skips white spaces */
  zString = orxString_SkipWhiteSpaces(_zString);

  /* Default result and offset: decimal */
  u32Result = 10;
  u32Offset = 0;

  /* Depending on first character */
  switch(zString[0])
  {
    case '0':
    {
      /* Depending on second character */
      switch(zString[1] | 0x20)
      {
        case 'x':
        {
          /* Updates result and offset: hexadecimal */
          u32Result = 16;
          u32Offset = 2;

          break;
        }

        case 'b':
        {
          /* Updates result and offset: binary */
          u32Result = 2;
          u32Offset = 2;

          break;
        }

        default:
        {
          /* Octal? */
          if((zString[1] >= '0')
          && (zString[1] <= '9'))
          {
            /* Updates result and offset: octal */
            u32Result = 8;
            u32Offset = 1;
          }

          break;
        }
      }

      break;
    }

    case '#':
    {
      /* Updates result and offset: hexadecimal */
      u32Result = 16;
      u32Offset = 1;

      break;
    }

    default:
    {
      break;
    }
  }

  /* Asks for remaining string? */
  if(_pzRemaining != orxNULL)
  {
    /* Stores it */
    *_pzRemaining = zString + u32Offset;
  }

  /* Done! */
  return u32Result;
}

/** Converts a string to a signed int value using the given base
 * @param[in]   _zString                String To convert
 * @param[in]   _u32Base                Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _ps32OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToS32Base(const orxSTRING _zString, orxU32 _u32Base, orxS32 *_ps32OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_ps32OutValue = (orxS32)strtol(_zString, &pcEnd, STRTO_CAST _u32Base);

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

/** Converts a string to a signed int value, guessing the base
 * @param[in]   _zString                String To convert
 * @param[out]  _ps32OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToS32(const orxSTRING _zString, orxS32 *_ps32OutValue, const orxSTRING *_pzRemaining)
{
  const orxSTRING zValue;
  orxU32          u32Base;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Extracts base */
  u32Base = orxString_ExtractBase(_zString, &zValue);

  /* Gets value */
  eResult = orxString_ToS32Base(zValue, u32Base, _ps32OutValue, _pzRemaining);

  /* Done! */
  return eResult;
}

/** Converts a string to an unsigned int value using the given base
 * @param[in]   _zString                String To convert
 * @param[in]   _u32Base                Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _pu32OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToU32Base(const orxSTRING _zString, orxU32 _u32Base, orxU32 *_pu32OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_pu32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_pu32OutValue = (orxU32)strtoul(_zString, &pcEnd, STRTO_CAST _u32Base);

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

/** Converts a string to an unsigned int value, guessing the base
 * @param[in]   _zString                String To convert
 * @param[out]  _pu32OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToU32(const orxSTRING _zString, orxU32 *_pu32OutValue, const orxSTRING *_pzRemaining)
{
  const orxSTRING zValue;
  orxU32          u32Base;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(_pu32OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Extracts base */
  u32Base = orxString_ExtractBase(_zString, &zValue);

  /* Gets value */
  eResult = orxString_ToU32Base(zValue, u32Base, _pu32OutValue, _pzRemaining);

  /* Done! */
  return eResult;
}

/** Converts a string to a signed int value using the given base
 * @param[in]   _zString                String To convert
 * @param[in]   _u32Base                Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _ps64OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToS64Base(const orxSTRING _zString, orxU32 _u32Base, orxS64 *_ps64OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_ps64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_ps64OutValue = (orxS64)strtoll(_zString, &pcEnd, STRTO_CAST _u32Base);

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

/** Converts a string to a signed int value, guessing the base
 * @param[in]   _zString                String To convert
 * @param[out]  _ps64OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToS64(const orxSTRING _zString, orxS64 *_ps64OutValue, const orxSTRING *_pzRemaining)
{
  const orxSTRING zValue;
  orxU32          u32Base;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(_ps64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Extracts base */
  u32Base = orxString_ExtractBase(_zString, &zValue);

  /* Gets signed value */
  eResult = orxString_ToS64Base(zValue, u32Base, _ps64OutValue, _pzRemaining);

  /* Done! */
  return eResult;
}

/** Converts a string to an unsigned int value using the given base
 * @param[in]   _zString                String To convert
 * @param[in]   _u32Base                Base of the read value (generally 10, but can be 16 to read hexa)
 * @param[out]  _pu64OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToU64Base(const orxSTRING _zString, orxU32 _u32Base, orxU64 *_pu64OutValue, const orxSTRING *_pzRemaining)
{
  orxCHAR    *pcEnd;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(_pu64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Convert */
  *_pu64OutValue = (orxU64)strtoull(_zString, &pcEnd, STRTO_CAST _u32Base);

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

/** Converts a string to an unsigned int value, guessing the base
 * @param[in]   _zString                String To convert
 * @param[out]  _pu64OutValue           Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToU64(const orxSTRING _zString, orxU64 *_pu64OutValue, const orxSTRING *_pzRemaining)
{
  const orxSTRING zValue;
  orxU32          u32Base;
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT(_pu64OutValue != orxNULL);
  orxASSERT(_zString != orxNULL);

  /* Extracts base */
  u32Base = orxString_ExtractBase(_zString, &zValue);

  /* Gets signed value */
  eResult = orxString_ToU64Base(zValue, u32Base, _pu64OutValue, _pzRemaining);

  /* Done! */
  return eResult;
}

/** Converts a string to a value
 * @param[in]   _zString                String To convert
 * @param[out]  _pfOutValue             Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToFloat(const orxSTRING _zString, orxFLOAT *_pfOutValue, const orxSTRING *_pzRemaining)
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

/** Converts a string to a vector
 * @param[in]   _zString                String To convert
 * @param[out]  _pvOutValue             Converted value. N.B.: if only two components (x, y) are defined, the z component will be set to zero
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the number conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToVector(const orxSTRING _zString, orxVECTOR *_pvOutValue, const orxSTRING *_pzRemaining)
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
    orxCHAR cEndMarker;

    /* Gets end marker */
    cEndMarker = (*zString == orxSTRING_KC_VECTOR_START) ? orxSTRING_KC_VECTOR_END : orxSTRING_KC_VECTOR_END_ALT;

    /* Skips all white spaces */
    zString = orxString_SkipWhiteSpaces(zString + 1);

    /* Gets X value */
    if(orxString_ToFloat(zString, &(stValue.fX), &zString) != orxSTATUS_FAILURE)
    {
      /* Skips all white spaces */
      zString = orxString_SkipWhiteSpaces(zString);

      /* Is a vector separator character? */
      if(*zString == orxSTRING_KC_VECTOR_SEPARATOR)
      {
        /* Skips all white spaces */
        zString = orxString_SkipWhiteSpaces(zString + 1);

        /* Gets Y value */
        if(orxString_ToFloat(zString, &(stValue.fY), &zString) != orxSTATUS_FAILURE)
        {
          /* Skips all white spaces */
          zString = orxString_SkipWhiteSpaces(zString);

          /* Is a vector separator character? */
          if(*zString == orxSTRING_KC_VECTOR_SEPARATOR)
          {
            /* Skips all white spaces */
            zString = orxString_SkipWhiteSpaces(zString + 1);

            /* Gets Z value */
            if(orxString_ToFloat(zString, &(stValue.fZ), &zString) != orxSTATUS_FAILURE)
            {
              /* Skips all white spaces */
              zString = orxString_SkipWhiteSpaces(zString);

              /* Has a valid end marker? */
              if(*zString == cEndMarker)
              {
                /* Updates result */
                eResult = orxSTATUS_SUCCESS;
              }
            }
          }
          /* Has a valid end marker? */
          else if(*zString == cEndMarker)
          {
            /* Clears Z component */
            stValue.fZ = orxFLOAT_0;

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
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

/** Converts a string to a boolean
 * @param[in]   _zString                String To convert
 * @param[out]  _pbOutValue             Converted value
 * @param[out]  _pzRemaining            If non null, it will contain the remaining string after the boolean conversion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS                                orxString_ToBool(const orxSTRING _zString, orxBOOL *_pbOutValue, const orxSTRING *_pzRemaining)
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

/** Converts a string to lower case
 * @param[in] _zString                  String To convert
 * @return    The converted string
 */
static orxINLINE orxSTRING                                orxString_LowerCase(orxSTRING _zString)
{
  orxCHAR *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Converts the whole string */
  for(pc = _zString; *pc != orxCHAR_NULL; pc++)
  {
    /* Needs to be converted? */
    if((*pc >= 'A') && (*pc <= 'Z'))
    {
      /* Lower case */
      *pc |= 0x20;
    }
  }

  /* Done! */
  return _zString;
}

/** Converts a string to upper case
 * @param[in] _zString                  String To convert
 * @return    The converted string
 */
static orxINLINE orxSTRING                                orxString_UpperCase(orxSTRING _zString)
{
  orxCHAR *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Converts the whole string */
  for(pc = _zString; *pc != orxCHAR_NULL; pc++)
  {
    /* Needs to be converted? */
    if((*pc >= 'a') && (*pc <= 'z'))
    {
      /* Upper case */
      *pc &= ~0x20;
    }
  }

  /* Done! */
  return _zString;
}

/** Finds the first occurrence of _zString2 in _zString1
 * @param[in]   _zString1               String to analyze
 * @param[in]   _zString2               The string to find inside _zString1
 * @return      The pointer of the first occurrence of _zString2, or orxNULL if not found
 */
static orxINLINE const orxSTRING                          orxString_SearchString(const orxSTRING _zString1, const orxSTRING _zString2)
{
  /* Checks */
  orxASSERT(_zString1 != orxNULL);
  orxASSERT(_zString2 != orxNULL);

  /* Done! */
  return(strstr(_zString1, _zString2));
}

/** Finds the first occurrence of _cChar in _zString
 * @param[in]   _zString                String to analyze
 * @param[in]   _cChar                  The character to find
 * @return      The pointer of the first occurrence of _cChar, or orxNULL if not found
 */
static orxINLINE const orxSTRING                          orxString_SearchChar(const orxSTRING _zString, orxCHAR _cChar)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Done! */
  return(strchr(_zString, _cChar));
}

/** Finds the first occurrence of _cChar in _zString
 * @param[in]   _zString                String to analyze
 * @param[in]   _cChar                  The character to find
 * @param[in]   _s32Position            Search begin position
 * @return      The index of the next occurrence of requested character, starting at given position / -1 if not found
 */
static orxINLINE orxS32                                   orxString_SearchCharIndex(const orxSTRING _zString, orxCHAR _cChar, orxS32 _s32Position)
{
  orxS32          s32Index, s32Result = -1;
  const orxCHAR  *pc;

  /* Checks */
  orxASSERT(_zString != orxNULL);
  orxASSERT(_s32Position <= (orxS32)orxString_GetLength(_zString));

  /* For all characters */
  for(s32Index = _s32Position, pc = _zString + s32Index; *pc != orxCHAR_NULL; pc++, s32Index++)
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

/** Prints a formatted string to a memory buffer using a max size
 * @param[out]  _zDstString             Destination string
 * @param[in]   _u32CharNumber          Max number of character to print, including the terminating null character
 * @param[in]   _zSrcString             Source formatted string
 * @return      The number of written characters, excluding the terminating null character
 */
static orxINLINE orxS32 orxCDECL                          orxString_NPrint(orxSTRING _zDstString, orxU32 _u32CharNumber, const orxSTRING _zSrcString, ...)
{
  va_list stArgs;
  orxS32  s32Result;

  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);
  orxASSERT(_u32CharNumber > 0);

  /* Gets variable arguments & prints the string */
  va_start(stArgs, _zSrcString);
  s32Result = vsnprintf(_zDstString, (size_t)_u32CharNumber, _zSrcString, stArgs);
  va_end(stArgs);

  /* Overflow? */
  if(s32Result <= 0)
  {
    /* Updates result */
    s32Result = (orxS32)_u32CharNumber - 1;
  }
  else
  {
    /* Clamps result */
    s32Result = orxCLAMP(s32Result, 0, (orxS32)_u32CharNumber - 1);
  }

  /* Enforces terminating null character */
  _zDstString[s32Result] = orxCHAR_NULL;

  /* Done! */
  return s32Result;
}

/** Scans a formatted string from a memory buffer
 * @param[in]   _zString                String to scan
 * @param[in]   _zFormat                Format string
 * @return      The number of scanned items
 */
static orxINLINE orxS32 orxCDECL                          orxString_Scan(const orxSTRING _zString, const orxSTRING _zFormat, ...)
{
  va_list stArgs;
  orxS32  s32Result;

  /* Checks */
  orxASSERT(_zString != orxNULL);

  /* Gets variable arguments & scans the string */
  va_start(stArgs, _zFormat);
  s32Result = vsscanf(_zString, _zFormat, stArgs);
  va_end(stArgs);

  /* Clamps result */
  s32Result = orxMAX(s32Result, 0);

  /* Done! */
  return s32Result;
}

/** Gets the extension of a file name
 * @param[in]   _zFileName              Concerned file name
 * @return      Extension if present, orxSTRING_EMPTY otherwise
 */
static orxINLINE const orxSTRING                          orxString_GetExtension(const orxSTRING _zFileName)
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

/* *** String module functions *** */

/** Structure module setup
 */
extern orxDLLAPI void orxFASTCALL                         orxString_Setup();

/** Initializess the structure module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxString_Init();

/** Exits from the structure module
 */
extern orxDLLAPI void orxFASTCALL                         orxString_Exit();


/** Gets a string's ID (aka hash), without storing the string internally
 * @param[in]   _zString        Concerned string
 * @param[in]   _u32CharNumber  Number of character to process, should be <= orxString_GetLength(_zString)
 * @return      String's ID/hash
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL                  orxString_NHash(const orxSTRING _zString, orxU32 _u32CharNumber);

/** Gets a string's ID (aka hash), without storing the string internally
 * @param[in]   _zString        Concerned string
 * @return      String's ID/hash
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL                  orxString_Hash(const orxSTRING _zString);

/** Gets a string's ID (and stores the string internally to prevent duplication)
 * @param[in]   _zString        Concerned string
 * @return      String's ID
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL                  orxString_GetID(const orxSTRING _zString);

/** Gets a string from an ID (it should have already been stored internally with a call to orxString_GetID)
 * @param[in]   _stID           Concerned string ID
 * @return      orxSTRING if ID's found, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxString_GetFromID(orxSTRINGID _stID);

/** Stores a string internally: equivalent to an optimized call to orxString_GetFromID(orxString_GetID(_zString))
 * @param[in]   _zString        Concerned string
 * @return      Stored orxSTRING
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxString_Store(const orxSTRING _zString);


#ifdef __orxMSVC__

  #pragma warning(default : 4996)

  #undef strtoll
  #undef strtoull

#endif /* __orxMSVC__ */

#undef STRTO_CAST

#endif /* _orxSTRING_H_ */

/** @} */

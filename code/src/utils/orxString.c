/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxString.c
 * @date 21/04/2005
 * @author iarwain@orx-project.org
 *
 */


#include "utils/orxString.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxMemory.h"
#include "utils/orxHashTable.h"

#define XXH_INLINE_ALL
#include "xxhash.h"
#undef XXH_INLINE_ALL

#include <locale.h>


/** Module flags
 */

#define orxSTRING_KU32_STATIC_FLAG_NONE                   0x00000000

#define orxSTRING_KU32_STATIC_FLAG_READY                  0x00000001

#define orxSTRING_KU32_STATIC_MASK_ALL                    0xFFFFFFFF


/** Defines
 */
#define orxSTRING_KU32_ID_TABLE_SIZE                      32768


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSTRING_STATIC_t
{
  orxHASHTABLE *pstIDTable;                               /** String ID table */
  orxU32        u32Flags;                                 /**< Control flags */

} orxSTRING_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxSTRING_STATIC sstString;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** String module setup
 */
void orxFASTCALL orxString_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_STRING, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_STRING, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_STRING, orxMODULE_ID_PROFILER);

  /* Done! */
  return;
}

/** Initializess the string module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxString_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstString.u32Flags & orxSTRING_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstString, sizeof(orxSTRING_STATIC));

    /* Creates ID table */
    sstString.pstIDTable = orxHashTable_Create(orxSTRING_KU32_ID_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(sstString.pstIDTable != orxNULL)
    {
      /* Resets default locale in case it was modified by an external component, such as GTK */
      setlocale(LC_ALL, "C");

      /* Inits Flags */
      sstString.u32Flags = orxSTRING_KU32_STATIC_FLAG_READY;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Couldn't create StringID table.");
    }

    /* Everything's ok */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize string module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the string module
 */
void orxFASTCALL orxString_Exit()
{
  /* Initialized? */
  if(sstString.u32Flags & orxSTRING_KU32_STATIC_FLAG_READY)
  {
    orxU64    u64Key;
    orxHANDLE hIterator;
    orxSTRING zString;

    /* For all string IDs */
    for(hIterator = orxHashTable_GetNext(sstString.pstIDTable, orxHANDLE_UNDEFINED, &u64Key, (void **)&zString);
        hIterator != orxHANDLE_UNDEFINED;
        hIterator = orxHashTable_GetNext(sstString.pstIDTable, hIterator, &u64Key, (void **)&zString))
    {
      /* Deletes its string */
      orxString_Delete(zString);
    }

    /* Deletes ID table */
    orxHashTable_Delete(sstString.pstIDTable);

    /* Updates flags */
    sstString.u32Flags &= ~orxSTRING_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to exit string module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Gets a string's ID (and stores the string internally to prevent duplication)
 * @param[in]   _zString        Concerned string
 * @return      String's ID
 */
orxSTRINGID orxFASTCALL orxString_GetID(const orxSTRING _zString)
{
  const orxSTRING  *pzBucket;
  orxSTRINGID       stResult = 0;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxString_GetID");

  /* Checks */
  orxASSERT(sstString.u32Flags & orxSTRING_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);

  /* Gets its ID */
  stResult = orxString_Hash(_zString);

  /* Gets stored string bucket */
  pzBucket = (const orxSTRING *)orxHashTable_Retrieve(sstString.pstIDTable, stResult);

  /* Checks */
  orxASSERT(pzBucket != orxNULL);

  /* Not already stored? */
  if(*pzBucket == orxNULL)
  {
    /* Adds it */
    *pzBucket = orxString_Duplicate(_zString);
  }
#ifdef __orxDEBUG__
  else
  {
    /* Different strings? */
    if(orxString_Compare(_zString, *pzBucket) != 0)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: string ID collision detected between <%s> and <%s>: please modify one of them or you might end up with undefined result.", *pzBucket, _zString);
    }
  }
#endif /* __orxDEBUG__ */

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return stResult;
}

/** Gets a string from an ID (it should have already been stored internally with a call to orxString_GetID)
 * @param[in]   _stID           Concerned string ID
 * @return      orxSTRING if ID's found, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxString_GetFromID(orxSTRINGID _stID)
{
  const orxSTRING zResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxString_GetFromID");

  /* Gets string from table */
  zResult = (const orxSTRING)orxHashTable_Get(sstString.pstIDTable, _stID);

  /* Invalid? */
  if(zResult == orxNULL)
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return zResult;
}

/** Gets a string's ID (aka hash), without storing the string internally
 * @param[in]   _zString        Concerned string
 * @param[in]   _u32CharNumber  Number of character to process, should be <= orxString_GetLength(_zString)
 * @return      String's ID/hash
 */
orxSTRINGID orxFASTCALL orxString_NHash(const orxSTRING _zString, orxU32 _u32CharNumber)
{
  /* Checks */
  orxASSERT(_zString != orxNULL);
  orxASSERT(_u32CharNumber <= orxString_GetLength(_zString));

  /* Done! */
  return XXH3_64bits(_zString, _u32CharNumber);
}

/** Gets a string's ID (aka hash), without storing the string internally
 * @param[in]   _zString        Concerned string
 * @return      String's ID/hash
 */
orxSTRINGID orxFASTCALL orxString_Hash(const orxSTRING _zString)
{
  /* Done! */
  return orxString_NHash(_zString, orxString_GetLength(_zString));
}

/** Stores a string internally: equivalent to an optimized call to orxString_GetFromID(orxString_GetID(_zString))
 * @param[in]   _zString        Concerned string
 * @return      Stored orxSTRING
 */
const orxSTRING orxFASTCALL orxString_Store(const orxSTRING _zString)
{
  const orxSTRING  *pzBucket;
  const orxSTRING   zResult;
  orxSTRINGID       stID;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxString_Store");

  /* Checks */
  orxASSERT(sstString.u32Flags & orxSTRING_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);

  /* Gets its ID */
  stID = orxString_Hash(_zString);

  /* Gets stored string bucket */
  pzBucket = (const orxSTRING *)orxHashTable_Retrieve(sstString.pstIDTable, stID);

  /* Checks */
  orxASSERT(pzBucket != orxNULL);

  /* Not already stored? */
  if(*pzBucket == orxNULL)
  {
    /* Updates result */
    zResult = orxString_Duplicate(_zString);

    /* Adds it to table */
    *pzBucket = zResult;
  }
  else
  {
    /* Gets it */
    zResult = *pzBucket;

#ifdef __orxDEBUG__
    /* Different strings? */
    if(orxString_Compare(_zString, zResult) != 0)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: string ID collision detected between <%s> and <%s>: please modify one of them or you might end up with undefined result.", zResult, _zString);
    }
#endif /* __orxDEBUG__ */
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return zResult;
}

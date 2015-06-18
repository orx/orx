/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @author bestel@arcallians.org
 *
 */


#include "utils/orxString.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxMemory.h"
#include "utils/orxHashTable.h"


/** Module flags
 */

#define orxSTRING_KU32_STATIC_FLAG_NONE                   0x00000000

#define orxSTRING_KU32_STATIC_FLAG_READY                  0x00000001

#define orxSTRING_KU32_STATIC_MASK_ALL                    0xFFFFFFFF


/** Defines
 */
#define orxSTRING_KU32_ID_TABLE_SIZE                      16384

#define orxSTRING_KU32_ID_BUFFER_SIZE                     131072


/***************************************************************************
 * CRC Tables (slice-by-8)                                                 *
 ***************************************************************************/
orxU32 saau32CRCTable[8][256];


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
  orxU32 i;

  /* For all CRC lookup table entries */
  for(i = 0; i < 256; i++)
  {
    orxU32 u32CRC, j;

    /* For all bits */
    for(u32CRC = i, j = 0; j < 8; j++)
    {
      /* Updates CRC */
      u32CRC = (u32CRC >> 1) ^ ((u32CRC & 1) * orxSTRING_KU32_CRC_POLYNOMIAL);
    }

    /* Stores it */
    saau32CRCTable[0][i] = u32CRC;
  }

  /* For all CRC lookup table entries */
  for(i = 0; i < 256; i++)
  {
    orxU32 u32CRC, j;

    /* Gets original CRC */
    u32CRC = saau32CRCTable[0][i];

    /* For all other CRC lookup tables */
    for(j = 1; j < 8; j++)
    {
      /* Updates CRC */
      u32CRC = saau32CRCTable[0][u32CRC & 0xFF] ^ (u32CRC >> 8);

      /* Stores it */
      saau32CRCTable[j][i] = u32CRC;
    }
  }

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
orxU32 orxFASTCALL orxString_GetID(const orxSTRING _zString)
{
  const orxSTRING zStoredString;
  orxU32          u32Result = 0;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxString_GetID");

  /* Checks */
  orxASSERT(sstString.u32Flags & orxSTRING_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);

  /* Gets its ID */
  u32Result = orxString_ToCRC(_zString);

  /* Not already stored? */
  if((zStoredString = (const orxSTRING)orxHashTable_Get(sstString.pstIDTable, u32Result)) == orxNULL)
  {
    /* Adds it */
    orxHashTable_Add(sstString.pstIDTable, u32Result, orxString_Duplicate(_zString));
  }
#ifdef __orxDEBUG__
  else
  {
    /* Different strings? */
    if(orxString_Compare(_zString, zStoredString) != 0)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: string ID collision detected between <%s> and <%s>: please rename one of them or you might end up with undefined result.", zStoredString, _zString);
    }
  }
#endif /* __orxDEBUG__ */

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return u32Result;
}

/** Gets a string from an ID (it should have already been stored internally with a call to orxString_GetID)
 * @param[in]   _u32ID          Concerned string ID
 * @return      orxSTRING if ID's found, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxString_GetFromID(orxU32 _u32ID)
{
  const orxSTRING zResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxString_GetFromID");

  /* Gets string from table */
  zResult = (const orxSTRING)orxHashTable_Get(sstString.pstIDTable, _u32ID);

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

/** Stores a string internally: equivalent to an optimized call to orxString_GetFromID(orxString_GetID(_zString))
 * @param[in]   _zString        Concerned string
 * @return      Stored orxSTRING
 */
const orxSTRING orxFASTCALL orxString_Store(const orxSTRING _zString)
{
  orxU32          u32ID;
  const orxSTRING zResult;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxString_Store");

  /* Checks */
  orxASSERT(sstString.u32Flags & orxSTRING_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);

  /* Gets its ID */
  u32ID = orxString_ToCRC(_zString);

  /* Gets stored string */
  zResult = (const orxSTRING)orxHashTable_Get(sstString.pstIDTable, u32ID);

  /* Not already stored? */
  if(zResult == orxNULL)
  {
    /* Updates result */
    zResult = orxString_Duplicate(_zString);

    /* Adds it to table */
    orxHashTable_Add(sstString.pstIDTable, u32ID, (orxSTRING)zResult);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return zResult;
}

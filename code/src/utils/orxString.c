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
 * CRC Table                                                               *
 ***************************************************************************/
const orxU32 sau32CRCTable[256] =
{
  0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005,
  0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
  0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC, 0x5BD4B01B, 0x569796C2, 0x52568B75,
  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,
  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039, 0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,
  0xBE2B5B58, 0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,
  0xD4326D90, 0xD0F37027, 0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D,
  0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5, 0x2AC12072,
  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA,
  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C, 0x6211E6B5, 0x66D0FB02,
  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
  0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,
  0x8AAD2B2F, 0x8E6C3698, 0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,
  0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED, 0xD8FBA05A,
  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A, 0x58C1663D, 0x558240E4, 0x51435D53,
  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,
  0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,
  0xF12F560E, 0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,
  0xD727BBB6, 0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B,
  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74, 0x857130C3,
  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,
  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,
  0x119B4BE9, 0x155A565E, 0x18197087, 0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
  0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,
  0xC5A92679, 0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4,
  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662, 0x933EB0BB, 0x97FFAD0C,
  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};


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

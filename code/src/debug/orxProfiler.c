/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
 * @file orxProfiler.c
 * @date 29/04/2011
 * @author iarwain@orx-project.org
 *
 */


#include "debug/orxProfiler.h"

#include "memory/orxMemory.h"
#include "core/orxSystem.h"
#include "utils/orxString.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxPROFILER_KU32_STATIC_FLAG_NONE         0x00000000

#define orxPROFILER_KU32_STATIC_FLAG_READY        0x00000001

#define orxPROFILER_KU32_STATIC_MASK_ALL          0xFFFFFFFF


/** Misc defines
 */
#define orxPROFILER_KS32_MARKER_ID_ROOT           -2


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal marker structure
 */
typedef struct __orxPROFILER_MARKER_t
{
  orxDOUBLE           dTimeStamp, dCumulatedTime;
  orxS32              s32ParentID;
  orxU32              u32Key;
  orxSTRING           zName;

} orxPROFILER_MARKER;


/** Static structure
 */
typedef struct __orxPROFILER_STATIC_t
{
  orxHASHTABLE       *pstMarkerIDTable;
  orxS32              s32MarkerCounter;
  orxS32              s32CurrentMarker;
  orxS32              s32MarkerPopToSkip;
  orxU32              u32Flags;

  orxPROFILER_MARKER  astMarkerList[orxPROFILER_KU32_MAX_MARKER_NUMBER];

} orxPROFILER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxPROFILER_STATIC sstProfiler;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups Profiler module */
void orxFASTCALL orxProfiler_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_SYSTEM);

  return;
}

/** Inits the Profiler module 
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxProfiler_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstProfiler, sizeof(orxPROFILER_STATIC));

    /* Creates marker ID table */
    sstProfiler.pstMarkerIDTable  = orxHashTable_Create(orxPROFILER_KU32_MAX_MARKER_NUMBER, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(sstProfiler.pstMarkerIDTable != orxNULL)
    {
      /* Inits values */
      sstProfiler.s32MarkerCounter = 0;
      sstProfiler.s32CurrentMarker = orxPROFILER_KS32_MARKER_ID_ROOT;

      /* Updates flags */
      sstProfiler.u32Flags |= orxPROFILER_KU32_STATIC_FLAG_READY;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Failed to allocate marker ID table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Tried to initialize profiler module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Profiler module */
void orxFASTCALL orxProfiler_Exit()
{
  /* Initialized? */
  if(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY)
  {
    orxS32 i;

    /* Deletes marker ID table */
    orxHashTable_Delete(sstProfiler.pstMarkerIDTable);

    /* For all existing markers */
    for(i = 0; i < sstProfiler.s32MarkerCounter; i++)
    {
      /* Deletes its name */
      orxString_Delete(sstProfiler.astMarkerList[i].zName);
    }

    /* Updates flags */
    sstProfiler.u32Flags &= ~orxPROFILER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Tried to exit profiler module when it wasn't initialized.");
  }

  return;
}

/** Gets a marker ID given a name
 * @param[in] _zName            Name of the marker
 * @return Marker's ID / orxPROFILER_KS32_MARKER_ID_NONE
 */
orxS32 orxFASTCALL orxProfiler_GetID(const orxSTRING _zName)
{
  orxU32 u32MarkerKey;
  orxS32 s32MarkerID;

  /* Gets name CRC as key */
  u32MarkerKey = orxString_ToCRC(_zName);

  /* Can't find marker? */
  if((s32MarkerID = (orxS32)orxHashTable_Get(sstProfiler.pstMarkerIDTable, u32MarkerKey)) == 0)
  {
    /* Has free marker IDs? */
    if(sstProfiler.s32MarkerCounter < orxPROFILER_KU32_MAX_MARKER_NUMBER)
    {
      /* Gets marker ID */
      s32MarkerID = sstProfiler.s32MarkerCounter++;

      /* Adds it to table */
      orxHashTable_Add(sstProfiler.pstMarkerIDTable, u32MarkerKey, (void *)(s32MarkerID + 1));

      /* Inits it */
      sstProfiler.astMarkerList[s32MarkerID].dTimeStamp     = orxFLOAT_0;
      sstProfiler.astMarkerList[s32MarkerID].dCumulatedTime = orxFLOAT_0;
      sstProfiler.astMarkerList[s32MarkerID].s32ParentID    = orxPROFILER_KS32_MARKER_ID_NONE;
      sstProfiler.astMarkerList[s32MarkerID].u32Key         = u32MarkerKey;
      sstProfiler.astMarkerList[s32MarkerID].zName          = orxString_Duplicate(_zName);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get a marker ID for <%s> as the limit of %ld markers has been reached!", _zName, orxPROFILER_KU32_MAX_MARKER_NUMBER);

      /* Updates marker ID */
      s32MarkerID = orxPROFILER_KS32_MARKER_ID_NONE;
    }
  }
  else
  {
    /* Gets real ID */
    s32MarkerID--;
  }

  /* Done! */
  return s32MarkerID;
}

/** Pushes a marker (on a stack) and starts a timer for it
 * @param[in] _s32MarkerID      ID of the marker to push
 */
void orxFASTCALL orxProfiler_PushMarker(orxS32 _s32MarkerID)
{
  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    /* Not already pushed? */
    if(sstProfiler.astMarkerList[_s32MarkerID].s32ParentID == orxPROFILER_KS32_MARKER_ID_NONE)
    {
      /* Updates parent marker */
      sstProfiler.astMarkerList[_s32MarkerID].s32ParentID = sstProfiler.s32CurrentMarker;

      /* Updates current marker */
      sstProfiler.s32CurrentMarker = _s32MarkerID;

      /* Gets time stamp */
      sstProfiler.astMarkerList[_s32MarkerID].dTimeStamp = orxSystem_GetTime();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't push marker <%s> [ID: %ld] as it's already pushed.", sstProfiler.astMarkerList[_s32MarkerID].zName, _s32MarkerID);

      /* Updates marker pops to skip */
      sstProfiler.s32MarkerPopToSkip++;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't push marker: invalid ID [%ld].", _s32MarkerID);

    /* Updates marker pops to skip */
    sstProfiler.s32MarkerPopToSkip++;
  }
}

/** Pops a marker (from the stack) and updates its cumulated time (using the last marker push time)
 */
void orxFASTCALL orxProfiler_PopMarker()
{
  /* Should skip pop? */
  if(sstProfiler.s32MarkerPopToSkip > 0)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Skipping marker pop.");

    /* Updates it */
    sstProfiler.s32MarkerPopToSkip--;
  }
  else
  {
    /* Has pushed marker? */
    if(sstProfiler.s32CurrentMarker >= 0)
    {
      orxS32 s32ParentMarker;

      /* Updates cumulated time */
      sstProfiler.astMarkerList[sstProfiler.s32CurrentMarker].dCumulatedTime += orxSystem_GetTime() - sstProfiler.astMarkerList[sstProfiler.s32CurrentMarker].dTimeStamp;

      /* Pops previous marker */
      s32ParentMarker = sstProfiler.astMarkerList[sstProfiler.s32CurrentMarker].s32ParentID;
      sstProfiler.astMarkerList[sstProfiler.s32CurrentMarker].s32ParentID = orxPROFILER_KS32_MARKER_ID_NONE;
      sstProfiler.s32CurrentMarker = s32ParentMarker;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't pop marker: marker stack is empty.");
    }
  }
}

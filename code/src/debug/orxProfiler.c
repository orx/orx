/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2014 Orx-Project
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
#include "core/orxThread.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxPROFILER_KU32_STATIC_FLAG_NONE         0x00000000

#define orxPROFILER_KU32_STATIC_FLAG_READY        0x00000001
#define orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS   0x10000000
#define orxPROFILER_KU32_STATIC_FLAG_PAUSED       0x20000000

#define orxPROFILER_KU32_STATIC_MASK_ALL          0xFFFFFFFF


/** Marker info flags
 */
#define orxPROFILER_KU32_FLAG_NONE                0x00000000

#define orxPROFILER_KU32_FLAG_UNIQUE              0x00000001
#define orxPROFILER_KU32_FLAG_PUSHED              0x00000002
#define orxPROFILER_KU32_FLAG_INIT                0x00000004

#define orxPROFILER_KU16_MASK_ALL                 0xFFFFFFFF

/** Misc defines
 */
#define orxPROFILER_KU32_MAX_MARKER_NUMBER        (orxPROFILER_KU32_MASK_MARKER_ID + 1)
#define orxPROFILER_KU32_SHIFT_MARKER_ID          7
#define orxPROFILER_KU32_MASK_MARKER_ID           0x7F
#define orxPROFILER_KS32_MARKER_ID_ROOT           -2
#define orxPROFILER_KU32_MARKER_DATA_NUMBER       orxTHREAD_KU32_MAX_THREAD_NUMBER


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal marker info structure
 */
typedef struct __orxPROFILER_MARKER_INFO_t
{
  orxDOUBLE               dFirstTimeStamp;
  orxDOUBLE               dCumulatedTime;
  orxDOUBLE               dMaxCumulatedTime;
  orxU32                  u32PushCounter;
  orxU32                  u32Depth;

} orxPROFILER_MARKER_INFO;

/** Internal marker structure
 */
typedef struct __orxPROFILER_MARKER_t
{
  orxDOUBLE               dTimeStamp;
  orxSTRING               zName;
  orxS32                  s32ParentID;
  orxU32                  u32NameID;
  orxU32                  u32Flags;

} orxPROFILER_MARKER;

typedef struct __orxPROFILER_MARKER_DATA_t
{
  orxPROFILER_MARKER_INFO aastInfoListHistory[orxPROFILER_KU32_HISTORY_LENGTH][orxPROFILER_KU32_MAX_MARKER_NUMBER];
  orxPROFILER_MARKER      astMarkerList[orxPROFILER_KU32_MAX_MARKER_NUMBER];
  orxDOUBLE               adTimeStampHistory[orxPROFILER_KU32_HISTORY_LENGTH];
  orxS32                  s32MarkerCounter;
  orxS32                  s32CurrentMarker;
  orxS32                  s32MarkerPopToSkip;
  orxU32                  u32CurrentMarkerDepth;

} orxPROFILER_MARKER_DATA;


/** Static structure
 */
typedef struct __orxPROFILER_STATIC_t
{
  orxDOUBLE               dMaxResetTime;
  orxS32                  s32WaterStamp;
  orxU32                  u32HistoryIndex;
  orxU32                  u32HistoryQueryIndex;
  orxU32                  u32QueryDataIndex;
  orxU32                  u32Flags;

  orxPROFILER_MARKER_DATA *apstMarkerDataList[orxPROFILER_KU32_MARKER_DATA_NUMBER];

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

static orxINLINE orxPROFILER_MARKER_DATA *orxProfiler_GetMarkerData(orxU32 _u32ID, orxBOOL _bReadOnly)
{
  orxPROFILER_MARKER_DATA *pstResult;

  /* Checks */
  orxASSERT(_u32ID < orxPROFILER_KU32_MARKER_DATA_NUMBER);

  /* Gets marker data */
  pstResult = sstProfiler.apstMarkerDataList[_u32ID];

  /* Not yet created? */
  if((pstResult == orxNULL) && (_bReadOnly == orxFALSE))
  {
    /* Allocates it */
    pstResult = (orxPROFILER_MARKER_DATA *)orxMemory_Allocate(sizeof(orxPROFILER_MARKER_DATA), orxMEMORY_TYPE_MAIN);

    /* Checks */
    orxASSERT(pstResult != orxNULL);

    /* Inits it */
    orxMemory_Zero(pstResult, sizeof(orxPROFILER_MARKER_DATA));
    pstResult->adTimeStampHistory[sstProfiler.u32HistoryIndex] = orxSystem_GetTime();
    pstResult->s32CurrentMarker = orxPROFILER_KS32_MARKER_ID_ROOT;

    /* Stores it */
    orxMEMORY_BARRIER();
    sstProfiler.apstMarkerDataList[_u32ID] = pstResult;
  }

  /* Done! */
  return pstResult;
}

static orxINLINE orxPROFILER_MARKER_DATA *orxProfiler_GetCurrentMarkerData()
{
  orxU32                    u32ThreadID;
  orxPROFILER_MARKER_DATA  *pstResult;

  /* Gets current thread ID */
  u32ThreadID = orxThread_GetCurrent();

  /* Gets its marker data */
  pstResult = orxProfiler_GetMarkerData(u32ThreadID, orxFALSE);

  /* Done! */
  return pstResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups Profiler module */
void orxFASTCALL orxProfiler_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_SYSTEM);
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_THREAD);

  /* Done! */
  return;
}

/** Inits the Profiler module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxProfiler_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxMath_IsPowerOfTwo(orxPROFILER_KU32_MAX_MARKER_NUMBER) != orxFALSE);

  /* Not already Initialized? */
  if(!(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstProfiler, sizeof(orxPROFILER_STATIC));

    /* Creates data for main thread */
    orxProfiler_GetMarkerData(orxTHREAD_KU32_MAIN_THREAD_ID, orxFALSE);

    /* Gets water stamp */
    sstProfiler.s32WaterStamp = ((orxS32)(orxSystem_GetSystemTime() * 1e3)) << orxPROFILER_KU32_SHIFT_MARKER_ID;

    /* Updates flags */
    sstProfiler.u32Flags = orxPROFILER_KU32_STATIC_FLAG_READY | orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS;

    /* Selects query frame */
    orxProfiler_SelectQueryFrame(0, orxTHREAD_KU32_MAIN_THREAD_ID);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
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
    orxU32 i;

    /* For all marker data */
    for(i = 0; i < orxPROFILER_KU32_MARKER_DATA_NUMBER; i++)
    {
      orxPROFILER_MARKER_DATA *pstData;

      /* Gets it */
      pstData = sstProfiler.apstMarkerDataList[i];

      /* Valid? */
      if(pstData != orxNULL)
      {
        orxS32 j;

        /* For all existing markers */
        for(j = 0; j < pstData->s32MarkerCounter; j++)
        {
          /* Deletes its name */
          orxString_Delete(pstData->astMarkerList[j].zName);
        }

        /* Deletes data */
        orxMemory_Free(pstData);
        sstProfiler.apstMarkerDataList[i] = orxNULL;
      }
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
orxS32 orxFASTCALL orxProfiler_GetIDFromName(const orxSTRING _zName)
{
  orxS32                    s32MarkerID;
  orxU32                    u32NameID;
  orxPROFILER_MARKER_DATA  *pstData;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Gets current marker data */
  pstData = orxProfiler_GetCurrentMarkerData();

  /* Gets name ID */
  u32NameID = orxString_ToCRC(_zName);

  /* For all markers */
  for(s32MarkerID = 0; s32MarkerID < pstData->s32MarkerCounter; s32MarkerID++)
  {
    /* Matches? */
    if(u32NameID == pstData->astMarkerList[s32MarkerID].u32NameID)
    {
      /* Stops */
      break;
    }
  }

  /* Not found? */
  if(s32MarkerID >= pstData->s32MarkerCounter)
  {
    /* Has free marker IDs? */
    if(pstData->s32MarkerCounter < orxPROFILER_KU32_MAX_MARKER_NUMBER)
    {
      /* Updates marker counter */
      pstData->s32MarkerCounter++;

      /* Inits it */
      pstData->astMarkerList[s32MarkerID].zName        = orxString_Duplicate(_zName);
      pstData->astMarkerList[s32MarkerID].s32ParentID  = orxPROFILER_KS32_MARKER_ID_NONE;
      pstData->astMarkerList[s32MarkerID].u32NameID    = u32NameID;
      pstData->astMarkerList[s32MarkerID].u32Flags     = orxPROFILER_KU32_FLAG_UNIQUE;

      /* Stamps result */
      s32MarkerID |= sstProfiler.s32WaterStamp;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get a marker ID for <%s> as the limit of %d markers has been reached!", _zName, orxPROFILER_KU32_MAX_MARKER_NUMBER);

      /* Updates marker ID */
      s32MarkerID = orxPROFILER_KS32_MARKER_ID_NONE;
    }
  }
  else
  {
    /* Stamps result */
    s32MarkerID |= sstProfiler.s32WaterStamp;
  }

  /* Done! */
  return s32MarkerID;
}

/** Is the given marker valid? (Useful when storing markers in static variables and still allow normal hot restart)
 * @param[in] _s32MarkerID      ID of the marker to test
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxProfiler_IsMarkerIDValid(orxS32 _s32MarkerID)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = ((_s32MarkerID & ~orxPROFILER_KU32_MASK_MARKER_ID) == sstProfiler.s32WaterStamp) ? orxTRUE : orxFALSE;

  /* Done! */
  return bResult;
}

/** Pushes a marker (on a stack) and starts a timer for it
 * @param[in] _s32MarkerID      ID of the marker to push
 */
void orxFASTCALL orxProfiler_PushMarker(orxS32 _s32MarkerID)
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Not paused and are operations enabled? */
  if(orxFLAG_GET(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_PAUSED | orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS) == orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS)
  {
    orxS32                    s32ID;
    orxPROFILER_MARKER_DATA  *pstData;

    /* Gets current marker data */
    pstData = orxProfiler_GetCurrentMarkerData();

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      orxPROFILER_MARKER *pstMarker;
      orxPROFILER_MARKER_INFO  *pstInfo;

      /* Gets marker & current info */
      pstMarker = &(pstData->astMarkerList[s32ID]);
      pstInfo   = &(pstData->aastInfoListHistory[sstProfiler.u32HistoryIndex][s32ID]);

      /* Not already pushed? */
      if(!orxFLAG_TEST(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_PUSHED))
      {
        orxDOUBLE dTimeStamp;
        orxBOOL   bFirstTime;

        /* Updates first time status */
        bFirstTime = orxFLAG_TEST(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_INIT) ? orxFALSE : orxTRUE;

        /* Is unique and already by someone else pushed? */
        if(orxFLAG_TEST(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_UNIQUE) && (pstInfo->u32PushCounter != 0) && (pstMarker->s32ParentID != pstData->s32CurrentMarker))
        {
          orxS32 i;

          /* Updates flags */
          orxFLAG_SET(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_PUSHED|orxPROFILER_KU32_FLAG_INIT, orxPROFILER_KU32_FLAG_UNIQUE);

          /* For all markers */
          for(i = 0; i < pstData->s32MarkerCounter; i++)
          {
            orxPROFILER_MARKER *pstTestMarker;

            /* Gets it */
            pstTestMarker = &(pstData->astMarkerList[i]);

            /* Is child of current marker? */
            if(pstTestMarker->s32ParentID == s32ID)
            {
              /* Updates its depth */
              pstData->aastInfoListHistory[sstProfiler.u32HistoryIndex][i].u32Depth--;
            }
          }
        }
        else
        {
          /* Updates flags */
          orxFLAG_SET(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_PUSHED|orxPROFILER_KU32_FLAG_INIT, orxPROFILER_KU32_FLAG_NONE);
        }

        /* Is unique? */
        if(orxFLAG_TEST(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_UNIQUE))
        {
          /* Stores its push depth */
          pstInfo->u32Depth = ++pstData->u32CurrentMarkerDepth;
        }
        else
        {
          /* Clears push depth */
          pstInfo->u32Depth = 0;
        }

        /* Updates its push counter */
        pstInfo->u32PushCounter++;

        /* Updates parent marker */
        pstMarker->s32ParentID = pstData->s32CurrentMarker;

        /* Updates current marker */
        pstData->s32CurrentMarker = s32ID;

        /* Gets time stamp */
        dTimeStamp = orxSystem_GetTime();

        /* First time? */
        if(bFirstTime != orxFALSE)
        {
          /* Stores initial time */
          pstInfo->dFirstTimeStamp = dTimeStamp;
        }

        /* Stores time stamp */
        pstMarker->dTimeStamp = dTimeStamp;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't push marker <%s> [ID: %d] as it's already currently pushed.", pstMarker->zName, _s32MarkerID);

        /* Updates marker pops to skip */
        pstData->s32MarkerPopToSkip++;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't push marker: invalid ID [%d].", _s32MarkerID);

      /* Updates marker pops to skip */
      pstData->s32MarkerPopToSkip++;
    }
  }
}

/** Pops a marker (from the stack) and updates its cumulated time (using the last marker push time)
 */
void orxFASTCALL orxProfiler_PopMarker()
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Not paused and are operations enabled? */
  if(orxFLAG_GET(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_PAUSED | orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS) == orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS)
  {
    orxPROFILER_MARKER_DATA *pstData;

    /* Gets current marker data */
    pstData = orxProfiler_GetCurrentMarkerData();

    /* Should skip pop? */
    if(pstData->s32MarkerPopToSkip > 0)
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Skipping marker pop.");

      /* Updates it */
      pstData->s32MarkerPopToSkip--;
    }
    else
    {
      /* Has pushed marker? */
      if(pstData->s32CurrentMarker >= 0)
      {
        orxPROFILER_MARKER *pstMarker;
        orxPROFILER_MARKER_INFO  *pstInfo;

        /* Gets marker & current info */
        pstMarker = &(pstData->astMarkerList[pstData->s32CurrentMarker]);
        pstInfo   = &(pstData->aastInfoListHistory[sstProfiler.u32HistoryIndex][pstData->s32CurrentMarker]);

        /* Updates cumulated time */
        pstInfo->dCumulatedTime += orxSystem_GetTime() - pstMarker->dTimeStamp;

        /* Updates max cumulated time */
        if(pstInfo->dCumulatedTime > pstInfo->dMaxCumulatedTime)
        {
          pstInfo->dMaxCumulatedTime = pstInfo->dCumulatedTime;
        }

        /* Pops previous marker */
        pstData->s32CurrentMarker = pstMarker->s32ParentID;

        /* Is unique? */
        if(orxFLAG_TEST(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_UNIQUE))
        {
          /* Updates push depth */
          pstData->u32CurrentMarkerDepth--;
        }

        /* Updates flags */
        orxFLAG_SET(pstMarker->u32Flags, orxPROFILER_KU32_FLAG_NONE, orxPROFILER_KU32_FLAG_PUSHED);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't pop marker: marker stack is empty.");
      }
    }
  }
}

/** Enables marker push/pop operations
 * @param[in] _bEnable          Enable
 */
void orxFASTCALL orxProfiler_EnableMarkerOperations(orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates flags */
    orxFLAG_SET(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS, orxPROFILER_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Updates flags */
    orxFLAG_SET(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_NONE, orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS);
  }
}

/** Are marker push/pop operations enabled?
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxProfiler_AreMarkerOperationsEnabled()
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Done! */
  return orxFLAG_TEST(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_ENABLE_OPS) ? orxTRUE : orxFALSE;
}

/** Pauses/unpauses the profiler
 * @param[in] _bPause           Pause
 */
void orxFASTCALL orxProfiler_Pause(orxBOOL _bPause)
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Pause? */
  if(_bPause != orxFALSE)
  {
    /* Updates flags */
    orxFLAG_SET(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_PAUSED, orxPROFILER_KU32_STATIC_FLAG_NONE);
  }
  else
  {
    /* Updates flags */
    orxFLAG_SET(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_NONE, orxPROFILER_KU32_STATIC_FLAG_PAUSED);
  }
}

/** Is profiler paused?
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxProfiler_IsPaused()
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Done! */
  return orxFLAG_TEST(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_PAUSED) ? orxTRUE : orxFALSE;
}

/** Resets all markers (usually called at the end of the frame)
 */
void orxFASTCALL orxProfiler_ResetAllMarkers()
{
  /* Not paused? */
  if(!orxFLAG_TEST(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_PAUSED))
  {
    orxU32 i, u32PreviousIndex;

    /* Checks */
    orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

    /* Updates history indices */
    u32PreviousIndex                  = sstProfiler.u32HistoryIndex;
    sstProfiler.u32HistoryQueryIndex  = sstProfiler.u32HistoryIndex;
    sstProfiler.u32HistoryIndex       = (sstProfiler.u32HistoryIndex + 1) % orxPROFILER_KU32_HISTORY_LENGTH;

    /* For all marker data */
    for(i = 0; i < orxPROFILER_KU32_MARKER_DATA_NUMBER; i++)
    {
      orxPROFILER_MARKER_DATA  *pstData;

      /* Gets it */
      pstData = sstProfiler.apstMarkerDataList[i];

      /* Valid? */
      if(pstData != orxNULL)
      {
        orxS32 j;

        /* For all markers */
        for(j = 0; j < pstData->s32MarkerCounter; j++)
        {
          orxPROFILER_MARKER_INFO *pstInfo;

          /* Gets its info */
          pstInfo = &(pstData->aastInfoListHistory[sstProfiler.u32HistoryIndex][j]);

          /* Inits it */
          pstInfo->dFirstTimeStamp    = orxDOUBLE_0;
          pstInfo->dCumulatedTime     = orxDOUBLE_0;
          pstInfo->dMaxCumulatedTime  = pstData->aastInfoListHistory[u32PreviousIndex][j].dMaxCumulatedTime;
          pstInfo->u32PushCounter     = 0;
          pstInfo->u32Depth           = 0;

          /* Resets marker */
          pstData->astMarkerList[j].dTimeStamp   = orxDOUBLE_0;
          pstData->astMarkerList[j].s32ParentID  = orxPROFILER_KS32_MARKER_ID_NONE;
          orxFLAG_SET(pstData->astMarkerList[j].u32Flags, orxPROFILER_KU32_FLAG_NONE, orxPROFILER_KU32_FLAG_PUSHED|orxPROFILER_KU32_FLAG_INIT);
        }

        /* Updates new time stamps */
        pstData->adTimeStampHistory[sstProfiler.u32HistoryIndex] = orxSystem_GetTime();
      }
    }
  }

  /* Done! */
  return;
}

/** Resets all maxima (usually called at a regular interval)
 */
void orxFASTCALL orxProfiler_ResetAllMaxima()
{
  /* Not paused? */
  if(!orxFLAG_TEST(sstProfiler.u32Flags, orxPROFILER_KU32_STATIC_FLAG_PAUSED))
  {
    orxU32 i;

    /* Checks */
    orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

    /* For all marker data */
    for(i = 0; i < orxPROFILER_KU32_MARKER_DATA_NUMBER; i++)
    {
      orxPROFILER_MARKER_DATA  *pstData;

      /* Gets it */
      pstData = sstProfiler.apstMarkerDataList[i];

      /* Valid? */
      if(pstData != orxNULL)
      {
        orxS32 j;

        /* For all markers */
        for(j = 0; j < pstData->s32MarkerCounter; j++)
        {
          /* Resets its maximum */
          pstData->aastInfoListHistory[sstProfiler.u32HistoryIndex][j].dMaxCumulatedTime = pstData->aastInfoListHistory[sstProfiler.u32HistoryIndex][j].dCumulatedTime;
        }

      }
    }

    /* Resets max reset time */
    sstProfiler.dMaxResetTime = orxDOUBLE_0;
  }

  /* Done! */
  return;
}

/** Gets the time elapsed between the last two resets
 * @return Time elapsed between the last two resets, in seconds
 */
orxDOUBLE orxFASTCALL orxProfiler_GetResetTime()
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxDOUBLE                 dResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Gets current marker data */
  pstData = orxProfiler_GetMarkerData(orxTHREAD_KU32_MAIN_THREAD_ID, orxFALSE);

  /* Updates result */
  dResult = pstData->adTimeStampHistory[(sstProfiler.u32HistoryQueryIndex + 1) % orxPROFILER_KU32_HISTORY_LENGTH] - pstData->adTimeStampHistory[sstProfiler.u32HistoryQueryIndex];

  /* Longer than previous? */
  if(dResult > sstProfiler.dMaxResetTime)
  {
    /* Stores it */
    sstProfiler.dMaxResetTime = dResult;
  }

  /* Done! */
  return dResult;
}

/** Gets the maximum reset time
 * @return Max reset time, in seconds
 */
orxDOUBLE orxFASTCALL orxProfiler_GetMaxResetTime()
{
  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstProfiler.dMaxResetTime;
}

/** Gets the number of registered markers
 * @return Number of registered markers
 */
orxS32 orxFASTCALL orxProfiler_GetMarkerCounter()
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxS32                    s32Result;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Updates result */
  s32Result = (pstData != orxNULL) ? pstData->s32MarkerCounter : 0;

  /* Done! */
  return s32Result;
}

/** Gets the next registered marker ID
 * @param[in] _s32MarkerID      ID of the current marker, orxPROFILER_KS32_MARKER_ID_NONE to get the first one
 * @return Next registered marker's ID / orxPROFILER_KS32_MARKER_ID_NONE if the current marker was the last one
 */
orxS32 orxFASTCALL orxProfiler_GetNextMarkerID(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxS32                    s32Result;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT((_s32MarkerID == orxPROFILER_KS32_MARKER_ID_NONE) ||(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE));

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    /* No marker? */
    if(_s32MarkerID == orxPROFILER_KS32_MARKER_ID_NONE)
    {
      /* Updates result */
      s32Result = sstProfiler.s32WaterStamp;
    }
    else
    {
      /* Updates result */
      s32Result = _s32MarkerID + 1;
    }

    /* Not valid? */
    if((s32Result & orxPROFILER_KU32_MASK_MARKER_ID) >= pstData->s32MarkerCounter)
    {
      /* Updates result */
      s32Result = orxPROFILER_KS32_MARKER_ID_NONE;
    }
  }
  else
  {
    /* Updates result */
    s32Result = orxPROFILER_KS32_MARKER_ID_NONE;
  }

  /* Done! */
  return s32Result;
}

/** Gets the ID of the next marker, sorted by their push time
 * @param[in] _s32MarkerID      ID of the current pushed marker, orxPROFILER_KS32_MARKER_ID_NONE to get the first one
 * @return Next registered marker's ID / orxPROFILER_KS32_MARKER_ID_NONE if the current marker was the last one
 */
orxS32 orxFASTCALL orxProfiler_GetNextSortedMarkerID(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxS32                    s32Result = orxPROFILER_KS32_MARKER_ID_NONE;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT((_s32MarkerID == orxPROFILER_KS32_MARKER_ID_NONE) ||(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE));

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxDOUBLE dPreviousTime, dBestTime;
    orxS32    i, s32ID;

    /* Valid ID? */
    if(_s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE)
    {
      /* Gets ID */
      s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

      /* Get previous marker's time stamp */
      dPreviousTime = (s32ID < pstData->s32MarkerCounter) ? pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][s32ID].dFirstTimeStamp : orxDOUBLE_0;
    }
    else
    {
      /* Gets ID */
      s32ID = orxPROFILER_KS32_MARKER_ID_NONE;

      /* Get previous marker's time stamp */
      dPreviousTime = orxDOUBLE_0;
    }

    /* For all markers */
    for(i = 0, dBestTime = orxSystem_GetTime(); i < pstData->s32MarkerCounter; i++)
    {
      orxDOUBLE dTime;

      /* Gets its time */
      dTime = pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][i].dFirstTimeStamp;

      /* Is better candidate? */
      if((((dTime == dPreviousTime)
        && (i > s32ID))
       || (dTime > dPreviousTime))
      && (dTime < dBestTime))
      {
        /* Updates result */
        s32Result = i | sstProfiler.s32WaterStamp;
        dBestTime = dTime;
      }
    }
  }

  /* Done! */
  return s32Result;
}

/** Sets the query frame for all GetMarker* functions below, in number of frame elapsed from the last one
 * @param[in] _u32QueryFrame    Query frame, in number of frame elapsed since the last one (ie. 0 -> last frame, 1 -> frame before last, ...)
 * @param[in] _u32ThreadID      Concerned thread ID, if no data is found for this thread, orxSTATUS_FAILURE is returned
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILRE
 */
orxSTATUS orxFASTCALL orxProfiler_SelectQueryFrame(orxU32 _u32QueryFrame, orxU32 _u32ThreadID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxSTATUS                 eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(_u32ThreadID, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    /* Is frame valid? */
    if(_u32QueryFrame < orxPROFILER_KU32_HISTORY_LENGTH - 1)
    {
      /* Computes index */
      sstProfiler.u32HistoryQueryIndex = ((sstProfiler.u32HistoryIndex + orxPROFILER_KU32_HISTORY_LENGTH - 1) - _u32QueryFrame) % orxPROFILER_KU32_HISTORY_LENGTH;

      /* Stores thread ID */
      sstProfiler.u32QueryDataIndex = _u32ThreadID;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Gets the marker's cumulated time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's cumulated time
 */
orxDOUBLE orxFASTCALL orxProfiler_GetMarkerTime(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxDOUBLE                 dResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Updates result */
      dResult = pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][s32ID].dCumulatedTime;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      dResult = orxDOUBLE_0;
    }
  }
  else
  {
    /* Updates result */
    dResult = orxDOUBLE_0;
  }

  /* Done! */
  return dResult;
}

/** Gets the marker's maximum cumulated time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's max cumulated time
 */
orxDOUBLE orxFASTCALL orxProfiler_GetMarkerMaxTime(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxDOUBLE                 dResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Updates result */
      dResult = pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][s32ID].dMaxCumulatedTime;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      dResult = orxDOUBLE_0;
    }
  }
  else
  {
    /* Updates result */
    dResult = orxDOUBLE_0;
  }

  /* Done! */
  return dResult;
}

/** Gets the marker's name
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's name
 */
const orxSTRING orxFASTCALL orxProfiler_GetMarkerName(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  const orxSTRING           zResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Updates result */
      zResult = pstData->astMarkerList[s32ID].zName;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Gets the marker's push counter
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Number of time the marker has been pushed since last reset
 */
orxU32 orxFASTCALL orxProfiler_GetMarkerPushCounter(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxU32                    u32Result;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Updates result */
      u32Result = pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][s32ID].u32PushCounter;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      u32Result = 0;
    }
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Has the marker been pushed by a unique parent?
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxProfiler_IsUniqueMarker(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxBOOL                   bResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Updates result */
      bResult = orxFLAG_TEST(pstData->astMarkerList[s32ID].u32Flags, orxPROFILER_KU32_FLAG_UNIQUE) ? orxTRUE : orxFALSE;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      bResult = orxFALSE;
    }
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets the uniquely pushed marker's start time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's start time / 0.0
 */
orxDOUBLE orxFASTCALL orxProfiler_GetUniqueMarkerStartTime(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxDOUBLE                 dResult;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Is unique? */
      if(orxFLAG_TEST(pstData->astMarkerList[s32ID].u32Flags, orxPROFILER_KU32_FLAG_UNIQUE))
      {
        /* Updates result */
        dResult = pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][s32ID].dFirstTimeStamp;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get start time of marker <%s> [ID: %d] as it hasn't been uniquely pushed.", pstData->astMarkerList[s32ID].zName, _s32MarkerID);

        /* Updates result */
        dResult = orxDOUBLE_0;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      dResult = orxDOUBLE_0;
    }
  }
  else
  {
    /* Updates result */
    dResult = orxDOUBLE_0;
  }

  /* Done! */
  return dResult;
}

/** Gets the uniquely pushed marker's depth, 1 being the depth of the top level
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's push depth / 0 if this marker hasn't been uniquely pushed
 */
orxU32 orxFASTCALL orxProfiler_GetUniqueMarkerDepth(orxS32 _s32MarkerID)
{
  orxPROFILER_MARKER_DATA  *pstData;
  orxU32                    u32Result;

  /* Checks */
  orxASSERT(sstProfiler.u32Flags & orxPROFILER_KU32_STATIC_FLAG_READY);
  orxASSERT(orxProfiler_IsMarkerIDValid(_s32MarkerID) != orxFALSE);

  /* Gets marker data */
  pstData = orxProfiler_GetMarkerData(sstProfiler.u32QueryDataIndex, orxTRUE);

  /* Valid? */
  if(pstData != orxNULL)
  {
    orxS32 s32ID;

    /* Gets ID */
    s32ID = _s32MarkerID & orxPROFILER_KU32_MASK_MARKER_ID;

    /* Valid marker ID? */
    if((s32ID >= 0) && (s32ID < pstData->s32MarkerCounter))
    {
      /* Is unique? */
      if(orxFLAG_TEST(pstData->astMarkerList[s32ID].u32Flags, orxPROFILER_KU32_FLAG_UNIQUE))
      {
        /* Updates result */
        u32Result = pstData->aastInfoListHistory[sstProfiler.u32HistoryQueryIndex][s32ID].u32Depth;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get push depth of marker <%s> [ID: %d] as it hasn't been uniquely pushed.", pstData->astMarkerList[s32ID].zName, _s32MarkerID);

        /* Updates result */
        u32Result = 0;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%d].", _s32MarkerID);

      /* Updates result */
      u32Result = 0;
    }
  }
  else
  {
    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

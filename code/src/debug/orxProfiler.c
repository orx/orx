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


/** Module flags
 */
#define orxPROFILER_KU32_STATIC_FLAG_NONE         0x00000000

#define orxPROFILER_KU32_STATIC_FLAG_READY        0x00000001

#define orxPROFILER_KU32_STATIC_MASK_ALL          0xFFFFFFFF


/** Marker info flags
 */
#define orxPROFILER_KU16_FLAG_NONE                0x0000

#define orxPROFILER_KU16_FLAG_UNIQUE              0x0001
#define orxPROFILER_KU16_FLAG_PUSHED              0x0002

#define orxPROFILER_KU16_MASK_ALL                 0xFFFF

/** Misc defines
 */
#define orxPROFILER_KS32_MARKER_ID_ROOT           -2


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal marker structure
 */
typedef struct __orxPROFILER_MARKER_INFO_t
{
  orxDOUBLE           dTimeStamp;
  orxDOUBLE           dCumulatedTime;
  orxS32              s32ParentID;
  orxU32              u32PushCounter;
  orxSTRING           zName;
  orxU16              u16Depth;
  orxU16              u16Flags;

} orxPROFILER_MARKER;


/** Static structure
 */
typedef struct __orxPROFILER_STATIC_t
{
  orxDOUBLE           dTimeStamp;
  orxS32              s32MarkerCounter;
  orxS32              s32CurrentMarker;
  orxS32              s32MarkerPopToSkip;
  orxU32              u32Flags;
  orxU16              u16CurrentMarkerDepth;

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
  orxModule_AddDependency(orxMODULE_ID_PROFILER, orxMODULE_ID_SYSTEM);

  /* Done! */
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

    /* Gets time stamp */
    sstProfiler.dTimeStamp = orxSystem_GetTime();

    /* Inits current marker */
    sstProfiler.s32CurrentMarker = orxPROFILER_KS32_MARKER_ID_ROOT;

    /* Updates flags */
    sstProfiler.u32Flags |= orxPROFILER_KU32_STATIC_FLAG_READY;

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
    orxS32 i;

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
orxS32 orxFASTCALL orxProfiler_GetIDFromName(const orxSTRING _zName)
{
  orxS32 s32MarkerID;

  /* For all markers */
  for(s32MarkerID = 0; s32MarkerID < sstProfiler.s32MarkerCounter; s32MarkerID++)
  {
    /* Matches? */
    if(!orxString_Compare(_zName, sstProfiler.astMarkerList[s32MarkerID].zName))
    {
      /* Stops */
      break;
    }
  }

  /* Not found? */
  if(s32MarkerID >= sstProfiler.s32MarkerCounter)
  {
    /* Has free marker IDs? */
    if(sstProfiler.s32MarkerCounter < orxPROFILER_KU32_MAX_MARKER_NUMBER)
    {
      /* Updates marker counter */
      sstProfiler.s32MarkerCounter++;

      /* Inits it */
      sstProfiler.astMarkerList[s32MarkerID].dTimeStamp     = orx2D(0.0);
      sstProfiler.astMarkerList[s32MarkerID].dCumulatedTime = orx2D(0.0);
      sstProfiler.astMarkerList[s32MarkerID].s32ParentID    = orxPROFILER_KS32_MARKER_ID_NONE;
      sstProfiler.astMarkerList[s32MarkerID].u32PushCounter = 0;
      sstProfiler.astMarkerList[s32MarkerID].zName          = orxString_Duplicate(_zName);
      sstProfiler.astMarkerList[s32MarkerID].u16Depth       = 0;
      sstProfiler.astMarkerList[s32MarkerID].u16Flags       = orxPROFILER_KU16_FLAG_UNIQUE;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get a marker ID for <%s> as the limit of %ld markers has been reached!", _zName, orxPROFILER_KU32_MAX_MARKER_NUMBER);

      /* Updates marker ID */
      s32MarkerID = orxPROFILER_KS32_MARKER_ID_NONE;
    }
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
    orxPROFILER_MARKER *pstMarker;

    /* Gets marker */
    pstMarker = &(sstProfiler.astMarkerList[_s32MarkerID]);

    /* Not already pushed? */
    if(!orxFLAG_TEST(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_PUSHED))
    {
      /* Is unique and already pushed? */
      if(orxFLAG_TEST(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_UNIQUE) && (pstMarker->u32PushCounter != 0))
      {
        orxS32 i;

        /* Updates flags */
        orxFLAG_SET(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_PUSHED, orxPROFILER_KU16_FLAG_UNIQUE);

        /* For all markers */
        for(i = 0; i < sstProfiler.s32MarkerCounter; i++)
        {
          orxPROFILER_MARKER *pstTestMarker;

          /* Gets it */
          pstTestMarker = &(sstProfiler.astMarkerList[i]);

          /* Is child of current marker? */
          if(pstTestMarker->s32ParentID == _s32MarkerID)
          {
            /* Updates its depth */
            pstTestMarker->u16Depth--;
          }
        }
      }
      else
      {
        /* Updates flags */
        orxFLAG_SET(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_PUSHED, orxPROFILER_KU16_FLAG_NONE);
      }

      /* Is unique? */
      if(orxFLAG_TEST(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_UNIQUE))
      {
        /* Stores its push depth */
        pstMarker->u16Depth = ++sstProfiler.u16CurrentMarkerDepth;
      }
      else
      {
        /* Clears push depth */
        pstMarker->u16Depth = 0;
      }

      /* Updates its push counter */
      pstMarker->u32PushCounter++;

      /* Updates parent marker */
      pstMarker->s32ParentID = sstProfiler.s32CurrentMarker;

      /* Updates current marker */
      sstProfiler.s32CurrentMarker = _s32MarkerID;

      /* Gets time stamp */
      pstMarker->dTimeStamp = orxSystem_GetTime();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't push marker <%s> [ID: %ld] as it's already currently pushed.", pstMarker->zName, _s32MarkerID);

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
      orxPROFILER_MARKER *pstMarker;

      /* Gets marker */
      pstMarker = &(sstProfiler.astMarkerList[sstProfiler.s32CurrentMarker]);

      /* Updates cumulated time */
      pstMarker->dCumulatedTime += orxSystem_GetTime() - pstMarker->dTimeStamp;

      /* Pops previous marker */
      sstProfiler.s32CurrentMarker = pstMarker->s32ParentID;

      /* Is unique? */
      if(orxFLAG_TEST(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_UNIQUE))
      {
        /* Updates push depth */
        sstProfiler.u16CurrentMarkerDepth--;
      }
      else
      {
        /* Cleans marker's parent ID */
        pstMarker->s32ParentID = orxPROFILER_KS32_MARKER_ID_NONE;
      }

      /* Updates flags */
      orxFLAG_SET(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_NONE, orxPROFILER_KU16_FLAG_PUSHED);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't pop marker: marker stack is empty.");
    }
  }
}

/** Resets all markers (usually called at the end of the frame)
 */
void orxFASTCALL orxProfiler_ResetAllMarkers()
{
  orxS32 i;

  /* For all markers */
  for(i = 0; i < sstProfiler.s32MarkerCounter; i++)
  {
    orxPROFILER_MARKER *pstMarker;

    /* Gets it */
    pstMarker = &(sstProfiler.astMarkerList[i]);

    /* Resets it */
    pstMarker->dTimeStamp     = orx2D(0.0);
    pstMarker->dCumulatedTime = orx2D(0.0);
    pstMarker->s32ParentID    = orxPROFILER_KS32_MARKER_ID_NONE;
    pstMarker->u32PushCounter = 0;
    pstMarker->u16Depth       = 0;
  }

  /* Updates time stamp*/
  sstProfiler.dTimeStamp = orxSystem_GetTime();

  /* Done! */
  return;
}

/** Gets the time elapsed since last reset
 * @return Time elapsed since the last reset, in seconds
 */
orxDOUBLE orxFASTCALL orxProfiler_GetResetTime()
{
  orxDOUBLE dResult;

  /* Updates result */
  dResult = orxSystem_GetTime() - sstProfiler.dTimeStamp;

  /* Done! */
  return dResult;
}

/** Gets the number of registered markers
 * @return Number of registered markers
 */
orxS32 orxFASTCALL orxProfiler_GetMarkerCounter()
{
  /* Done! */
  return sstProfiler.s32MarkerCounter;
}

/** Gets the next registered marker ID
 * @param[in] _s32MarkerID      ID of the current marker, orxPROFILER_KS32_MARKER_ID_NONE to get the first one
 * @return Next registered marker's ID / orxPROFILER_KS32_MARKER_ID_NONE if the current marker was the last one
 */
orxS32 orxFASTCALL orxProfiler_GetNextMarkerID(orxS32 _s32MarkerID)
{
  orxS32 s32Result;

  /* No marker? */
  if(_s32MarkerID == orxPROFILER_KS32_MARKER_ID_NONE)
  {
    /* Updates result */
    s32Result = 0;
  }
  else
  {
    /* Updates result */
    s32Result = _s32MarkerID + 1;
  }

  /* Not valid? */
  if(s32Result >= sstProfiler.s32MarkerCounter)
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
  orxDOUBLE dPreviousTime, dBestTime;
  orxS32    i, s32Result = orxPROFILER_KS32_MARKER_ID_NONE;

  /* Get previous marker's time stamp */
  dPreviousTime = ((_s32MarkerID != orxPROFILER_KS32_MARKER_ID_NONE) && (_s32MarkerID < sstProfiler.s32MarkerCounter)) ? sstProfiler.astMarkerList[_s32MarkerID].dTimeStamp : orx2D(0.0);

  /* For all markers */
  for(i = 0, dBestTime = orxSystem_GetTime(); i < sstProfiler.s32MarkerCounter; i++)
  {
    orxDOUBLE dTime;

    /* Gets its time */
    dTime = sstProfiler.astMarkerList[i].dTimeStamp;

    /* Is better candidate? */
    if((dTime > dPreviousTime) && (dTime < dBestTime))
    {
      /* Updates result */
      s32Result = i;
      dBestTime = dTime;
    }
  }

  /* Done! */
  return s32Result;
}

/** Gets the marker's cumulated time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's cumulated time
 */
orxDOUBLE orxFASTCALL orxProfiler_GetMarkerTime(orxS32 _s32MarkerID)
{
  orxDOUBLE dResult;

  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    /* Updates result */
    dResult = sstProfiler.astMarkerList[_s32MarkerID].dCumulatedTime;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%ld].", _s32MarkerID);

    /* Updates result */
    dResult = orx2D(0.0);
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
  const orxSTRING zResult;

  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    /* Updates result */
    zResult = sstProfiler.astMarkerList[_s32MarkerID].zName;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%ld].", _s32MarkerID);

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
  orxU32 u32Result;

  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    /* Updates result */
    u32Result = sstProfiler.astMarkerList[_s32MarkerID].u32PushCounter;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%ld].", _s32MarkerID);

    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

/** Has the marker been uniquely pushed?
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxProfiler_IsUniqueMarker(orxS32 _s32MarkerID)
{
  orxBOOL bResult;

  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    /* Updates result */
    bResult = orxFLAG_TEST(sstProfiler.astMarkerList[_s32MarkerID].u16Flags, orxPROFILER_KU16_FLAG_UNIQUE) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%ld].", _s32MarkerID);

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
  orxDOUBLE dResult;

  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    orxPROFILER_MARKER *pstMarker;

    /* Gets marker */
    pstMarker = &(sstProfiler.astMarkerList[_s32MarkerID]);

    /* Is unique? */
    if(orxFLAG_TEST(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_UNIQUE))
    {
      /* Updates result */
      dResult = pstMarker->dTimeStamp;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get start time of marker <%s> [ID: %ld] as it hasn't been uniquely pushed.", pstMarker->zName, _s32MarkerID);

      /* Updates result */
      dResult = orx2D(0.0);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%ld].", _s32MarkerID);

    /* Updates result */
    dResult = orx2D(0.0);
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
  orxU32 u32Result;

  /* Valid marker ID? */
  if((_s32MarkerID >= 0) && (_s32MarkerID < sstProfiler.s32MarkerCounter))
  {
    orxPROFILER_MARKER *pstMarker;

    /* Gets marker */
    pstMarker = &(sstProfiler.astMarkerList[_s32MarkerID]);

    /* Is unique? */
    if(orxFLAG_TEST(pstMarker->u16Flags, orxPROFILER_KU16_FLAG_UNIQUE))
    {
      /* Updates result */
      u32Result = (orxU32)pstMarker->u16Depth;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't get push depth of marker <%s> [ID: %ld] as it hasn't been uniquely pushed.", pstMarker->zName, _s32MarkerID);

      /* Updates result */
      u32Result = 0;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PROFILER, "Can't access marker data: invalid ID [%ld].", _s32MarkerID);

    /* Updates result */
    u32Result = 0;
  }

  /* Done! */
  return u32Result;
}

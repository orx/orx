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
 * @file orxProfiler.h
 * @date 29/04/2011
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxProfiler
 * 
 * Profiler Module
 * Allows to profile code execution
 *
 * @{
 */


#ifndef _orxPROFILER_H_
#define _orxPROFILER_H_


#include "orxInclude.h"


/* *** Uncomment the line below to enable orx profiling in non debug builds *** */
//#define __orxPROFILER__


#ifdef __orxDEBUG__

  #define __orxPROFILER__

#endif /* __orxDEBUG__ */


/** Profiler macros
 */
#ifdef __orxPROFILER__

  #define orxPROFILER_PUSH_MARKER(NAME)                           \
  do                                                              \
  {                                                               \
    static orxS32 s32ProfilerID = orxPROFILER_KS32_MARKER_ID_NONE;\
                                                                  \
    if(orxProfiler_IsMarkerIDValid(s32ProfilerID) == orxFALSE)    \
    {                                                             \
      s32ProfilerID = orxProfiler_GetIDFromName(NAME);            \
    }                                                             \
                                                                  \
    orxProfiler_PushMarker(s32ProfilerID);                        \
  } while(orxFALSE)


  #define orxPROFILER_POP_MARKER()                orxProfiler_PopMarker();

#else /* __orxPROFILER__ */

  #define orxPROFILER_PUSH_MARKER(NAME)

  #define orxPROFILER_POP_MARKER()

#endif /* __orxPROFILER__ */


/** Defines
 */
#define orxPROFILER_KS32_MARKER_ID_NONE           -1


/** Setups Profiler module */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_Setup();

/** Inits the Profiler module 
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxProfiler_Init();

/** Exits from the Profiler module */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_Exit();


/** Gets a marker ID given a name
 * @param[in] _zName            Name of the marker
 * @return Marker's ID / orxPROFILER_KS32_MARKER_ID_NONE
 */
extern orxDLLAPI orxS32 orxFASTCALL               orxProfiler_GetIDFromName(const orxSTRING _zName);

/** Is the given marker valid? (Useful when storing markers in static variables and still allow normal hot restart)
 * @param[in] _s32MarkerID      ID of the marker to test
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxProfiler_IsMarkerIDValid(orxS32 _s32MarkerID);


/** Pushes a marker (on a stack) and starts a timer for it
 * @param[in] _s32MarkerID      ID of the marker to push
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_PushMarker(orxS32 _s32MarkerID);

/** Pops a marker (from the stack) and updates its cumulated time (using the last marker push time)
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_PopMarker();

/** Enables marker push/pop operations
 * @param[in] _bEnable          Enable
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_EnableMarkerOperations(orxBOOL _bEnable);


/** Resets all markers (usually called at the end of the frame)
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_ResetAllMarkers();

/** Resets all maxima (usually called at a regular interval)
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_ResetAllMaxima();

/** Gets the time elapsed since last reset
 * @return Time elapsed since the last reset, in seconds
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL            orxProfiler_GetResetTime();

/** Gets the maximum reset time
 * @return Max reset time, in seconds
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL            orxProfiler_GetMaxResetTime();


/** Gets the number of registered markers
 * @return Number of registered markers
 */
extern orxDLLAPI orxS32 orxFASTCALL               orxProfiler_GetMarkerCounter();


/** Gets the next registered marker ID
 * @param[in] _s32MarkerID      ID of the current marker, orxPROFILER_KS32_MARKER_ID_NONE to get the first one
 * @return Next registered marker's ID / orxPROFILER_KS32_MARKER_ID_NONE if the current marker was the last one
 */
extern orxDLLAPI orxS32 orxFASTCALL               orxProfiler_GetNextMarkerID(orxS32 _s32MarkerID);

/** Gets the ID of the next marker, sorted by their push time
 * @param[in] _s32MarkerID      ID of the current pushed marker, orxPROFILER_KS32_MARKER_ID_NONE to get the first one
 * @return Next registered marker's ID / orxPROFILER_KS32_MARKER_ID_NONE if the current marker was the last one
 */
extern orxDLLAPI orxS32 orxFASTCALL               orxProfiler_GetNextSortedMarkerID(orxS32 _s32MarkerID);


/** Gets the marker's cumulated time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's cumulated time
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL            orxProfiler_GetMarkerTime(orxS32 _s32MarkerID);

/** Gets the marker's maximum cumulated time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's max cumulated time
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL            orxProfiler_GetMarkerMaxTime(orxS32 _s32MarkerID);

/** Gets the marker's name
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL      orxProfiler_GetMarkerName(orxS32 _s32MarkerID);

/** Gets the marker's push counter
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Number of time the marker has been pushed since last reset
 */
extern orxDLLAPI orxU32 orxFASTCALL               orxProfiler_GetMarkerPushCounter(orxS32 _s32MarkerID);


/** Has the marker been pushed by a unique parent?
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxProfiler_IsUniqueMarker(orxS32 _s32MarkerID);


/** Gets the uniquely pushed marker's start time
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's start time / 0.0
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL            orxProfiler_GetUniqueMarkerStartTime(orxS32 _s32MarkerID);

/** Gets the uniquely pushed marker's depth, 1 being the depth of the top level
 * @param[in] _s32MarkerID      Concerned marker ID
 * @return Marker's push depth / 0 if this marker hasn't been uniquely pushed
 */
extern orxDLLAPI orxU32 orxFASTCALL               orxProfiler_GetUniqueMarkerDepth(orxS32 _s32MarkerID);


#endif /* _orxPROFILER_H_ */

/** @} */

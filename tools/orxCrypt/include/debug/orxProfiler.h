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


/** Helpers
 */
#define orxPROFILER_PUSH_MARKER(NAME)         \
do                                            \
{                                             \
  static orxS32 s32ProfilerID = -1;           \
                                              \
  if(s32ProfilerID < 0)                       \
  {                                           \
    s32ProfilerID = orxProfiler_GetID(NAME);  \
  }                                           \
                                              \
  orxProfiler_PushMarker(s32ProfilerID);      \
} while(orxFALSE)

#define orxPROFILER_POP_MARKER()                  orxProfiler_PopMarker();


/** Defines
 */
#define orxPROFILER_KU32_MAX_MARKER_NUMBER        128
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
extern orxDLLAPI orxS32 orxFASTCALL               orxProfiler_GetID(const orxSTRING _zName);

/** Pushes a marker (on a stack) and starts a timer for it
 * @param[in] _s32MarkerID      ID of the marker to push
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_PushMarker(orxS32 _s32MarkerID);

/** Pops a marker (from the stack) and updates its cumulated time (using the last marker push time)
 */
extern orxDLLAPI void orxFASTCALL                 orxProfiler_PopMarker();


#endif /* _orxPROFILER_H_ */

/** @} */

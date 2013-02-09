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
 * @file orxSystem.h
 * @date 25/05/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSystem
 * 
 * Module file
 * Code that handles modules and their dependencies
 *
 * @{
 */


#ifndef _orxSYSTEM_H_
#define _orxSYSTEM_H_

#include "orxInclude.h"
#include "math/orxVector.h"

/** Event enum
 */
typedef enum __orxSYSTEM_EVENT_t
{
  orxSYSTEM_EVENT_CLOSE = 0,
  orxSYSTEM_EVENT_FOCUS_GAINED,
  orxSYSTEM_EVENT_FOCUS_LOST,

  orxSYSTEM_EVENT_BACKGROUND,
  orxSYSTEM_EVENT_FOREGROUND,

  orxSYSTEM_EVENT_GAME_LOOP_START,
  orxSYSTEM_EVENT_GAME_LOOP_STOP,

  orxSYSTEM_EVENT_TOUCH_BEGIN,
  orxSYSTEM_EVENT_TOUCH_MOVE,
  orxSYSTEM_EVENT_TOUCH_END,
  orxSYSTEM_EVENT_ACCELERATE,
  orxSYSTEM_EVENT_MOTION_SHAKE,
  
  orxSYSTEM_EVENT_NUMBER,

  orxSYSTEM_EVENT_NONE = orxENUM_NONE

} orxSYSTEM_EVENT;

/** System event payload
 */
typedef struct __orxSYSTEM_EVENT_PAYLOAD_t
{
  union
  {
    orxU32      u32FrameCounter;

    /* Touch event */
    struct
    {
      orxDOUBLE dTime;
      orxU32    u32ID;
      orxFLOAT  fX, fY, fPressure;
    } stTouch;

    /* Accelerometer event */
    struct
    {
      orxDOUBLE dTime;
      orxVECTOR vAcceleration;
    } stAccelerometer;
  };

} orxSYSTEM_EVENT_PAYLOAD;


/** System module setup
 */
extern orxDLLAPI void orxFASTCALL       orxSystem_Setup();

/** Inits the system module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxSystem_Init();

/** Exits from the system module
 */
extern orxDLLAPI void orxFASTCALL       orxSystem_Exit();

/** Gets current time (elapsed from the beginning of the application)
 * @return Current time
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL  orxSystem_GetTime();

/** Gets current real time (in seconds)
 * @return Current real time
 */
extern orxDLLAPI orxS32 orxFASTCALL     orxSystem_GetRealTime();

/** Gets current internal system time (in seconds)
 * @return Current internal system time
 */
extern orxDLLAPI orxDOUBLE orxFASTCALL  orxSystem_GetSystemTime();

/** Delay the program for given number of seconds
 * @param[in] _fSeconds             Number of seconds to wait
 */
extern orxDLLAPI void orxFASTCALL       orxSystem_Delay(orxFLOAT _fSeconds);

#endif /* _orxSYSTEM_H_ */

/** @} */

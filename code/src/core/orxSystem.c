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
 * @file orxSystem.c
 * @date 26/05/2005
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxSystem.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"

#include <time.h>

#ifdef __orxWINDOWS__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#else /* __orxWINDOWS__ */

#include <unistd.h>
#include <sys/time.h>

#endif /* __orxWINDOWS__ */


/** Module flags
 */
#define orxSYSTEM_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxSYSTEM_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSYSTEM_STATIC_t
{
  orxU32    u32Flags;
  orxDOUBLE dStartTime;

#ifdef __orxWINDOWS__

  orxDOUBLE dFrequency;
  orxBOOL   bUseHighPerformanceTimer;

#endif /* __orxWINDOWS__ */

} orxSYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSYSTEM_STATIC sstSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxINLINE orxDOUBLE orxSystem_GetSystemTime()
{
  orxDOUBLE dResult;

#ifdef __orxWINDOWS__

  /* Use high performance timer? */
  if(sstSystem.bUseHighPerformanceTimer != orxFALSE)
  {
    LARGE_INTEGER s64CurrentTime;

    /* Gets current time */
    QueryPerformanceCounter(&s64CurrentTime);

    /* Updates result */
    dResult = orx2D(s64CurrentTime.QuadPart) / sstSystem.dFrequency;
  }
  else
  {
    /* Updates result */
    dResult = orx2D(GetTickCount()) * orx2D(0.001);
  }
  
#else /* __orxWINDOWS__ */

  #ifdef __orxANDROID_NATIVE__

  struct timespec stCurrentTime;
  
  /* Gets current time */
  if(clock_gettime(CLOCK_MONOTONIC, &stCurrentTime) == 0)
  {
    /* Updates result */
    dResult = orx2D(stCurrentTime.tv_sec) + (orx2D(stCurrentTime.tv_nsec) * orx2D(0.000000001));
  }  
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: can't get system time.");

    /* Updates result */
    dResult = orx2D(0.0);
  }

  #else /* __orxANDROID_NATIVE__ */

  struct timeval stCurrentTime;

  /* Gets current time */
  if(gettimeofday(&stCurrentTime, NULL) == 0)
  {
    /* Updates result */
    dResult = orx2D(stCurrentTime.tv_sec) + (orx2D(stCurrentTime.tv_usec) * orx2D(0.000001));
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: can't get system time.");

    /* Updates result */
    dResult = orx2D(0.0);
  }

  #endif /* __orxANDROID_NATIVE__ */

#endif /* __orxWINDOWS__ */

  /* Done! */
  return dResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** System module setup
 */
void orxFASTCALL orxSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_MEMORY);

  return;
}

/** Init the system module
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxSystem_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY))
  {
#ifdef __orxWINDOWS__

    LARGE_INTEGER s64Frequency;

#endif /* __orxWINDOWS__ */

    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));

#ifdef __orxWINDOWS__

    /* Should use high performance timer? */
    if(QueryPerformanceFrequency(&s64Frequency))
    {
      /* Stores its frequency */
      sstSystem.dFrequency = orx2D(s64Frequency.QuadPart);

      /* Updates status */
      sstSystem.bUseHighPerformanceTimer = orxTRUE;
    }
    else
    {
      /* Updates status */
      sstSystem.bUseHighPerformanceTimer = orxFALSE;
    }

#endif /* __orxWINDOWS__ */

    /* Gets start time */
    sstSystem.dStartTime = orxSystem_GetSystemTime();

    /* Updates status */
    sstSystem.u32Flags |= orxSYSTEM_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exit the system module
 */
void orxFASTCALL orxSystem_Exit()
{
  /* Module initialized ? */
  if((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));
  }

  return;
}

/** Gets App Elapsed time.
 * @return Returns the amount of seconds elapsed from the application start.
 */
orxDOUBLE orxFASTCALL orxSystem_GetTime()
{
  orxDOUBLE dCurrentTime;

  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets current time */
  dCurrentTime = orxSystem_GetSystemTime();

  /* Done! */
  return(dCurrentTime - sstSystem.dStartTime);
}

/** Gets real time
 * @return Returns the amount of seconds elapsed since reference time
 */
orxS32 orxFASTCALL orxSystem_GetRealTime()
{
  time_t  stTime;
  orxS32  s32Result;

  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets time */
  s32Result = (orxS32)time(&stTime);

  /* Done! */
  return s32Result;
}

/** Delays the program for given number of seconds.
 * @param[in] _fSeconds Number of seconds to wait.
 */
void orxFASTCALL orxSystem_Delay(orxFLOAT _fSeconds)
{
  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_fSeconds >= orxFLOAT_0);

#ifdef __orxWINDOWS__

  /* Sleeps */
  Sleep(orxF2U(_fSeconds * orx2F(1000.0f)));

#else /* __orxWINDOWS__ */

  /* Sleeps */
  usleep(orxF2U(_fSeconds * orx2F(1000000.0f)));

#endif /* __orxWINDOWS__ */

  return;
}

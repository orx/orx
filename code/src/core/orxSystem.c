/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    dResult = (orxDOUBLE)s64CurrentTime.QuadPart / sstSystem.dFrequency;
  }
  else
  {
    /* Updates result */
    dResult = (orxDOUBLE)GetTickCount() * (orxDOUBLE)0.001;
  }

#else /* __orxWINDOWS__ */

  struct timeval stCurrentTime;

  /* Gets current time */
  if(gettimeofday(&stCurrentTime, NULL) == 0)
  {
    /* Updates result */
    dResult = (orxDOUBLE)stCurrentTime.tv_sec + ((orxDOUBLE)stCurrentTime.tv_usec * (orxDOUBLE)0.000001);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: can't get system time.");

    /* Updates result */
    dResult = 0.0;
  }

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
      sstSystem.dFrequency = (orxDOUBLE)s64Frequency.QuadPart;

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

    /* Inits random generator based on current time */
    orxMath_InitRandom(1000 * orxSystem_GetRealTime());
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
orxFLOAT orxFASTCALL orxSystem_GetTime()
{
  orxDOUBLE dCurrentTime;

  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets current time */
  dCurrentTime = orxSystem_GetSystemTime();

  /* Done! */
  return (orxFLOAT)(dCurrentTime - sstSystem.dStartTime);
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

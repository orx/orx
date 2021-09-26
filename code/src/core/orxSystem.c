/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
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

#include "core/orxEvent.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"

#include <time.h>

#ifdef __orxWINDOWS__

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

#else /* __orxWINDOWS__ */

  #if defined(__orxMAC__) || defined(__orxIOS__)

    #include <mach/mach_time.h>

  #endif /* __orxMAC__ || __orxIOS__ */

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
  orxDOUBLE dStartTime;

#ifdef __orxWINDOWS__

  orxDOUBLE dFrequency;
  orxBOOL   bUseHighPerformanceTimer;

#else /* __orxWINDOWS__ */

  #if defined(__orxMAC__) || defined(__orxIOS__)

  orxDOUBLE dResolution;

  #else /* __orxMAC__ || __orxIOS__ */

  orxBOOL bUseMonotonic;

  #endif /* __orxMAC__ || __orxIOS__ */

#endif /* __orxWINDOWS__ */

  orxU32    u32Flags;

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


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** System module setup
 */
void orxFASTCALL orxSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_MEMORY);

  /* Done! */
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

#else /* __orxWINDOWS__ */

  #if defined(__orxMAC__) || defined(__orxIOS__)

    mach_timebase_info_data_t stInfo;

  #else /* __orxMAC__ || __orxIOS__ */

    #ifdef CLOCK_MONOTONIC

    struct timespec stCurrentTime;

    #endif /* CLOCK_MONOTONIC */

  #endif /* __orxMAC__ || __orxIOS__ */

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

#else /* __orxWINDOWS __ */

  #if defined(__orxMAC__) || defined(__orxIOS__)

    /* Gets time base info */
    mach_timebase_info(&stInfo);

    /* Stores resolution */
    sstSystem.dResolution = orx2D(stInfo.numer) / orx2D(stInfo.denom * 1.0e9);

  #else /* __orxMAC__ || __orxIOS__ */

    #ifdef CLOCK_MONOTONIC

    /* Can get monotonic time? */
    if(clock_gettime(CLOCK_MONOTONIC, &stCurrentTime) == 0)
    {
      /* Updates status */
      sstSystem.bUseMonotonic = orxTRUE;
    }

    #endif /* CLOCK_MONOTONIC */

  #endif /* __orxMAC__ || __orxIOS__ */

#endif /* __orxWINDOWS__ */

    /* Gets start time */
    sstSystem.dStartTime = orxSystem_GetSystemTime();

    /* Updates status */
    sstSystem.u32Flags |= orxSYSTEM_KU32_STATIC_FLAG_READY;

    /* Inits random with time-based seed */
    orxMath_InitRandom((orxU32)(1000.0 * orxSystem_GetSystemTime()));
  }

  /* Done! */
  return eResult;
}

/** Exit the system module
 */
void orxFASTCALL orxSystem_Exit()
{
  /* Checks */
  if((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));
  }

  /* Done! */
  return;
}

/** Gets current time (elapsed from the beginning of the application, in seconds)
 * @return Current time
 */
orxDOUBLE orxFASTCALL orxSystem_GetTime()
{
  orxDOUBLE dCurrentTime;

  /* Checks */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets current time */
  dCurrentTime = orxSystem_GetSystemTime();

  /* Done! */
  return(dCurrentTime - sstSystem.dStartTime);
}

/** Gets real time (in seconds)
 * @return Returns the amount of seconds elapsed since reference time (epoch)
 */
orxU64 orxFASTCALL orxSystem_GetRealTime()
{
  time_t  stTime;
  orxU64  u64Result;

  /* Checks */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets time */
  u64Result = (orxU64)time(&stTime);

  /* Done! */
  return u64Result;
}

/** Gets current internal system time (in seconds)
 * @return Current internal system time
 */
orxDOUBLE orxFASTCALL orxSystem_GetSystemTime()
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

  #if defined(__orxMAC__) || defined(__orxIOS__)

  dResult = orx2D(mach_absolute_time()) * sstSystem.dResolution;

  #else /* __orxMAC__ || __orxIOS__ */

    #ifdef CLOCK_MONOTONIC

  /* Use monotonic clock? */
  if(sstSystem.bUseMonotonic != orxFALSE)
  {
    struct timespec stCurrentTime;

    /* Gets current time */
    clock_gettime(CLOCK_MONOTONIC, &stCurrentTime);

    /* Updates result */
    dResult = orx2D(stCurrentTime.tv_sec) + (orx2D(stCurrentTime.tv_nsec) * orx2D(0.000000001));
  }
  else

    #endif /* CLOCK_MONOTONIC */

  {

    #if !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__)

    struct timeval stCurrentTime;

    /* Gets current time */
    if(gettimeofday(&stCurrentTime, NULL) == 0)
    {
      /* Updates result */
      dResult = orx2D(stCurrentTime.tv_sec) + (orx2D(stCurrentTime.tv_usec) * orx2D(0.000001));
    }
    else

    #endif /* !__orxANDROID__ && !__orxANDROID_NATIVE__ */

    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Error: can't get system time.");

      /* Updates result */
      dResult = orxDOUBLE_0;
    }
  }

  #endif /* __orxMAC__ || __orxIOS__ */

#endif /* __orxWINDOWS__ */

  /* Done! */
  return dResult;
}

/** Delay the program for given number of seconds
 * @param[in] _fSeconds             Number of seconds to wait
 */
void orxFASTCALL orxSystem_Delay(orxFLOAT _fSeconds)
{
  /* Checks */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_fSeconds >= orxFLOAT_0);

#ifdef __orxWINDOWS__

  /* Sleeps */
  Sleep(orxF2U(_fSeconds * orx2F(1000.0f)));

#else /* __orxWINDOWS__ */

  /* Sleeps */
  usleep(orxF2U(_fSeconds * orx2F(1000000.0f)));

#endif /* __orxWINDOWS__ */

  /* Done! */
  return;
}

/** Gets orx version (compiled)
 * @param[out] _pstVersion          Structure to fill with current version
 * @return Compiled version
 */
orxVERSION *orxFASTCALL orxSystem_GetVersion(orxVERSION *_pstVersion)
{
  /* Checks */
  orxASSERT(_pstVersion != orxNULL);

  /* Updates result */
  _pstVersion->u32Major = __orxVERSION_MAJOR__;
  _pstVersion->u32Minor = __orxVERSION_MINOR__;
  _pstVersion->u32Build = __orxVERSION_BUILD__;
  _pstVersion->zRelease = orxSTRINGIFY(__orxVERSION_RELEASE__);

  /* Done! */
  return _pstVersion;
}

/** Gets orx version literal (compiled), excluding build number
 * @return Compiled version literal
 */
const orxSTRING orxFASTCALL orxSystem_GetVersionString()
{
  const orxSTRING zResult;

  /* Updates result */
  zResult = __orxVERSION_STRING__;

  /* Done! */
  return zResult;
}

/** Gets orx version literal (compiled), including build number
 * @return Compiled version literal
 */
const orxSTRING orxFASTCALL orxSystem_GetVersionFullString()
{
  const orxSTRING zResult;

  /* Updates result */
  zResult = __orxVERSION_FULL_STRING__;

  /* Done! */
  return zResult;
}

/** Gets orx version absolute numeric value (compiled)
 * @return Absolute numeric value of compiled version
 */
orxU32 orxFASTCALL orxSystem_GetVersionNumeric()
{
  orxU32 u32Result;

  /* Updates result */
  u32Result = __orxVERSION__;

  /* Done! */
  return u32Result;
}

/** Gets clipboard's content
 * @return Clipboard's content / orxSTRING_EMPTY, valid until next call to orxSystem_GetClipboard/orxSystem_SetClipboard
 */
const orxSTRING orxFASTCALL orxSystem_GetClipboard()
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Is event module initialized? */
  if(orxModule_IsInitialized(orxMODULE_ID_EVENT) != orxFALSE)
  {
    orxEVENT                stEvent;
    orxSYSTEM_EVENT_PAYLOAD stPayload;

    /* Inits event */
    orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
    orxEVENT_INIT(stEvent, orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLIPBOARD, orxNULL, orxNULL, &stPayload);

    /* Sends event */
    if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
    {
      /* Success? */
      if(stPayload.stClipboard.zValue != orxNULL)
      {
        /* Updates result */
        zResult = stPayload.stClipboard.zValue;
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Sets clipboard's content
 * @param[in] _zValue               Value to set in the clipboard, orxNULL to clear
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSystem_SetClipboard(const orxSTRING _zValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Is event module initialized? */
  if(orxModule_IsInitialized(orxMODULE_ID_EVENT) != orxFALSE)
  {
    orxEVENT                stEvent;
    orxSYSTEM_EVENT_PAYLOAD stPayload;

    /* Inits event */
    orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
    stPayload.stClipboard.zValue = (_zValue != orxNULL) ? _zValue : orxSTRING_EMPTY;
    orxEVENT_INIT(stEvent, orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLIPBOARD, orxNULL, orxNULL, &stPayload);

    /* Sends event and updates result */
    eResult = orxEvent_Send(&stEvent);
  }

  /* Done! */
  return eResult;
}

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
 * @file orxThread.c
 * @date 24/11/2013
 * @author iarwain@orx-project.org
 *
 */

#define NO_WIN32_LEAN_AND_MEAN

#include "core/orxThread.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"

#ifndef __orxWINDOWS__

#include <pthread.h>

  #if defined(__orxLINUX__) || defined(__orxRASPBERRY_PI__)

    #include <sched.h>

  #endif /* __orxLINUX__ || __orxRASPBERRY_PI__ */

#endif /* !__orxWINDOWS__ */


/** Module flags
 */
#define orxTHREAD_KU32_STATIC_FLAG_NONE               0x00000000  /**< No flags have been set */
#define orxTHREAD_KU32_STATIC_FLAG_READY              0x00000001  /**< Static flag */
#define orxTHREAD_KU32_STATIC_MASK_ALL                0xFFFFFFFF  /**< The module has been initialized */

#define orxTHREAD_KU32_INFO_FLAG_NONE                 0x00000000  /**< No flags have been set */
#define orxTHREAD_KU32_INFO_FLAG_INITIALIZED          0x00000001  /**< Initialized flag */
#define orxTHREAD_KU32_INFO_FLAG_STOP                 0x10000000  /**< Stop flag */
#define orxTHREAD_KU32_INFO_MASK_ALL                  0xFFFFFFFF  /**< The module has been initialized */

#define orxTHREAD_KU32_MAIN_THREAD_ID                 0           /**< Main thread ID */
#define orxTHREAD_KU32_MAX_THREAD_NUMBER              16          /**< Max thread number */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/* Thread info structure
 */
typedef struct __orxTHREAD_INFO_t
{
#ifdef __orxWINDOWS__

  HANDLE                  hThread;

#else /* __orxWINDOWS__ */

  pthread_t               hThread;

#endif /* __orxWINDOWS__ */

  orxU32                  u32Flags;

} orxTHREAD_INFO;

/** Static structure
 */
typedef struct __orxTHREAD_STATIC_t
{
  volatile orxTHREAD_INFO astThreadInfoList[orxTHREAD_KU32_MAX_THREAD_NUMBER];

  orxU32                  u32Flags;

} orxTHREAD_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxTHREAD_STATIC sstThread;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Thread module setup
 */
void orxFASTCALL orxThread_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_THREAD, orxMODULE_ID_MEMORY);

  /* Done! */
  return;
}

/** Init the thread module
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxThread_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstThread.u32Flags & orxTHREAD_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstThread, sizeof(orxTHREAD_STATIC));

#if defined(__orxWINDOWS__)

    /* Stores main thread ID */
    sstThread.astThreadInfoList[orxTHREAD_KU32_MAIN_THREAD_ID].hThread = GetCurrentThread();

    /* Sets thread CPU affinity to remain on the same core */
    SetThreadAffinityMask(sstThread.astThreadInfoList[orxTHREAD_KU32_MAIN_THREAD_ID].hThread, 1);

    /* Asks for small time slices */
    timeBeginPeriod(1);

#elif defined(__orxLINUX__) || defined(__orxRASPBERRY_PI__)

    {
      cpu_set_t stSet;

      /* Stores main thread ID */
      sstThread.astThreadInfoList[orxTHREAD_KU32_MAIN_THREAD_ID].hThread = pthread_self();

      /* Sets CPU affinity mask */
      CPU_ZERO(&stSet);
      CPU_SET(0, &stSet);

      /* Applies it */
      pthread_setaffinity_np(sstThread.astThreadInfoList[orxTHREAD_KU32_MAIN_THREAD_ID].hThread, sizeof(cpu_set_t), &stSet);
    }

#endif

    /* Updates status */
    sstThread.u32Flags |= orxTHREAD_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exit the thread module
 */
void orxFASTCALL orxThread_Exit()
{
  /* Checks */
  if((sstThread.u32Flags & orxTHREAD_KU32_STATIC_FLAG_READY) == orxTHREAD_KU32_STATIC_FLAG_READY)
  {
#ifdef __orxWINDOWS__

   /* Resets time slices */
   timeEndPeriod(1);

#endif /* __orxWINDOWS__ */

    /* Cleans static controller */
    orxMemory_Zero(&sstThread, sizeof(orxTHREAD_STATIC));
  }

  /* Done! */
  return;
}

/** Creates a new thread
 * @param[in]   _pfnRun                               Function to run on the new thread
 * @param[in]   _pContext                             Context that will be transmitted to the function when called
 * @return      Thread ID if successful, orxU32_UNDEFINED otherwise
 */
orxU32 orxFASTCALL orxThread_Create(const orxTHREAD_FUNCTION _pfnRun, void *_pContext)
{
  orxU32 u32Result = orxU32_UNDEFINED;

  /* Checks */
  orxASSERT((sstThread.u32Flags & orxTHREAD_KU32_STATIC_FLAG_READY) == orxTHREAD_KU32_STATIC_FLAG_READY);

  //! TODO

  /* Done! */
  return u32Result;
}

/** Joins a thread (blocks & waits until the other thread finishes)
 * @param[in]   _u32ThreadID                          ID of the thread for which to wait
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxThread_Join(orxU32 _u32ThreadID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstThread.u32Flags & orxTHREAD_KU32_STATIC_FLAG_READY) == orxTHREAD_KU32_STATIC_FLAG_READY);
  orxASSERT(_u32ThreadID < orxTHREAD_KU32_MAX_THREAD_NUMBER);

  /* Updates stop flag */
  orxFLAG_SET(sstThread.astThreadInfoList[_u32ThreadID].u32Flags, orxTHREAD_KU32_INFO_FLAG_STOP, orxTHREAD_KU32_INFO_FLAG_NONE);

#ifdef __orxWINDOWS__

  /* Waits for thread */
  WaitForSingleObject(sstThread.astThreadInfoList[_u32ThreadID].hThread, INFINITE);

#else /* __orxWINDOWS__ */

  /* Joins thread */
  pthread_join(sstThread.astThreadInfoList[_u32ThreadID].hThread, NULL);

#endif /* __orxWINDOWS__ */

  /* Clears thread info */
  sstThread.astThreadInfoList[_u32ThreadID].u32Flags  = orxTHREAD_KU32_INFO_FLAG_NONE;
  orxMEMORY_BARRIER();
  sstThread.astThreadInfoList[_u32ThreadID].hThread   = 0;

  /* Done! */
  return eResult;
}

/** Gets current thread ID
 * @return      Current thread ID
 */
orxU32 orxFASTCALL orxThread_GetCurrent()
{
  orxU32 u32Result = orxU32_UNDEFINED, i;

  /* Checks */
  orxASSERT((sstThread.u32Flags & orxTHREAD_KU32_STATIC_FLAG_READY) == orxTHREAD_KU32_STATIC_FLAG_READY);

#ifdef __orxWINDOWS__

  HANDLE hThread;

  /* Gets current thread ID */
  hThread = GetCurrentThread();

#else /* __orxWINDOWS__ */

  pthread_t hThread;

  /* Gets current thread ID */
  hThread = pthread_self();

#endif /* __orxWINDOWS__ */

  /* For all threads */
  for(i = 0; i < orxTHREAD_KU32_MAX_THREAD_NUMBER; i++)
  {
    /* Matches? */
    if(sstThread.astThreadInfoList[i].hThread == hThread)
    {
      /* Updates result */
      u32Result = i;
      break;
    }
  }

  /* Done! */
  return u32Result;
}

/** Yields to other threads
 */
void orxFASTCALL orxThread_Yield()
{
  /* Checks */
  orxASSERT((sstThread.u32Flags & orxTHREAD_KU32_STATIC_FLAG_READY) == orxTHREAD_KU32_STATIC_FLAG_READY);

#ifdef __orxWINDOWS__

  Sleep(0);

#else /* __orxWINDOWS__ */

  sched_yield();

#endif /* __orxWINDOWS__ */

  /* Done! */
  return;
}

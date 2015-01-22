/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxThread.h
 * @date 24/11/2013
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxThread
 *
 * Thread file
 * Code that handles threads
 *
 * @{
 */


#ifndef _orxTHREAD_H_
#define _orxTHREAD_H_

#include "orxInclude.h"

#ifdef __orxWINDOWS__
  #include "memory/orxMemory.h"
#else /* __orxWINDOWS__ */
  #include <semaphore.h>
#endif /* __orxWINDOWS__ */


#define orxTHREAD_KU32_MAIN_THREAD_ID                 0           /**< Main thread ID */
#define orxTHREAD_KU32_MAX_THREAD_NUMBER              16          /**< Max thread number */

#define orxTHREAD_KU32_FLAG_NONE                      0           /**< Flag none (for orxThread_Enable) */
#define orxTHREAD_KU32_MASK_ALL                       (((1 << orxTHREAD_KU32_MAX_THREAD_NUMBER) - 1) & ~(1 << orxTHREAD_KU32_MAIN_THREAD_ID)) /* Mask all (for orxThread_Enable) */
#define orxTHREAD_GET_FLAG_FROM_ID(ID)                (1 << ID)   /**< Gets thread flag from ID */


/** Semaphore structure */
typedef struct __orxTHREAD_SEMAPHORE_t                orxTHREAD_SEMAPHORE;


/** Thread run function type */
typedef orxSTATUS (orxFASTCALL *orxTHREAD_FUNCTION)(void *_pContext);


/** Thread module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxThread_Setup();

/** Inits the thread module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_Init();

/** Exits from the thread module
 */
extern orxDLLAPI void orxFASTCALL                     orxThread_Exit();


/** Starts a new thread
 * @param[in]   _pfnRun                               Function to run on the new thread
 * @param[in]   _zName                                Thread's name
 * @param[in]   _pContext                             Context that will be transmitted to the function when called
 * @return      Thread ID if successful, orxU32_UNDEFINED otherwise
 */
extern orxDLLAPI orxU32 orxFASTCALL                   orxThread_Start(const orxTHREAD_FUNCTION _pfnRun, const orxSTRING _zName, void *_pContext);

/** Joins a thread (blocks & waits until the other thread finishes)
 * @param[in]   _u32ThreadID                          ID of the thread for which to wait
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_Join(orxU32 _u32ThreadID);

/** Joins all threads (blocks & waits until the other threads finish)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_JoinAll();

/** Gets a thread name
 * @param[in]   _u32ThreadID                          ID of the concerned thread
 * @return      Thread name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxThread_GetName(orxU32 _u32ThreadID);

/** Enables / disables threads
 * @param[in]   _u32EnableThreads   Mask of threads to enable (1 << ThreadID)
 * @param[in]   _u32DisableThreads  Mask of threads to disable (1 << ThreadID)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_Enable(orxU32 _u32EnableThreads, orxU32 _u32DisableThreads);

/** Gets current thread ID
 * @return      Current thread ID
 */
extern orxDLLAPI orxU32 orxFASTCALL                   orxThread_GetCurrent();

/** Yields to other threads
 */
extern orxDLLAPI void orxFASTCALL                     orxThread_Yield();


/** Inits a semaphore with a given value
 * @param[in]   _u32Value                             Value with which to init the semaphore
 * @return      orxTHREAD_SEMAPHORE / orxNULL
 */
extern orxDLLAPI orxTHREAD_SEMAPHORE *orxFASTCALL     orxThread_CreateSemaphore(orxU32 _u32Value);

/** Deletes a semaphore
 * @param[in]   _pstSemaphore                         Concerned semaphore
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_DeleteSemaphore(orxTHREAD_SEMAPHORE *_pstSemaphore);

/** Waits for a semaphore
 * @param[in]   _pstSemaphore                         Concerned semaphore
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_WaitSemaphore(orxTHREAD_SEMAPHORE *_pstSemaphore);

/** Signals a semaphore
 * @param[in]   _pstSemaphore                         Concerned semaphore
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_SignalSemaphore(orxTHREAD_SEMAPHORE *_pstSemaphore);


/** Runs an asynchronous task and optional follow-ups
 * @param[in]   _pfnRun                               Asynchronous task to run, executed on a different thread dedicated to tasks, if orxNULL defaults to an empty task that always succeed
 * @param[in]   _pfnThen                              Executed (on the main thread) if Run does *not* return orxSTATUS_FAILURE, can be orxNULL
 * @param[in]   _pfnElse                              Executed (on the main thread) if Run returns orxSTATUS_FAILURE, can be orxNULL
 * @param[in]   _pContext                             Context that will be transmitted to all the task functions
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxThread_RunTask(const orxTHREAD_FUNCTION _pfnRun, const orxTHREAD_FUNCTION _pfnThen, const orxTHREAD_FUNCTION _pfnElse, void *_pContext);

/** Gets number of pending asynchronous tasks awaiting full completion (might pump task notifications if called from main thread)
 * @return      Number of pending asynchronous tasks
 */
extern orxDLLAPI orxU32 orxFASTCALL                   orxThread_GetTaskCounter();


#endif /* _orxTHREAD_H_ */

/** @} */

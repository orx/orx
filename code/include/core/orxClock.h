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
 * @file orxClock.h
 * @date 28/01/2004
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add freezing behaviour
 */

/**
 * @addtogroup orxClock
 *
 * Clock module
 * Module that handles clocks which is the low level kernel part of orx execution.
 * @{
 */

#ifndef _orxCLOCK_H_
#define _orxCLOCK_H_


#include "orxInclude.h"
#include "core/orxSystem.h"
#include "utils/orxString.h"


#define orxCLOCK_KU32_CLOCK_BANK_SIZE                 16          /**< Clock bank size */
#define orxCLOCK_KU32_TIMER_BANK_SIZE                 32          /**< Timer bank size */

#define orxCLOCK_KU32_FUNCTION_BANK_SIZE              16          /**< Function bank size */

#define orxCLOCK_KZ_CORE                              "core"


/** Clock modifier enum
 */
typedef enum __orxCLOCK_MODIFIER_t
{
  orxCLOCK_MODIFIER_FIXED = 0,                                    /**< The given DT will be constant, set to this modifier value */
  orxCLOCK_MODIFIER_MULTIPLY,                                     /**< The given DT will be multiplied by this modifier value */
  orxCLOCK_MODIFIER_MAXED,                                        /**< The given DT will be maxed by this modifier value */
  orxCLOCK_MODIFIER_AVERAGE,                                      /**< The given DT will be averaged over a number of past updates defined by this modifier value */

  orxCLOCK_MODIFIER_NUMBER,

  orxCLOCK_MODIFIER_NONE = orxENUM_NONE,

} orxCLOCK_MODIFIER;


/** Clock priority
 */
typedef enum __orxCLOCK_PRIORITY_t
{
  orxCLOCK_PRIORITY_LOWEST = 0,
  orxCLOCK_PRIORITY_LOWER,
  orxCLOCK_PRIORITY_LOW,
  orxCLOCK_PRIORITY_NORMAL,
  orxCLOCK_PRIORITY_HIGH,
  orxCLOCK_PRIORITY_HIGHER,
  orxCLOCK_PRIORITY_HIGHEST,

  orxCLOCK_PRIORITY_NUMBER,

  orxCLOCK_PRIORITY_NONE = orxENUM_NONE,

} orxCLOCK_PRIORITY;


/** Clock info structure
 */
typedef struct __orxCLOCK_INFO_t
{
  orxFLOAT          fTickSize;                                    /**< Clock tick size (in seconds) : 4 */
  orxFLOAT          fDT;                                          /**< Clock DT (time elapsed between 2 clock calls in seconds) : 8 */
  orxFLOAT          fTime;                                        /**< Clock time : 12 */
  orxFLOAT          afModifierList[orxCLOCK_MODIFIER_NUMBER];     /**< Clock modifiers : 28 */

} orxCLOCK_INFO;


/** Event enum
 */
typedef enum __orxCLOCK_EVENT_t
{
  orxCLOCK_EVENT_RESTART = 0,                                     /**< Event sent when a clock restarts */
  orxCLOCK_EVENT_RESYNC,                                          /**< Event sent when a clock resyncs */
  orxCLOCK_EVENT_PAUSE,                                           /**< Event sent when a clock is paused */
  orxCLOCK_EVENT_UNPAUSE,                                         /**< Event sent when a clock is unpaused */

  orxCLOCK_EVENT_NUMBER,

  orxCLOCK_EVENT_NONE = orxENUM_NONE

} orxCLOCK_EVENT;


/** Clock structure */
typedef struct __orxCLOCK_t                           orxCLOCK;

/** Clock callback function type to use with clock bindings */
typedef void (orxFASTCALL *orxCLOCK_FUNCTION)(const orxCLOCK_INFO *_pstClockInfo, void *_pContext);


/** Clock module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxClock_Setup();

/** Inits the clock module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Init();

/** Exits from the clock module
 */
extern orxDLLAPI void orxFASTCALL                     orxClock_Exit();


/** Updates the clock system
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Update();

/** Creates a clock
 * @param[in]   _fTickSize                            Tick size for the clock (in seconds)
 * @return      orxCLOCK / orxNULL
 */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_Create(orxFLOAT _fTickSize);

/** Creates a clock from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxCLOCK / orxNULL
 */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Delete(orxCLOCK *_pstClock);

/** Resyncs a clock (accumulated DT => 0)
 * @param[in]   _pstClock                             Concerned clock
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Resync(orxCLOCK *_pstClock);

/** Resyncs all clocks (accumulated DT => 0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_ResyncAll();

/** Restarts a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Restart(orxCLOCK *_pstClock);

/** Pauses a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Pause(orxCLOCK *_pstClock);

/** Unpauses a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Unpause(orxCLOCK *_pstClock);

/** Is a clock paused?
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxTRUE if paused, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxClock_IsPaused(const orxCLOCK *_pstClock);

/** Gets clock info
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK_INFO / orxNULL
 */
extern orxDLLAPI const orxCLOCK_INFO *orxFASTCALL     orxClock_GetInfo(const orxCLOCK *_pstClock);

/** Gets clock from its info
 * @param[in]   _pstClockInfo                         Concerned clock info
 * @return      orxCLOCK / orxNULL
 */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_GetFromInfo(const orxCLOCK_INFO *_pstClockInfo);


/** Sets a clock's modifier
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _eModifier                            Concerned modifier
 * @param[in]   _fValue                               Modifier value, orxFLOAT_0 to deactivate the modifier
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_SetModifier(orxCLOCK *_pstClock, orxCLOCK_MODIFIER _eModifier, orxFLOAT _fValue);

/** Gets a clock's modifier
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _eModifier                            Concerned modifier
 * @return      Modifier value / orxFLOAT_0 if deactivated
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxClock_GetModifier(orxCLOCK *_pstClock, orxCLOCK_MODIFIER _eModifier);

/** Sets a clock tick size
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _fTickSize                            Tick size, -1 for 'display'
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_SetTickSize(orxCLOCK *_pstClock, orxFLOAT _fTickSize);


/** Registers a callback function to a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Callback to register
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @param[in]   _eModuleID                            ID of the module related to this callback
 * @param[in]   _ePriority                            Priority for the function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Register(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, void *_pContext, orxMODULE_ID _eModuleID, orxCLOCK_PRIORITY _ePriority);

/** Unregisters a callback function from a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Callback to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_Unregister(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback);

/** Gets a callback function context
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @return      Registered context
 */
extern orxDLLAPI void *orxFASTCALL                    orxClock_GetContext(const orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback);

/** Sets a callback function context
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_SetContext(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, void *_pContext);


/** Gets next existing clock in list (can be used to parse all existing clocks)
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK / orxNULL
 */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_GetNext(const orxCLOCK *_pstClock);

/** Gets clock given its name
 * @param[in]   _zName          Clock name
 * @return      orxCLOCK / orxNULL
 */
extern orxDLLAPI orxCLOCK *orxFASTCALL                orxClock_Get(const orxSTRING _zName);

/** Gets clock config name
 * @param[in]   _pstClock       Concerned clock
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxClock_GetName(const orxCLOCK *_pstClock);


/** Adds a timer function to a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned timer callback
 * @param[in]   _fDelay                               Timer's delay between 2 calls, must be strictly positive
 * @param[in]   _s32Repetition                        Number of times this timer should be called before removed, -1 for infinite
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_AddTimer(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, orxS32 _s32Repetition, void *_pContext);

/** Removes a timer function from a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned timer callback to remove, orxNULL to remove all occurrences regardless of their callback
 * @param[in]   _fDelay                               Delay between 2 calls of the timer to remove, -1.0f to remove all occurrences regardless of their respective delay
 * @param[in]   _pContext                             Context of the timer to remove, orxNULL to remove all occurrences regardless of their context
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_RemoveTimer(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, void *_pContext);

/** Adds a global timer function (ie. using the main core clock)
 * @param[in]   _pfnCallback                          Concerned timer callback
 * @param[in]   _fDelay                               Timer's delay between 2 calls, must be strictly positive
 * @param[in]   _s32Repetition                        Number of times this timer should be called before removed, -1 for infinite
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_AddGlobalTimer(const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, orxS32 _s32Repetition, void *_pContext);

/** Removes a global timer function (ie. from the main core clock)
 * @param[in]   _pfnCallback                          Concerned timer callback to remove, orxNULL to remove all occurrences regardless of their callback
 * @param[in]   _fDelay                               Delay between 2 calls of the timer to remove, -1.0f to remove all occurrences regardless of their respective delay
 * @param[in]   _pContext                             Context of the timer to remove, orxNULL to remove all occurrences regardless of their context
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxClock_RemoveGlobalTimer(const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, void *_pContext);


#endif /* _orxCLOCK_H_ */

/** @} */

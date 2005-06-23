/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date 20/06/2005
 * @author (C) Arcallians
 * 
 * @todo 
 */

/**
 * @addtogroup StateMachine
 * 
 * State Machine.
 * Allows to create state machines, for various purposes.
 * @{
 */
 
#ifndef _orxSTATEMACHINE_H_
#define _orxSTATEMACHINE_H_
 
#include "orxInclude.h"
#include "memory/orxMemory.h"

/* Internal state machine structure. */
typedef struct __orxSTATEMACHINE_t orxSTATEMACHINE;

/* Internal state structure. */
typedef struct __orxSTATEMACHINE_STATE_t orxSTATEMACHINE_STATE;

/* Internal link structure. */
typedef struct __orxSTATEMACHINE_LINK_t orxSTATEMACHINE_LINK;

/* Define flags. */
#define orxSTATEMACHINE_KU32_FLAGS_NONE            0x00000000  /**< No flags (default behaviour) */
#define orxSTATEMACHINE_KU32_FLAGS_NOT_EXPANDABLE  0x00000001  /**< The state machine will not be expandable */

/** @name Module management.
 * @{ */
/** Initialize StateMachine Module.
 * @return Returns the initialization status.
 */
extern orxDLLAPI orxSTATUS orxStateMachine_Init();

/** Exit StateMachine module.
 */
extern orxDLLAPI orxVOID orxStateMachine_Exit();
/** @} */


/***************************************************************************
 * Enum declaration                                                        *
 ***************************************************************************/

/** State types. */
typedef enum __orxSTATEMACHINE_STATE_TYPE_t
{
  orxSTATEMACHINE_STATE_TYPE_INDIFFERENT = 0,
  orxSTATEMACHINE_STATE_TYPE_ACCEPTOR,
  orxSTATEMACHINE_STATE_TYPE_NONACCEPTOR
} orxSTATEMACHINE_STATE_TYPE;


/***************************************************************************
 * Pointer declaration                                                     *
 ***************************************************************************/

/** Action pointer for state machines. */
typedef orxVOID (*orxSTATEMACHINE_ACTION_PTR)(void);

/** condition pointer for state machines. */
typedef orxBOOL (*orxSTATEMACHINE_CONDITION_PTR)(void);


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Create a state machine and return a pointer on it.
 * @param[in] _u16NbStates          Number of states.
 * @param[in] _u32NbLinks           Number of links.
 * @param[in] _u32Flags             Flags used by the state machine.
 * @param[in] _eMemType             Memory type to use.
 * @return Returns a pointer on the state machine or orxNULL if failed.
 */
extern orxDLLAPI orxSTATEMACHINE *          orxStateMachine_Create(orxU16 _u16NbStates, orxU32 _u32NbLinks, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Delete a state machine.
 * @param[in] _pstStateMachine      The state machine to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxVOID                    orxStateMachine_Delete(orxSTATEMACHINE * _pstStateMachine);

/** Clear a state machine
 * @param[in] _pstStateMachine      The state machine to clear.
 */
extern orxDLLAPI orxVOID                    orxStateMachine_Clear(orxSTATEMACHINE * _pstStateMachine);

/** Add a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                Identifier for the state.
 * @param[in] _eStateType           Type of state.
 * @param[in] _pAction              Action callback.
 * @return Returns the new state.
 */
extern orxDLLAPI orxSTATEMACHINE_STATE *    orxStateMachine_State_Add(orxSTATEMACHINE * _pstStateMachine, orxU16 _u16Id, orxSTATEMACHINE_STATE_TYPE _eStateType, orxSTATEMACHINE_ACTION_PTR _pAction);

/** Find a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                The identifier of the state.
 * @return Returns the state.
 */
extern orxDLLAPI orxSTATEMACHINE_STATE *    orxStateMachine_State_Get(orxSTATEMACHINE * _pstStateMachine, orxU16 _u16Id);

/** Remove a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS                  orxStateMachine_State_Remove(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstState, orxBOOL _bRemoveLinks);

/** Add a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @param[in] _pCondition           Condition callback.
 * @param[in] _pAction              Action callback.
 * @return Returns the new link.
 */
extern orxDLLAPI orxSTATEMACHINE_LINK *     orxStateMachine_Link_Add(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstBeginningState, orxSTATEMACHINE_STATE * _pstEndingState, orxSTATEMACHINE_CONDITION_PTR _pCondition, orxSTATEMACHINE_ACTION_PTR _pAction);

/** Find a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @return Returns the corresponding link.
 */
extern orxDLLAPI orxSTATEMACHINE_LINK *     orxStateMachine_Link_Get(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstBeginningState, orxSTATEMACHINE_STATE * _pstEndingState);

/** Remove a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstLink              The link to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS                  orxStateMachine_Link_Remove(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_LINK * _pstLink);

/** Clear all links.
 * @param[in] _pstStateMachine      The state machine.
 */
extern orxDLLAPI orxVOID                  orxStateMachine_Link_Clear(orxSTATEMACHINE * _pstStateMachine);

/** Get next state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The current state.
 * @return Returns the next state by following a link (and executes respective callbacks). If _pstState is orxNULL, the initial state will be returned (and its callback executed). Returns orxNULL when no more state can be returned.
 */
extern orxDLLAPI orxSTATEMACHINE_STATE *    orxStateMachine_Link_GetNext(orxSTATEMACHINE * _pstStateMachine, orxSTATEMACHINE_STATE * _pstState);


#endif /** _orxSTATEMACHINE_H_ */

/** @} */

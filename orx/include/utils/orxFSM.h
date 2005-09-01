/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxFSM.h
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

#ifndef _orxFSM_H_
#define _orxFSM_H_

#include "orxInclude.h"
#include "memory/orxMemory.h"

/* Internal state structure. */
typedef struct __orxFSM_STATE_t orxFSM_STATE;

/* Internal link structure. */
typedef struct __orxFSM_LINK_t orxFSM_LINK;

/* Internal state machine structure. */
typedef struct __orxFSM_t orxFSM;

/* Internal instance structure. */
typedef struct __orxFSM_INSTANCE_t orxFSM_INSTANCE;

/* Define flags. */
#define orxFSM_KU32_FLAGS_NONE            0x00000000  /**< No flags (default behaviour) */
#define orxFSM_KU32_FLAGS_NOT_EXPANDABLE  0x00000001  /**< The state machine will not be expandable */

/** @name Module management.
 * @{ */
/** Initialize StateMachine Module.
 * @return Returns the initialization status.
 */
extern orxDLLAPI orxSTATUS orxFSM_Init();

/** Exit StateMachine module.
 */
extern orxDLLAPI orxVOID orxFSM_Exit();
/** @} */


/***************************************************************************
 * Pointer declaration                                                     *
 ***************************************************************************/

/** Action pointer for state machines. */
typedef orxVOID (*orxFSM_ACTION_PTR)(orxVOID);

/** condition pointer for state machines. */
typedef orxBOOL (*orxFSM_CONDITION_PTR)(orxVOID);


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Create a state machine and return a pointer on it.
 * @param[in] _u16NbStates          Number of states.
 * @param[in] _u32NbLinks           Number of links.
 * @param[in] _u32NbInstances       Number of instances.
 * @param[in] _u32Flags             Flags used by the state machine.
 * @param[in] _eMemType             Memory type to use.
 * @return Returns a pointer on the state machine or orxNULL if failed.
 */
extern orxFSM * orxDLLAPI                   orxFSM_Create(orxU16 _u16NbStates, orxU32 _u32NbLinks, orxU32 _u32NbInstances, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/** Delete a state machine.
 * @param[in] _pstStateMachine      The state machine to remove.
 * @return Returns the status of the operation.
 */
extern orxSTATUS orxDLLAPI                  orxFSM_Delete(orxFSM * _pstStateMachine);

/** Clear a state machine
 * @param[in] _pstStateMachine      The state machine to clear.
 */
extern orxVOID orxDLLAPI                    orxFSM_Clear(orxFSM * _pstStateMachine);

/** Add a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                Identifier for the state.
 * @param[in] _cbInit               Init callback.
 * @param[in] _cbExecute            Execute callback.
 * @param[in] _cbExit               Exit callback.
 * @return Returns the new state.
 */
extern orxFSM_STATE * orxDLLAPI             orxFSM_State_Add(orxFSM * _pstStateMachine, orxU16 _u16Id, orxFSM_ACTION_PTR _cbInit, orxFSM_ACTION_PTR _cbExecute, orxFSM_ACTION_PTR _cbExit);

/** Find a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                The identifier of the state.
 * @return Returns the state.
 */
extern orxFSM_STATE * orxDLLAPI             orxFSM_State_Get(orxFSM * _pstStateMachine, orxU16 _u16Id);

/** Remove a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state to remove.
 * @return Returns the status of the operation.
 */
extern orxSTATUS orxDLLAPI                  orxFSM_State_Remove(orxFSM * _pstStateMachine, orxFSM_STATE * _pstState, orxBOOL _bRemoveLinks);

/** Add a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @param[in] _cbCondition          Condition callback.
 * @return Returns the new link.
 */
extern orxFSM_LINK * orxDLLAPI              orxFSM_Link_Add(orxFSM * _pstStateMachine, orxFSM_STATE * _pstBeginningState, orxFSM_STATE * _pstEndingState, orxFSM_CONDITION_PTR _cbCondition);

/** Find a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @return Returns the corresponding link.
 */
extern orxFSM_LINK * orxDLLAPI              orxFSM_Link_Get(orxFSM * _pstStateMachine, orxFSM_STATE * _pstBeginningState, orxFSM_STATE * _pstEndingState);

/** Remove a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstLink              The link to remove.
 * @return Returns the status of the operation.
 */
extern orxSTATUS orxDLLAPI                  orxFSM_Link_Remove(orxFSM * _pstStateMachine, orxFSM_LINK * _pstLink);

/** Clear all links.
 * @param[in] _pstStateMachine      The state machine.
 */
extern orxVOID orxDLLAPI                    orxFSM_Link_Clear(orxFSM * _pstStateMachine);

/** Create an instance of a state machine.
 * @param[in] _pstStateMachine      The state machine.
 * @return Returns the instance.
 */
extern orxFSM_INSTANCE * orxDLLAPI          orxFSM_Instance_Create(orxFSM * _pstStateMachine);

/** Remove an instance of a state machine.
 * @param[in] _pstInstance          The instance to remove.
 * @return Returns the status of the operation.
 */
extern orxSTATUS orxDLLAPI                  orxFSM_Instance_Remove(orxFSM_INSTANCE * _pstInstance);

/** Update an instance of a state machine. If current state is orxNULL, it enters the initial state. 
 * @param[in] _pstInstance          The instance.
 * @return Returns the status of the operation. It fails if nothing has happend.
 */
extern orxSTATUS orxDLLAPI                  orxFSM_Instance_Update(orxFSM_INSTANCE * _pstInstance);

/** Update all instances of a state machine. If current state is orxNULL, it enters the initial state.
 * @param[in] _pstStateMachine      The state machine.
 * @return Returns the status of the operation. It fails if nothing has happend.
 */
extern orxSTATUS orxDLLAPI                  orxFSM_Update(orxFSM * _pstStateMachine);


#endif /** _orxFSM_H_ */

/** @} */

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
 * @file orxFSM.h
 * @date 20/06/2005
 * @author Corrosif (???@???.???)
 *
 * @todo
 */

/**
 * @addtogroup orxFSM
 * 
 * FSM module
 * Module that handles FSMs
 *
 * @{
 */


#ifndef _orxFSM_H_
#define _orxFSM_H_

#include "orxInclude.h"
#include "memory/orxMemory.h"


/*Define flags. */
#define orxFSM_KU32_FLAG_NONE                 0x00000000  /**< No flags (default behaviour) */
#define orxFSM_KU32_FLAG_NOT_EXPANDABLE       0x00000001  /**< The state machine will not be expandable */


/*Internal state structure. */
typedef struct __orxFSM_STATE_t               orxFSM_STATE;

/*Internal link structure. */
typedef struct __orxFSM_LINK_t                orxFSM_LINK;

/*Internal state machine structure. */
typedef struct __orxFSM_t                     orxFSM;

/*Internal instance structure. */
typedef struct __orxFSM_INSTANCE_t            orxFSM_INSTANCE;


/** @name Module management.
 * @{ */

/** StateMachine module setup.
 */
extern orxDLLAPI void orxFASTCALL             orxFSM_Setup();
/**Initialize StateMachine Module.
 * @return Returns the initialization status.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_Init();

/**Exit StateMachine module.
 */
extern orxDLLAPI void orxFASTCALL             orxFSM_Exit();
/** @} */


/***************************************************************************
 *Pointer declaration                                                     *
 ***************************************************************************/

/**Action pointer for state machines. */
typedef void (orxFASTCALL *orxFSM_ACTION_FUNCTION)();

/**condition pointer for state machines. */
typedef orxBOOL (orxFASTCALL *orxFSM_CONDITION_FUNCTION)();


/***************************************************************************
 *Public functions                                                        *
 ***************************************************************************/

/**Create a state machine and return a pointer on it.
 * @param[in] _u16NbStates          Number of states.
 * @param[in] _u32NbLinks           Number of links.
 * @param[in] _u32NbInstances       Number of instances.
 * @param[in] _u32Flags             Flags used by the state machine.
 * @param[in] _eMemType             Memory type to use.
 * @return Returns a pointer on the state machine or orxNULL if failed.
 */
extern orxDLLAPI orxFSM *orxFASTCALL          orxFSM_Create(orxU16 _u16NbStates, orxU32 _u32NbLinks, orxU32 _u32NbInstances, orxU32 _u32Flags, orxMEMORY_TYPE _eMemType);

/**Delete a state machine.
 * @param[in] _pstStateMachine      The state machine to remove.
 */
extern orxDLLAPI void orxFASTCALL             orxFSM_Delete(orxFSM *_pstStateMachine);

/**Clear a state machine
 * @param[in] _pstStateMachine      The state machine to clear.
 */
extern orxDLLAPI void orxFASTCALL             orxFSM_Clear(orxFSM *_pstStateMachine);

/**Add a state, setting it as the initial state if it is the first one.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                Identifier for the state.
 * @param[in] _pfnInit               Init callback.
 * @param[in] _pfnExecute            Execute callback.
 * @param[in] _pfnExit               Exit callback.
 * @return Returns the new state.
 */
extern orxDLLAPI orxFSM_STATE *orxFASTCALL    orxFSM_AddState(orxFSM *_pstStateMachine, orxU16 _u16Id, const orxFSM_ACTION_FUNCTION _pfnInit, const orxFSM_ACTION_FUNCTION _pfnExecute, const orxFSM_ACTION_FUNCTION _pfnExit);

/**Set an initial state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstInitialState      The initial state.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_SetInitState(orxFSM *_pstStateMachine, orxFSM_STATE *_pstInitialState);

/**Get a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _u16Id                The identifier of the state.
 * @return Returns the state.
 */
extern orxDLLAPI orxFSM_STATE *orxFASTCALL    orxFSM_GetState(const orxFSM *_pstStateMachine, orxU16 _u16Id);

/**Get a state Id.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state.
 * @return Returns the Id of the state.
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxFSM_GetStateID(const orxFSM *_pstStateMachine, const orxFSM_STATE *_pstState);

/**Remove a state.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstState             The state to remove.
 * @param[in] _bRemoveLinks         Should remove links?
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_RemoveState(orxFSM *_pstStateMachine, orxFSM_STATE *_pstState, orxBOOL _bRemoveLinks);

/**Add a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @param[in] _pfnCondition         Condition callback.
 * @return Returns the new link.
 */
extern orxDLLAPI orxFSM_LINK *orxFASTCALL     orxFSM_AddLink(orxFSM *_pstStateMachine, orxFSM_STATE *_pstBeginningState, orxFSM_STATE *_pstEndingState, const orxFSM_CONDITION_FUNCTION _pfnCondition);

/**Find a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstBeginningState    The state marking the beginning of the link.
 * @param[in] _pstEndingState       The state marking the ending of the link.
 * @return Returns the corresponding link.
 */
extern orxDLLAPI orxFSM_LINK *orxFASTCALL     orxFSM_GetLink(const orxFSM *_pstStateMachine, const orxFSM_STATE *_pstBeginningState, const orxFSM_STATE *_pstEndingState);

/**Remove a link.
 * @param[in] _pstStateMachine      The state machine.
 * @param[in] _pstLink              The link to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_RemoveLink(orxFSM *_pstStateMachine, orxFSM_LINK *_pstLink);

/**Clear all links.
 * @param[in] _pstStateMachine      The state machine.
 */
extern orxDLLAPI void orxFASTCALL             orxFSM_ClearLink(orxFSM *_pstStateMachine);

/**Create an instance of a state machine.
 * @param[in] _pstStateMachine      The state machine.
 * @return Returns the instance.
 */
extern orxDLLAPI orxFSM_INSTANCE *orxFASTCALL orxFSM_CreateInstance(orxFSM *_pstStateMachine);

/**Remove an instance of a state machine.
 * @param[in] _pstInstance          The instance to remove.
 * @return Returns the status of the operation.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_DeleteInstance(orxFSM_INSTANCE *_pstInstance);

/**Get the state machine associated to an instance.
 * @param[in] _pstInstance          The instance.
 * @return Returns the state machine.
 */
extern orxDLLAPI orxFSM *orxFASTCALL          orxFSM_GetFSM(const orxFSM_INSTANCE *_pstInstance);

/**Get the current state of an instance.
 * @param[in] _pstInstance          The instance.
 * @return Returns the current state.
 */
extern orxDLLAPI orxFSM_STATE *orxFASTCALL    orxFSM_GetInstanceState(const orxFSM_INSTANCE *_pstInstance);

/**Update an instance of a state machine. If current state is orxNULL, it enters the initial state. 
 * @param[in] _pstInstance          The instance.
 * @return Returns the status of the operation. It fails if nothing has happend.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_UpdateInstance(orxFSM_INSTANCE *_pstInstance);

/**Update all instances of a state machine. If current state is orxNULL, it enters the initial state.
 * @param[in] _pstStateMachine      The state machine.
 * @return Returns the status of the operation. It fails if nothing has happend.
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxFSM_Update(orxFSM *_pstStateMachine);

#endif /**_orxFSM_H_ */

/** @} */

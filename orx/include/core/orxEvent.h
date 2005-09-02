/**
 * @file core/orxEvent.h
 */

/***************************************************************************
 begin                : 01/09/2005
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @addtogroup Event
 * @{
 */

#ifndef _orxEVENT_H_
#define _orxEVENT_H_

#include "orxInclude.h"

/**
 *  Event message manipulation :
 */

/** Event message ID type.*/
typedef orxU32 orxEVENT_MESSAGE_ID;

/** Event message owner/type type.*/
typedef orxU16 orxEVENT_MESSAGE_TYPE;

/** Event message life time step type.*/
typedef orxS16 orxEVENT_MESSAGE_LIFETIME;

/** Event message ID constructor.*/
#define orxEVENT_MESSAGE_GET_ID(_TYPE, _LIFETIME) ((orxU32)((_TYPE<<16)|_LIFETIME))

/** Event message type accessor.*/
#define orxEVENT_MESSAGE_GET_TYPE(_ID)	((orxU16)((_ID&0xFFFF0000)>>16))

/** Event message lifetime accessor.*/
#define orxEVENT_MESSAGE_GET_LIFETIME(_ID)	((orxS16)(_ID&0xFFFF))

/**
 * Event handler callback prototype.
 */
typedef orxVOID (*orxEVENT_CB)(orxEVENT_MESSAGE_TYPE, orxEVENT_MESSAGE_LIFETIME, orxVOID*);


/**
 * Event manager definition and manipulation :
 */
 
/** Event manager.*/
typedef struct __orxEVENT_MANAGER_t orxEVENT_MANAGER;

/** Create an event manager.
 * @param _u16EventNumber Number of event the manager can store.
 * @param _u16HandlerNumber Number of handler the manager can store.
 * @return Address of the manager structure, orxNULL if failed.
 */
extern orxEVENT_MANAGER *orxEventManager_Create(orxU16 _u16EventNumber, orxU16 _u16HandlerNumber);

/** Delete an event manager.
 * @param _pstEventManager Event manager to destroy.
 */
extern orxVOID orxEventManager_Delete(orxEVENT_MANAGER* _pstEventManager);

/** Register an event callback function.
 * Unregister previous handler and set the new instead.
 * @param _pstEventManager Event manager.
 * @param _u16Type Type of event to intercept.
 * @param _cbHandler Event callback function, orxNULL to only unregister previous handler.
 */
extern orxVOID orxEventManager_RegisterHandler(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_CB _cbHandler);

/** Add an event to the manager.
 * @param _pstEventManager Event manager.
 * @param _u16Type Type of event.
 * @param _s16Life Remaining lifetime.
 * @param _pExtraData Address of extra data.
 */
extern orxVOID orxEventManager_AddEvent(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_MESSAGE_LIFETIME _s16Life, orxVOID* _pExtraData);

/** Process the events.
 * @param _pstEventManager Event manager.
 */
extern orxVOID orxEventManager_ProcessEvent(orxEVENT_MANAGER* _pstEventManager);


/** Initialize Event Module
 */
extern orxSTATUS orxDLLAPI orxEvent_Init();

/** Exit Event module
 */
extern orxVOID orxDLLAPI orxEvent_Exit();

#endif /*_orxEVENT_H_*/

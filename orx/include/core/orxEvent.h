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
#include "core/orxClock.h"

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

/** Event message lifetime constant.
 * Minimum negative value.*/
#define orxEVENT_MESSAGE_LIFETIME_CONSTANT	0x7FFF

/**
 * Event handler callback prototype.
 */
typedef orxVOID (*orxEVENT_FUNCTION)(orxEVENT_MESSAGE_TYPE, orxEVENT_MESSAGE_LIFETIME, orxVOID*);


/**
 * Event manager definition and manipulation :
 */
 
/** Event manager.*/
typedef struct __orxEVENT_MANAGER_t orxEVENT_MANAGER;

/** Event manager manipulation flags.*/
typedef enum __orxEVENT_MANAGER_MANIPULATION_t
{
  orxEVENT_MANAGER_MANIPULATION_STANDARD 						= 0x0000,	/**< Nothing special is done.*/
  orxEVENT_MANAGER_MANIPULATION_REMOVE_NEGATIVE_LIFETIME_EVENT	= 0x0001,   /**< Remove negative timelife event and do not process it. */
  orxEVENT_MANAGER_MANIPULATION_REMOVE_UNPROCESSED				= 0x0002,	/**< Remove all unprocessed events. Dangerous if used with partial process : possible loss of data.*/
  orxEVENT_MANAGER_MANIPULATION_PARTIAL_PROCESS					= 0x0010,	/**< Just do a partial process. */
  orxEVENT_MANAGER_MANIPULATION_ERROR = orxENUM_NONE    /**< Invalid Manipulation flag. */

} orxEVENT_MANAGER_MANIPULATION;


/** Initialize Event Module
 */
extern orxDLLAPI orxSTATUS                  orxEvent_Init();

/** Exit Event module
 */
extern orxDLLAPI orxVOID                    orxEvent_Exit();

/** Create an event manager.
 * @param _u16EventNumber Number of event the manager can store.
 * @param _u16HandlerNumber Number of handler the manager can store.
 * @param _u32Flags Flags of event manager.
 * @return Address of the manager structure, orxNULL if failed.
 */
extern orxDLLAPI orxEVENT_MANAGER *         orxEventManager_Create(orxU16 _u16EventNumber, orxU16 _u16HandlerNumber, orxU32 _u32Flags);

/** Delete an event manager.
 * @param _pstEventManager Event manager to destroy.
 */
extern orxDLLAPI orxVOID                    orxEventManager_Delete(orxEVENT_MANAGER* _pstEventManager);

/** Set the flags of event manager.
 * @param _pstEventManager Event manager.
 * @param _u32Flags Flags of event manager.
 */
extern orxDLLAPI orxVOID                    orxEventManager_SetFlags(orxEVENT_MANAGER* _pstEventManager, orxU32 _u32Flags);

/** Retrieve the flags of the event manager.
 * @param _pstEventManager Event manager.
 * @return Flags.
 */
extern orxDLLAPI orxU32                     orxEventManager_GetFlags(orxEVENT_MANAGER* _pstEventManager);

/** Register an event callback function.
 * Unregister previous handler and set the new instead.
 * @param _pstEventManager Event manager.
 * @param _u16Type Type of event to intercept.
 * @param _pfnHandler Event callback function, orxNULL to only unregister previous handler.
 */
extern orxDLLAPI orxVOID                    orxEventManager_RegisterHandler(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_FUNCTION _pfnHandler);

/** Add an event to the manager.
 * @param _pstEventManager Event manager.
 * @param _u16Type Type of event.
 * @param _s16Life Remaining lifetime.
 * @param _pExtraData Address of extra data.
 */
extern orxDLLAPI orxVOID                    orxEventManager_AddEvent(orxEVENT_MANAGER* _pstEventManager, orxEVENT_MESSAGE_TYPE _u16Type, orxEVENT_MESSAGE_LIFETIME _s16Life, orxVOID* _pExtraData);

/** Process events of the manager.
 * Process the events.
 * @param _pstEventManager Event manager.
 * @param _s16Ticks Number of ticks to remove from the lifetime.
 */
extern orxDLLAPI orxVOID                    orxEventManager_ProcessEvent(orxEVENT_MANAGER* _pstEventManager, orxS16 _s16Ticks);

/** Update the event manager.
 * Function to plug into clock system.
 * @param _pstClockInfo Clock infos.
 * @param _pstContext Context. Here it is orxEVENT_MANAGER address.
 **/
extern orxDLLAPI orxVOID                    orxEventManager_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext);


#endif /*_orxEVENT_H_*/

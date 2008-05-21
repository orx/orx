/**
 * @file orxEvent.c
 */

/***************************************************************************
 orxEvent.c
 Event module
 begin                : 20/05/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxEVENT_H_
#define _orxEVENT_H_

#include "orxInclude.h"


/** Event type enum
 */
typedef enum __orxEVENT_TYPE_t
{
  orxEVENT_TYPE_SYSTEM = 0,
  orxEVENT_TYPE_RENDER,
  orxEVENT_TYPE_PHYSICS,

  orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_USER_DEFINED = orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_MAX_NUMBER = 256,

  orxEVENT_TYPE_NONE = orxENUM_NONE,

} orxEVENT_TYPE;

/** Public event structure
 */
typedef struct __orxEVENT_t
{
  orxEVENT_TYPE     eType;                            /**< Event type : 4 */
  orxHANDLE         hSender;                          /**< Sender handle : 8 */
  orxHANDLE         hRecipient;                       /**< Recipient handle : 12 */
  orxVOID          *pstPayload;                       /**> Event payload : 16 */

} orxEVENT;


/**
 * Event handler type / return orxTRUE if events processing should be stopped for the current event, orxFALSE otherwise
 */
typedef orxBOOL (orxFASTCALL *orxEVENT_HANDLER)(orxCONST orxEVENT *_pstEvent);


/** Event module setup
 */
extern orxDLLAPI orxVOID                orxEvent_Setup();

/** Initializes the event Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxEvent_Init();

/** Exits from the event Module
 */
extern orxDLLAPI orxVOID                orxEvent_Exit();

/** Adds an event handler
 * @param _eEventType           Concerned type of event
 * @param _pfnHandler           Event handler to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_AddHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler);

/** Sends an event
 * @param _pstEvent             Event to send
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_Send(orxCONST orxEVENT *_pstEvent);

/** Removes an event handler
 * @param _eEventType           Concerned type of event
 * @param _pfnHandler           Event handler to remove
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_RemoveHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler);


#endif /*_orxEVENT_H_*/

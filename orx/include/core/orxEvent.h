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
 * @file orxEvent.h
 * @date 20/05/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxEvent
 * 
 * Event module
 * Module that handles internal events
 * @{
 */


#ifndef _orxEVENT_H_
#define _orxEVENT_H_

#include "orxInclude.h"


/** Event type enum
 */
typedef enum __orxEVENT_TYPE_t
{
  orxEVENT_TYPE_SYSTEM = 0,
  orxEVENT_TYPE_ANIM,
  orxEVENT_TYPE_FX,
  orxEVENT_TYPE_RENDER,
  orxEVENT_TYPE_PHYSICS,
  orxEVENT_TYPE_SOUND,
  orxEVENT_TYPE_STRUCTURE,

  orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_USER_DEFINED = orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_USER_MAX_NUMBER = 256,

  orxEVENT_TYPE_FIRST_RESERVED,

  orxEVENT_TYPE_NONE = orxENUM_NONE,

} orxEVENT_TYPE;

/** Public event structure
 */
typedef struct __orxEVENT_t
{
  orxEVENT_TYPE     eType;                            /**< Event type : 4 */
  orxENUM           eID;                              /**< Event ID : 8 */
  orxHANDLE         hSender;                          /**< Sender handle : 12 */
  orxHANDLE         hRecipient;                       /**< Recipient handle : 16 */
  orxVOID          *pstPayload;                       /**> Event payload : 20 */

} orxEVENT;


/**
 * Event handler type / return orxSTATUS_FAILURE if events processing should be stopped for the current event, orxSTATUS_FAILURE otherwise
 */
typedef orxSTATUS (orxFASTCALL *orxEVENT_HANDLER)(orxCONST orxEVENT *_pstEvent);


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

/** @} */

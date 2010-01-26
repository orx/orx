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


/** Helper defines
 */
#define orxEVENT_INIT(EVENT, TYPE, ID, SENDER, RECIPIENT, PAYLOAD)  \
do                                                                  \
{                                                                   \
  EVENT.eType       = (orxEVENT_TYPE)(TYPE);                        \
  EVENT.eID         = (orxENUM)(ID);                                \
  EVENT.hSender     = (orxHANDLE)(SENDER);                          \
  EVENT.hRecipient  = (orxHANDLE)(RECIPIENT);                       \
  EVENT.pstPayload  = (void *)(PAYLOAD);                            \
} while(0)

#define orxEVENT_SEND(TYPE, ID, SENDER, RECIPIENT, PAYLOAD)         \
do                                                                  \
{                                                                   \
  orxEVENT stEvent;                                                 \
  orxEVENT_INIT(stEvent, TYPE, ID, SENDER, RECIPIENT, PAYLOAD);     \
  orxEvent_Send(&stEvent);                                          \
} while(0)


/** Event type enum
 */
typedef enum __orxEVENT_TYPE_t
{
  orxEVENT_TYPE_ANIM = 0,
  orxEVENT_TYPE_CLOCK,
  orxEVENT_TYPE_FX,
  orxEVENT_TYPE_INPUT,
  orxEVENT_TYPE_JOYSTICK,
  orxEVENT_TYPE_KEYBOARD,
  orxEVENT_TYPE_LOCALE,
  orxEVENT_TYPE_MOUSE,
  orxEVENT_TYPE_OBJECT,
  orxEVENT_TYPE_RENDER,
  orxEVENT_TYPE_PHYSICS,
  orxEVENT_TYPE_SHADER,
  orxEVENT_TYPE_SOUND,
  orxEVENT_TYPE_SPAWNER,
  orxEVENT_TYPE_SYSTEM,

  orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_FIRST_RESERVED = orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_LAST_RESERVED = 127,

  orxEVENT_TYPE_USER_DEFINED,

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
  void             *pstPayload;                       /**< Event payload : 20 */

} orxEVENT;


/**
 * Event handler type / return orxSTATUS_FAILURE if events processing should be stopped for the current event, orxSTATUS_FAILURE otherwise
 */
typedef orxSTATUS (orxFASTCALL *orxEVENT_HANDLER)(const orxEVENT *_pstEvent);


/** Event module setup
 */
extern orxDLLAPI void orxFASTCALL       orxEvent_Setup();

/** Initializes the event Module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_Init();

/** Exits from the event Module
 */
extern orxDLLAPI void orxFASTCALL       orxEvent_Exit();

/** Adds an event handler
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnEventHandler      Event handler to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_AddHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler);

/** Removes an event handler
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnEventHandler      Event handler to remove
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_RemoveHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler);

/** Sends an event
 * @param[in] _pstEvent             Event to send
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_Send(const orxEVENT *_pstEvent);

/** Sends a simple event
 * @param[in] _eEventType           Event type
 * @param[in] _eEventID             Event ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_SendShort(orxEVENT_TYPE _eEventType, orxENUM _eEventID);

#endif /*_orxEVENT_H_*/

/** @} */

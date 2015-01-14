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
} while(orxFALSE)

#define orxEVENT_SEND(TYPE, ID, SENDER, RECIPIENT, PAYLOAD)         \
do                                                                  \
{                                                                   \
  orxEVENT stEvent;                                                 \
  orxEVENT_INIT(stEvent, TYPE, ID, SENDER, RECIPIENT, PAYLOAD);     \
  orxEvent_Send(&stEvent);                                          \
} while(orxFALSE)


/** Event type enum
 */
typedef enum __orxEVENT_TYPE_t
{
  orxEVENT_TYPE_ANIM = 0,
  orxEVENT_TYPE_CLOCK,
  orxEVENT_TYPE_CONFIG,
  orxEVENT_TYPE_DISPLAY,
  orxEVENT_TYPE_FX,
  orxEVENT_TYPE_INPUT,
  orxEVENT_TYPE_LOCALE,
  orxEVENT_TYPE_OBJECT,
  orxEVENT_TYPE_RENDER,
  orxEVENT_TYPE_PHYSICS,
  orxEVENT_TYPE_RESOURCE,
  orxEVENT_TYPE_SHADER,
  orxEVENT_TYPE_SOUND,
  orxEVENT_TYPE_SPAWNER,
  orxEVENT_TYPE_SYSTEM,
  orxEVENT_TYPE_TEXTURE,
  orxEVENT_TYPE_TIMELINE,
  orxEVENT_TYPE_VIEWPORT,

  orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_FIRST_RESERVED = orxEVENT_TYPE_CORE_NUMBER,

  orxEVENT_TYPE_LAST_RESERVED = 255,

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
  void             *pContext;                         /**< Optional user-provided context : 24 */

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

/** Adds an event handler with user-defined context
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnEventHandler      Event handler to add
 * @param[in] _pContext             Context that will be stored in events sent to this handler
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_AddHandlerWithContext(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler, void *_pContext);

/** Removes an event handler
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnEventHandler      Event handler to remove
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_RemoveHandler(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler);

/** Removes an event handler which matches given context
 * @param[in] _eEventType           Concerned type of event
 * @param[in] _pfnEventHandler      Event handler to remove
 * @param[in] _pContext             Context of the handler to remove, orxNULL for removing all occurrences regardless of their context
 * return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_RemoveHandlerWithContext(orxEVENT_TYPE _eEventType, orxEVENT_HANDLER _pfnEventHandler, void *_pContext);

/** Sends an event
 * @param[in] _pstEvent             Event to send
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_Send(orxEVENT *_pstEvent);

/** Sends a simple event
 * @param[in] _eEventType           Event type
 * @param[in] _eEventID             Event ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxEvent_SendShort(orxEVENT_TYPE _eEventType, orxENUM _eEventID);

/** Is currently sending an event?
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxEvent_IsSending();

#endif /*_orxEVENT_H_*/

/** @} */

/**
 * \file event.h
 * 
 * Event Module.
 * Manages events.
 * 
 * \todo
 * ???
 */

/***************************************************************************
 event.h
 messaging system

 begin                : 02/09/2002
 author               : (C) Gdp
 email                : snegri@free.fr
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _EVENT_H_
#define _EVENT_H_

#include "include.h"

typedef enum e_event_type_t
{
  EVENT_NONE,
  MSG1,
  TIMER_5S,
  SIGNAL,
  EVENT_ALL
} event_e_type;

typedef void(*pfn_event_callback)(event_e_type);

extern void   event_init();
extern uint32 event_register(event_e_type _e_type, pfn_event_callback _pst_function);
extern uint32 event_unregister_function(event_e_type _e_type, pfn_event_callback _pst_function);
extern uint32 event_send(event_e_type _e_type); /* future: queue event to be sent async */

#endif /* _EVENT_H_ */

/**
 * \file keyboard.h
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "include.h"


extern void     keyboard_plugin_init();

extern uint32 (*keyboard_init)();
extern void   (*keyboard_exit)();
extern int32  (*keyboard_hit)();
extern int32  (*keyboard_read)();
extern int32  (*keyboard_keystate_get)(uint8 _uc_key);
extern void   (*keyboard_buffer_clear)();

#endif /* _KEYBOARD_H_ */

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

#include "orxInclude.h"


extern orxVOID     keyboard_plugin_init();

extern orxU32 (*keyboard_init)();
extern orxVOID   (*keyboard_exit)();
extern orxS32  (*keyboard_hit)();
extern orxS32  (*keyboard_read)();
extern orxS32  (*keyboard_keystate_get)(orxU8 _uc_key);
extern orxVOID   (*keyboard_buffer_clear)();

#endif /* _KEYBOARD_H_ */

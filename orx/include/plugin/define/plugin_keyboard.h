/**
 * \file plugin_keyboard.h
 * This header is used to define ID for keyboard plugin registration.
 */

/*
 begin                : 21/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _PLUGIN_KEYBOARD_H_
#define _PLUGIN_KEYBOARD_H_

/*********************************************
 Constants
 *********************************************/

#define PLUGIN_KEYBOARD_KU32_PLUGIN_ID              0x10000300

#define PLUGIN_KEYBOARD_KU32_FUNCTION_NUMBER        0x00000006

#define PLUGIN_KEYBOARD_KU32_ID_INIT                0x10000300
#define PLUGIN_KEYBOARD_KU32_ID_EXIT                0x10000301
#define PLUGIN_KEYBOARD_KU32_ID_HIT                 0x10000302
#define PLUGIN_KEYBOARD_KU32_ID_READ                0x10000303
#define PLUGIN_KEYBOARD_KU32_ID_KEYSTATE_GET        0x10000304
#define PLUGIN_KEYBOARD_KU32_ID_BUFFER_CLEAR        0x10000305

#endif /* _PLUGIN_KEYBOARD_H_ */

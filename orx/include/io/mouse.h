/**
 * \file mouse.h
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

#ifndef _MOUSE_H_
#define _MOUSE_H_

#include "include.h"


extern void     mouse_plugin_init();

extern uint32 (*mouse_init)();
extern void   (*mouse_exit)();

extern void   (*mouse_move_get)(int32 *_i_dx, int32 *_i_dy);

#endif /* _MOUSE_H_ */

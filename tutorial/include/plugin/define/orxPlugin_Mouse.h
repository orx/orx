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
 * @file orxPlugin_Mouse.h
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all IDs of the mouse plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_MOUSE_H_
#define _orxPLUGIN_MOUSE_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_MOUSE_t
{
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_SET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_GET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_IS_BUTTON_PRESSED,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_GET_MOVE_DELTA,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_GET_WHEEL_DELTA,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_SHOW_CURSOR,

  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_MOUSE;

#endif /* _orxPLUGIN_MOUSE_H_ */

/** @} */

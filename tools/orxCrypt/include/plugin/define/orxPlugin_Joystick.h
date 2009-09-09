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
 * @file orxPlugin_Joystick.h
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all IDs of the joystick plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_JOYSTICK_H_
#define _orxPLUGIN_JOYSTICK_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_t
{
  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_EXIT,

  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_GET_AXIS_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_IS_BUTTON_PRESSED,

  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK;

#endif /* _orxPLUGIN_JOYSTICK_H_ */

/** @} */

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
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
  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_IS_CONNECTED,

  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_JOYSTICK;

#endif /* _orxPLUGIN_JOYSTICK_H_ */

/** @} */

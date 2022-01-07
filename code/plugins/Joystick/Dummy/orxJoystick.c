/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxJoystick.c
 * @date 23/01/2010
 * @author iarwain@orx-project.org
 *
 * Dummy joystick plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATUS orxFASTCALL orxJoystick_Dummy_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_Dummy_Exit()
{
  /* Done! */
  return;
}

orxFLOAT orxFASTCALL orxJoystick_Dummy_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_Dummy_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxBOOL bResult = orxFALSE;

  /* Done! */
  return bResult;
}

orxBOOL orxFASTCALL orxJoystick_Dummy_IsConnected(orxU32 _u32ID)
{
  orxBOOL bResult = orxFALSE;

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Dummy_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Dummy_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Dummy_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Dummy_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_Dummy_IsConnected, JOYSTICK, IS_CONNECTED);
orxPLUGIN_USER_CORE_FUNCTION_END();

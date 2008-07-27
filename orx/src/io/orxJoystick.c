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
 * @file orxJoystick.c
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "io/orxJoystick.h"
#include "plugin/orxPluginCore.h"


/** Joystick module setup
 */
orxVOID orxJoystick_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_MEMORY);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_GetAxisValue, orxFLOAT, orxU32, orxJOYSTICK_AXIS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_IsButtonPressed, orxBOOL, orxU32, orxJOYSTICK_BUTTON);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(JOYSTICK)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, INIT, orxJoystick_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, EXIT, orxJoystick_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, GET_AXIS_VALUE, orxJoystick_GetAxisValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, IS_BUTTON_PRESSED, orxJoystick_IsButtonPressed)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(JOYSTICK)


/* *** Core function implementations *** */

/** Inits the joystick module
 * @return Returns the status of the operation
 */
orxSTATUS orxJoystick_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Init)();
}

/** Exits from the joystick module
 */
orxVOID orxJoystick_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Exit)();
}

/** Gets mouse on screen position
 * @param _u32ID        ID of the joystick to test
 * @param _eAxis        Joystick axis to check
 * @return Value of the axis
 */
orxFLOAT orxJoystick_GetAxisValue(orxU32 _u32ID, orxJOYSTICK_AXIS _eAxis)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_GetAxisValue)(_u32ID, _eAxis);
}

/** Is joystick button pressed?
 * @param _u32ID        ID of the joystick to test
 * @param _eButton      Joystick button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxBOOL orxJoystick_IsButtonPressed(orxU32 _u32ID, orxJOYSTICK_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_IsButtonPressed)(_u32ID, _eButton);
}

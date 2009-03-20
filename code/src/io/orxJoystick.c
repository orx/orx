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
#include "debug//orxDebug.h"


/** Misc defines
 */
#define orxJOYSTICK_KZ_LITERAL_PREFIX             "JOY_"


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Joystick module setup
 */
void orxJoystick_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_MEMORY);

  return;
}

/** Gets axis literal name
 * @param[in] _eAxis        Concerned axis
 * @return Axis's name
 */
const orxSTRING orxFASTCALL orxJoystick_GetAxisName(orxJOYSTICK_AXIS _eAxis)
{
  orxSTRING zResult;

#define orxJOYSTICK_DECLARE_AXIS_NAME(AXIS) case orxJOYSTICK_##AXIS: zResult = orxJOYSTICK_KZ_LITERAL_PREFIX#AXIS; break

  /* Checks */
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Depending on axis */
  switch(_eAxis)
  {
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_X);
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_Y);
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_Z);
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_R);
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_U);
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_V);
    orxJOYSTICK_DECLARE_AXIS_NAME(AXIS_POV);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "No name defined for axis #%ld.", _eAxis);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets button literal name
 * @param[in] _eButton      Concerned button
 * @return Button's name
 */
const orxSTRING orxFASTCALL orxJoystick_GetButtonName(orxJOYSTICK_BUTTON _eButton)
{
  orxSTRING zResult;

#define orxJOYSTICK_DECLARE_BUTTON_NAME(BUTTON) case orxJOYSTICK_BUTTON_##BUTTON: zResult = orxJOYSTICK_KZ_LITERAL_PREFIX#BUTTON; break

  /* Checks */
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Depending on button */
  switch(_eButton)
  {
    orxJOYSTICK_DECLARE_BUTTON_NAME(1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(5);
    orxJOYSTICK_DECLARE_BUTTON_NAME(6);
    orxJOYSTICK_DECLARE_BUTTON_NAME(7);
    orxJOYSTICK_DECLARE_BUTTON_NAME(8);
    orxJOYSTICK_DECLARE_BUTTON_NAME(9);
    orxJOYSTICK_DECLARE_BUTTON_NAME(10);
    orxJOYSTICK_DECLARE_BUTTON_NAME(11);
    orxJOYSTICK_DECLARE_BUTTON_NAME(12);
    orxJOYSTICK_DECLARE_BUTTON_NAME(13);
    orxJOYSTICK_DECLARE_BUTTON_NAME(14);
    orxJOYSTICK_DECLARE_BUTTON_NAME(15);
    orxJOYSTICK_DECLARE_BUTTON_NAME(16);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "No name defined for button #%ld.", _eButton);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_Exit, void, void);
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
void orxJoystick_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Exit)();
}

/** Gets mouse on screen position
 * @param[in] _u32ID        ID of the joystick to test
 * @param[in] _eAxis        Joystick axis to check
 * @return Value of the axis
 */
orxFLOAT orxJoystick_GetAxisValue(orxU32 _u32ID, orxJOYSTICK_AXIS _eAxis)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_GetAxisValue)(_u32ID, _eAxis);
}

/** Is joystick button pressed?
 * @param[in] _u32ID        ID of the joystick to test
 * @param[in] _eButton      Joystick button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxBOOL orxJoystick_IsButtonPressed(orxU32 _u32ID, orxJOYSTICK_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_IsButtonPressed)(_u32ID, _eButton);
}

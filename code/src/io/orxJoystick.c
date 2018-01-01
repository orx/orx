/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxJoystick.h"
#include "plugin/orxPluginCore.h"
#include "debug/orxDebug.h"


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
void orxFASTCALL orxJoystick_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_JOYSTICK, orxMODULE_ID_DISPLAY);

  return;
}

/** Gets button literal name
 * @param[in] _eButton      Concerned button
 * @return Button's name
 */
const orxSTRING orxFASTCALL orxJoystick_GetButtonName(orxJOYSTICK_BUTTON _eButton)
{
  const orxSTRING zResult;

#define orxJOYSTICK_DECLARE_BUTTON_NAME(BUTTON) case orxJOYSTICK_BUTTON_##BUTTON: zResult = orxJOYSTICK_KZ_LITERAL_PREFIX#BUTTON; break

  /* Checks */
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Depending on button */
  switch(_eButton)
  {
    orxJOYSTICK_DECLARE_BUTTON_NAME(1_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(2_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(3_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(4_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(5_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(6_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(7_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(8_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(9_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(10_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(11_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(12_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(13_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(14_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(15_1);
    orxJOYSTICK_DECLARE_BUTTON_NAME(16_1);

    orxJOYSTICK_DECLARE_BUTTON_NAME(1_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(2_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(3_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(4_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(5_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(6_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(7_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(8_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(9_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(10_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(11_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(12_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(13_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(14_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(15_2);
    orxJOYSTICK_DECLARE_BUTTON_NAME(16_2);

    orxJOYSTICK_DECLARE_BUTTON_NAME(1_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(2_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(3_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(4_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(5_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(6_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(7_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(8_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(9_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(10_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(11_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(12_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(13_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(14_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(15_3);
    orxJOYSTICK_DECLARE_BUTTON_NAME(16_3);

    orxJOYSTICK_DECLARE_BUTTON_NAME(1_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(2_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(3_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(4_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(5_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(6_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(7_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(8_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(9_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(10_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(11_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(12_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(13_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(14_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(15_4);
    orxJOYSTICK_DECLARE_BUTTON_NAME(16_4);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "No name defined for button #%d.", _eButton);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets axis literal name
 * @param[in] _eAxis        Concerned axis
 * @return Axis's name
 */
const orxSTRING orxFASTCALL orxJoystick_GetAxisName(orxJOYSTICK_AXIS _eAxis)
{
  const orxSTRING zResult;

#define orxJOYSTICK_DECLARE_AXIS_NAME(AXIS) case orxJOYSTICK_AXIS_##AXIS: zResult = orxJOYSTICK_KZ_LITERAL_PREFIX#AXIS; break

  /* Checks */
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Depending on axis */
  switch(_eAxis)
  {
    orxJOYSTICK_DECLARE_AXIS_NAME(X_1);
    orxJOYSTICK_DECLARE_AXIS_NAME(Y_1);
    orxJOYSTICK_DECLARE_AXIS_NAME(Z_1);
    orxJOYSTICK_DECLARE_AXIS_NAME(R_1);
    orxJOYSTICK_DECLARE_AXIS_NAME(U_1);
    orxJOYSTICK_DECLARE_AXIS_NAME(V_1);
    orxJOYSTICK_DECLARE_AXIS_NAME(POV_1);

    orxJOYSTICK_DECLARE_AXIS_NAME(X_2);
    orxJOYSTICK_DECLARE_AXIS_NAME(Y_2);
    orxJOYSTICK_DECLARE_AXIS_NAME(Z_2);
    orxJOYSTICK_DECLARE_AXIS_NAME(R_2);
    orxJOYSTICK_DECLARE_AXIS_NAME(U_2);
    orxJOYSTICK_DECLARE_AXIS_NAME(V_2);
    orxJOYSTICK_DECLARE_AXIS_NAME(POV_2);

    orxJOYSTICK_DECLARE_AXIS_NAME(X_3);
    orxJOYSTICK_DECLARE_AXIS_NAME(Y_3);
    orxJOYSTICK_DECLARE_AXIS_NAME(Z_3);
    orxJOYSTICK_DECLARE_AXIS_NAME(R_3);
    orxJOYSTICK_DECLARE_AXIS_NAME(U_3);
    orxJOYSTICK_DECLARE_AXIS_NAME(V_3);
    orxJOYSTICK_DECLARE_AXIS_NAME(POV_3);

    orxJOYSTICK_DECLARE_AXIS_NAME(X_4);
    orxJOYSTICK_DECLARE_AXIS_NAME(Y_4);
    orxJOYSTICK_DECLARE_AXIS_NAME(Z_4);
    orxJOYSTICK_DECLARE_AXIS_NAME(R_4);
    orxJOYSTICK_DECLARE_AXIS_NAME(U_4);
    orxJOYSTICK_DECLARE_AXIS_NAME(V_4);
    orxJOYSTICK_DECLARE_AXIS_NAME(POV_4);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_JOYSTICK, "No name defined for axis #%d.", _eAxis);

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
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_GetAxisValue, orxFLOAT, orxJOYSTICK_AXIS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_IsButtonPressed, orxBOOL, orxJOYSTICK_BUTTON);


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
orxSTATUS orxFASTCALL orxJoystick_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Init)();
}

/** Exits from the joystick module
 */
void orxFASTCALL orxJoystick_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Exit)();
}

/** Gets mouse on screen position
 * @param[in] _u32ID        ID of the joystick to test
 * @return Value of the axis
 */
orxFLOAT orxFASTCALL orxJoystick_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_GetAxisValue)(_eAxis);
}

/** Is joystick button pressed?
 * @param[in] _u32ID        ID of the joystick to test
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_IsButtonPressed)(_eButton);
}

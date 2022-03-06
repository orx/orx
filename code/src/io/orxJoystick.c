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
#define orxJOYSTICK_DECLARE_BUTTONS(ID)           \
  orxJOYSTICK_DECLARE_BUTTON_NAME(A_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(B_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(X_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(Y_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(LBUMPER_##ID);  \
  orxJOYSTICK_DECLARE_BUTTON_NAME(RBUMPER_##ID);  \
  orxJOYSTICK_DECLARE_BUTTON_NAME(BACK_##ID);     \
  orxJOYSTICK_DECLARE_BUTTON_NAME(START_##ID);    \
  orxJOYSTICK_DECLARE_BUTTON_NAME(GUIDE_##ID);    \
  orxJOYSTICK_DECLARE_BUTTON_NAME(LTHUMB_##ID);   \
  orxJOYSTICK_DECLARE_BUTTON_NAME(RTHUMB_##ID);   \
  orxJOYSTICK_DECLARE_BUTTON_NAME(UP_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(RIGHT_##ID);    \
  orxJOYSTICK_DECLARE_BUTTON_NAME(DOWN_##ID);     \
  orxJOYSTICK_DECLARE_BUTTON_NAME(LEFT_##ID);     \
  orxJOYSTICK_DECLARE_BUTTON_NAME(1_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(2_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(3_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(4_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(5_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(6_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(7_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(8_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(9_##ID);        \
  orxJOYSTICK_DECLARE_BUTTON_NAME(10_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(11_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(12_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(13_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(14_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(15_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(16_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(17_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(18_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(19_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(20_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(21_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(22_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(23_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(24_##ID);       \
  orxJOYSTICK_DECLARE_BUTTON_NAME(25_##ID);

  /* Checks */
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Depending on button */
  switch(_eButton)
  {
    orxJOYSTICK_DECLARE_BUTTONS(1);
    orxJOYSTICK_DECLARE_BUTTONS(2);
    orxJOYSTICK_DECLARE_BUTTONS(3);
    orxJOYSTICK_DECLARE_BUTTONS(4);
    orxJOYSTICK_DECLARE_BUTTONS(5);
    orxJOYSTICK_DECLARE_BUTTONS(6);
    orxJOYSTICK_DECLARE_BUTTONS(7);
    orxJOYSTICK_DECLARE_BUTTONS(8);
    orxJOYSTICK_DECLARE_BUTTONS(9);
    orxJOYSTICK_DECLARE_BUTTONS(10);
    orxJOYSTICK_DECLARE_BUTTONS(11);
    orxJOYSTICK_DECLARE_BUTTONS(12);
    orxJOYSTICK_DECLARE_BUTTONS(13);
    orxJOYSTICK_DECLARE_BUTTONS(14);
    orxJOYSTICK_DECLARE_BUTTONS(15);
    orxJOYSTICK_DECLARE_BUTTONS(16);

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
#define orxJOYSTICK_DECLARE_AXES(ID)            \
  orxJOYSTICK_DECLARE_AXIS_NAME(LX_##ID);       \
  orxJOYSTICK_DECLARE_AXIS_NAME(LY_##ID);       \
  orxJOYSTICK_DECLARE_AXIS_NAME(RX_##ID);       \
  orxJOYSTICK_DECLARE_AXIS_NAME(RY_##ID);       \
  orxJOYSTICK_DECLARE_AXIS_NAME(LTRIGGER_##ID); \
  orxJOYSTICK_DECLARE_AXIS_NAME(RTRIGGER_##ID);

  /* Checks */
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Depending on axis */
  switch(_eAxis)
  {
    orxJOYSTICK_DECLARE_AXES(1);
    orxJOYSTICK_DECLARE_AXES(2);
    orxJOYSTICK_DECLARE_AXES(3);
    orxJOYSTICK_DECLARE_AXES(4);
    orxJOYSTICK_DECLARE_AXES(5);
    orxJOYSTICK_DECLARE_AXES(6);
    orxJOYSTICK_DECLARE_AXES(7);
    orxJOYSTICK_DECLARE_AXES(8);
    orxJOYSTICK_DECLARE_AXES(9);
    orxJOYSTICK_DECLARE_AXES(10);
    orxJOYSTICK_DECLARE_AXES(11);
    orxJOYSTICK_DECLARE_AXES(12);
    orxJOYSTICK_DECLARE_AXES(13);
    orxJOYSTICK_DECLARE_AXES(14);
    orxJOYSTICK_DECLARE_AXES(15);
    orxJOYSTICK_DECLARE_AXES(16);

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
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_IsConnected, orxBOOL, orxU32);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(JOYSTICK)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, INIT, orxJoystick_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, EXIT, orxJoystick_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, GET_AXIS_VALUE, orxJoystick_GetAxisValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, IS_BUTTON_PRESSED, orxJoystick_IsButtonPressed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, IS_CONNECTED, orxJoystick_IsConnected)

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
 * @return orxTRUE if pressed / orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_IsButtonPressed)(_eButton);
}

/** Is joystick connected?
 * @param[in] _u32ID        ID of the joystick, 1-based index
 * @return orxTRUE if connected / orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxJoystick_IsConnected(orxU32 _u32ID)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_IsConnected)(_u32ID);
}

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxJoystick.h
 * @date 22/11/2003
 * @author (C) Arcallians
 */

/**
 * @addtogroup IO
 * 
 * Joystick plugin module
 * Plugin module that handles joystick-like peripherals
 *
 * @{
 */


#ifndef _orxJOYSTICK_H_
#define _orxJOYSTICK_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/** Button enum
 */
typedef enum __orxJOYSTICK_BUTTON_t
{
  #ifdef __orxGP2X__
  
  #else /* __orxGP2X__ */

    orxJOYSTICK_BUTTON_1 = 0,
    orxJOYSTICK_BUTTON_2,
    orxJOYSTICK_BUTTON_3,
    orxJOYSTICK_BUTTON_4,
    orxJOYSTICK_BUTTON_5,
    orxJOYSTICK_BUTTON_6,
    orxJOYSTICK_BUTTON_7,
    orxJOYSTICK_BUTTON_8,
    orxJOYSTICK_BUTTON_9,
    orxJOYSTICK_BUTTON_10,
    orxJOYSTICK_BUTTON_11,
    orxJOYSTICK_BUTTON_12,
    orxJOYSTICK_BUTTON_13,
    orxJOYSTICK_BUTTON_14,
    orxJOYSTICK_BUTTON_15,
    orxJOYSTICK_BUTTON_16,

  #endif /* __orxGP2X__ */

  orxJOYSTICK_BUTTON_NUMBER,

  orxJOYSTICK_BUTTON_NONE = orxENUM_NONE

} orxJOYSTICK_BUTTON;


/** Axis enu,
 */
typedef enum __orxJOYSTICK_AXIS_t
{
  #ifdef __orxGP2X__
  
  #else /* __orxGP2X__ */

    orxJOYSTICK_AXIS_X = 0,
    orxJOYSTICK_AXIS_Y,
    orxJOYSTICK_AXIS_Z,
    orxJOYSTICK_AXIS_R,
    orxJOYSTICK_AXIS_U,
    orxJOYSTICK_AXIS_V,
    orxJOYSTICK_AXIS_POV,

  #endif /* __orxGP2X__ */

  orxJOYSTICK_AXIS_NUMBER,

  orxJOYSTICK_AXIS_NONE = orxENUM_NONE

} orxJOYSTICK_AXIS;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Mouse module setup
 */
extern orxDLLAPI orxVOID                              orxJoystick_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxJoystick_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxJoystick_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxJoystick_GetAxisValue, orxFLOAT, orxU32, orxJOYSTICK_AXIS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxJoystick_IsButtonPressed, orxBOOL, orxU32, orxJOYSTICK_BUTTON);


/** Init the joystick module
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxJoystick_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Init)();
}

/** Exit from the joystick module
 */
orxSTATIC orxINLINE orxVOID orxJoystick_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_Exit)();
}

/** Gets mouse on screen position
 * @param _u32ID        ID of the joystick to test
 * @param _eAxis        Joystick axis to check
 * @return Value of the axis
 */
orxSTATIC orxINLINE orxFLOAT orxJoystick_GetAxisValue(orxU32 _u32ID, orxJOYSTICK_AXIS _eAxis)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_GetAxisValue)(_u32ID, _eAxis);
}

/** Is joystick button pressed?
 * @param _u32ID        ID of the joystick to test
 * @param _eButton      Joystick button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxSTATIC orxINLINE orxBOOL orxJoystick_IsButtonPressed(orxU32 _u32ID, orxJOYSTICK_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxJoystick_IsButtonPressed)(_u32ID, _eButton);
}


#endif /* _orxJOYSTICK_H_ */

/** @} */

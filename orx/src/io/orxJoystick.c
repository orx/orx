/**
 * @file orxJoystick.c
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


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


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(JOYSTICK)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, INIT, orxJoystick_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, EXIT, orxJoystick_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, GET_AXIS_VALUE, orxJoystick_GetAxisValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(JOYSTICK, IS_BUTTON_PRESSED, orxJoystick_IsButtonPressed)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(JOYSTICK)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_GetAxisValue, orxFLOAT, orxU32, orxJOYSTICK_AXIS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxJoystick_IsButtonPressed, orxBOOL, orxU32, orxJOYSTICK_BUTTON);

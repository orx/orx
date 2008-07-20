/**
 * @file orxMouse.c
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


#include "io/orxMouse.h"
#include "plugin/orxPluginCore.h"


/** Mouse module setup
 */
orxVOID orxMouse_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_DISPLAY);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(MOUSE)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, INIT, orxMouse_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, EXIT, orxMouse_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, GET_POSITION, orxMouse_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, IS_BUTTON_PRESSED, orxMouse_IsButtonPressed)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(MOUSE)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_GetPosition, orxSTATUS, orxS32 *, orxS32 *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_IsButtonPressed, orxBOOL, orxMOUSE_BUTTON);

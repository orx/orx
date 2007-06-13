/**
 * \file keyboard.c
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "io/orxKeyboard.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 orxKeyboard_Setup
 Keyboard module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxKeyboard_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_KEYBOARD, orxMODULE_ID_TREE);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(KEYBOARD)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, INIT, orxKeyboard_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, EXIT, orxKeyboard_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, HIT, orxKeyboard_Hit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, READ, orxKeyboard_Read)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(KEYBOARD, CLEAR_BUFFER, orxKeyboard_ClearBuffer)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(KEYBOARD)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Hit, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_Read, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxKeyboard_ClearBuffer, orxVOID);

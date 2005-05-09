/**
 * \file keyboard.c
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "io/keyboard.h"

#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastKeyboardPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &keyboard_init,                orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_INIT},
  {(orxPLUGIN_FUNCTION *) &keyboard_exit,                orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_EXIT},
  {(orxPLUGIN_FUNCTION *) &keyboard_hit,                 orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_HIT},
  {(orxPLUGIN_FUNCTION *) &keyboard_read,                orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_READ},
  {(orxPLUGIN_FUNCTION *) &keyboard_keystate_get,        orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_GET_KEYSTATE},
  {(orxPLUGIN_FUNCTION *) &keyboard_buffer_clear,        orxPLUGIN_FUNCTION_BASE_ID_KEYBOARD_CLEAR_BUFFER}
};

orxVOID keyboard_plugin_init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_KEYBOARD, sastKeyboardPluginFunctionInfo, sizeof(sastKeyboardPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(keyboard_init, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(keyboard_exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(keyboard_hit, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(keyboard_read, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(keyboard_keystate_get, orxS32, orxU8);
orxPLUGIN_DEFINE_CORE_FUNCTION(keyboard_buffer_clear, orxVOID);

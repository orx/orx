/**
 * \file mouse.c
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

#include "io/mouse.h"

#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastMousePluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &mouse_init,                orxPLUGIN_FUNCTION_BASE_ID_MOUSE_INIT},
  {(orxPLUGIN_FUNCTION *) &mouse_exit,                orxPLUGIN_FUNCTION_BASE_ID_MOUSE_EXIT},
  {(orxPLUGIN_FUNCTION *) &mouse_move_get,            orxPLUGIN_FUNCTION_BASE_ID_MOUSE_GET_MOVE},
};

orxVOID mouse_plugin_init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_MOUSE, sastMousePluginFunctionInfo, sizeof(sastMousePluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(mouse_init, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(mouse_exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(mouse_move_get, orxVOID, orxS32 *, orxS32 *);

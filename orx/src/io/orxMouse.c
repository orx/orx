/**
 * \file orxMouse.c
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

#include "io/orxMouse.h"
#include "plugin/orxPluginCore.h"

/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastMousePluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &orxMouse_Init,     orxPLUGIN_FUNCTION_BASE_ID_MOUSE_INIT},
  {(orxPLUGIN_FUNCTION *) &orxMouse_Exit,     orxPLUGIN_FUNCTION_BASE_ID_MOUSE_EXIT},
  {(orxPLUGIN_FUNCTION *) &orxMouse_GetMove,  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_GET_MOVE},
};

/** Init the mouse core plugin
 */
orxVOID orxMouse_Plugin_Init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_MOUSE, sastMousePluginFunctionInfo, sizeof(sastMousePluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}

/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_GetMove, orxVOID, orxS32 *, orxS32 *);

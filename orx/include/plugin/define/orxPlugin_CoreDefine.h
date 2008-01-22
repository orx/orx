/**
 * @file orxPlugin_CoreDefine.h
 * 
 * Header that regroups all core plugin defines.
 */ 
 
 /***************************************************************************
 orxPlugin_CoreDefine.h
 
 begin                : 09/05/2005
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


#ifndef _orxPLUGIN_COREDEFINE_H_
#define _orxPLUGIN_COREDEFINE_H_

/*
 * Macros for core plugin register function use
 */
#define orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)  _orxRegisterFunction_##PLUGIN_SUFFIX
#define orxPLUGIN_CORE_REGISTER_FUNCTION(PLUGIN_SUFFIX)                             \
  extern orxDLLAPI orxVOID orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)();  \
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)();


/*
 * Includes all core plugin headers
 */
#include "plugin/define/orxPlugin_Display.h"
#include "plugin/define/orxPlugin_File.h"
#include "plugin/define/orxPlugin_FileSystem.h"
#include "plugin/define/orxPlugin_Joystick.h"
#include "plugin/define/orxPlugin_Keyboard.h"
#include "plugin/define/orxPlugin_Mouse.h"
#include "plugin/define/orxPlugin_Package.h"
#include "plugin/define/orxPlugin_Registry.h"
#include "plugin/define/orxPlugin_Render.h"
#include "plugin/define/orxPlugin_Script.h"
#include "plugin/define/orxPlugin_Sound.h"
#include "plugin/define/orxPlugin_Time.h"


/*
 * Inline core plugin registration function
 */
orxSTATIC orxINLINE orxVOID orxPlugin_RegisterCorePlugins()
{
  orxPLUGIN_CORE_REGISTER_FUNCTION(DISPLAY);
  orxPLUGIN_CORE_REGISTER_FUNCTION(FILE);
  orxPLUGIN_CORE_REGISTER_FUNCTION(FILESYSTEM);
  orxPLUGIN_CORE_REGISTER_FUNCTION(JOYSTICK);
  orxPLUGIN_CORE_REGISTER_FUNCTION(KEYBOARD);
  orxPLUGIN_CORE_REGISTER_FUNCTION(MOUSE);
  orxPLUGIN_CORE_REGISTER_FUNCTION(PACKAGE);
  orxPLUGIN_CORE_REGISTER_FUNCTION(REGISTRY);
  orxPLUGIN_CORE_REGISTER_FUNCTION(RENDER);
  orxPLUGIN_CORE_REGISTER_FUNCTION(SCRIPT);
  orxPLUGIN_CORE_REGISTER_FUNCTION(SOUND);
  orxPLUGIN_CORE_REGISTER_FUNCTION(TIME);
}


#endif /*_orxPLUGIN_COREDEFINE_H_*/

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
 * Macros for core plugin register function declaration
 */
#define orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX) _orxRegisterFunction_##PLUGIN_SUFFIX

#define orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(PLUGIN_SUFFIX) extern orxDLLAPI orxVOID orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)()


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
 * Defines all core plugin register function
 */
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(DISPLAY);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(FILE);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(FILESYSTEM);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(JOYSTICK);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(KEYBOARD);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(MOUSE);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(PACKAGE);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(REGISTRY);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(RENDER);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(SCRIPT);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(SOUND);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(TIME);

/*
 * Inline core plugin registration function
 */
orxSTATIC orxINLINE orxVOID orxPlugin_RegisterCorePlugins()
{
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(DISPLAY)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(FILE)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(FILESYSTEM)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(JOYSTICK)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(KEYBOARD)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(MOUSE)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PACKAGE)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(REGISTRY)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(RENDER)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(SCRIPT)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(SOUND)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(TIME)();
}


#endif /*_orxPLUGIN_COREDEFINE_H_*/

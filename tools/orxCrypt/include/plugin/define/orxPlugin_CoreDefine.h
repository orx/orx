/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxPlugin_CoreDefine.h
 * @date 09/05/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that regroups all core plugin defines.
 *
 * @{
 */


#ifndef _orxPLUGIN_COREDEFINE_H_
#define _orxPLUGIN_COREDEFINE_H_

/*
 * Macros for core plugin register function declaration
 */
#define orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX) _registerFunction_##PLUGIN_SUFFIX

#define orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(PLUGIN_SUFFIX) extern orxDLLAPI void orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PLUGIN_SUFFIX)()


/*
 * Includes all core plugin headers
 */
#include "plugin/define/orxPlugin_Display.h"
#include "plugin/define/orxPlugin_Joystick.h"
#include "plugin/define/orxPlugin_Keyboard.h"
#include "plugin/define/orxPlugin_Mouse.h"
#include "plugin/define/orxPlugin_Physics.h"
#include "plugin/define/orxPlugin_Render.h"
#include "plugin/define/orxPlugin_SoundSystem.h"


/*
 * Defines all core plugin register function
 */
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(DISPLAY);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(JOYSTICK);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(KEYBOARD);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(MOUSE);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(PHYSICS);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(RENDER);
orxPLUGIN_DECLARE_CORE_REGISTER_FUNCTION(SOUNDSYSTEM);

/*
 * Inline core plugin registration function
 */
static orxINLINE void orxPlugin_RegisterCorePlugins()
{
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(DISPLAY)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(JOYSTICK)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(KEYBOARD)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(MOUSE)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(PHYSICS)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(RENDER)();
  orxPLUGIN_CORE_REGISTER_FUNCTION_NAME(SOUNDSYSTEM)();
}

#endif /*_orxPLUGIN_COREDEFINE_H_*/

/** @} */

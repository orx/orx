/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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

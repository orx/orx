/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
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
 * @file orxPlugin.h
 * @date 06/09/2002
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add execute function for user-registered functions + arg parser/checker
 */

/**
 * @addtogroup orxPlugin
 *
 * Plugin module
 * This module provides functions for loading dynamic code into the core
 * engine and finding symbols within the dynamic modules
 *
 * @{
 */


#ifndef _orxPLUGIN_H_
#define _orxPLUGIN_H_


#include "orxInclude.h"

#include "plugin/orxPluginType.h"


/*********************************************
 Function prototypes
 *********************************************/

/** Plugin module setup
 */
extern orxDLLAPI void orxFASTCALL               orxPlugin_Setup();

/** Inits the plugin module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxPlugin_Init();

/** Exits from the plugin module
 */
extern orxDLLAPI void orxFASTCALL               orxPlugin_Exit();


/** Loads a plugin (using OS common library extension + release/debug suffixes if not found)
 * @param[in] _zPluginFileName  The complete path of the plugin file, with or without its library extension
 * @param[in] _zPluginName      The name that the plugin will be given in the plugin list
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
extern orxDLLAPI orxHANDLE orxFASTCALL          orxPlugin_Load(const orxSTRING _zPluginFileName, const orxSTRING _zPluginName);

/** Loads a plugin, doing a shadow copy and watching for any change on-disk to trigger an auto-swap
 * @param[in] _zPluginFileName  The complete path of the plugin file, with or without its library extension
 * @param[in] _zPluginName      The name that the plugin will be given in the plugin list
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
extern orxDLLAPI orxHANDLE orxFASTCALL          orxPlugin_LoadShadow(const orxSTRING _zPluginFileName, const orxSTRING _zPluginName);

/** Unloads a plugin
 * @param[in] _hPluginHandle The handle of the plugin to unload
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxPlugin_Unload(orxHANDLE _hPluginHandle);

/** Gets a function from a plugin
 * @param[in] _hPluginHandle The plugin handle
 * @param[in] _zFunctionName The name of the function to find
 * @return orxPLUGIN_FUNCTION / orxNULL
 */
extern orxDLLAPI orxPLUGIN_FUNCTION orxFASTCALL orxPlugin_GetFunction(orxHANDLE _hPluginHandle, const orxSTRING _zFunctionName);

/** Gets the handle of a plugin given its name
 * @param[in] _zPluginName The plugin name
 * @return Its orxHANDLE / orxHANDLE_UNDEFINED
 */
extern orxDLLAPI orxHANDLE orxFASTCALL          orxPlugin_GetHandle(const orxSTRING _zPluginName);

/** Gets the name of a plugin given its handle
 * @param[in] _hPluginHandle The plugin handle
 * @return The plugin name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL    orxPlugin_GetName(orxHANDLE _hPluginHandle);

#endif /* _orxPLUGIN_H_ */

/** @} */

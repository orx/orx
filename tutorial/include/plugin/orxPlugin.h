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


/** Loads a plugin (using its exact complete)
 * @param[in] _zPluginFileName  The complete path of the plugin file, including its extension
 * @param[in] _zPluginName      The name that the plugin will be given in the plugin list
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
extern orxDLLAPI orxHANDLE orxFASTCALL          orxPlugin_Load(const orxSTRING _zPluginFileName, const orxSTRING _zPluginName);

/** Loads a plugin using OS common library extension + release/debug suffixes
 * @param[in] _zPluginFileName  The complete path of the plugin file, without its library extension
 * @param[in] _zPluginName      The name that the plugin will be given in the plugin list
 * @return The plugin handle on success, orxHANDLE_UNDEFINED on failure
 */
extern orxDLLAPI orxHANDLE orxFASTCALL          orxPlugin_LoadUsingExt(const orxSTRING _zPluginFileName, const orxSTRING _zPluginName);

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

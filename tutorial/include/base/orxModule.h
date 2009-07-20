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
 * @file orxModule.h
 * @date 12/09/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add internal/external dependency system
 * - Optimize dependencies storage
 */

/**
 * @addtogroup orxModule
 *
 * Module file
 * Code that handles modules and their dependencies
 *
 * @{
 */


#ifndef _orxMODULE_H_
#define _orxMODULE_H_


#include "base/orxType.h"


/** Module enum */
typedef enum __orxMODULE_ID_t
{
  orxMODULE_ID_ANIM = 0,
  orxMODULE_ID_ANIMPOINTER,
  orxMODULE_ID_ANIMSET,
  orxMODULE_ID_BANK,
  orxMODULE_ID_BODY,
  orxMODULE_ID_CAMERA,
  orxMODULE_ID_CLOCK,
  orxMODULE_ID_CONFIG,
  orxMODULE_ID_DISPLAY,
  orxMODULE_ID_EVENT,
  orxMODULE_ID_FILE,
  orxMODULE_ID_FILESYSTEM,
  orxMODULE_ID_FPS,
  orxMODULE_ID_FRAME,
  orxMODULE_ID_FX,
  orxMODULE_ID_FXPOINTER,
  orxMODULE_ID_GRAPHIC,
  orxMODULE_ID_INPUT,
  orxMODULE_ID_JOYSTICK,
  orxMODULE_ID_KEYBOARD,
  orxMODULE_ID_LOCALE,
  orxMODULE_ID_MAIN,
  orxMODULE_ID_MEMORY,
  orxMODULE_ID_MOUSE,
  orxMODULE_ID_OBJECT,
  orxMODULE_ID_PARAM,
  orxMODULE_ID_PHYSICS,
  orxMODULE_ID_PLUGIN,
  orxMODULE_ID_RENDER,
  orxMODULE_ID_SCREENSHOT,
  orxMODULE_ID_SHADER,
  orxMODULE_ID_SHADERPOINTER,
  orxMODULE_ID_SOUND,
  orxMODULE_ID_SOUNDPOINTER,
  orxMODULE_ID_SOUNDSYSTEM,
  orxMODULE_ID_SPAWNER,
  orxMODULE_ID_STRUCTURE,
  orxMODULE_ID_SYSTEM,
  orxMODULE_ID_TEXT,
  orxMODULE_ID_TEXTURE,
  orxMODULE_ID_VIEWPORT,

  orxMODULE_ID_NUMBER,

  orxMODULE_ID_MAX_NUMBER = 64,

  orxMODULE_ID_NONE = orxENUM_NONE

} orxMODULE_ID;


/* *** setup/init/exit/run function prototypes *** */
typedef orxSTATUS                         (orxFASTCALL *orxMODULE_INIT_FUNCTION)  ();
typedef void                              (orxFASTCALL *orxMODULE_EXIT_FUNCTION)  ();
typedef orxSTATUS                         (orxFASTCALL *orxMODULE_RUN_FUNCTION)   ();
typedef void                              (orxFASTCALL *orxMODULE_SETUP_FUNCTION) ();


/** Registers a module
 * @param[in]   _eModuleID                Concerned module ID
 * @param[in]   _pfnSetup                 Module setup callback
 * @param[in]   _pfnInit                  Module init callback
 * @param[in]   _pfnExit                  Module exit callback
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI void orxFASTCALL         orxModule_Register(orxMODULE_ID _eModuleID, const orxMODULE_SETUP_FUNCTION _pfnSetup, const orxMODULE_INIT_FUNCTION _pfnInit, const orxMODULE_EXIT_FUNCTION _pfnExit);

/** Registers all modules. ! Needs to be updated for all new modules ! */
extern orxDLLAPI void orxFASTCALL         orxModule_RegisterAll();

/** Adds dependencies between 2 modules
 * @param[in]   _eModuleID                Concerned module ID
 * @param[in]   _eDependID                Module ID of the needed module
 */
extern orxDLLAPI void orxFASTCALL         orxModule_AddDependency(orxMODULE_ID _eModuleID, orxMODULE_ID _eDependID);

/** Adds optional dependencies between 2 modules
 * @param[in]   _eModuleID                Concerned module ID
 * @param[in]   _eDependID                Module ID of the optionally needed module
 */
extern orxDLLAPI void orxFASTCALL         orxModule_AddOptionalDependency(orxMODULE_ID _eModuleID, orxMODULE_ID _eDependID);

/** Updates dependencies for all modules */
extern orxDLLAPI void orxFASTCALL         orxModule_UpdateDependencies();

/** Calls a module setup callback
 * @param[in]   _eModuleID                Concerned module ID
 */
extern orxDLLAPI void orxFASTCALL         orxModule_Setup(orxMODULE_ID _eModuleID);

/** Calls all module setups */
extern orxDLLAPI void orxFASTCALL         orxModule_SetupAll();

/** Inits a module
 * @param[in]   _eModuleID                Concerned module ID
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxModule_Init(orxMODULE_ID _eModuleID);

/** Inits all modules
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxModule_InitAll();

/** Exits from a module
 * @param[in]   _eModuleID                Concerned module ID
 */
extern orxDLLAPI void orxFASTCALL         orxModule_Exit(orxMODULE_ID _eModuleID);

/** Exits from all modules.*/
extern orxDLLAPI void orxFASTCALL         orxModule_ExitAll();

/** Is module initialized?
 * @param[in]   _eModuleID                Concerned module ID
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL      orxModule_IsInitialized(orxMODULE_ID _eModuleID);

#endif /* _orxMODULE_H_ */

/** @} */

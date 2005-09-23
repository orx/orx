/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxModule.h
 * @date 12/09/2005
 * @author (C) Arcallians
 * 
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
  orxMODULE_ID_CAMERA,
  orxMODULE_ID_CLOCK,
  orxMODULE_ID_DISPLAY,
  orxMODULE_ID_EVENT,
  orxMODULE_ID_FILE,
  orxMODULE_ID_FPS,
  orxMODULE_ID_FRAME,
  orxMODULE_ID_FSM,
  orxMODULE_ID_HASHTABLE,
  orxMODULE_ID_JOYSTICK,
  orxMODULE_ID_KEYBOARD,
  orxMODULE_ID_LINKLIST,
  orxMODULE_ID_MATHSET,
  orxMODULE_ID_MEMORY,
  orxMODULE_ID_MOUSE,
  orxMODULE_ID_OBJECT,
  orxMODULE_ID_PACKAGE,
  orxMODULE_ID_PARAM,
  orxMODULE_ID_PLUGIN,
  orxMODULE_ID_QUEUE,
  orxMODULE_ID_RENDER,
  orxMODULE_ID_SCRIPT,
  orxMODULE_ID_SOUND,
  orxMODULE_ID_SCREENSHOT,
  orxMODULE_ID_STRUCTURE,
  orxMODULE_ID_TEST,
  orxMODULE_ID_TEXTIO,
  orxMODULE_ID_TEXTURE,
  orxMODULE_ID_TIME,
  orxMODULE_ID_TREE,
  orxMODULE_ID_VIEWPORT,

  orxMODULE_ID_NUMBER,

  orxMODULE_ID_MAX_NUMBER = 64,

  orxMODULE_ID_NONE = orxENUM_NONE

} orxMODULE_ID;


/* *** init/exit function prototypes *** */
typedef orxSTATUS                         (*orxMODULE_INIT_FUNCTION)  ();
typedef orxVOID                           (*orxMODULE_EXIT_FUNCTION)   ();
typedef orxVOID                           (*orxMODULE_SETUP_FUNCTION) ();


/** Registers a module. */
extern orxDLLAPI orxVOID orxFASTCALL      orxModule_Register(orxMODULE_ID _eModuleID, orxCONST orxMODULE_SETUP_FUNCTION _pfnSetup, orxCONST orxMODULE_INIT_FUNCTION _pfnInit, orxCONST orxMODULE_EXIT_FUNCTION _pfnExit);

/** Registers all modules. ! Needs to be updated for all new modules ! */
extern orxDLLAPI orxVOID                  orxModule_RegisterAll();

/** Adds dependencies between 2 modules. */
extern orxDLLAPI orxVOID orxFASTCALL      orxModule_AddDependency(orxMODULE_ID _eModuleID, orxMODULE_ID _eDependID);

/** Updates dependencies for all modules. */
extern orxDLLAPI orxVOID                  orxModule_UpdateDependencies();

/** Calls a module setup. */
extern orxDLLAPI orxVOID orxFASTCALL      orxModule_Setup(orxMODULE_ID _eModuleID);

/** Calls all module setups.*/
extern orxDLLAPI orxVOID                  orxModule_SetupAll();

/** Inits a module. */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxModule_Init(orxMODULE_ID _eModuleID);

/** Inits all modules.*/
extern orxDLLAPI orxSTATUS                orxModule_InitAll();

/** Exits from a module. */
extern orxDLLAPI orxVOID orxFASTCALL      orxModule_Exit(orxMODULE_ID _eModuleID);

/** Exits from all modules.*/
extern orxDLLAPI orxVOID                  orxModule_ExitAll();


#endif /* _orxMODULE_H_ */

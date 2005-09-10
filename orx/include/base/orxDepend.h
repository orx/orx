/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date 04/09/2005
 * @author (C) Arcallians
 * 
 */

/**
 * @addtogroup Depend
 * 
 * Header for the dependency module. This module manage system initialisation
 * for the engine.
 * @{
 */
 
#ifndef _orxDEPEND_H_
#define _orxDEPEND_H_


#include "base/orxType.h"

/** WARNING : This enum definition doesn't respect the standard coding style.
 * The complete enum name should use upper case but this exception is needed
 * to have an automatic working init/exit macro system.
 */
typedef enum __orxDEPEND_MODULE_t
{
  orxDEPEND_MODULE_Memory = 0,
  orxDEPEND_MODULE_Bank,
  orxDEPEND_MODULE_AnimSet,
  orxDEPEND_MODULE_Anim,
  orxDEPEND_MODULE_AnimPointer,
  orxDEPEND_MODULE_Plugin,
  orxDEPEND_MODULE_Structure,
  orxDEPEND_MODULE_Frame,
  orxDEPEND_MODULE_Texture,
  orxDEPEND_MODULE_Display,
  orxDEPEND_MODULE_Camera,
  orxDEPEND_MODULE_Viewport,
  orxDEPEND_MODULE_Time,
  orxDEPEND_MODULE_Object,
  orxDEPEND_MODULE_Clock,
  orxDEPEND_MODULE_String,
  orxDEPEND_MODULE_LinkList,
  orxDEPEND_MODULE_Tree,
  orxDEPEND_MODULE_TextIO,
  orxDEPEND_MODULE_HashTable,
  orxDEPEND_MODULE_File,
  orxDEPEND_MODULE_Test,
  orxDEPEND_MODULE_Queue,
  orxDEPEND_MODULE_Event,
  orxDEPEND_MODULE_Depend,
  orxDEPEND_MODULE_FSM,
  orxDEPEND_MODULE_Param,
    
  orxDEPEND_MODULE_NUMBER,
  orxDEPEND_MODULE_NONE = orxENUM_NONE
} orxDEPEND_MODULE;

typedef orxSTATUS (*orxDEPEND_INIT_FUNCTION)();
typedef orxVOID   (*orxDEPEND_EXIT_FUNCTION)();

/** Initialize the Dependency's module
 */
extern orxDLLAPI orxSTATUS              orxDepend_Init();

/** Uninitialize the Dependency's module
 */
extern orxDLLAPI orxVOID                orxDepend_Exit();

/** Call the Init callback function for a module
 * @param[in] _zName    Module's name
 * @param[in] _eModule  Module's type
 * @param[in] _pfnInit   Init function
 * @return Module's Init status
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxDepend_InitModule(orxCONST orxSTRING _zName, orxDEPEND_MODULE _eModule, orxDEPEND_INIT_FUNCTION _pfnInit);

/** Call the Exit callback function for a module
 * @param[in] _zName    Module's name
 * @param[in] _eModule  Module's type
 * @param[in] _pfnExit   Exit function
 */
extern orxDLLAPI orxVOID orxFASTCALL    orxDepend_ExitModule(orxCONST orxSTRING _zName, orxDEPEND_MODULE _eModule, orxDEPEND_EXIT_FUNCTION _pfnExit);

/** Macro that automatically call Init function and register module Init/Exit function.
 */
#define orxDEPEND_INIT(ModuleName)                                                                \
(                                                                                                 \
  /* Call the module init function */                                                             \
  orxDepend_InitModule(#ModuleName, orxDEPEND_MODULE_ ## ModuleName, orx ## ModuleName ## _Init)  \
)

/** Macro that call modules' exit function
 */
#define orxDEPEND_EXIT(ModuleName)                                                                \
{                                                                                                 \
  /* Call the module exit function */                                                             \
  orxDepend_ExitModule(#ModuleName, orxDEPEND_MODULE_ ## ModuleName, orx ## ModuleName ## _Exit); \
}


#endif /*_orxDEPEND_H_*/

/** @} */

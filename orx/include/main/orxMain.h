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
 * @date 11/07/2005
 * @author (C) Arcallians
 * 
 */

/**
 * @addtogroup Main
 * 
 * Header for the main module. The main module will run the main loop
 * and hold the init system.
 * @{
 */
 
#ifndef _orxMAIN_H_
#define _orxMAIN_H_

#include "orxInclude.h"
#include "debug/orxDebug.h"

/** WARNING : This enum definition doesn't respect the standard coding style.
 * The complete enum name should use upper case but this excepetion is needed
 * to have an automatic working init/exit macro system.
 */
typedef enum __orxMAIN_MODULE_t
{
  orxMAIN_MODULE_Memory = 0,
  orxMAIN_MODULE_Bank,
  orxMAIN_MODULE_AnimSet,
  orxMAIN_MODULE_Anim,
  orxMAIN_MODULE_AnimPointer,
  orxMAIN_MODULE_Plugin,
  orxMAIN_MODULE_Structure,
  orxMAIN_MODULE_Frame,
  orxMAIN_MODULE_Texture,
  orxMAIN_MODULE_Display,
  orxMAIN_MODULE_Camera,
  orxMAIN_MODULE_Viewport,
  orxMAIN_MODULE_Time,
  orxMAIN_MODULE_Object,
  orxMAIN_MODULE_Clock,
  orxMAIN_MODULE_String,
  orxMAIN_MODULE_LinkList,
  orxMAIN_MODULE_Tree,
  orxMAIN_MODULE_TextIO,
  orxMAIN_MODULE_HashTable,
  orxMAIN_MODULE_File,
  orxMAIN_MODULE_Test,
  orxMAIN_MODULE_Queue,
  orxMAIN_MODULE_String,
  orxMAIN_MODULE_Event,
    
  orxMAIN_MODULE_NUMBER,
  orxMAIN_MODULE_NONE = 0xFFFFFFFF
} orxMAIN_MODULE;

typedef orxSTATUS (*orxMAIN_MODULE_INIT_CB)();
typedef orxVOID   (*orxMAIN_MODULE_EXIT_CB)();

/** Initialize the main module (will initialize all needed modules)
 */
extern orxSTATUS orxDLLAPI orxMain_Init();

/** Exit main module
 */
extern orxVOID orxDLLAPI orxMain_Exit();

/** Call the Init callback function for a module
 * @param[in] _eModule  Module's type
 * @param[in] _cbInit   Init function
 * @return Module's Init status
 */
extern orxSTATUS orxMain_InitModule(orxMAIN_MODULE _eModule, orxMAIN_MODULE_INIT_CB _cbInit);

/** Call the Exit callback function for a module
 * @param[in] _eModule  Module's type
 * @param[in] _cbExit   Exit function
 */
extern orxVOID orxMain_ExitModule(orxMAIN_MODULE _eModule, orxMAIN_MODULE_EXIT_CB _cbExit);

/** Macro that automatically call Init function and register module Init/Exit function.
 */
#define orxMAIN_INIT_MODULE(ModuleName)                                                           \
(                                                                                                 \
  /* Trace */                                                                                     \
  orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "Init : "#ModuleName),                                         \
                                                                                                  \
  /* Call the module init function */                                                             \
  orxMain_InitModule(orxMAIN_MODULE_ ## ModuleName, orx ## ModuleName ## _Init)                   \
)

/** Macro that call modules' exit function
 */
#define orxMAIN_EXIT_MODULE(ModuleName)                                                           \
{                                                                                                 \
  /* Trace */                                                                                     \
  orxDEBUG_LOG(orxDEBUG_LEVEL_LOG, "Exit : "#ModuleName),                                         \
                                                                                                  \
  /* Call the module exit function */                                                             \
  orxMain_ExitModule(orxMAIN_MODULE_ ## ModuleName, orx ## ModuleName ## _Exit);                  \
}

#endif /*_orxMAIN_H_*/

/** @} */

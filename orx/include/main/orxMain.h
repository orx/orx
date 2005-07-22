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

/** WARNING : This enum definition doesn't respect the standard coding style.
 * The complete enum name should use upper case but this excepetion is needed
 * to have an automatic working init/exit macro system.
 */
typedef enum __orxMAIN_MODULE_t
{
  orxMAIN_MODULE_Memory = 0,
  orxMAIN_MODULE_Bank,

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

/** Set Init and Exit Callback for a Module
 * @param[in] _eModule  Module's type
 * @param[in] _cbInit   Init callback
 * @param[in] _cbExit   Exit callback
 */
extern orxVOID orxDLLAPI orxMain_SetModuleCallback(orxMAIN_MODULE _eModule, orxMAIN_MODULE_INIT_CB _cbInit, orxMAIN_MODULE_EXIT_CB _cbExit);

/** Call the Init callback function for a module
 * @param[in] _eModule  Module's type
 */
extern orxVOID orxDLLAPI orxMain_InitModule(orxMAIN_MODULE _eModule);

/** Call the Exit callback function for a module
 * @param[in] _eModule  Module's type
 */
extern orxVOID orxDLLAPI orxMain_ExitModule(orxMAIN_MODULE _eModule);

/** Macro that automatically call Init function and register module Init/Exit function.
 */
#define orxMAIN_INIT_MODULE(ModuleName)                                                           \
{                                                                                                 \
  /* Set the module callback functions (constrcut a module name and init/exit function name */    \
  orxMain_SetModuleCallback(orxMAIN_MODULE_ ## ModuleName, orx ## ModuleName ## _Init, orx ## ModuleName ## _Exit); \
                                                                                                  \
  /* Call the module init function */                                                             \
  orxMain_InitModule(orxMAIN_MODULE_ ## ModuleName);                                              \
}

/** Macro that call modules' exit function
 */
#define orxMAIN_EXIT_MODULE(ModuleName)                                                           \
{                                                                                                 \
  /* Call the module exit function */                                                             \
  orxMain_ExitModule(orxMAIN_MODULE_ ## ModuleName);                                              \
}

#endif /*_orxMAIN_H_*/

/** @} */

///***************************************************************************
// *                                                                         *
// *   This program is free software; you can redistribute it and/or modify  *
// *   it under the terms of the GNU General Public License as published by  *
// *   the Free Software Foundation; either version 2 of the License, or     *
// *   (at your option) any later version.                                   *
// *                                                                         *
// ***************************************************************************/
//
///**
// * @file
// * @date 04/09/2005
// * @author (C) Arcallians
// * 
// */
//
///**
// * @addtogroup Depend
// * 
// * Header for the dependency module. This module manage system initialisation
// * for the engine.
// * @{
// */
// 
//#ifndef _orxDEPEND_H_
//#define _orxDEPEND_H_
//
//
//#include "base/orxModule.h"
//
//
///** Inits the dependency module. */
//extern orxDLLAPI orxSTATUS                orxDepend_Init();
///** Exits from the dependency module. */
//extern orxDLLAPI orxVOID                  orxDepend_Exit();
//
//
///** Adds a dependency to a module.
// * @param[in] _eModuleID  Module which depends on another one
// * @param[in] _eDependID  Module on which the first on depends
// */
//extern orxDLLAPI orxVOID orxFASTCALL      orxModule_AddDependency(orxMODULE_ID _eModuleID, orxMODULE_ID _eDependID);
//
///** Computes all dependencies. */
//extern orxDLLAPI orxVOID                  orxDepend_ComputeAll();
//
///** Call the Init callback function for a module
// * @param[in] _zName    Module's name
// * @param[in] _eModule  Module's type
// * @param[in] _pfnInit   Init function
// * @return Module's Init status
// */
//extern orxDLLAPI orxSTATUS orxFASTCALL    orxDepend_InitModule(orxMODULE_ID _eModuleID);
//
///** Call the Exit callback function for a module
// * @param[in] _zName    Module's name
// * @param[in] _eModule  Module's type
// * @param[in] _pfnExit   Exit function
// */
//extern orxDLLAPI orxVOID orxFASTCALL      orxDepend_ExitModule(orxMODULE_ID _eModuleID);
//
///** Inits all modules
// * @return All modules' init status
// */
//extern orxDLLAPI orxSTATUS                orxDepend_InitAllModules();
//
//
//#endif /*_orxDEPEND_H_*/
//
///** @} */

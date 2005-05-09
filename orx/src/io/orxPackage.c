/**
 * @file orxPackage.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxPackage.c
 Package management
 
 begin                : 01/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "io/orxPackage.h"
#include "debug/orxDebug.h"
#include "plugin/orxPluginCore.h"

/********************
 *  Plugin Related  *
 ********************/
orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastPackagePluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &orxPackage_Init,        orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_INIT},
  {(orxPLUGIN_FUNCTION *) &orxPackage_Exit,        orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_EXIT},
  {(orxPLUGIN_FUNCTION *) &orxPackage_Open,        orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_OPEN},
  {(orxPLUGIN_FUNCTION *) &orxPackage_Close,       orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_CLOSE},
  {(orxPLUGIN_FUNCTION *) &orxPackage_SetFlags,    orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_SET_FLAGS},
  {(orxPLUGIN_FUNCTION *) &orxPackage_TestFlags,   orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_TEST_FLAGS},
  {(orxPLUGIN_FUNCTION *) &orxPackage_Commit,      orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_COMMIT},
  {(orxPLUGIN_FUNCTION *) &orxPackage_Extract,     orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_EXTRACT},
  {(orxPLUGIN_FUNCTION *) &orxPackage_FindFirst,   orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_FIRST},
  {(orxPLUGIN_FUNCTION *) &orxPackage_FindNext,    orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_NEXT},
  {(orxPLUGIN_FUNCTION *) &orxPackage_FindClose,   orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_CLOSE},
  {(orxPLUGIN_FUNCTION *) &orxPackage_Read,        orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_READ}
};

/********************
 *   Core Related   *
 ********************/
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Open, orxPACKAGE*, orxSTRING, orxSTRING, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Close, orxVOID, orxPACKAGE*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_SetFlags, orxVOID, orxPACKAGE*, orxU32, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_TestFlags, orxBOOL, orxPACKAGE*, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Commit, orxSTATUS, orxPACKAGE*, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Extract, orxSTATUS, orxPACKAGE*, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindFirst, orxBOOL, orxPACKAGE*, orxSTRING, orxPACKAGE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindNext, orxBOOL, orxPACKAGE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindClose, orxVOID, orxPACKAGE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Read, orxU32, orxVOID*, orxU32, orxPACKAGE*, orxSTRING);

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

struct __orxPACKAGE_t
{
};

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Function that initialize the File plugin module
 */
orxVOID orxPackage_Plugin_Init()
{
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_PACKAGE, sastPackagePluginFunctionInfo, sizeof(sastPackagePluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));  
}


/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Debug function that print the list of all started search with extra informations.
 */
orxVOID orxPackage_DebugPrint()
{
    /* TODO */
}

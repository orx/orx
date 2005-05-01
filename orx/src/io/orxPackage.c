/**
 * @file orxFile.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFile.c
 File / Directory management
 
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
#include "plugin/plugin_core.h"

/********************
 *  Plugin Related  *
 ********************/
orxSTATIC plugin_core_st_function plugin_package_spst_function[orxPLUGIN_PACKAGE_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &orxPackage_Init,        orxPLUGIN_PACKAGE_KU32_ID_INIT},
  {(plugin_function *) &orxPackage_Exit,        orxPLUGIN_PACKAGE_KU32_ID_EXIT},
  {(plugin_function *) &orxPackage_Open,        orxPLUGIN_PACKAGE_KU32_ID_OPEN},
  {(plugin_function *) &orxPackage_Close,       orxPLUGIN_PACKAGE_KU32_ID_CLOSE},
  {(plugin_function *) &orxPackage_SetFlags,    orxPLUGIN_PACKAGE_KU32_ID_SET_FLAGS},
  {(plugin_function *) &orxPackage_TestFlags,   orxPLUGIN_PACKAGE_KU32_ID_TEST_FLAGS},
  {(plugin_function *) &orxPackage_Commit,      orxPLUGIN_PACKAGE_KU32_ID_COMMIT},
  {(plugin_function *) &orxPackage_Extract,     orxPLUGIN_PACKAGE_KU32_ID_EXTRACT},
  {(plugin_function *) &orxPackage_FindFirst,   orxPLUGIN_PACKAGE_KU32_ID_FIND_FIRST},
  {(plugin_function *) &orxPackage_FindNext,    orxPLUGIN_PACKAGE_KU32_ID_FIND_NEXT},
  {(plugin_function *) &orxPackage_FindClose,   orxPLUGIN_PACKAGE_KU32_ID_FIND_CLOSE},
  {(plugin_function *) &orxPackage_Read,        orxPLUGIN_PACKAGE_KU32_ID_READ}
};

/********************
 *   Core Related   *
 ********************/
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Init, orxSTATUS);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Exit, orxVOID);

PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Open, orxPACKAGE*, orxSTRING, orxSTRING, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Close, orxVOID, orxPACKAGE*);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_SetFlags, orxVOID, orxPACKAGE*, orxU32, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_TestFlags, orxBOOL, orxPACKAGE*, orxU32);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Commit, orxSTATUS, orxPACKAGE*, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Extract, orxSTATUS, orxPACKAGE*, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_FindFirst, orxBOOL, orxPACKAGE*, orxSTRING, orxPACKAGE_INFOS*);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_FindNext, orxBOOL, orxPACKAGE_INFOS*);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_FindClose, orxVOID, orxPACKAGE_INFOS*);
PLUGIN_CORE_FUNCTION_DEFINE(orxPackage_Read, orxU32, orxVOID*, orxU32, orxPACKAGE*, orxSTRING);

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
  plugin_core_info_add(orxPLUGIN_PACKAGE_KU32_PLUGIN_ID, plugin_package_spst_function, orxPLUGIN_PACKAGE_KU32_FUNCTION_NUMBER);  
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

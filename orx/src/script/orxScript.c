/**
 * @file orxScript.c
 * 
 * Module for core script extension management.
 */ 
 
 /***************************************************************************
 orxScript.c
 Module for core script extension management.
 
 begin                : 03/05/2005
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

#include "script/orxScript.h"
#include "debug/orxDebug.h"
#include "plugin/plugin_core.h"

/********************
 *  Plugin Related  *
 ********************/
orxSTATIC plugin_core_st_function plugin_script_spst_function[orxPLUGIN_SCRIPT_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &orxScript_Init,        orxPLUGIN_SCRIPT_KU32_ID_INIT},
  {(plugin_function *) &orxScript_Exit,        orxPLUGIN_SCRIPT_KU32_ID_EXIT},
  {(plugin_function *) &orxScript_Create,      orxPLUGIN_SCRIPT_KU32_ID_CREATE},
  {(plugin_function *) &orxScript_Delete,      orxPLUGIN_SCRIPT_KU32_ID_DELETE},
  {(plugin_function *) &orxScript_LoadFile,    orxPLUGIN_SCRIPT_KU32_ID_LOAD_FILE},
  {(plugin_function *) &orxScript_CallFunc,    orxPLUGIN_SCRIPT_KU32_ID_CALL_FUNC},
  {(plugin_function *) &orxScript_Execute,     orxPLUGIN_SCRIPT_KU32_ID_EXECUTE}
};

/********************
 *   Core Related   *
 ********************/
PLUGIN_CORE_FUNCTION_DEFINE(orxScript_Init, orxSTATUS);
PLUGIN_CORE_FUNCTION_DEFINE(orxScript_Exit, orxVOID);

PLUGIN_CORE_FUNCTION_DEFINE(orxScript_Create, orxSCRIPT*, orxVOID);
PLUGIN_CORE_FUNCTION_DEFINE(orxScript_Delete, orxVOID, orxSCRIPT*);
PLUGIN_CORE_FUNCTION_DEFINE(orxScript_LoadFile, orxSTATUS, orxSCRIPT*, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxScript_CallFunc, orxSTATUS, orxSCRIPT*, orxSTRING, orxSTRING, ...);
PLUGIN_CORE_FUNCTION_DEFINE(orxScript_Execute, orxSTATUS, orxSCRIPT*, orxSTRING);

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Function that initialize the Script plugin module
 */
orxVOID orxScript_Plugin_Init()
{
  plugin_core_info_add(orxPLUGIN_SCRIPT_KU32_PLUGIN_ID, plugin_script_spst_function, orxPLUGIN_SCRIPT_KU32_FUNCTION_NUMBER);  
}

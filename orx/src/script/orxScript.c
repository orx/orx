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

#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/
orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastScriptPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &orxScript_Init,              orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_INIT},
  {(orxPLUGIN_FUNCTION *) &orxScript_Exit,              orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_EXIT},
  {(orxPLUGIN_FUNCTION *) &orxScript_Create,            orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_CREATE},
  {(orxPLUGIN_FUNCTION *) &orxScript_Delete,            orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_DELETE},
  {(orxPLUGIN_FUNCTION *) &orxScript_LoadFile,          orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_LOAD_FILE},
  {(orxPLUGIN_FUNCTION *) &orxScript_CallFunc,          orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_CALL_FUNCTION},
  {(orxPLUGIN_FUNCTION *) &orxScript_Execute,           orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_EXECUTE}
};

/********************
 *   Core Related   *
 ********************/
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Create, orxSCRIPT*, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Delete, orxVOID, orxSCRIPT*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_LoadFile, orxSTATUS, orxSCRIPT*, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_CallFunc, orxSTATUS, orxSCRIPT*, orxSTRING, orxSTRING, ...);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Execute, orxSTATUS, orxSCRIPT*, orxSTRING);

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
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_SCRIPT, sastScriptPluginFunctionInfo, sizeof(sastScriptPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));  
}

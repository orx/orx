/**
 * @file orxRegistry.c
 * 
 * Module for registry / config management.
 */ 


 /***************************************************************************
 orxRegistry.c
 Registry / config management
 
 begin                : 05/12/2007
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "io/orxRegistry.h"
#include "plugin/orxPluginCore.h"


/** Setups registry module
 */
orxVOID orxRegistry_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_REGISTRY, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_REGISTRY, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_REGISTRY, orxMODULE_ID_TREE);

  return;
}


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(REGISTRY)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, INIT, orxRegistry_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, EXIT, orxRegistry_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, LOAD, orxRegistry_Load)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, SAVE, orxRegistry_Save)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, GET_INT32, orxRegistry_GetInt32)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, GET_FLOAT, orxRegistry_GetFloat)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, GET_STRING, orxRegistry_GetString)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, GET_BOOL, orxRegistry_GetBool)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, SET_INT32, orxRegistry_SetInt32)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, SET_FLOAT, orxRegistry_SetFloat)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, SET_STRING, orxRegistry_SetString)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(REGISTRY, SET_BOOL, orxRegistry_SetBool)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(REGISTRY)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_Load, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_Save, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_GetInt32, orxS32, orxCONST orxSTRING, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_GetFloat, orxFLOAT, orxCONST orxSTRING, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_GetString, orxSTRING, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_GetBool, orxBOOL, orxCONST orxSTRING, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_SetInt32, orxVOID, orxCONST orxSTRING, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_SetFloat, orxVOID, orxCONST orxSTRING, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_SetString, orxVOID, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRegistry_SetBool, orxVOID, orxCONST orxSTRING, orxBOOL);

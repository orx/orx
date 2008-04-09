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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#include "script/orxScript.h"

/********************
 *  Plugin Related  *
 ********************/

/***************************************************************************
 orxScript_Setup
 Script module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxScript_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SCRIPT, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_SCRIPT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SCRIPT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SCRIPT, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_SCRIPT, orxMODULE_ID_TREE);

  return;
}


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SCRIPT)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, INIT, orxScript_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, EXIT, orxScript_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, RUN_FILE, orxScript_RunFile)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, RUN_STRING, orxScript_RunString)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, GET_TYPE, orxScript_GetType)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, GET_S32_VALUE, orxScript_GetS32Value)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, GET_FLOAT_VALUE, orxScript_GetFloatValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, GET_STRING_VALUE, orxScript_GetStringValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, SET_S32_VALUE, orxScript_SetS32Value)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, SET_FLOAT_VALUE, orxScript_SetFloatValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, SET_STRING_VALUE, orxScript_SetStringValue)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, REGISTER_FUNCTION, orxScript_RegisterFunction)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SCRIPT)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_RunFile, orxSTATUS, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_RunString, orxSTATUS, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_GetType, orxSCRIPT_TYPE, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_GetS32Value, orxSTATUS, orxCONST orxSTRING, orxS32*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_GetFloatValue, orxSTATUS, orxCONST orxSTRING, orxFLOAT*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_GetStringValue, orxSTATUS, orxCONST orxSTRING, orxSTRING*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_SetS32Value, orxSTATUS, orxCONST orxSTRING, orxS32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_SetFloatValue, orxSTATUS, orxCONST orxSTRING, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_SetStringValue, orxSTATUS, orxCONST orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxScript_RegisterFunction, orxSTATUS, orxS32);


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

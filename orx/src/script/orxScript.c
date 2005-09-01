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

/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */
orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SCRIPT)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, INIT, orxScript_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, EXIT, orxScript_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, CREATE, orxScript_Create)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, DELETE, orxScript_Delete)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, LOAD_FILE, orxScript_LoadFile)
//orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, CALL_FUNCTION, orxScript_CallFunc)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SCRIPT, EXECUTE, orxScript_Execute)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SCRIPT)

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

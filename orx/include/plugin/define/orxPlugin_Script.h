/**
 * @file orxPlugin_Script.h
 * 
 * Module to define script plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_Script.h
 
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


#ifndef _orxPLUGIN_SCRIPT_H_
#define _orxPLUGIN_SCRIPT_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_t
{
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_RUN_FILE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_RUN_STRING,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_TYPE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_S32_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_FLOAT_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_STRING_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_SET_S32_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_SET_FLOAT_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_SET_STRING_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_REGISTER_FUNCTION,

  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_SCRIPT;


#endif /* _orxPLUGIN_SCRIPT_H_ */

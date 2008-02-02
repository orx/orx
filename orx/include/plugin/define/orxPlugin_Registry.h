/**
 * @file orxPlugin_Registry.h
 * 
 * Module to define registry plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_Registry.h
 
 begin                : 09/12/2007
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modIDy  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_REGISTRY_H_
#define _orxPLUGIN_REGISTRY_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_t
{
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_LOAD,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_SAVE,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_GET_INT32,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_GET_FLOAT,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_GET_STRING,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_GET_BOOL,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_SET_INT32,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_SET_FLOAT,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_SET_STRING,
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_SET_BOOL,
  
  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_REGISTRY_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_REGISTRY;


#endif /* _orxPLUGIN_REGISTRY_H_ */

/**
 * @file orxPlugin_Package.h
 * 
 * Module to define package plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_Package.h
 
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


#ifndef _orxPLUGIN_PACKAGE_H_
#define _orxPLUGIN_PACKAGE_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_t
{
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_OPEN,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_SET_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_TEST_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_COMMIT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_EXTRACT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_FIRST,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_NEXT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_READ,

  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_NONE = 0xFFFFFFFF
    
} orxPLUGIN_FUNCTION_BASE_ID_PACKAGE;


#endif /* _orxPLUGIN_PACKAGE_H_ */

/**
 * @file orxPlugin_FileSystem.h
 * 
 * Module to define file plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_FileSystem.h
 
 begin                : 01/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modIDy  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_FILESYSTEM_H_
#define _orxPLUGIN_FILESYSTEM_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_t
{
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_FIND_FIRST,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_FIND_NEXT,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_FIND_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_INFO,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_COPY,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_RENAME,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_DELETE,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_CREATE_DIR,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_DELETE_DIR,

  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM;


#endif /* _orxPLUGIN_FILESYSTEM_H_ */

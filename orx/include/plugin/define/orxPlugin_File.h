/**
 * @file orxPlugin_File.h
 * 
 * Module to define file plugin registration ID
 */ 
 
 /***************************************************************************
 orxPlugin_File.h
 
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


#ifndef _orxPLUGIN_FILE_H_
#define _orxPLUGIN_FILE_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_FILE_t
{
  orxPLUGIN_FUNCTION_BASE_ID_FILE_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_FIND_FIRST,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_FIND_NEXT,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_FIND_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_INFO,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_COPY,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_RENAME,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_DELETE,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_CREATE_DIR,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_DELETE_DIR,

  orxPLUGIN_FUNCTION_BASE_ID_FILE_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_FILE_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_FILE;


#endif /* _orxPLUGIN_FILE_H_ */

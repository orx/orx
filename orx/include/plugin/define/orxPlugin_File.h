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
  orxPLUGIN_FUNCTION_BASE_ID_FILE_OPEN,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_READ,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_WRITE,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_GETS,
  orxPLUGIN_FUNCTION_BASE_ID_FILE_CLOSE,

  orxPLUGIN_FUNCTION_BASE_ID_FILE_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_FILE_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_FILE;


#endif /* _orxPLUGIN_FILE_H_ */

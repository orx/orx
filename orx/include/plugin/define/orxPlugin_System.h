/**
 * \file orxPlugin_System.h
 * This header is used to define ID for systemr plugin registration.
 */

/*
 begin                : 25/05/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_SYSTEM_H_
#define _orxPLUGIN_SYSTEM_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_t
{
  orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_GET_TIME,
  orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_DELAY,

  orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_SYSTEM_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_SYSTEM;


#endif /* _orxPLUGIN_SYSTEM_H_ */

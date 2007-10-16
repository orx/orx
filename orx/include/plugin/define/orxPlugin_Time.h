/**
 * \file orxPlugin_Time.h
 * This header is used to define ID for timer plugin registration.
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


#ifndef _orxPLUGIN_TIME_H_
#define _orxPLUGIN_TIME_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_TIME_t
{
  orxPLUGIN_FUNCTION_BASE_ID_TIME_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_TIME_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_TIME_GET_TIME,
  orxPLUGIN_FUNCTION_BASE_ID_TIME_DELAY,

  orxPLUGIN_FUNCTION_BASE_ID_TIME_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_TIME_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_TIME;


#endif /* _orxPLUGIN_TIME_H_ */

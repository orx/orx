/**
 * \file orxPlugin_Mouse.h
 * This header is used to define ID for mouse plugin registration.
 */

/*
 begin                : 22/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 */

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_MOUSE_H_
#define _orxPLUGIN_MOUSE_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_MOUSE_t
{
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_GET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_IS_BUTTON_PRESSED,

  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_MOUSE_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_MOUSE;


#endif /* _orxPLUGIN_MOUSE_H_ */

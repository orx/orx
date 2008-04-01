/**
 * \file orxPlugin_Physics.h
 * This header is used to define ID for physics plugin registration.
 */

/*
 begin                : 24/03/2008
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

#ifndef _orxPLUGIN_PHYSICS_H_
#define _orxPLUGIN_PHYSICS_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_t
{
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_EXIT,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_CREATE_BODY,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_DELETE_BODY,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_CREATE_BODY_PART,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_DELETE_BODY_PART,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_ROTATION,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_ROTATION,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_PHYSICS;


#endif /* _orxPLUGIN_PHYSICS_H_ */

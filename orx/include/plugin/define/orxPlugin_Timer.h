/**
 * \file orxPlugin_Timer.h
 * This header is used to define ID for timer plugin registration.
 */

/*
 begin                : 22/11/2003
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


#ifndef _orxPLUGIN_TIMER_H_
#define _orxPLUGIN_TIMER_H_

#include "orxPlugin_CoreID.h"


/*********************************************
 Constants
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_BASE_ID_TIMER_t
{
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_SET_CLOCK_UPDATE,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_REGISTER_CLOCK,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_UNREGISTER_CLOCK,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_GAME_TIME,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_GAME_DT,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_REAL_TIME,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_REAL_DT,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_SET_TIME_COEF,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_TIME_COEF,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_UPDATE,
  orxPLUGIN_FUNCTION_BASE_ID_TIMER_WRITE_DATE,

  orxPLUGIN_FUNCTION_BASE_ID_TIMER_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_TIMER_NONE = orxENUM_NONE
    
} orxPLUGIN_FUNCTION_BASE_ID_TIMER;


#endif /* _orxPLUGIN_TIMER_H_ */

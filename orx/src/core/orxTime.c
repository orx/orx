/**
 * \file orxTime.c
 */

/***************************************************************************
 begin                : 26/05/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "core/orxTime.h"
#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */
orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(TIME)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, INIT, orxTime_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, EXIT, orxTime_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, GET_TIME, orxTime_GetTime)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, GET_DATE, orxTime_GetDate)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, DELAY, orxTime_Delay)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(TIME)

/**
 * \file timer.c
 */

/***************************************************************************
 begin                : 22/11/2003
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

#include "core/timer.h"
#include "plugin/orxPluginCore.h"


/********************
 *  Plugin Related  *
 ********************/

orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastTimerPluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_TIMER_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &timer_init,                       orxPLUGIN_FUNCTION_BASE_ID_TIMER_INIT},
  {(orxPLUGIN_FUNCTION *) &timer_exit,                       orxPLUGIN_FUNCTION_BASE_ID_TIMER_EXIT},
  {(orxPLUGIN_FUNCTION *) &timer_clock_update_function_set,  orxPLUGIN_FUNCTION_BASE_ID_TIMER_SET_CLOCK_UPDATE},
  {(orxPLUGIN_FUNCTION *) &timer_clock_register,             orxPLUGIN_FUNCTION_BASE_ID_TIMER_REGISTER_CLOCK},
  {(orxPLUGIN_FUNCTION *) &timer_clock_unregister,           orxPLUGIN_FUNCTION_BASE_ID_TIMER_UNREGISTER_CLOCK},
  {(orxPLUGIN_FUNCTION *) &timer_game_time_get,              orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_GAME_TIME},
  {(orxPLUGIN_FUNCTION *) &timer_game_dt_get,                orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_GAME_DT},
  {(orxPLUGIN_FUNCTION *) &timer_real_time_get,              orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_REAL_TIME},
  {(orxPLUGIN_FUNCTION *) &timer_real_dt_get,                orxPLUGIN_FUNCTION_BASE_ID_TIMER_GET_REAL_DT},
  {(orxPLUGIN_FUNCTION *) &timer_time_coef_set,              orxPLUGIN_FUNCTION_BASE_ID_TIMER_SET_TIME_COEF},
  {(orxPLUGIN_FUNCTION *) &timer_time_coef_get,              orxPLUGIN_FUNCTION_BASE_ID_TIMER_SET_TIME_COEF},
  {(orxPLUGIN_FUNCTION *) &timer_update,                     orxPLUGIN_FUNCTION_BASE_ID_TIMER_UPDATE},
  {(orxPLUGIN_FUNCTION *) &timer_date_write,                 orxPLUGIN_FUNCTION_BASE_ID_TIMER_WRITE_DATE}
};

orxVOID timer_plugin_init()
{
  /* Plugin init */
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_TIMER, sastTimerPluginFunctionInfo, sizeof(sastTimerPluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));

  return;
}



/********************
 *   Core Related   *
 ********************/

orxPLUGIN_DEFINE_CORE_FUNCTION(timer_init, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(timer_exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(timer_clock_update_function_set, orxU32, timer_fn_clock_update);
orxPLUGIN_DEFINE_CORE_FUNCTION(timer_clock_register, orxU32, orxU32, orxVOID *);
orxPLUGIN_DEFINE_CORE_FUNCTION(timer_clock_unregister, orxVOID, orxU32);


orxPLUGIN_DEFINE_CORE_FUNCTION(timer_game_time_get, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(timer_game_dt_get, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION(timer_real_time_get, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(timer_real_dt_get, orxU32);

orxPLUGIN_DEFINE_CORE_FUNCTION(timer_update, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(timer_time_coef_set, orxVOID, orxFLOAT, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(timer_time_coef_get, orxFLOAT, orxU32 *);

orxPLUGIN_DEFINE_CORE_FUNCTION(timer_date_write, orxS32, orxU8 *, orxS32, orxCONST orxU8 *);

/**
 * \file timer.c
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
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
#include "plugin/plugin_core.h"


/********************
 *  Plugin Related  *
 ********************/

static plugin_core_st_function plugin_timer_spst_function[PLUGIN_TIMER_KUL_FUNCTION_NUMBER] =
{
  {(plugin_function *) &timer_init,                       PLUGIN_TIMER_KUL_ID_INIT},
  {(plugin_function *) &timer_exit,                       PLUGIN_TIMER_KUL_ID_EXIT},
  {(plugin_function *) &timer_clock_update_function_set,  PLUGIN_TIMER_KUL_ID_CLOCK_UPDATE_SET},
  {(plugin_function *) &timer_clock_register,             PLUGIN_TIMER_KUL_ID_CLOCK_REGISTER},
  {(plugin_function *) &timer_clock_unregister,           PLUGIN_TIMER_KUL_ID_CLOCK_UNREGISTER},
  {(plugin_function *) &timer_game_time_get,              PLUGIN_TIMER_KUL_ID_GAME_TIME_GET},
  {(plugin_function *) &timer_game_dt_get,                PLUGIN_TIMER_KUL_ID_GAME_DT_GET},
  {(plugin_function *) &timer_real_time_get,              PLUGIN_TIMER_KUL_ID_REAL_TIME_GET},
  {(plugin_function *) &timer_real_dt_get,                PLUGIN_TIMER_KUL_ID_REAL_DT_GET},
  {(plugin_function *) &timer_time_coef_set,              PLUGIN_TIMER_KUL_ID_TIME_COEF_SET},
  {(plugin_function *) &timer_time_coef_get,              PLUGIN_TIMER_KUL_ID_TIME_COEF_GET},
  {(plugin_function *) &timer_update,                     PLUGIN_TIMER_KUL_ID_UPDATE},
  {(plugin_function *) &timer_date_write,                 PLUGIN_TIMER_KUL_ID_DATE_WRITE}
};

void timer_plugin_init()
{
  /* Plugin init */
  plugin_core_info_add(PLUGIN_TIMER_KUL_PLUGIN_ID, plugin_timer_spst_function, PLUGIN_TIMER_KUL_FUNCTION_NUMBER);

  return;
}



/********************
 *   Core Related   *
 ********************/

PLUGIN_CORE_FUNCTION_DEFINE(timer_init, uint32);
PLUGIN_CORE_FUNCTION_DEFINE(timer_exit, void);

PLUGIN_CORE_FUNCTION_DEFINE(timer_clock_update_function_set, uint32, timer_fn_clock_update);
PLUGIN_CORE_FUNCTION_DEFINE(timer_clock_register, uint32, uint32, void *);
PLUGIN_CORE_FUNCTION_DEFINE(timer_clock_unregister, void, uint32);


PLUGIN_CORE_FUNCTION_DEFINE(timer_game_time_get, uint32);
PLUGIN_CORE_FUNCTION_DEFINE(timer_game_dt_get, uint32);

PLUGIN_CORE_FUNCTION_DEFINE(timer_real_time_get, uint32);
PLUGIN_CORE_FUNCTION_DEFINE(timer_real_dt_get, uint32);

PLUGIN_CORE_FUNCTION_DEFINE(timer_update, void);

PLUGIN_CORE_FUNCTION_DEFINE(timer_time_coef_set, void, float, uint32);
PLUGIN_CORE_FUNCTION_DEFINE(timer_time_coef_get, float, uint32 *);

PLUGIN_CORE_FUNCTION_DEFINE(timer_date_write, int32, char *, int32, const char *);

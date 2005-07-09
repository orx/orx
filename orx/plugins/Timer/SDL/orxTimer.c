/**
 * \file timer_plug.c
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

#include "orxInclude.h"

#include "plugin/orxPluginUser.h"
#include "core/orxTimerType.h"

#include <time.h>
#include <SDL/sdl.h>


#define KZ_DEFAULT_DATE_FORMAT    "[%Y-%m-%d %H:%M:%S']"


/********************
 *   Core Related   *
 ********************/

static orxTIMER_CLOCK_UPDATE spfn_clock_update = NULL;
static uint32 su32_real_time = 0;
static uint32 su32_game_time = 0;
static uint32 su32_game_dt   = 0;
static uint32 su32_real_dt   = 0;
static float  sf_time_coef  = 0.0;
static uint32 su32_coef_type = 0;


/* Timer callback for clocks */
Uint32 timer_callback(Uint32 _u32_delay, void *_p_data)
{
  /* Has clock update function? */
  if(spfn_clock_update != NULL)
  {
    /* Calls it! */
    spfn_clock_update(_p_data);
  }
  else
  {
    /* No clock update function! */
    /* !!! MSG !!! */
  }

  /* Fixed delay as for now! */
  return _u32_delay;
}

/* Updates game values */
void timer_game_update()
{
  /* Depends on coefficient type */
  switch(su32_coef_type)
  {
    case TIMER_KUL_COEF_TYPE_FIXED:
      su32_game_dt = (uint32)rintf(sf_time_coef);
      break;

    case TIMER_KUL_COEF_TYPE_MULTIPLY:
      su32_game_dt = (uint32)rintf(sf_time_coef * (float)su32_real_dt);
      break;

    default:
      /* !!! MSG !!! */
      su32_game_dt = 0;
  }

  /* Updates game time */
  su32_game_time += su32_game_dt;

  return;
}


/** Inits timer plugin. */
uint32 timer_init()
{
  uint32 u32_result;

  /* Inits SDL_Timer */
  if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
  {
    u32_result = SDL_InitSubSystem(SDL_INIT_TIMER);
  }
  else
  {
    u32_result = SDL_Init(SDL_INIT_TIMER);
  }

  /* Inits internal real/game time mecanism */
  spfn_clock_update = NULL;

  su32_real_time = SDL_GetTicks();
  su32_game_time = SDL_GetTicks();
  su32_real_dt   = 0;
  su32_game_dt   = 0;
  sf_time_coef  = 1.0;
  su32_coef_type = TIMER_KUL_COEF_TYPE_MULTIPLY;

  return u32_result;
}

/** Exits from timer plugin. */
void timer_exit()
{
  /* Cleans internal real/game time mecanism */
  spfn_clock_update = NULL;

  /* Quits SDL Timer */
  if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_TIMER)
  {
    SDL_Quit();
  }
  else
  {
    SDL_QuitSubSystem(SDL_INIT_TIMER);
  }

  return;
}

/** Sets clock update function that will be called according to corresponding timer period. */
void timer_clock_update_function_set(orxTIMER_CLOCK_UPDATE _pfn_clock_update)
{
  /* Stores clock update function */
  spfn_clock_update = _pfn_clock_update;

  return;
}

/** Registers a clock. */
SDL_TimerID timer_clock_register(uint32 _u32_delay, void *_p_data)
{
  /* Registers internal update function with given delay */
  return SDL_AddTimer(_u32_delay, &timer_callback, _p_data);
}

/** Unregisters a clock. */
void timer_clock_unregister(SDL_TimerID _u32_timer_id)
{
  /* Registers internal update function with given delay */
  SDL_RemoveTimer(_u32_timer_id);

  return;
}

/** Gets game time. */
uint32 timer_game_time_get()
{
  return su32_game_time;
}

/** Gets game time dt. */
uint32 timer_game_dt_get()
{
  return su32_game_dt;
}

/** Gets real time. */
uint32 timer_real_time_get()
{
  return su32_real_time;
}

/** Gets real time dt. */
uint32 timer_real_dt_get()
{
  return su32_game_time;
}

/** Sets the real/game time coefficient. Second parameter tells about coefficient type (multiplicator, fixed, ...). */
void timer_time_coef_set(float _f_time_coef, uint32 _u32_coef_type)
{
  /* Stores values */
  sf_time_coef = _f_time_coef;
  su32_coef_type = _u32_coef_type;

  return;
}

/** Updates timer (must be called at each 'frame'. */
void timer_update()
{
  uint32 u32_time;

  /* Gets real time */
  u32_time       = SDL_GetTicks();

  /* Updates real values */
  su32_real_dt   = u32_time - su32_real_time;
  su32_real_time = u32_time;

  /* Update game values */
  timer_game_update();

  return;
}

/** Gets the real/game time coefficient. Parameter gets coefficient type (multiplicator, fixed, ...). */
float timer_time_coef_get(uint32 *_pu32_coef_type)
{
  if(_pu32_coef_type != NULL)
  {
    /* Gets values */
    *_pu32_coef_type = su32_coef_type;
  }

  return sf_time_coef;
}


int32 timer_date_write(char *_ac_buffer, int32 _i_size, const char *_z_format)
{
  time_t l_time;

  /* Gets system time */
  time(&l_time);

  /* Has a date format been specified? */
  if(_z_format != NULL)
  {
    return strftime(_ac_buffer, _i_size, _z_format, localtime(&l_time));
  }
  else
  {
    return strftime(_ac_buffer, _i_size, KZ_DEFAULT_DATE_FORMAT, localtime(&l_time));
  }
}


/********************
 *  Plugin Related  *
 ********************/

static plugin_user_st_function_info timer_plug_spst_function[PLUGIN_TIMER_KUL_FUNCTION_NUMBER];

void plugin_init(int32 *_pi_fn_number, plugin_user_st_function_info **_ppst_fn_info)
{
  PLUGIN_USER_FUNCTION_START(timer_plug_spst_function);


  PLUGIN_USER_CORE_FUNCTION_ADD(timer_init, TIMER, INIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_exit, TIMER, EXIT);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_clock_update_function_set, TIMER, CLOCK_UPDATE_SET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_clock_register, TIMER, CLOCK_REGISTER);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_game_time_get, TIMER, GAME_TIME_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_game_dt_get, TIMER, GAME_DT_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_real_time_get, TIMER, REAL_TIME_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_real_dt_get, TIMER, REAL_DT_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_time_coef_set, TIMER, TIME_COEF_SET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_time_coef_get, TIMER, TIME_COEF_GET);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_update, TIMER, UPDATE);

  PLUGIN_USER_CORE_FUNCTION_ADD(timer_date_write, TIMER, DATE_WRITE);


  PLUGIN_USER_FUNCTION_END(_pi_fn_number, _ppst_fn_info);

  return;
}

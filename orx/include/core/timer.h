/**
 * \file timer.h
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

#ifndef _TIMER_H_
#define _TIMER_H_

#include "include.h"

#include "core/timer_type.h"

#include "msg/msg_timer.h"


/** Inits Timer plugin system. */
extern void     timer_plugin_init();

/** Inits Timer module. */
extern uint32 (*timer_init)();
/** Exits from Timer module. */
extern void   (*timer_exit)();

/** Sets clock update function that will be called according to corresponding timer period. */
extern uint32 (*timer_clock_update_function_set)(timer_fn_clock_update _pfn_clock_update);
/** Registers a clock. */
extern uint32 (*timer_clock_register)(uint32 _u32_delay, void *_p_data);
/** Unregisters a clock. */
extern void   (*timer_clock_unregister)(uint32 _u32_timer_id);


/** Gets game time. */
extern uint32 (*timer_game_time_get)();
/** Gets game time dt. */
extern uint32 (*timer_game_dt_get)();

/** Gets real time. */
extern uint32 (*timer_real_time_get)();
/** Gets real time dt. */
extern uint32 (*timer_real_dt_get)();

/** Sets the real/game time coefficient. Second parameters tells about coefficient type (multiplicator, fixed, ...). */
extern void   (*timer_time_coef_set)(float _f_time_coef, uint32 _u32_coef_type);
/** Gets the real/game time coefficient. Parameter gets coefficient type (multiplicator, fixed, ...). */
extern float  (*timer_time_coef_get)(uint32 *_pu32_coef_type);

/** Updates timer (must be called at each 'frame'). */
extern void   (*timer_update)();

/** Writes the current date/time into a string buffer. Uses formating for output. */
extern int32  (*timer_date_write)(char *_ac_buffer, int32 _i_size, const char *_z_format);


#endif /* _TIMER_H_ */

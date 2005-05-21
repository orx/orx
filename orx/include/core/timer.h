/**
 * \file timer.h
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

#ifndef _TIMER_H_
#define _TIMER_H_

#include "orxInclude.h"

#include "core/timer_type.h"

#include "msg/msg_timer.h"


/** Inits Timer plugin system. */
extern orxVOID     timer_plugin_init();

/** Inits Timer module. */
extern orxU32 (*timer_init)();
/** Exits from Timer module. */
extern orxVOID   (*timer_exit)();

/** Sets clock update function that will be called according to corresponding timer period. */
extern orxU32 (*timer_clock_update_function_set)(timer_fn_clock_update _pfn_clock_update);
/** Registers a clock. */
extern orxU32 (*timer_clock_register)(orxU32 _u32_delay, orxVOID *_p_data);
/** Unregisters a clock. */
extern orxVOID   (*timer_clock_unregister)(orxU32 _u32_timer_id);


/** Gets game time. */
extern orxU32 (*timer_game_time_get)();
/** Gets game time dt. */
extern orxU32 (*timer_game_dt_get)();

/** Gets real time. */
extern orxU32 (*timer_real_time_get)();
/** Gets real time dt. */
extern orxU32 (*timer_real_dt_get)();

/** Sets the real/game time coefficient. Second parameters tells about coefficient type (multiplicator, fixed, ...). */
extern orxVOID   (*timer_time_coef_set)(orxFLOAT _f_time_coef, orxU32 _u32_coef_type);
/** Gets the real/game time coefficient. Parameter gets coefficient type (multiplicator, fixed, ...). */
extern orxFLOAT  (*timer_time_coef_get)(orxU32 *_pu32_coef_type);

/** Updates timer (must be called at each 'frame'). */
extern orxVOID   (*timer_update)();

/** Writes the current date/time into a string buffer. Uses formating for output. */
extern orxS32  (*timer_date_write)(orxU8 *_ac_buffer, orxS32 _i_size, orxCONST orxU8 *_zFormat);


#endif /* _TIMER_H_ */

/**
 * \file clock.h
 */

/***************************************************************************
 begin                : 28/01/2004
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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "include.h"

#define CLOCK_KI_CLOCK_MAX_NUMBER       32          /**< Max clock number */

#define CLOCK_KI_FUNCTION_MAX_NUMBER    32          /**< Max function per clock */


/** Clock ID flags */
#define CLOCK_KUL_ID_FLAG_NONE          0x00000000  /**< No flags */
#define CLOCK_KUL_ID_FLAG_HAS_ROOM      0x00100000  /**< Clock has room for function registering */
#define CLOCK_KUL_ID_FLAG_GAME_TIME     0x01000000  /**< Clock is game-time updated */
#define CLOCK_KUL_ID_FLAG_REAL_TIME     0x02000000  /**< Clock is real-time updated */
#define CLOCK_KUL_ID_FLAG_USER          0x10000000  /**< Clock is user defined */


/** Clock info structure. */
typedef struct clock_st_clock_info_t
{
  uint32 u32_clock_id;                 /**< Clock ID : 4 */
  uint32 u32_clock_period;             /**< Clock period (in millisecond) : 8 */
  uint32 u32_clock_start;              /**< Clock start date : 12 */
  uint32 u32_clock_iteration;          /**< Clock current iteration : 16 */
} clock_st_clock_info;

/** Clock structure. */
typedef struct clock_st_clock_t clock_st_clock;

/** Clock callback function type for used with clock bindings. */
typedef void (*clock_fn_callback)(clock_st_clock_info *_pst_clock_info);


/** Inits Clock module. */
extern uint32               clock_init();
/** Exits from Clock module. */
extern void                 clock_exit();

/** Adds a Clock. */
extern uint32               clock_add(uint32 _u32_period);
/** Removes a Clock. */
extern void                 clock_remove(uint32 _u32_clock_id);

/** Gets informations about a Clock. */
extern clock_st_clock_info *clock_info_get(uint32 _u32_clock_id);
/** Gets a Clock's current iteration number, given it's ID. */
extern uint32               clock_iteration_get(uint32 _u32_clock_id);
/** Gets a Clock's current iteration number, given it's info structure. */
extern uint32               clock_info_iteration_get(clock_st_clock_info *_pst_clock_info);

/** Registers a callback function to a clock. */
extern bool                 clock_cb_function_register(uint32 _u32_clock_id, clock_fn_callback _pfn_callback);
/** Unregisters a callback function from a clock. */
extern bool                 clock_cb_function_unregister(uint32 _u32_clock_id, clock_fn_callback _pfn_callback);

/** Adds a callback function called at a given period to a realtime user clock. */
extern bool                 clock_cb_function_add(clock_fn_callback _pfn_callback, uint32 _u32_period);
/** Removes a callback function called at the given period from a realtime user clock. */
extern bool                 clock_cb_function_remove(clock_fn_callback _pfn_callback, uint32 _u32_period);

/** Sets/Resets a clock id flags. */
extern void                 clock_id_flags_set(uint32 _u32_clock_id, uint32 _u32_flags_add, uint32 _u32_flags_remove);
/** Tests a clock id flag. */
extern bool                 clock_id_flag_test(uint32 _u32_clock_id, uint32 _u32_flag);

/** Finds a clock according to id flags and period. */
extern uint32               clock_find(uint32 _u32_period, uint32 _u32_match_flags);

#endif /* _CLOCK_H_ */

/**
 * \file clock.h
 */

/***************************************************************************
 begin                : 28/01/2004
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

#ifndef _CLOCK_H_
#define _CLOCK_H_

#include "orxInclude.h"

#define CLOCK_KS32_CLOCK_MAX_NUMBER       32          /**< Max clock number */

#define CLOCK_KS32_FUNCTION_MAX_NUMBER    32          /**< Max function per clock */


/** Clock ID flags */
#define CLOCK_KU32_ID_FLAG_NONE          0x00000000  /**< No flags */
#define CLOCK_KU32_ID_FLAG_HAS_ROOM      0x00100000  /**< Clock has room for function registering */
#define CLOCK_KU32_ID_FLAG_GAME_TIME     0x01000000  /**< Clock is game-time updated */
#define CLOCK_KU32_ID_FLAG_REAL_TIME     0x02000000  /**< Clock is real-time updated */
#define CLOCK_KU32_ID_FLAG_USER          0x10000000  /**< Clock is user defined */


/** Clock info structure. */
typedef struct clock_st_clock_info_t
{
  orxU32 u32_clock_id;                 /**< Clock ID : 4 */
  orxU32 u32_clock_period;             /**< Clock period (in millisecond) : 8 */
  orxU32 u32_clock_start;              /**< Clock start date : 12 */
  orxU32 u32_clock_iteration;          /**< Clock current iteration : 16 */
} clock_st_clock_info;

/** Clock structure. */
typedef struct clock_st_clock_t clock_st_clock;

/** Clock callback function type for used with clock bindings. */
typedef orxVOID (*clock_fn_callback)(clock_st_clock_info *_pst_clock_info);


/** Inits Clock module. */
extern orxU32               clock_init();
/** Exits from Clock module. */
extern orxVOID                 clock_exit();

/** Adds a Clock. */
extern orxU32               clock_add(orxU32 _u32_period);
/** Removes a Clock. */
extern orxVOID                 clock_remove(orxU32 _u32_clock_id);

/** Gets informations about a Clock. */
extern clock_st_clock_info *clock_info_get(orxU32 _u32_clock_id);
/** Gets a Clock's current iteration number, given it's ID. */
extern orxU32               clock_iteration_get(orxU32 _u32_clock_id);
/** Gets a Clock's current iteration number, given it's info structure. */
extern orxU32               clock_info_iteration_get(clock_st_clock_info *_pst_clock_info);

/** Registers a callback function to a clock. */
extern orxBOOL                 clock_cb_function_register(orxU32 _u32_clock_id, clock_fn_callback _pfn_callback);
/** Unregisters a callback function from a clock. */
extern orxBOOL                 clock_cb_function_unregister(orxU32 _u32_clock_id, clock_fn_callback _pfn_callback);

/** Adds a callback function called at a given period to a realtime user clock. */
extern orxBOOL                 clock_cb_function_add(clock_fn_callback _pfn_callback, orxU32 _u32_period);
/** Removes a callback function called at the given period from a realtime user clock. */
extern orxBOOL                 clock_cb_function_remove(clock_fn_callback _pfn_callback, orxU32 _u32_period);

/** Sets/Resets a clock id flags. */
extern orxVOID                 clock_id_flags_set(orxU32 _u32_clock_id, orxU32 _u32Flags_add, orxU32 _u32Flags_remove);
/** Tests a clock id flag. */
extern orxBOOL                 clock_id_flag_test(orxU32 _u32_clock_id, orxU32 _u32Flag);

/** Finds a clock according to id flags and period. */
extern orxU32               clock_find(orxU32 _u32_period, orxU32 _u32_match_flags);

#endif /* _CLOCK_H_ */

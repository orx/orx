/**
 * \file clock.c
 */

/***************************************************************************
 begin                : 04/02/2004
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


#include "core/clock.h"

#include "core/timer.h"


/*
 * Platform independant defines
 */

#define CLOCK_KUL_FLAG_NONE             0x00000000
#define CLOCK_KUL_FLAG_READY            0x00000001
#define CLOCK_KUL_FLAG_NO_CLOCK_LEFT    0x00000100
#define CLOCK_KUL_FLAG_UPDATE_LOCK      0x10000000

#define CLOCK_KUL_FLAG_DEFAULT          0x00000000


#define CLOCK_KUL_ID_FLAG_DEFAULT       CLOCK_KUL_ID_FLAG_REAL_TIME|CLOCK_KUL_ID_FLAG_HAS_ROOM|CLOCK_KUL_ID_FLAG_USER

#define CLOCK_KUL_ID_MASK_INDEX         0x0000001F
#define CLOCK_KUL_ID_MASK_FLAGS         0xFFFF0000


/*
 * Internal clock structure.
 */
struct clock_st_clock_t
{
  /* Clock Info Structure : 16 */
  clock_st_clock_info st_clock_info;

  /* Callback functions : 144 */
  clock_fn_callback apfn_function[CLOCK_KI_FUNCTION_MAX_NUMBER];

  /* Callback function map : 148 */
  uint32 u32_function_map;

  /* Clock id flags : 152 */
  uint32 u32_id_flags;

  /* Associated timer id : 156 */
  uint32 u32_timer_id;

  /* 4 extra bytes of padding : 160 */
  uint8 auc_unused[4];
};


/*
 * Static members
 */
static uint32 clock_su32_flags = CLOCK_KUL_FLAG_DEFAULT;
static clock_st_clock clock_sast_clocks[CLOCK_KI_CLOCK_MAX_NUMBER];
static uint32 clock_su32_map = 0x00000000;

/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 clock_get
 Gets a clock given its id.

 returns: clock pointer
 ***************************************************************************/
inline clock_st_clock *clock_get(uint32 _u32_clock_id)
{
  /* Returns the clock structure pointer */
  return(&(clock_sast_clocks[_u32_clock_id]));
}

/***************************************************************************
 clock_find_from
 Finds a clock according to its id flags and period, starting at given index.

 returns: Clock id on success / KUL_UNDEFINED on failure
 ***************************************************************************/
inline uint32 clock_find_from(uint32 _u32_period, uint32 _u32_match_flags, uint32 _u32_start_index)
{
  clock_st_clock *pst_clock;
  uint32 u32_index, u32_map;

  /* For all registered clocks */
  for(u32_map = clock_su32_map >> _u32_start_index, u32_index = _u32_start_index;
      u32_map != 0x00000000;
      u32_map >>= 1, u32_index++)
  {
    /* Is the clock used? */
    if(u32_map & 0x00000001)
    {
      /* Gets the clock */
      pst_clock = clock_get(u32_index);

      /* Tests the clock period */
      if((pst_clock->st_clock_info).u32_clock_period == _u32_period)
      {
        /* Tests the clock flags */
        if((pst_clock->u32_id_flags & _u32_match_flags) == _u32_match_flags)
        {
          /* Found! */
          return u32_index;
        }
      }
    }
  }

  /* No clock found! */
  /* !!! MSG !!! */

  return KUL_UNDEFINED;
}

/***************************************************************************
 clock_update
 Updates running clocks.

 returns: void
 ***************************************************************************/
inline void clock_update(uint32 _u32_clock_id)
{
  /* Is there no update in progress (thread-safe)? */
  if(!(clock_su32_flags & CLOCK_KUL_FLAG_UPDATE_LOCK))
  {
    clock_st_clock *pst_clock;
    uint32 u32_map, u32_index;

    /* Locks update */
    clock_su32_flags |= CLOCK_KUL_FLAG_UPDATE_LOCK;

    /* Gets clock */
    pst_clock = clock_get(_u32_clock_id);

    /* Updates all functions */
    for(u32_map = pst_clock->u32_function_map, u32_index = 0;
        u32_map != 0x00000000;
        u32_map >>= 1, u32_index++)
    {
      /* Is function registered? */
      if(u32_map & 0x00000001)
      {
        /* Calls function */
        (pst_clock->apfn_function[u32_index])(&(pst_clock->st_clock_info));
      }
    }

    /* Unlocks update */
    clock_su32_flags &= ~CLOCK_KUL_FLAG_UPDATE_LOCK;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 clock_map_get
 Clock map get function.

 returns: Clock map if used / 0x00000000 if not
 ***************************************************************************/
inline uint32 clock_map_get(uint32 _u32_clock_id)
{
  uint32 u32_map;

  /* Computes map for the given index */
  u32_map = 0x00000001 << _u32_clock_id;

  /* Returns test result */
  return(clock_su32_map & u32_map);
}

/***************************************************************************
 clock_info_init
 Inits a clock info.

 returns: void
 ***************************************************************************/
inline void clock_info_init(clock_st_clock_info *_pst_clock_info, uint32 _u32_id, uint32 _u32_period)
{
  /* Stores info */
  _pst_clock_info->u32_clock_id        = _u32_id;
  _pst_clock_info->u32_clock_period    = _u32_period;
  _pst_clock_info->u32_clock_start     = timer_real_time_get();
  _pst_clock_info->u32_clock_iteration = 0;

  return;
}

/***************************************************************************
 clock_function_find
 Finds a function in a clock.

 returns: Function index if found / KUL_UNDEFINED else
 ***************************************************************************/
inline uint32 clock_function_find(clock_st_clock *_pst_clock, clock_fn_callback _pfn_function)
{
  uint32 u32_map, u32_index;

  /* Find first */
  for(u32_map = _pst_clock->u32_function_map, u32_index = 0;
      u32_map != 0x00000000;
      u32_map >>= 1, u32_index++)
  {
    /* Is the function slot used? */
    if(u32_map & 0x00000001)
    {
      /* Is this the good function? */
      if(_pst_clock->apfn_function[u32_index] == _pfn_function)
      {
        /* Returns the function index */
        return u32_index;
      }
    }
  }

  /* Not found */
  /* !!! MSG !!! */

  return KUL_UNDEFINED;
}

/***************************************************************************
 clock_function_add
 Adds a function to a clock.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
inline bool clock_function_add(clock_st_clock *_pst_clock, clock_fn_callback _pfn_function)
{
  uint32 u32_index = 0x00000000, ul;

  /* Are there free function slots? */
  if(_pst_clock->u32_id_flags & CLOCK_KUL_ID_FLAG_HAS_ROOM)
  {
    /* Gets first free index */
    for(ul = _pst_clock->u32_function_map; ul & 0x00000001; ul >>= 1)
    {
      u32_index++;
    }

    /* Stores function */
    (_pst_clock->apfn_function)[u32_index] = _pfn_function;

    /* Updates function map */
    _pst_clock->u32_function_map |= (0x00000001 << u32_index);

    /* Are all slots used? */
    if(u32_index == CLOCK_KI_FUNCTION_MAX_NUMBER - 1)
    {
      /* !!! MSG !!! */

      /* Updates flags */
      _pst_clock->u32_id_flags &= ~CLOCK_KUL_ID_FLAG_HAS_ROOM;
    }
  }
  else
  {
    /*!!! MSG !!! */

    return FALSE;
  }

  return TRUE;
}

/***************************************************************************
 clock_function_remove
 Removes a function from a clock.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
inline bool clock_function_remove(clock_st_clock *_pst_clock, clock_fn_callback _pfn_function)
{
  uint32 u32_index;

  /* Finds function in the given clock */
  u32_index = clock_function_find(_pst_clock, _pfn_function);

  /* Found ? */
  if(u32_index != KUL_UNDEFINED)
  {
    /* Cleans function pointer */
    _pst_clock->apfn_function[u32_index] = NULL;

    /* Updates function map */
    _pst_clock->u32_function_map &= ~(0x00000001 << u32_index);

    /* Updates flag? */
    if(!(_pst_clock->u32_id_flags & CLOCK_KUL_ID_FLAG_HAS_ROOM))
    {
      /* !!! MSG !!! */

      /* Updates flags */
      _pst_clock->u32_id_flags |= CLOCK_KUL_ID_FLAG_HAS_ROOM;
    }
  }
  else
  {
    /*!!! MSG !!! */

    return FALSE;
  }

  return TRUE;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 clock_init
 Inits the clock module.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 clock_init()
{
  /* Not already Initialized? */
  if(!(clock_su32_flags & CLOCK_KUL_FLAG_READY))
  {
    /* Inits timer before */
    timer_init();

    /* Inits ID Flags */
    clock_su32_flags = CLOCK_KUL_FLAG_DEFAULT|CLOCK_KUL_FLAG_READY;

    /* Inits clock map */
    clock_su32_map ^= clock_su32_map;

    /* Sets timer clock update function */
    if(timer_clock_update_function_set((timer_fn_clock_update) &clock_update) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */
      return EXIT_FAILURE;
    }

    /* Everything's fine */
    return EXIT_SUCCESS;
  }

  /* Already initialized */
  /* !!! MSG !!! */

  return EXIT_FAILURE;
}

/***************************************************************************
 clock_exit
 Exits from clock module.

 returns: void
 ***************************************************************************/
void clock_exit()
{
  /* Initialized? */
  if(clock_su32_flags & CLOCK_KUL_FLAG_READY)
  {
    /* Updates flags */
    clock_su32_flags &= ~(CLOCK_KUL_FLAG_READY|CLOCK_KUL_FLAG_NO_CLOCK_LEFT);

    /* Cleans clock map */
    clock_su32_map ^= clock_su32_map;
  }

  return;
}


/***************************************************************************
 clock_add
 Adds a clock.

 returns: Clock's ID on success / KUL_UNDEFINED on failure
 ***************************************************************************/
uint32 clock_add(uint32 _u32_period)
{
  uint32 u32_index = 0x00000000, ul;
  clock_st_clock *pst_clock;

  /* Are all clocks used? */
  if(clock_su32_flags & CLOCK_KUL_FLAG_NO_CLOCK_LEFT)
  {
    /*!!! MSG !!! */

    return KUL_UNDEFINED;
  }

  /* Gets first free index */
  for(ul = clock_su32_map; ul & 0x00000001; ul >>= 1)
  {
    u32_index++;
  }

  /* Gets clock */
  pst_clock = clock_get(u32_index);

  /* Inits clock structure */
  pst_clock->u32_function_map  = 0x00000000;
  pst_clock->u32_id_flags      = CLOCK_KUL_ID_FLAG_DEFAULT;
  clock_info_init(&(pst_clock->st_clock_info), u32_index, _u32_period);

  /* Updates clock map */
  clock_su32_flags |= (0x00000001 << u32_index);

  /* Are all clocks used? */
  if(u32_index == CLOCK_KI_CLOCK_MAX_NUMBER - 1)
  {
    /* !!! MSG !!! */
   
    /* Updates flags */
    clock_su32_flags |= CLOCK_KUL_FLAG_NO_CLOCK_LEFT;
  }

  /* Registers clock timer */
  pst_clock->u32_timer_id = timer_clock_register(_u32_period, (void *)u32_index);

  /* Returns clock ID */
  return u32_index;
}

/***************************************************************************
 clock_remove
 Removes a clock.

 returns: void
 ***************************************************************************/
void clock_remove(uint32 _u32_clock_id)
{
  uint32 u32_map;
  clock_st_clock *pst_clock;

  /* Get clock map */
  u32_map = clock_map_get(_u32_clock_id);

  /* Is clock used? */
  if(u32_map != 0x00000000)
  {
    /* Gets clock */
    pst_clock = clock_get(_u32_clock_id);

    /* Unregisters clock timer */
    timer_clock_unregister(pst_clock->u32_timer_id);

    /* Cleans clock structure */
    pst_clock->u32_function_map  = 0x00000000;
    pst_clock->u32_id_flags      = CLOCK_KUL_ID_FLAG_NONE;
    clock_info_init(&(pst_clock->st_clock_info), 0, 0);

    /* Updates clock map */
    clock_su32_map &= ~u32_map;

    /* Updates flag if needed */
    if(clock_su32_flags & CLOCK_KUL_FLAG_NO_CLOCK_LEFT)
    {
      clock_su32_flags &= ~CLOCK_KUL_FLAG_NO_CLOCK_LEFT;
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 clock_find
 Finds a clock according to its id flags and period.

 returns: Clock id on success / KUL_UNDEFINED on failure
 ***************************************************************************/
uint32 clock_find(uint32 _u32_period, uint32 _u32_match_flags)
{
  /* Returns first clock found */
  return(clock_find_from(_u32_period, _u32_match_flags, 0));
}

/***************************************************************************
 clock_info_get
 Gets informations about a clock.

 returns: Pointer on the clock info structure
 ***************************************************************************/
clock_st_clock_info *clock_info_get(uint32 _u32_clock_id)
{
  clock_st_clock_info *pst_clock_info = NULL;
  clock_st_clock *pst_clock;

  /* Gets clock */
  pst_clock = clock_get(_u32_clock_id);

  /* Stores the clock info structure pointer */
  pst_clock_info = &(pst_clock->st_clock_info);

  return pst_clock_info;
}

/***************************************************************************
 clock_id_flags_set
 Sets/Resets a clock id flags.

 returns: void
 ***************************************************************************/
inline void clock_id_flags_set(uint32 _u32_clock_id, uint32 _u32_flags_add, uint32 _u32_flags_remove)
{
  clock_st_clock *pst_clock;

  /* Gets clock */
  pst_clock = clock_get(_u32_clock_id);

  /* Updates clock id flags */
  pst_clock->u32_id_flags &= ~(_u32_flags_remove & CLOCK_KUL_ID_MASK_FLAGS);
  pst_clock->u32_id_flags |= (_u32_flags_add & CLOCK_KUL_ID_MASK_FLAGS);

  return;
}

/***************************************************************************
 clock_id_flag_test
 Tests a clock id flag.

 returns: TRUE / FALSE
 ***************************************************************************/
inline bool clock_id_flag_test(uint32 _u32_clock_id, uint32 _u32_flag)
{
  clock_st_clock *pst_clock;

  /* Gets clock */
  pst_clock = clock_get(_u32_clock_id);

  /* Returns test result */
  return((pst_clock->u32_id_flags & _u32_flag) ? TRUE : FALSE);
}

/***************************************************************************
 clock_cb_function_register
 Registers a callback function to a clock.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
bool clock_cb_function_register(uint32 _u32_clock_id, clock_fn_callback _pfn_callback)
{
  clock_st_clock *pst_clock;
  bool bResult = FALSE;

  /* Gets clock */
  pst_clock = clock_get(_u32_clock_id);

  /* Found? */
  if(pst_clock != NULL)
  {
    /* Adds function to clock, if possible */
    bResult = clock_function_add(pst_clock, _pfn_callback);
  }

  return bResult;
}

/***************************************************************************
 clock_cb_function_unregister
 Unregisters a callback function from a clock.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
bool clock_cb_function_unregister(uint32 _u32_clock_id, clock_fn_callback _pfn_callback)
{
  clock_st_clock *pst_clock;

  /* Gets clock */
  pst_clock = clock_get(_u32_clock_id);

  /* Found? */
  if(pst_clock != NULL)
  {
    /* Removes function from clock, if present */
    clock_function_remove(pst_clock, _pfn_callback);
  }
  else
  {
    /* !!! MSG !!! */

    return FALSE;
  }

  return TRUE;
}

/***************************************************************************
 clock_cb_function_add
 Adds a callback function called at a given period to a realtime user clock.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
bool clock_cb_function_add(clock_fn_callback _pfn_callback, uint32 _u32_period)
{
  uint32 u32_clock_id;

  /* Finds a realtime clock of the requested period with room left for a function */
  u32_clock_id = clock_find_from(_u32_period, CLOCK_KUL_ID_FLAG_DEFAULT, 0);

  /* Found? */
  if(u32_clock_id != KUL_UNDEFINED)
  {
    /* Registers function to the clock */
    clock_cb_function_register(u32_clock_id, _pfn_callback);
  }
  else
  {
    /* Adds a new realtime clock */
    /* !!! MSG !!! */

    u32_clock_id = clock_add(_u32_period);

    /* Everything's ok? */
    if(u32_clock_id != KUL_UNDEFINED)
    {
      /* Registers function to the clock */
      clock_cb_function_register(u32_clock_id, _pfn_callback);
    }
    else
    {
      /* No room left for extra clock! */
      /* !!! MSG !!! */
      
      return FALSE;
    }
  }

  return TRUE;
}

/***************************************************************************
 clock_cb_function_remove
 Removes a callback function called at the given period from a realtime user clock.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
bool clock_cb_function_remove(clock_fn_callback _pfn_callback, uint32 _u32_period)
{
  clock_st_clock *pst_clock;
  uint32 u32_clock_id;

  /* Finds all realtime clock of given period */
  for(u32_clock_id = clock_find_from(_u32_period, CLOCK_KUL_ID_FLAG_REAL_TIME|CLOCK_KUL_ID_FLAG_USER, 0);
      u32_clock_id != KUL_UNDEFINED;
      u32_clock_id = clock_find_from(_u32_period, CLOCK_KUL_ID_FLAG_REAL_TIME|CLOCK_KUL_ID_FLAG_USER, u32_clock_id + 1))
  {
    /* Gets clock */
    pst_clock = clock_get(u32_clock_id);

    /* Try to remove function from this clock */
    if(clock_function_remove(pst_clock, _pfn_callback) != FALSE)
    {
      /* Done ! */
      return TRUE;
    }
  }

  /* Function not found in any clock! */
  /* !!! MSG !!! */

  return FALSE;
}

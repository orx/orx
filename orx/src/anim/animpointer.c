/***************************************************************************
 animpointer.c
 Animation Pointer module
 
 begin                : 03/03/2004
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


#include "anim/animpointer.h"

#include "debug/debug.h"
#include "core/timer.h"


/*
 * Platform independant defines
 */

#define ANIMPOINTER_KUL_FLAG_NONE                 0x00000000
#define ANIMPOINTER_KUL_FLAG_READY                0x00000001
#define ANIMPOINTER_KUL_FLAG_DEFAULT              0x00000000

#define ANIMPOINTER_KUL_ID_FLAG_NONE              0x00000000

#define ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM  0x01000000
#define ANIMPOINTER_KUL_ID_FLAG_HAS_NEXT_ANIM     0x02000000
#define ANIMPOINTER_KUL_ID_FLAG_ANIMSET           0x10000000
#define ANIMPOINTER_KUL_ID_FLAG_LINK_TABLE        0x20000000
#define ANIMPOINTER_KUL_ID_FLAG_PAUSED            0x40000000

#define ANIMPOINTER_KUL_ID_FLAG_ALL               0xFFFF0000

#define ANIMPOINTER_KF_FREQUENCY_DEFAULT          1.0


/*
 * Animation Pointer structure
 */
struct st_animpointer_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Id flags : 20 */
  uint32 u32_id_flags;

  /* Referenced AnimationSet : 24 */
  animset_st_animset *pst_animset;

  /* Link table pointer : 28 */
  animset_st_link_table *pst_link_table;

  /* Current animation ID : 32 */
  uint32 u32_crt_anim_id;

  /* Destination animation ID : 36 */
  uint32 u32_dst_anim_id;

  /* Current TimeStamp (Relative to current animation) : 40 */
  uint32 u32_crt_anim_timestamp;

  /* Current TimeStamp (Absolute) : 44 */
  uint32 u32_timestamp;

  /* Current animation frequency : 48 */
  float f_frequency;
};



/*
 * Static members
 */
static uint32 animpointer_su32_flags = ANIMPOINTER_KUL_FLAG_DEFAULT;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 animpointer_list_delete
 Deletes all Animation Pointers.

 returns: void
 ***************************************************************************/
inline void animpointer_list_delete()
{
  animpointer_st_animpointer *pst_animpointer = (animpointer_st_animpointer *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER);

  /* Non empty? */
  while(pst_animpointer != NULL)
  {
    /* Deletes Animation Set */
    animpointer_delete(pst_animpointer);

    /* Gets first Animation Set */
    pst_animpointer = (animpointer_st_animpointer *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 animpointer_init
 Inits Animation Pointer system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 animpointer_init()
{
  /* Already Initialized? */
  if(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Inits AnimSet before */
  animset_init();

  /* Inits Flags */
  animpointer_su32_flags = ANIMPOINTER_KUL_FLAG_READY;

  return EXIT_SUCCESS;
}

/***************************************************************************
 animpointer_exit
 Exits from the Animation Set system.

 returns: void
 ***************************************************************************/
void animpointer_exit()
{
  /* Initialized? */
  if(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY)
  {
    /* Deletes animpointer list */
    animpointer_list_delete();

    /* Updates flags */
    animpointer_su32_flags &= ~ANIMPOINTER_KUL_FLAG_READY;

    /* Exits from AnimSet after */
    animset_exit();
  }

  return;
}

/***************************************************************************
 animpointer_create
 Creates an empty AnimationPointer, given its AnimationSet reference.

 returns: Created animpointer.
 ***************************************************************************/
animpointer_st_animpointer *animpointer_create(animset_st_animset *_pst_animset)
{
  animpointer_st_animpointer *pst_animpointer;

  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Creates animpointer */
  pst_animpointer = (animpointer_st_animpointer *)malloc(sizeof(animpointer_st_animpointer));

  /* Non null? */
  if(pst_animpointer != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_animpointer, STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_animpointer);

      /* Returns nothing */
      return NULL;
    }

    /* Stores animset */
    pst_animpointer->pst_animset = _pst_animset;

    /* Adds a reference on the animset */
    animset_reference_add(_pst_animset);

    /* Inits flags */
    animpointer_flag_set(pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_ANIMSET | ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM, ANIMPOINTER_KUL_ID_FLAG_ALL);

    /* Inits value */
    pst_animpointer->u32_crt_anim_id = 0;
    pst_animpointer->u32_crt_anim_timestamp = 0;
    pst_animpointer->f_frequency = ANIMPOINTER_KF_FREQUENCY_DEFAULT;
    pst_animpointer->u32_timestamp = timer_game_time_get();
    pst_animpointer->u32_dst_anim_id = KUL_UNDEFINED;

    /* Is animset link table non-static? */
    if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_LINK_STATIC) == FALSE)
    {
      /* Stores link table */
      pst_animpointer->pst_link_table = animset_link_table_duplicate(_pst_animset);

      /* Updates flags */
      animpointer_flag_set(pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_LINK_TABLE, ANIMPOINTER_KUL_ID_FLAG_NONE);
    }
  }

  /* Done! */
  return pst_animpointer;
}

/***************************************************************************
 animpointer_delete
 Deletes an Animation Pointer.

 returns: void
 ***************************************************************************/
void animpointer_delete(animpointer_st_animpointer *_pst_animpointer)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  /* Has an animset? */
  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_ANIMSET) != FALSE)
  {
    /* Removes the reference from the animset */
    animset_reference_remove(_pst_animpointer->pst_animset);
  }

  /* Has a link table? */
  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_LINK_TABLE) != FALSE)
  {
    /* Deletes it */
    animset_link_table_delete(_pst_animpointer->pst_link_table);
  }

  /* Cleans structure */
  structure_struct_clean((structure_st_struct *)_pst_animpointer);

  /* Frees animpointer memory */
  free(_pst_animpointer);

  return;
}

/***************************************************************************
 animpointer_animset_get
 Gets the referenced Animation Set.

 returns: void
 ***************************************************************************/
animset_st_animset *animpointer_animset_get(animpointer_st_animpointer *_pst_animpointer)
{
  animset_st_animset *pst_animset = NULL;

  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  /* Has animset? */
  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_ANIMSET) != FALSE)
  {
    pst_animset = _pst_animpointer->pst_animset;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pst_animset;
}

/***************************************************************************
 animpointer_compute
 Computes animation for the given TimeStamp.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animpointer_compute(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_timestamp)
{
  uint32 u32_new_anim_id, u32_timedt;

  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);
  ASSERT(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_ANIMSET) != FALSE);

  /* Paused? */
  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_PAUSED) != FALSE)
  {
    /* !!! MSG !!! */
    
    return EXIT_SUCCESS;
  }

  /* No current animation */
  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM) == FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Computes TimeDT */
  u32_timedt = (uint32)((float)(_u32_timestamp - _pst_animpointer->u32_timestamp) * _pst_animpointer->f_frequency);

  /* Updates TimeStamps */
  _pst_animpointer->u32_timestamp = _u32_timestamp;
  _pst_animpointer->u32_crt_anim_timestamp += u32_timedt;

  /* Computes & updates anim*/
  u32_new_anim_id = animset_anim_compute(_pst_animpointer->pst_animset, _pst_animpointer->u32_crt_anim_id, _pst_animpointer->u32_dst_anim_id, &(_pst_animpointer->u32_crt_anim_timestamp), _pst_animpointer->pst_link_table);

  /* Change happened? */
  if(u32_new_anim_id != _pst_animpointer->u32_crt_anim_id)
  {
    /* Updates anim id */
    _pst_animpointer->u32_crt_anim_id = u32_new_anim_id;

    /* No next anim? */
    if(u32_new_anim_id == KUL_UNDEFINED)
    {
      /* Updates flags */
      animpointer_flag_set(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM, ANIMPOINTER_KUL_ID_FLAG_NONE);
    }

    /* !!! TODO !!! */
  }

  return EXIT_SUCCESS;
}

/***************************************************************************
 animpointer_update_all
 Updates all AnimationPointer according to the given TimeStamp.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animpointer_update_all(uint32 _u32_timestamp)
{
  animpointer_st_animpointer *pst_animpointer;

  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);

  /* For all animpointers */
  for(pst_animpointer = (animpointer_st_animpointer *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER);
      pst_animpointer != NULL;
      pst_animpointer = (animpointer_st_animpointer *)structure_struct_next_get((structure_st_struct *)pst_animpointer))
  {
    /* Computes anim */
    animpointer_compute(pst_animpointer, _u32_timestamp);
  }

  /* Done! */
  return EXIT_SUCCESS;
}

/***************************************************************************
 animpointer_anim_get
 AnimationPointer current Animation get accessor.

 returns: anim_st_anim
 ***************************************************************************/
anim_st_anim *animpointer_anim_get(animpointer_st_animpointer *_pst_animpointer)
{
  anim_st_anim *pst_anim = NULL;

  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  if((animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_ANIMSET) != FALSE)
  && (animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM) != FALSE))
  {
    pst_anim = animset_anim_get(_pst_animpointer->pst_animset, _pst_animpointer->u32_crt_anim_id);
  }
  else
  {
    /* !!! MSG !!! */

  }

  return pst_anim;    
}

/***************************************************************************
 animpointer_time_get
 AnimationPointer current Animation TimeStamp get accessor.

 returns: uint32 timestamp
 ***************************************************************************/
uint32 animpointer_time_get(animpointer_st_animpointer *_pst_animpointer)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM) != FALSE)
  {
    return _pst_animpointer->u32_crt_anim_timestamp;
  }
  else
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }
}

/***************************************************************************
 animpointer_frequency_get
 AnimationPointer Frequency get accessor.

 returns: float frequency
 ***************************************************************************/
float animpointer_frequency_get(animpointer_st_animpointer *_pst_animpointer)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  return _pst_animpointer->f_frequency;
}

/***************************************************************************
 animpointer_anim_set
 AnimationPointer current Animation set accessor.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animpointer_anim_set(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_anim_id)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  /* No Animset? */
  if(animpointer_flag_test(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_ANIMSET) != FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Out of range? */
  if(_u32_anim_id >= animset_anim_counter_get(_pst_animpointer->pst_animset))
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Stores ID */
  _pst_animpointer->u32_crt_anim_id = _u32_anim_id;

  /* Updates absolute timestamp */
  _pst_animpointer->u32_timestamp = timer_game_time_get();

  /* Updates flags */
  animpointer_flag_set(_pst_animpointer, ANIMPOINTER_KUL_ID_FLAG_HAS_CURRENT_ANIM, ANIMPOINTER_KUL_ID_FLAG_NONE);

  /* Computes animpointer */
  animpointer_compute(_pst_animpointer, _pst_animpointer->u32_timestamp);

  return EXIT_SUCCESS;    
}

/***************************************************************************
 animpointer_time_set
 AnimationPointer current Animation TimeStamp set accessor.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animpointer_time_set(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_timestamp)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  /* Stores relative timestamp */
  _pst_animpointer->u32_crt_anim_timestamp = _u32_timestamp;
  
  /* Updates absolute timestamp */
  _pst_animpointer->u32_timestamp = timer_game_time_get();

  /* Computes animpointer */
  animpointer_compute(_pst_animpointer, _pst_animpointer->u32_timestamp);

  return EXIT_SUCCESS;
}

/***************************************************************************
 animpointer_frequency_set
 AnimationPointer Frequency set accessor.

 returns: float frequency
 ***************************************************************************/
uint32 animpointer_frequency_set(animpointer_st_animpointer *_pst_animpointer, float _f_frequency)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);
  ASSERT(_f_frequency >= 0.0);

  /* Computes animpointer */
  animpointer_compute(_pst_animpointer, timer_game_time_get());

  /* Stores frequency */
  _pst_animpointer->f_frequency = _f_frequency; 

  return EXIT_SUCCESS;
}

/***************************************************************************
 animpointer_flag_test
 Animation set flag test accessor.

 returns: bool
 ***************************************************************************/
bool animpointer_flag_test(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_flag)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  if(_pst_animpointer->u32_id_flags & _u32_flag)
  {
    return TRUE;
  }

  return FALSE;
}

/***************************************************************************
 animpointer_flag_set
 Animation set flag get/set accessor.

 returns: void
 ***************************************************************************/
void animpointer_flag_set(animpointer_st_animpointer *_pst_animpointer, uint32 _u32_add_flags, uint32 _u32_remove_flags)
{
  /* Checks */
  ASSERT(animpointer_su32_flags & ANIMPOINTER_KUL_FLAG_READY);
  ASSERT(_pst_animpointer != NULL);

  _pst_animpointer->u32_id_flags &= ~_u32_remove_flags;
  _pst_animpointer->u32_id_flags |= _u32_add_flags;

  return;
}

/***************************************************************************
 animset.c
 Animation Set module
 
 begin                : 13/02/2004
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


#include "anim/animset.h"

#include "debug/debug.h"


/*
 * Platform independant defines
 */

/* Global Flags */
#define ANIMSET_KUL_FLAG_NONE             0x00000000
#define ANIMSET_KUL_FLAG_READY            0x00000001
#define ANIMSET_KUL_FLAG_DEFAULT          0x00000000


/* Animset ID flags */
#define ANIMSET_KUL_ID_FLAG_NONE          0x00000000

#define ANIMSET_KUL_ID_MASK_SIZE          0x000000FF
#define ANIMSET_KUL_ID_MASK_COUNTER       0x0000FF00
#define ANIMSET_KUL_ID_MASK_FLAGS         0xFFFF0000

#define ANIMSET_KI_ID_SHIFT_SIZE          0
#define ANIMSET_KI_ID_SHIFT_COUNTER       8

/* Link table link flags */
#define ANIMSET_KUL_LINK_FLAG_NONE        0x00000000

#define ANIMSET_KUL_LINK_FLAG_PATH        0x01000000
#define ANIMSET_KUL_LINK_FLAG_LINK        0x02000000

#define ANIMSET_KUL_LINK_MASK_ANIM        0x000000FF
#define ANIMSET_KUL_LINK_MASK_LENGTH      0x0000FF00
#define ANIMSET_KUL_LINK_MASK_PRIORITY    0x000F0000
#define ANIMSET_KUL_LINK_MASK_FLAGS       0xFF000000

#define ANIMSET_KI_LINK_SHIFT_ANIM        0
#define ANIMSET_KI_LINK_SHIFT_LENGTH      8
#define ANIMSET_KI_LINK_SHIFT_PRIORITY    16

#define ANIMSET_KUL_LINK_DEFAULT_NONE     0x00000000

#define ANIMSET_KUL_LINK_DEFAULT_PRIORITY 0x00000008

/* Link table (status) flags */
#define ANIMSET_KUL_LINK_TABLE_FLAG_READY 0x01000000
#define ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY 0x02000000

#define ANIMSET_KUL_LINK_TABLE_MASK_FLAGS 0xFFFF0000


/*
 * Internal Link Update Info structure
 */
typedef struct st_link_update_info_t
{
  /* Link Table : 4 */
  animset_st_link_table *pst_link_table;

  /* Link update info : 8 */
  uint8 *auc_link_info;

  /* Byte number per animation : 12 */
  uint32 u32_byte_number;

  /* 4 extra bytes of padding : 16 */
  uint8 auc_unused[4];
} animset_st_link_update_info;

/*
 * Internal Animation Set Link table structure
 */
struct st_link_table_t
{
  /* Link array : 4 */
  uint32 *au32_link_array;

  /* Loop array : 8 */
  uint8 *auc_loop_array;

  /* Link Counter : 10 */
  uint16 uw_counter;

  /* Table size : 12 */
  uint16 uw_size;

  /* Flags : 16 */
  uint32 u32_flags;
};

/*
 * Animation Set structure
 */
struct st_animset_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Id flags : 20 */
  uint32 u32_id_flags;

  /* Used Animation pointer array : 24 */
  anim_st_anim **past_anim;

  /* Link table pointer : 28 */
  animset_st_link_table *pst_link_table;

  /* 4 extra bytes of padding : 32 */
  uint8 auc_unused[4];
};



/*
 * Static members
 */
static uint32 animset_su32_flags = ANIMSET_KUL_FLAG_DEFAULT;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 animset_link_table_flag_test
 Link table set flag test accessor.

 returns: bool
 ***************************************************************************/
bool animset_link_table_flag_test(animset_st_link_table *_pst_link_table, uint32 _u32_flag)
{
  /* Checks */
  ASSERT(_pst_link_table != NULL);

  if(_pst_link_table->u32_flags & _u32_flag)
  {
    return TRUE;
  }

  return FALSE;
}

/***************************************************************************
 animset_link_table_flag_set
 Link table set flag get/set accessor.

 returns: void
 ***************************************************************************/
void animset_link_table_flag_set(animset_st_link_table *_pst_link_table, uint32 _u32_add_flags, uint32 _u32_remove_flags)
{
  /* Checks */
  ASSERT(_pst_link_table != NULL);

  _pst_link_table->u32_flags &= ~_u32_remove_flags;
  _pst_link_table->u32_flags |= _u32_add_flags;

  return;
}

/***************************************************************************
 animset_link_table_link_get
 Gets a link value from link table.

 returns: uint32 link value
 ***************************************************************************/
uint32 animset_link_table_link_get(animset_st_link_table *_pst_link_table, uint32 _u32_index)
{
  /* Checks */
  ASSERT(_pst_link_table != NULL);
  ASSERT (_u32_index < (uint32)(_pst_link_table->uw_size) * (uint32)(_pst_link_table->uw_size));

  return _pst_link_table->au32_link_array[_u32_index];
}

/***************************************************************************
 animset_link_table_link_set
 Gets a link value in a link table.

 returns: EXIT_SUCCES / EXIT_FAILURE
 ***************************************************************************/
uint32 animset_link_table_link_set(animset_st_link_table *_pst_link_table, uint32 _u32_index, uint32 _u32_link_value)
{
  /* Checks */
  ASSERT(_pst_link_table != NULL);
  ASSERT (_u32_index < (uint32)(_pst_link_table->uw_size) * (uint32)(_pst_link_table->uw_size));

  /* Sets link */
  _pst_link_table->au32_link_array[_u32_index] = _u32_link_value;

  return EXIT_SUCCESS;
}

/***************************************************************************
 animset_link_table_property_set
 Sets a link property at the given value.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animset_link_table_property_set(animset_st_link_table *_pst_link_table, uint32 _u32_link_id, uint32 _u32_property, uint32 _u32_value)
{
  /* Checks */
  ASSERT(_pst_link_table != NULL);
  ASSERT(_u32_link_id < (uint32)(_pst_link_table->uw_counter * _pst_link_table->uw_counter));

  /* No link? */
  if(!(_pst_link_table->au32_link_array[_u32_link_id] & ANIMSET_KUL_LINK_FLAG_LINK))
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Depends on property */
  switch(_u32_property & ANIMSET_KUL_LINK_MASK_FLAGS)
  {
    /* Loop Counter */
    case ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER:
      _pst_link_table->auc_loop_array[_u32_link_id] = (uint8)_u32_value;
      if(_u32_value != 0)
      {
        _pst_link_table->au32_link_array[_u32_link_id] |= ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER;
      }
      else
      {
        _pst_link_table->au32_link_array[_u32_link_id] &= ~ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER;
      }
      break;

    /* Priority */
    case ANIMSET_KUL_LINK_FLAG_PRIORITY:
      if(_u32_value != 0)
      {
        _pst_link_table->au32_link_array[_u32_link_id] |= ANIMSET_KUL_LINK_FLAG_PRIORITY
                                                 + ((_u32_value << ANIMSET_KI_LINK_SHIFT_PRIORITY) & ANIMSET_KUL_LINK_MASK_PRIORITY);
      }
      else
      {
        _pst_link_table->au32_link_array[_u32_link_id] &= ~(ANIMSET_KUL_LINK_FLAG_PRIORITY | ANIMSET_KUL_LINK_MASK_PRIORITY);
      }
      break;

    /* Invalid */
    default :
      /* !!! MSG !!! */

      return EXIT_FAILURE;
  }

  /* Animset has to be computed again */
  animset_link_table_flag_set(_pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);

  /* Done */
  return EXIT_SUCCESS;
}

/***************************************************************************
 animset_link_table_property_get
 Gets a link property.

 returns: uint32 Property value / KUL_UNDEFINED
 ***************************************************************************/
uint32 animset_link_table_property_get(animset_st_link_table *_pst_link_table, uint32 _u32_link_id, uint32 _u32_property)
{
  uint32 u32_value;

  /* Checks */
  ASSERT(_pst_link_table != NULL);
  ASSERT(_u32_link_id < (uint32)(_pst_link_table->uw_size * _pst_link_table->uw_size));

  /* No link? */
  if(!(_pst_link_table->au32_link_array[_u32_link_id] & ANIMSET_KUL_LINK_FLAG_LINK))
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }

  /* Depends on property */
  switch(_u32_property & ANIMSET_KUL_LINK_MASK_FLAGS)
  {
    /* Loop Counter */
    case ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER:
      /* Has a counter? */
      if(_pst_link_table->au32_link_array[_u32_link_id] & ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER)
      {
        u32_value = (uint32)(_pst_link_table->auc_loop_array[_u32_link_id]);
      }
      else
      {
        /* !!! MSG !!! */

        u32_value = KUL_UNDEFINED;
      }
      break;

    /* Priority */
    case ANIMSET_KUL_LINK_FLAG_PRIORITY:
    /* Has priority? */
    if(_pst_link_table->au32_link_array[_u32_link_id] & ANIMSET_KUL_LINK_FLAG_PRIORITY)
    {
      u32_value = (_pst_link_table->au32_link_array[_u32_link_id] & ANIMSET_KUL_LINK_MASK_PRIORITY) >> ANIMSET_KI_LINK_SHIFT_PRIORITY;
    }
    else
    {
      /* !!! MSG !!! */

      u32_value = (uint32)ANIMSET_KUL_LINK_DEFAULT_PRIORITY;
    }
      break;

    /* Invalid */
    default :
      /* !!! MSG !!! */

      return KUL_UNDEFINED;
  }

  /* Done */
  return u32_value;
}

/***************************************************************************
 animset_link_info_set
 Sets a link info.

 returns: Nothing
 ***************************************************************************/
inline void animset_link_info_set(animset_st_link_update_info *_pst_info, uint32 _u32_src_index, uint32 _u32_dst_index)
{
  uint32 u32_index;
  uint8 uc_mask;

  /* Checks */
  ASSERT(_pst_info != NULL);
  ASSERT(_u32_src_index < (uint32)(_pst_info->pst_link_table->uw_size));
  ASSERT(_u32_dst_index < (uint32)(_pst_info->pst_link_table->uw_size));

  /* Computes real index */
  u32_index = (_u32_src_index * _pst_info->u32_byte_number) + (_u32_dst_index >> 3);

  /* Computes mask */
  uc_mask = 0x01 << (0x07 - (_u32_dst_index & 0x00000007));

  /* Sets info */
  _pst_info->auc_link_info[u32_index] |= uc_mask;

  return;
}

/***************************************************************************
 animset_link_info_reset
 Resets a link info.

 returns: Nothing
 ***************************************************************************/
inline void animset_link_info_reset(animset_st_link_update_info *_pst_info, uint32 _u32_src_index, uint32 _u32_dst_index)
{
  uint32 u32_index;
  uint8 uc_mask;

  /* Checks */
  ASSERT(_pst_info != NULL);
  ASSERT(_u32_src_index < (uint32)(_pst_info->pst_link_table->uw_size));
  ASSERT(_u32_dst_index < (uint32)(_pst_info->pst_link_table->uw_size));

  /* Computes real index */
  u32_index = (_u32_src_index * _pst_info->u32_byte_number) + (_u32_dst_index >> 3);

  /* Computes mask */
  uc_mask = 0x01 << (0x07 - (_u32_dst_index & 0x00000007));

  /* Resets info */
  _pst_info->auc_link_info[u32_index] &= ~uc_mask;

  return;
}

/***************************************************************************
 animset_link_info_clean
 Cleans a link info.

 returns: Nothing
 ***************************************************************************/
inline void animset_link_info_clean(animset_st_link_update_info *_pst_info, uint32 _u32_src_index)
{
  uint32 u32_index, i;

  /* Checks */
  ASSERT(_pst_info != NULL);
  ASSERT(_u32_src_index < (uint32)(_pst_info->pst_link_table->uw_size));

  /* Computes real index */
  u32_index = _u32_src_index * _pst_info->u32_byte_number;

  /* Cleans info */
  for(i = u32_index; i < u32_index + _pst_info->u32_byte_number; i++)
  {
    _pst_info->auc_link_info[i] = 0x00;
  }

  return;
}

/***************************************************************************
 animset_link_info_get
 Gets a link info.

 returns: TRUE on success / FALSE on failure
 ***************************************************************************/
inline bool animset_link_info_get(animset_st_link_update_info *_pst_info, uint32 _u32_src_index, uint32 _u32_dst_index)
{
  uint32 u32_index;
  uint8 uc_mask;

  /* Checks */
  ASSERT(_pst_info != NULL);
  ASSERT(_u32_src_index < (uint32)(_pst_info->pst_link_table->uw_size));
  ASSERT(_u32_dst_index < (uint32)(_pst_info->pst_link_table->uw_size));

  /* Computes real index */
  u32_index = (_u32_src_index * _pst_info->u32_byte_number) + (_u32_dst_index >> 3);

  /* Computes mask */
  uc_mask = 0x01 << (0x07 - (_u32_dst_index & 0x00000007));

  /* Returns info */
  return((_pst_info->auc_link_info[u32_index] & uc_mask) ? TRUE : FALSE);
}

/***************************************************************************
 animset_link_info_update
 Updates links information from one animation to another.

 returns: TRUE if there are changes / FALSE otherwise
 ***************************************************************************/
inline bool animset_link_info_update(animset_st_link_update_info *_pst_info, uint32 _u32_src_index, uint32 _u32_dst_index)
{
  bool bChange = FALSE;
  uint32 u32_src_link, u32_dst_link, u32_src_length, u32_dst_length;
  uint32 u32_direct_link, u32_direct_priority, u32_priority;
  uint32 u32_src_base_index, u32_dst_base_index, i;
  animset_st_link_table *pst_link_table;

  /* Checks */
  ASSERT(_pst_info != NULL);
  ASSERT(_u32_src_index < (uint32)(_pst_info->pst_link_table->uw_size));
  ASSERT(_u32_dst_index < (uint32)(_pst_info->pst_link_table->uw_size));

  /* Gets link table */
  pst_link_table = _pst_info->pst_link_table;

  /* Computes base indexes */
  u32_src_base_index = _u32_src_index * (uint32)(pst_link_table->uw_size);
  u32_dst_base_index = _u32_dst_index * (uint32)(pst_link_table->uw_size);

  /* Gets direct link */
  u32_direct_link = pst_link_table->au32_link_array[u32_dst_base_index + _u32_src_index];

  /* Checks link */
  ASSERT(u32_direct_link & ANIMSET_KUL_LINK_FLAG_LINK);

  /* Gets direct link priority */
  if(u32_direct_link & ANIMSET_KUL_LINK_FLAG_PRIORITY)
  {
    u32_direct_priority = (u32_direct_link & ANIMSET_KUL_LINK_MASK_PRIORITY) >> ANIMSET_KI_LINK_SHIFT_PRIORITY;
  }
  else
  {
    u32_direct_priority = (uint32)ANIMSET_KUL_LINK_DEFAULT_PRIORITY;
  }

  /* Compares all paths */
  for(i = 0; i < (uint32)(pst_link_table->uw_counter); i++)
  {
    /* Gets both values */
    u32_src_link = pst_link_table->au32_link_array[u32_src_base_index + i];
    u32_dst_link = pst_link_table->au32_link_array[u32_dst_base_index + i];

    /* Is not a direct link? */
    if(!(u32_dst_link & ANIMSET_KUL_LINK_FLAG_LINK))
    {
      /* New path found? */
      if(u32_src_link & ANIMSET_KUL_LINK_FLAG_PATH)
      {
        /* Gets both lengths */
        u32_src_length = (u32_src_link & ANIMSET_KUL_LINK_MASK_LENGTH) >> ANIMSET_KI_LINK_SHIFT_LENGTH;
        u32_dst_length = (u32_dst_link & ANIMSET_KUL_LINK_MASK_LENGTH) >> ANIMSET_KI_LINK_SHIFT_LENGTH;

        /* Has already a path? */
        if(u32_dst_link & ANIMSET_KUL_LINK_FLAG_PATH)
        {
          /* Computes old path priority */
          if(u32_dst_link & ANIMSET_KUL_LINK_FLAG_PRIORITY)
          {
            u32_priority = (u32_dst_link & ANIMSET_KUL_LINK_MASK_PRIORITY) >> ANIMSET_KI_LINK_SHIFT_PRIORITY;
          }
          else
          {
            u32_priority = (uint32)ANIMSET_KUL_LINK_DEFAULT_PRIORITY;
          }

          /* Lowest priority found? */
          if(u32_priority > u32_direct_priority)
          {
            /* Next path comparison */
            continue;
          }
          /* Same priority */
          else if(u32_priority == u32_direct_priority)
          {
            /* No shortest path found? */
            if(u32_dst_length <= u32_src_length + 1)
            {
              /* Next path comparison */
              continue;
            }
          }
        }

        /* New found path has higher priority or shortest length */

        /* Computes new path */
        u32_dst_link = ANIMSET_KUL_LINK_FLAG_PATH
          + ((u32_src_length + 1) << ANIMSET_KI_LINK_SHIFT_LENGTH)
          + (_u32_src_index << ANIMSET_KI_LINK_SHIFT_ANIM);

        /* Stores it */
        pst_link_table->au32_link_array[u32_dst_base_index + i] = u32_dst_link;

        /* Changes have been made */
        bChange = TRUE;
      }
    }
  }

  return bChange;
}

/***************************************************************************
 animset_link_update
 Updates link for the given animation.

 returns: Nothing
 ***************************************************************************/
void animset_link_update(uint32 _u32_index, animset_st_link_update_info *_pst_info)
{
  uint32 u32_base_index;
  animset_st_link_table *pst_link_table;
  uint16 i;

  /* Checks */
  ASSERT(_pst_info != NULL);
  ASSERT(_u32_index < (uint32)(_pst_info->pst_link_table->uw_size));

  /* Are links already computed/updated for this animations */
  if(animset_link_info_get(_pst_info, _u32_index, _u32_index) != FALSE)
  {
    return;
  }

  /* Gets link table */
  pst_link_table = _pst_info->pst_link_table;

  /* Mark animation as read */
  animset_link_info_set(_pst_info, _u32_index, _u32_index);

  /* Computes base index */
  u32_base_index = _u32_index * (uint32)(pst_link_table->uw_size);

  /* For all found links */
  for(i = 0; i < (uint32)(pst_link_table->uw_counter); i++)
  {
    /* Link found? */
    if(pst_link_table->au32_link_array[u32_base_index + i] & ANIMSET_KUL_LINK_FLAG_LINK)
    {
      /* Is animation 'dirty' for this one? */
      if(animset_link_info_get(_pst_info, i, _u32_index) == FALSE)
      {
        /* Updates this animation */
        animset_link_update(i, _pst_info);

        /* Updates 'dirty' status */
        animset_link_info_set(_pst_info, i, _u32_index);

        /* Updates info */
        if(animset_link_info_update(_pst_info, i, _u32_index) != FALSE)
        {
          /* Links have been modified for current animation */
          animset_link_info_clean(_pst_info, _u32_index);
          animset_link_info_set(_pst_info, _u32_index, _u32_index);
        }
      }
    }
  }

  /* Unmark animation as read */
  animset_link_info_reset(_pst_info, _u32_index, _u32_index);

  return;
}

/***************************************************************************
 animset_link_update_info_create
 Creates and init the Link Update Info.

 returns: animset_st_link_update_info pointer / NULL on failure
 ***************************************************************************/
inline animset_st_link_update_info *animset_link_update_info_create(animset_st_link_table *_pst_link_table)
{
  animset_st_link_update_info *pst_info;
  uint32 u32_number;
  uint32 i;

  /* Checks */
  ASSERT(_pst_link_table != NULL);

  /* Allocates memory */
  pst_info = (animset_st_link_update_info *)malloc(sizeof(animset_st_link_update_info));

  /* Not allocated? */
  if(pst_info == NULL)
  {
    /* !!! MSG !!! */

    return NULL;
  }

  /* Stores link table */
  pst_info->pst_link_table = _pst_link_table;

  /* Computes number of uint8 needed for one link info : ((size - 1) / 8) + 1 */
  u32_number = (uint32)(((_pst_link_table->uw_counter - 1) >> 3) + 1);

  /* Stores it */
  pst_info->u32_byte_number = u32_number;

  /* Allocates link info array */
  pst_info->auc_link_info = (uint8 *)malloc(u32_number * (uint32)(_pst_link_table->uw_counter) * sizeof(uint8));

  /* Not allocated? */
  if(pst_info->auc_link_info == NULL)
  {
    /* !!! MSG !!! */

    /* Frees previously allocated memory */
    free(pst_info);

    return NULL;
  }

  /* Cleans all */
  for(i = 0; i < (uint32)(_pst_link_table->uw_counter) * u32_number; i++)
  {
    pst_info->auc_link_info[i] = 0;
  }

  /* Done! */
  return pst_info;
}

/***************************************************************************
 animset_link_update_info_delete
 Deletes the Link Update Info.

 returns: Nothing
 ***************************************************************************/
inline void animset_link_update_info_delete(animset_st_link_update_info *_pst_link_update_info)
{
  /* Checks */
  ASSERT(_pst_link_update_info != NULL);

  /* Frees all */
  free(_pst_link_update_info->auc_link_info);
  free(_pst_link_update_info);

  return;
}

/***************************************************************************
 animset_link_table_clean
 Cleans a Link Table for a given animation.

 returns: void
 ***************************************************************************/
inline void animset_link_table_clean(animset_st_link_table *_pst_link_table, uint32 _u32_anim_id)
{
  uint32 u32_size, u32_anim_base_index, i;

  /* Checks */
  ASSERT(_pst_link_table != NULL);

  /* Gets storage size */
  u32_size = (uint32)(_pst_link_table->uw_size);

  /* Checks anim index */
  ASSERT(_u32_anim_id < u32_size);

  /* Gets base index */
  u32_anim_base_index = _u32_anim_id * u32_size;

  /* Cleans paths coming from this animation */
  for(i = u32_anim_base_index; i < u32_anim_base_index + u32_size; i++)
  {
    _pst_link_table->au32_link_array[i] = ANIMSET_KUL_LINK_DEFAULT_NONE;
    _pst_link_table->auc_loop_array[i] = 0;
  }

  /* Cleans paths going to this animation */
  for(i = _u32_anim_id; i < u32_size; i += u32_size)
  {
    _pst_link_table->au32_link_array[i] = ANIMSET_KUL_LINK_DEFAULT_NONE;
    _pst_link_table->auc_loop_array[i] = 0;
  }

  /* Updates flags */
  animset_link_table_flag_set(_pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);

  return;
}

/***************************************************************************
 animset_link_table_create
 Creates and init the Link Table.

 returns: animset_st_link_table *
 ***************************************************************************/
inline animset_st_link_table *animset_link_table_create(uint32 _u32_size)
{
  uint32 i;
  animset_st_link_table *pst_link_table;

  /* Checks */
  ASSERT(_u32_size < 0xFFFF);

  /* Allocates link table */
  pst_link_table = (animset_st_link_table *)malloc(sizeof(animset_st_link_table));

  /* Not allocated? */
  if(pst_link_table == NULL)
  {
    /* !!! MSG !!! */

    return NULL;
  }

  /* Creates link array */
  pst_link_table->au32_link_array = (uint32 *)malloc(_u32_size * _u32_size * sizeof(uint32));

  /* Not allocated? */
  if(pst_link_table->au32_link_array == NULL)
  {
    /* !!! MSG !!! */

    /* Frees link table */
    free(pst_link_table);

    return NULL;
  }

  /* Allocates loop table */
  pst_link_table->auc_loop_array = (uint8 *)malloc(_u32_size * _u32_size * sizeof(uint8));

  /* Not allocated? */
  if(pst_link_table->auc_loop_array == NULL)
  {
    /* !!! MSG !!! */

    /* Frees previously allocated memory */
    free(pst_link_table->au32_link_array);
    free(pst_link_table);

    return NULL;
  }

  /* Inits tables */
  for(i = 0; i < _u32_size * _u32_size; i++)
  {
    pst_link_table->au32_link_array[i] = ANIMSET_KUL_LINK_DEFAULT_NONE;
    pst_link_table->auc_loop_array[i] = 0;
  }

  /* Inits values */
  pst_link_table->uw_size = (uint16)_u32_size;
  pst_link_table->uw_counter = 0;

  /* Inits flags */
  animset_link_table_flag_set(pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_READY | ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, ANIMSET_KUL_LINK_TABLE_MASK_FLAGS);

  /* Done! */
  return pst_link_table;
}

/***************************************************************************
 animset_link_table_copy
 Copies an Animset link table info into another one.

 returns: Nothing
 ***************************************************************************/
inline void animset_link_table_copy(animset_st_link_table *_pst_src_link_table, animset_st_link_table *_pst_dst_link_table)
{
  uint32 i;

  /* Checks */
  ASSERT(_pst_src_link_table != NULL);
  ASSERT(_pst_dst_link_table != NULL);

  /* Copies all */
  _pst_dst_link_table->u32_flags = _pst_src_link_table->u32_flags;
  _pst_dst_link_table->uw_counter = _pst_src_link_table->uw_counter;
  _pst_dst_link_table->uw_size = _pst_src_link_table->uw_size;

  for(i = 0; i < (uint32)(_pst_src_link_table->uw_size) * (uint32)(_pst_src_link_table->uw_size); i++)
  {
    _pst_dst_link_table->au32_link_array[i] = _pst_src_link_table->au32_link_array[i];
    _pst_dst_link_table->auc_loop_array[i] = _pst_src_link_table->auc_loop_array[i];
  }

  return;
}

/***************************************************************************
 animset_anim_storage_size_set
 Sets an animation set anim storage size.

 returns: void
 ***************************************************************************/
inline void animset_anim_storage_size_set(animset_st_animset *_pst_animset, uint32 _u32_size)
{
  /* Checks */
  ASSERT(_pst_animset != NULL);
  ASSERT(_u32_size <= ANIMSET_KI_ANIM_MAX_NUMBER);

  /* Updates storage size */
  _pst_animset->u32_id_flags &= ~ANIMSET_KUL_ID_MASK_SIZE;
  _pst_animset->u32_id_flags |= _u32_size << ANIMSET_KI_ID_SHIFT_SIZE;

  return;
}  

/***************************************************************************
 animset_anim_counter_set
 Sets an animation set internal anim counter.

 returns: void
 ***************************************************************************/
inline void animset_anim_counter_set(animset_st_animset *_pst_animset, uint32 _u32_anim_counter)
{
  /* Checks */
  ASSERT(_u32_anim_counter <= animset_anim_storage_size_get(_pst_animset));

  /* Updates counter */
  _pst_animset->u32_id_flags &= ~ANIMSET_KUL_ID_MASK_COUNTER;
  _pst_animset->u32_id_flags |= _u32_anim_counter << ANIMSET_KI_ID_SHIFT_COUNTER;

  return;
}

/***************************************************************************
 animset_anim_counter_increase
 Increases an animation set internal anim counter.

 returns: void
 ***************************************************************************/
inline void animset_anim_counter_increase(animset_st_animset *_pst_animset)
{
  uint32 u32_anim_counter;

  /* Checks */
  ASSERT(_pst_animset != NULL);

  /* Gets anim counter */
  u32_anim_counter = animset_anim_counter_get(_pst_animset);

  /* Updates anim counter*/
  animset_anim_counter_set(_pst_animset, u32_anim_counter + 1);

  return;
}  

/***************************************************************************
 animset_anim_counter_decrease
 Decreases an animation set internal anim counter.

 returns: void
 ***************************************************************************/
inline void animset_anim_counter_decrease(animset_st_animset *_pst_animset)
{
  uint32 u32_anim_counter;

  /* Checks */
  ASSERT(_pst_animset != NULL);

  /* Gets anim counter */
  u32_anim_counter = animset_anim_counter_get(_pst_animset);

  /* Updates anim counter*/
  animset_anim_counter_set(_pst_animset, u32_anim_counter - 1);

  return;
}  

/***************************************************************************
 animset_list_delete
 Deletes all Animation Sets.

 returns: void
 ***************************************************************************/
inline void animset_list_delete()
{
  animset_st_animset *pst_animset = (animset_st_animset *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIMSET);

  /* Non empty? */
  while(pst_animset != NULL)
  {
    /* Deletes Animation Set */
    animset_delete(pst_animset);

    /* Gets first Animation Set */
    pst_animset = (animset_st_animset *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIMSET);
  }

  return;
}

/***************************************************************************
 animset_anim_next_compute
 Gets next animation, updating link status.

 returns: uint32 animation index / KUL_UNDEFINED if none found
 ***************************************************************************/
inline uint32 animset_anim_next_compute(animset_st_link_table *_pst_link_table, uint32 _u32_anim_id)
{
  uint32 u32_size, u32_base_index;
  uint32 u32_link_priority, u32_res_link_priority, u32_loop;
  uint32 u32_res_anim = KUL_UNDEFINED, u32_res_link, u32_link;
  uint32 i;

  /* Checks */
  ASSERT(_pst_link_table != NULL);

  /* Gets animation storage size */
  u32_size = (uint32)(_pst_link_table->uw_size);

  /* Gets animation base index */
  u32_base_index = _u32_anim_id * u32_size;

  /* Inits anim & link value */
  u32_res_anim = KUL_UNDEFINED;
  u32_res_link = ANIMSET_KUL_LINK_DEFAULT_NONE;
  u32_res_link_priority = 0;

  /* Search for all links */
  for(i = u32_base_index; i < u32_base_index + u32_size; i++)
  {
    /* Gets value */
    u32_link = _pst_link_table->au32_link_array[i];

    /* Link found? */
    if(u32_link & ANIMSET_KUL_LINK_FLAG_LINK)
    {
      /* Gets link loop counter */
      u32_loop = animset_link_table_property_get(_pst_link_table, i, ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER);

      /* Has an empty loop counter (if no loop, value is KUL_UNDEFINED)? */
      if(u32_loop == 0)
      {
        /* Checks next link */
        continue;
      }

      /* Gets path priority */
      u32_link_priority = animset_link_table_property_get(_pst_link_table, i, ANIMSET_KUL_LINK_FLAG_PRIORITY);

      /* Is priority lower or equal than previous one? */
      if(u32_link_priority <= u32_res_link_priority)
      {
        /* Checks next link */
        continue;
      }

      /* Stores new link info */
      u32_res_anim = i;
      u32_res_link = u32_link;
      u32_res_link_priority = u32_link_priority;
    }
  }

  /* Link found? */
  if(u32_res_anim != KUL_UNDEFINED)
  {
    /* Gets current loop counter */
    u32_loop = animset_link_table_property_get(_pst_link_table, u32_res_anim, ANIMSET_KUL_LINK_FLAG_PRIORITY);

    /* Updates loop counter if used */
    if(u32_loop != KUL_UNDEFINED)
    {
      animset_link_table_property_set(_pst_link_table, u32_res_anim, ANIMSET_KUL_LINK_FLAG_PRIORITY, u32_loop - 1);

      /* Is link table dirty again? */
      if(u32_loop <= 1)
      {
        /* Updates flags */
        animset_link_table_flag_set(_pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);
      }
    }

    /* Gets real anim index */
    u32_res_anim = u32_res_anim - u32_base_index;
  }

  /* Done! */
  return u32_res_anim;
}

/***************************************************************************
 animset_anim_next_using_dst_compute
 Gets next animation using destination one, updating link status.

 returns: uint32 animation index / KUL_UNDEFINED if none found
 ***************************************************************************/
inline uint32 animset_anim_next_using_dst_compute(animset_st_link_table *_pst_link_table, uint32 _u32_src_anim_id, uint32 _u32_dst_anim_id)
{
  uint32 u32_base_index, u32_size, u32_loop;
  uint32 u32_anim = KUL_UNDEFINED, u32_link, u32_link_id;

  /* Checks */
  ASSERT(_pst_link_table != NULL);

  /* Gets animation storage size */
  u32_size = (uint32)(_pst_link_table->uw_size);

  /* Gets animation base index */
  u32_base_index = _u32_src_anim_id * u32_size;

  /* Gets link value */
  u32_link = _pst_link_table->au32_link_array[u32_base_index + _u32_dst_anim_id];

  /* Is there a path? */
  if(u32_link & ANIMSET_KUL_LINK_FLAG_PATH)
  {
    /* Gets anim index */
    u32_anim = (u32_link & ANIMSET_KUL_LINK_MASK_ANIM) >> ANIMSET_KI_LINK_SHIFT_ANIM;

    /* Gets direct link id */
    u32_link_id = u32_base_index + u32_anim;

    /* Gets direct link */
    u32_link = _pst_link_table->au32_link_array[u32_link_id];

    /* Gets current loop counter */
    u32_loop = animset_link_table_property_get(_pst_link_table, u32_link_id, ANIMSET_KUL_LINK_FLAG_PRIORITY);

    /* Updates loop counter if used */
    if(u32_loop != KUL_UNDEFINED)
    {
      animset_link_table_property_set(_pst_link_table, u32_link_id, ANIMSET_KUL_LINK_FLAG_PRIORITY, u32_loop - 1);

      /* Is link table dirty again? */
      if(u32_loop <= 1)
      {
        /* Updates flags */
        animset_link_table_flag_set(_pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);
      }
    }
  }

  /* Done! */
  return u32_anim;
}

/***************************************************************************
 animset_link_table_compute
 Computes all link relations.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animset_link_table_compute(animset_st_link_table *_pst_link_table)
{
  animset_st_link_update_info *pst_update_info;
  uint32 u32_counter, u32_size;
  uint32 i;

  /* Checks */
  ASSERT(_pst_link_table != NULL);

  /* Gets anim counter */
  u32_counter = (uint32)(_pst_link_table->uw_counter);

  /* Checks it */
  ASSERT(u32_counter > 0);

  /* Link table not dirty? */
  if(animset_link_table_flag_test(_pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY) == FALSE)
  {
    return EXIT_SUCCESS;
  }

  /* No ready link structure */
  if(animset_link_table_flag_test(_pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_READY) == FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Gets storage size */
  u32_size = (uint32)_pst_link_table->uw_size;

  /* Cleans link table */
  for(i = 0; i < u32_size * u32_size; i++)
  {
    /* No link found? */
    if(!(_pst_link_table->au32_link_array[i] & ANIMSET_KUL_LINK_FLAG_LINK))
    {
      _pst_link_table->au32_link_array[i] = ANIMSET_KUL_LINK_DEFAULT_NONE;
    }
  }

  /* Creates a link update info */
  pst_update_info = animset_link_update_info_create(_pst_link_table);

  /* Not allocated? */
  if(pst_update_info == NULL)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Updates all animations */
  for(i = 0; i < u32_counter; i++)
  {
    animset_link_update(i, pst_update_info);
  }

  /* Updates flags */
  animset_link_table_flag_set(_pst_link_table, 0, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY);

  /* Deletes link update info */
  animset_link_update_info_delete(pst_update_info);

  return EXIT_SUCCESS;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 animset_init
 Inits Animation Set system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 animset_init()
{
  /* Already Initialized? */
  if(animset_su32_flags & ANIMSET_KUL_FLAG_READY)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Inits Anim before */
  anim_init();

  /* Inits Flags */
  animset_su32_flags = ANIMSET_KUL_FLAG_READY;

  return EXIT_SUCCESS;
}

/***************************************************************************
 animset_exit
 Exits from the Animation Set system.

 returns: void
 ***************************************************************************/
void animset_exit()
{
  /* Initialized? */
  if(animset_su32_flags & ANIMSET_KUL_FLAG_READY)
  {
    /* Deletes anim list */
    animset_list_delete();

    /* Updates flags */
    animset_su32_flags &= ~ANIMSET_KUL_FLAG_READY;

    /* Exit from Anim after */
    anim_exit();
  }

  return;
}

/***************************************************************************
 animset_create
 Creates an empty Animation Set, given a storage size (<= ANIMSET_KI_TEXTURE_MAX_NUMBER).

 returns: Created animset.
 ***************************************************************************/
animset_st_animset *animset_create(uint32 _u32_size)
{
  animset_st_animset *pst_animset;
  uint32 i;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_u32_size <= ANIMSET_KI_ANIM_MAX_NUMBER);

  /* Creates animset */
  pst_animset = (animset_st_animset *)malloc(sizeof(animset_st_animset));

  /* Non null? */
  if(pst_animset != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_animset, STRUCTURE_KUL_STRUCT_ID_ANIMSET) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_animset);

      /* Returns nothing */
      return NULL;
    }

    /* Allocates anim pointer array */
    pst_animset->past_anim = (anim_st_anim **)malloc(_u32_size * sizeof(anim_st_anim *));

    if(pst_animset->past_anim == NULL)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_animset);

      /* Returns nothing */
      return NULL;
    }

    /* Set storage size & counter */
    animset_anim_storage_size_set(pst_animset, _u32_size);
    animset_anim_counter_set(pst_animset, 0);

    /* Creates link table */
    pst_animset->pst_link_table = animset_link_table_create(_u32_size);

    /* Not allocated? */
    if(pst_animset->pst_link_table == NULL)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_animset->past_anim);
      free(pst_animset);

      /* Returns nothing */
      return NULL;
    }

    /* Updates flags */
    animset_link_table_flag_set(pst_animset->pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_READY | ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);

    /* Cleans structure pointers */
    for(i = 0; i < _u32_size; i++)
    {
      pst_animset->past_anim[i] = NULL;
    }

    /* Inits flags */
    animset_flag_set(pst_animset, ANIMSET_KUL_ID_FLAG_LINK_STATIC, ANIMSET_KUL_ID_MASK_FLAGS);
  }

  return pst_animset;
}

/***************************************************************************
 animset_delete
 Deletes an Animation Set.

 returns: void
 ***************************************************************************/
void animset_delete(animset_st_animset *_pst_animset)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Cleans members */
  animset_anim_clean(_pst_animset);
  animset_link_table_delete(_pst_animset->pst_link_table);

  /* Cleans structure */
  structure_struct_clean((structure_st_struct *)_pst_animset);

  /* Frees animset memory */
  free(_pst_animset);

  return;
}

/***************************************************************************
 animset_reference_add
 Adds a reference on an AnimationSet.

 returns: Nothing
 ***************************************************************************/
void animset_reference_add(animset_st_animset *_pst_animset)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Locks animset */
  animset_flag_set(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK, 0);

  /* Updates reference counter */
    structure_struct_counter_increase((structure_st_struct *)_pst_animset);

  return;
}

/***************************************************************************
 animset_reference_add
 Removes a reference from an AnimationSet.

 returns: Nothing
 ***************************************************************************/
void animset_reference_remove(animset_st_animset *_pst_animset)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Updates reference counter */
    structure_struct_counter_decrease((structure_st_struct *)_pst_animset);

  /* No reference left? */
  if(structure_struct_counter_get((structure_st_struct *)_pst_animset) == 0)
  {
    /* Unlocks animset */
    animset_flag_set(_pst_animset, 0, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK);
  }

  return;
}



/***************************************************************************
 animset_anim_add
 Adds an Animation to an Animation Set.

 returns: Added Animation ID / KUL_UNDEFINED
 ***************************************************************************/
uint32 animset_anim_add(animset_st_animset *_pst_animset, anim_st_anim *_pst_anim)
{
  uint32 u32_counter, u32_size, u32_index;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);
  ASSERT(_pst_anim != NULL);

  /* Gets storage size & counter */
  u32_size = animset_anim_storage_size_get(_pst_animset);
  u32_counter = animset_anim_counter_get(_pst_animset);

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }

  /* Is there free room? */
  if(u32_counter >= u32_size)
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }
  else
  {
    /* Finds the first empty slot */
    for(u32_index = 0; u32_index < u32_size; u32_index++)
    {
      if(_pst_animset->past_anim[u32_index] == NULL)
      {
        break;
      }
    }

    /* Find check */
    ASSERT(u32_index < u32_size);

    /* Adds the extra animation */
    _pst_animset->past_anim[u32_index] = _pst_anim;

    /* Updates Animation reference counter */
    structure_struct_counter_increase((structure_st_struct *)_pst_anim);

    /* Updates Animation counter */
    animset_anim_counter_increase(_pst_animset);
  }

  /* Done! */
  return u32_index; 
}

/***************************************************************************
 animset_anim_remove
 Removes an Animation from an Animation Set given its ID.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 animset_anim_remove(animset_st_animset *_pst_animset, uint32 _u32_anim_id)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* No Animation at the given ID? */
  if(_pst_animset->past_anim[_u32_anim_id] == NULL)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }
  else
  {
    /* Updates counter */
    animset_anim_counter_decrease(_pst_animset);

    /* Updates Animation reference counter */
    structure_struct_counter_decrease((structure_st_struct *)(_pst_animset->past_anim[_u32_anim_id]));

    /* Removes animation */
    _pst_animset->past_anim[_u32_anim_id] = NULL;

    /* Cleans link table for this animation */
    animset_link_table_clean(_pst_animset->pst_link_table, _u32_anim_id);
  }

  /* Done! */
  return EXIT_SUCCESS;
}

/***************************************************************************
 animset_anim_clean
 Cleans all referenced Animations from an Animation Set.

 returns: EXIT_SUCCESS on success / EXIT_FAILURE otherwise
 ***************************************************************************/
uint32 animset_anim_clean(animset_st_animset *_pst_animset)
{
  uint32 u32_counter, i;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Gets counter */
  u32_counter = animset_anim_counter_get(_pst_animset);

  /* Until there are no texture left */
  for(i = 0; i < u32_counter; i++)
  {
    animset_anim_remove(_pst_animset, i);
  }

  /* Done! */
  return EXIT_SUCCESS;
}

/***************************************************************************
 anim_anim_get
 Animation used by an Animation Set get accessor, given its index.

 returns: anim_st_anim *
 ***************************************************************************/
anim_st_anim *animset_anim_get(animset_st_animset *_pst_animset, uint32 _u32_id)
{
  uint32 u32_counter;
  anim_st_anim *pst_anim = NULL;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Gets counter */
  u32_counter = animset_anim_counter_get(_pst_animset);

  /* Is index valid? */
  if(_u32_id < u32_counter)
  {
    /* Gets Animation */
    pst_anim = _pst_animset->past_anim[_u32_id];
  }
  else
  {
    /* !!! MSG !!! */

  }

  return pst_anim;
}

/***************************************************************************
 animset_link_add
 Adds a link between 2 Animations.

 returns: uint32 Link ID / KUL_UNDEFINED
 ***************************************************************************/
uint32 animset_link_add(animset_st_animset *_pst_animset, uint32 _u32_anim_src, uint32 _u32_anim_dst)
{
  uint32 u32_index, u32_size, u32_link;
  animset_st_link_table *pst_link_table;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Gets storage size */
  u32_size = animset_anim_storage_size_get(_pst_animset);

  /* Checks anim index validity */
  ASSERT(_u32_anim_src < u32_size);
  ASSERT(_u32_anim_dst < u32_size);

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }

  /* Gets link table */
  pst_link_table = _pst_animset->pst_link_table;

  /* Computes link index */
  u32_index = (_u32_anim_src * u32_size) + _u32_anim_dst;

  /* Gets link */
  u32_link = animset_link_table_link_get(pst_link_table, u32_index);

  /* Is there already a link? */
  if(u32_index & ANIMSET_KUL_LINK_FLAG_LINK)
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }

  /* Adds link */
  u32_link = (ANIMSET_KUL_LINK_FLAG_LINK | ANIMSET_KUL_LINK_FLAG_PATH)
          + (0x00000001 << ANIMSET_KI_LINK_SHIFT_LENGTH)
          + (ANIMSET_KUL_LINK_DEFAULT_PRIORITY << ANIMSET_KI_LINK_SHIFT_PRIORITY)
          + (_u32_anim_dst << ANIMSET_KI_LINK_SHIFT_ANIM);

  /* Stores it */
  animset_link_table_link_set(pst_link_table, u32_index, u32_link);

  /* Updates counter */
  pst_link_table->uw_counter++;

  /* Animset has to be computed again */
  animset_link_table_flag_set(pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);

  return u32_index;
}

/***************************************************************************
 animset_link_remove
 Removes a link given its ID.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animset_link_remove(animset_st_animset *_pst_animset, uint32 _u32_link_id)
{
  uint32 u32_size;
  animset_st_link_table *pst_link_table;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Gets link table */
  pst_link_table = _pst_animset->pst_link_table;

  /* Gets storage size */
  u32_size = animset_anim_storage_size_get(_pst_animset);

  /* Checks link index validity */
  ASSERT(_u32_link_id < u32_size * u32_size);

  /* No link found? */
  if(!(animset_link_table_link_get(pst_link_table, _u32_link_id) & ANIMSET_KUL_LINK_FLAG_LINK))
  {
    /* !!! MSG !!! */
    return EXIT_FAILURE;
  }

  /* Updates link table */
  animset_link_table_link_set(pst_link_table, _u32_link_id, ANIMSET_KUL_LINK_DEFAULT_NONE);
  animset_link_table_property_set(pst_link_table, _u32_link_id, ANIMSET_KUL_LINK_FLAG_PRIORITY, 0x00000000);

  /* Animset has to be computed again */
  animset_link_table_flag_set(pst_link_table, ANIMSET_KUL_LINK_TABLE_FLAG_DIRTY, 0);

  /* Done! */
  return EXIT_SUCCESS;
}

/***************************************************************************
 animset_link_compute
 Computes all link relations.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animset_link_compute(animset_st_animset *_pst_animset)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Gets work done */
  return animset_link_table_compute(_pst_animset->pst_link_table);
}

/***************************************************************************
 animset_link_get
 Gets a direct link between two animations (if none, result is KUL_UNDEFINED).

 returns: uint32 Link ID / KUL_UNDEFINED
 ***************************************************************************/
uint32 animset_link_get(animset_st_animset *_pst_animset, uint32 _u32_anim_src, uint32 _u32_anim_dst)
{
  uint32 u32_index, u32_size;
  animset_st_link_table *pst_link_table;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Gets storage size */
  u32_size = animset_anim_storage_size_get(_pst_animset);

  /* Checks anim index validity */
  ASSERT(_u32_anim_src < u32_size);
  ASSERT(_u32_anim_dst < u32_size);

  /* Computes link index */
  u32_index = (_u32_anim_src * u32_size) + _u32_anim_dst;

  /* Gets link table */
  pst_link_table = _pst_animset->pst_link_table;

  /* Is there a link? */
  if(animset_link_table_link_get(pst_link_table, u32_index) & ANIMSET_KUL_LINK_FLAG_LINK)
  {
    return u32_index;
  }

  /* No Link */
  return KUL_UNDEFINED;
}

/***************************************************************************
 animset_link_property_set
 Sets a link property at the given value.

 returns: EXIT_SUCCESS / EXIT_FAILURE
 ***************************************************************************/
uint32 animset_link_property_set(animset_st_animset *_pst_animset, uint32 _u32_link_id, uint32 _u32_property, uint32 _u32_value)
{
  uint32 u32_result;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);
  ASSERT(_u32_link_id < animset_anim_storage_size_get(_pst_animset) * animset_anim_storage_size_get(_pst_animset));

  /* Locked? */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_REFERENCE_LOCK) != FALSE)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Gets work done */
  u32_result = animset_link_table_property_set(_pst_animset->pst_link_table, _u32_link_id, _u32_property, _u32_value);

  /* Changes occured? */
  if(u32_result != EXIT_FAILURE)
  {
    /* Added loop counter? */
    if(_u32_property == ANIMSET_KUL_LINK_FLAG_LOOP_COUNTER)
    {
      /* Link table should be locally stored by animation pointers now */
      animset_flag_set(_pst_animset, 0, ANIMSET_KUL_ID_FLAG_LINK_STATIC);
    }
  }

  return u32_result; 
}

/***************************************************************************
 animset_link_property_get
 Gets a link property.

 returns: uint32 Property value / KUL_UNDEFINED
 ***************************************************************************/
uint32 animset_link_property_get(animset_st_animset *_pst_animset, uint32 _u32_link_id, uint32 _u32_property)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Returns property */
  return animset_link_table_property_get(_pst_animset->pst_link_table, _u32_link_id, _u32_property);
}

/***************************************************************************
 animset_anim_compute
 Computes active animation given current and destination Animation ID & a relative timestamp (writable).

 returns: active animation id (_pu32_timestamp is updated at need) / KUL_UNDEFINED if none
 ***************************************************************************/
uint32 animset_anim_compute(animset_st_animset *_pst_animset, uint32 _u32_src_anim_id, uint32 _u32_dst_anim_id, uint32 *_pu32_timestamp, animset_st_link_table *_pst_link_table)
{
  uint32 u32_duration, u32_anim;
  animset_st_link_table *pst_work_table;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);
  ASSERT(_pu32_timestamp != NULL);
  ASSERT(_u32_src_anim_id < animset_anim_counter_get(_pst_animset));
  ASSERT((_u32_dst_anim_id < animset_anim_counter_get(_pst_animset)) || (_u32_dst_anim_id == KUL_UNDEFINED));

  /* Gets Link Table */
  if(animset_flag_test(_pst_animset, ANIMSET_KUL_ID_FLAG_LINK_STATIC) == FALSE)
  {
    /* Use animation pointer local one */
    pst_work_table = _pst_link_table;
  }
  else
  {
    pst_work_table = _pst_animset->pst_link_table;
  }

  /* Checks working link table */
  ASSERT(pst_work_table != NULL);

  /* Computes link table if needed */
  animset_link_table_compute(pst_work_table);

  /* Gets current animation */
  u32_anim = _u32_src_anim_id;

  /* Gets current animation duration */
  u32_duration = anim_duration_get(_pst_animset->past_anim[_u32_src_anim_id]);

  /* Next animation? */
  while(*_pu32_timestamp > u32_duration)
  {
    /* Auto mode? */
    if(_u32_dst_anim_id == KUL_UNDEFINED)
    {
      /* Get next animation */
      u32_anim = animset_anim_next_compute(pst_work_table, u32_anim);
    }
    /* Destination mode */
    else
    {
      /* Get next animation according to destination aim */
      u32_anim = animset_anim_next_using_dst_compute(pst_work_table, u32_anim, _u32_dst_anim_id);
    }

    /* Updates timestamp */
    *_pu32_timestamp -= u32_duration;

    /* Has next animation? */
    if(u32_anim != KUL_UNDEFINED)
    {
      /* Gets new duration */
      u32_duration = anim_duration_get(_pst_animset->past_anim[u32_anim]);
    }
    else
    {
      /* !!! MSG !!! */

      return KUL_UNDEFINED;
    }
  }

  /* Return current anim */
  return u32_anim;
}


/* *** Structure accessors *** */


/***************************************************************************
 animset_anim_counter_get
 Animation Set internal Animation counter get accessor.

 returns: uint32 counter
 ***************************************************************************/
uint32 animset_anim_counter_get(animset_st_animset *_pst_animset)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Gets counter */
  return((_pst_animset->u32_id_flags & ANIMSET_KUL_ID_MASK_COUNTER) >> ANIMSET_KI_ID_SHIFT_COUNTER);
}

/***************************************************************************
 animset_anim_storage_size_get
 Animation Set internal Animation storage size get accessor.

 returns: uint32 storage size
 ***************************************************************************/
uint32 animset_anim_storage_size_get(animset_st_animset *_pst_animset)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Gets storage size */
  return((_pst_animset->u32_id_flags & ANIMSET_KUL_ID_MASK_SIZE) >> ANIMSET_KI_ID_SHIFT_SIZE);
}  

/***************************************************************************
 animset_flag_test
 Animation set flag test accessor.

 returns: bool
 ***************************************************************************/
bool animset_flag_test(animset_st_animset *_pst_animset, uint32 _u32_flag)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  if(_pst_animset->u32_id_flags & _u32_flag)
  {
    return TRUE;
  }

  return FALSE;
}

/***************************************************************************
 animset_flag_set
 Animation set flag get/set accessor.

 returns: void
 ***************************************************************************/
void animset_flag_set(animset_st_animset *_pst_animset, uint32 _u32_add_flags, uint32 _u32_remove_flags)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  _pst_animset->u32_id_flags &= ~_u32_remove_flags;
  _pst_animset->u32_id_flags |= _u32_add_flags;

  return;
}


/* *** Link Table public functions *** */

/***************************************************************************
 animset_link_table_delete
 Deletes the AnimationSet link table.

 returns: Nothing
 ***************************************************************************/
void animset_link_table_delete(animset_st_link_table *_pst_link_table)
{
  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_link_table != NULL);

  /* Frees memory*/
  free(_pst_link_table->au32_link_array);
  free(_pst_link_table->auc_loop_array);
  free(_pst_link_table);

  return;
}

/***************************************************************************
 animset_link_table_duplicate
 Duplicates an Animset link table.

 returns: animset_st_link_table *
 ***************************************************************************/
animset_st_link_table *animset_link_table_duplicate(animset_st_animset *_pst_animset)
{
  animset_st_link_table *pst_link_table;

  /* Checks */
  ASSERT(animset_su32_flags & ANIMSET_KUL_FLAG_READY);
  ASSERT(_pst_animset != NULL);

  /* Creates a new link table */
  pst_link_table = animset_link_table_create((uint32)(_pst_animset->pst_link_table->uw_size));

  /* Not allocated? */
  if(pst_link_table == NULL)
  {
    /* !!! MSG !!! */

    return NULL;
  }

  /* Copies it */
  animset_link_table_copy(_pst_animset->pst_link_table, pst_link_table);

  /* Done */
  return pst_link_table;
}

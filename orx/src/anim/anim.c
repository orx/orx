/***************************************************************************
 anim.c
 Animation (Data) module
 
 begin                : 12/02/2004
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


#include "anim/anim.h"

#include "debug/debug.h"


/*
 * Platform independant defines
 */

#define ANIM_KUL_FLAG_NONE              0x00000000
#define ANIM_KUL_FLAG_READY             0x00000001
#define ANIM_KUL_FLAG_DEFAULT           0x00000000

#define ANIM_KUL_ID_FLAG_NONE           0x00000000

#define ANIM_KUL_ID_MASK_SIZE           0x000000FF
#define ANIM_KUL_ID_MASK_COUNTER        0x0000FF00
#define ANIM_KUL_ID_MASK_FLAGS          0xFFFF0000

#define ANIM_KI_ID_SHIFT_SIZE           0
#define ANIM_KI_ID_SHIFT_COUNTER        8


/*
 * Animation structure
 */
struct st_anim_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Id flags : 20 */
  uint32 u32_id_flags;

  /* Used textures pointer array : 24 */
  texture_st_texture **past_texture;

  /* Timestamp array : 32 */
  uint32 *au32_timestamp;
};



/*
 * Static members
 */
static uint32 anim_su32_flags = ANIM_KUL_FLAG_DEFAULT;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 anim_2d_texture_index_find
 Gets a texture index given a timestamp

 returns: texture index / KUL_UNDEFINED
 ***************************************************************************/
inline uint32 anim_2d_texture_index_find(anim_st_anim *_pst_anim, uint32 _u32_timestamp)
{
  uint32 u32_counter, u32_max_index, u32_min_index, u32_index;

  /* Checks */
  ASSERT(_pst_anim != NULL);

  /* Gets counter */
  u32_counter = anim_2d_texture_counter_get(_pst_anim);

  /* Is animation empty? */
  if(u32_counter == 0)
  {
    /* !!! MSG !!! */
    
    return KUL_UNDEFINED;
  }
  else
  {
    /* Gets min & max index */
    u32_max_index = u32_counter - 1;
    u32_min_index = 0;
  }

  /* Dichotomic search */
  for(u32_min_index = 0, u32_max_index = u32_counter - 1, u32_index = u32_max_index >> 1;
      u32_min_index < u32_max_index;
      u32_index = (u32_min_index + u32_max_index) >> 1)
  {
    /* Updates search range */
    if(_u32_timestamp > _pst_anim->au32_timestamp[u32_index])
    {
      u32_min_index = u32_index + 1;
    }
    else
    {
      u32_max_index = u32_index;
    }
  }

  /* Not found? */
  if(_pst_anim->au32_timestamp[u32_index] < _u32_timestamp)
  {
    /* !!! MSG !!! */

    u32_index = KUL_UNDEFINED;
  }

  return u32_index;
}

/***************************************************************************
 anim_2d_texture_storage_size_set
 Sets 2D animation texture storage size.

 returns: void
 ***************************************************************************/
inline void anim_2d_texture_storage_size_set(anim_st_anim *_pst_anim, uint32 _u32_size)
{
  /* Checks */
  ASSERT(_pst_anim != NULL);
  ASSERT(_u32_size <= ANIM_KI_TEXTURE_MAX_NUMBER);

  /* Updates storage size */
  anim_flag_set(_pst_anim, _u32_size << ANIM_KI_ID_SHIFT_SIZE, ANIM_KUL_ID_MASK_SIZE);

  return;
}  

/***************************************************************************
 anim_2d_texture_counter_set
 Sets a 2D animation internal texture counter.

 returns: void
 ***************************************************************************/
inline void anim_2d_texture_counter_set(anim_st_anim *_pst_anim, uint32 _u32_texture_counter)
{
  /* Checks */
  ASSERT(_pst_anim != NULL);
  ASSERT(_u32_texture_counter <= anim_2d_texture_storage_size_get(_pst_anim));

  /* Updates counter */
  anim_flag_set(_pst_anim, _u32_texture_counter << ANIM_KI_ID_SHIFT_COUNTER, ANIM_KUL_ID_MASK_COUNTER);

  return;
}

/***************************************************************************
 anim_2d_texture_counter_increase
 Increases a 2D animation internal texture counter.

 returns: void
 ***************************************************************************/
inline void anim_2d_texture_counter_increase(anim_st_anim *_pst_anim)
{
  uint32 u32_texture_counter;

  /* Checks */
  ASSERT(_pst_anim != NULL);

  /* Gets texture counter */
  u32_texture_counter = anim_2d_texture_counter_get(_pst_anim);

  /* Updates texture counter*/
  anim_2d_texture_counter_set(_pst_anim, u32_texture_counter + 1);

  return;
}  

/***************************************************************************
 anim_2d_texture_counter_decrease
 Decreases a 2D animation internal texture counter.

 returns: void
 ***************************************************************************/
inline void anim_2d_texture_counter_decrease(anim_st_anim *_pst_anim)
{
  uint32 u32_texture_counter;

  /* Checks */
  ASSERT(_pst_anim != NULL);

  /* Gets texture counter */
  u32_texture_counter = anim_2d_texture_counter_get(_pst_anim);

  /* Updates texture counter*/
  anim_2d_texture_counter_set(_pst_anim, u32_texture_counter - 1);

  return;
}  

/***************************************************************************
 anim_list_delete
 Deletes all Animations.

 returns: void
 ***************************************************************************/
inline void anim_list_delete()
{
  anim_st_anim *pst_anim = (anim_st_anim *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIM);

  /* Non empty? */
  while(pst_anim != NULL)
  {
    /* Deletes Animation */
    anim_delete(pst_anim);

    /* Gets first Animation */
    pst_anim = (anim_st_anim *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_ANIM);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 anim_init
 Inits Animation system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 anim_init()
{
  /* Already Initialized? */
  if(anim_su32_flags & ANIM_KUL_FLAG_READY)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }

  /* Inits Flags */
  anim_su32_flags = ANIM_KUL_FLAG_READY;

  return EXIT_SUCCESS;
}

/***************************************************************************
 anim_exit
 Exits from the Animation system.

 returns: void
 ***************************************************************************/
void anim_exit()
{
  /* Initialized? */
  if(anim_su32_flags & ANIM_KUL_FLAG_READY)
  {
    /* Deletes anim list */
    anim_list_delete();

    /* Updates flags */
    anim_su32_flags &= ~ANIM_KUL_FLAG_READY;
  }

  return;
}

/***************************************************************************
 anim_create
 Creates an empty Animation, given its id type and storage size (<= ANIM_KI_TEXTURE_MAX_NUMBER).

 returns: Created anim.
 ***************************************************************************/
anim_st_anim *anim_create(uint32 _u32_id_flag, uint32 _u32_size)
{
  anim_st_anim *pst_anim;
  uint32 i;

  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_u32_size <= ANIM_KI_TEXTURE_MAX_NUMBER);

  /* Creates anim */
  pst_anim = (anim_st_anim *)malloc(sizeof(anim_st_anim));

  /* Non null? */
  if(pst_anim != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_anim, STRUCTURE_KUL_STRUCT_ID_ANIM) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_anim);

      /* Returns nothing */
      return NULL;
    }

    /* Inits flags */
    anim_flag_set(pst_anim, _u32_id_flag & ANIM_KUL_ID_MASK_FLAGS, ANIM_KUL_ID_MASK_FLAGS);

    /* 2D Animation? */
    if(_u32_id_flag & ANIM_KUL_ID_FLAG_2D)
    {
      /* Allocates texture pointer array */
      pst_anim->past_texture = (texture_st_texture **)malloc(_u32_size * sizeof(texture_st_texture *));

      if(pst_anim->past_texture == NULL)
      {
        /* !!! MSG !!! */

        /* Frees partially allocated texture */
        free(pst_anim);

        /* Returns nothing */
        return NULL;
      }

      /* Allocates timestamp array */
      pst_anim->au32_timestamp = (uint32 *)malloc(_u32_size * sizeof(uint32));

      if(pst_anim->au32_timestamp == NULL)
      {
        /* !!! MSG !!! */

        /* Frees partially allocated texture */
        free(pst_anim->past_texture);
        free(pst_anim);

        /* Returns nothing */
        return NULL;
      }

      /* Cleans structure pointers */
      for(i = 0; i < _u32_size; i++)
      {
        pst_anim->past_texture[i] = NULL;
        pst_anim->au32_timestamp[i] = 0x00000000;
      }

      /* Set storage size & counter */
      anim_2d_texture_storage_size_set(pst_anim, _u32_size);
      anim_2d_texture_counter_set(pst_anim, 0);
    }
    /* Other Animation Type? */
    else
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_anim);

      /* Returns nothing */
      return NULL;
    }
  }

  return pst_anim;
}

/***************************************************************************
 anim_delete
 Deletes an Animation.

 returns: void
 ***************************************************************************/
void anim_delete(anim_st_anim *_pst_anim)
{
  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);

  /* Cleans members */

  /* 2D Animation? */
  if(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D))
  {
    anim_2d_texture_clean(_pst_anim);
  }
  /* Other Animation Type? */
  else
  {
    /* !!! MSG !!! */
  }

  /* Cleans structure */
  structure_struct_clean((structure_st_struct *)_pst_anim);

  /* Frees anim memory */
  free(_pst_anim);

  return;
}

/***************************************************************************
 anim_2d_texture_add
 Adds a texture to a 2D Animation.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 anim_2d_texture_add(anim_st_anim *_pst_anim, texture_st_texture *_pst_texture, uint32 _u32_timestamp)
{
  uint32 u32_counter, u32_size;

  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(_pst_texture != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Gets storage size & counter */
  u32_size = anim_2d_texture_storage_size_get(_pst_anim);
  u32_counter = anim_2d_texture_counter_get(_pst_anim);

  /* Is there free room? */
  if(u32_counter >= u32_size)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }
  else
  {
    /* Adds the extra texture */
    _pst_anim->past_texture[u32_counter] = _pst_texture;
    _pst_anim->au32_timestamp[u32_counter] = _u32_timestamp;

    /* Updates texture reference counter */
    structure_struct_counter_increase((structure_st_struct *)_pst_texture);

    /* Updates texture counter */
    anim_2d_texture_counter_increase(_pst_anim);
  }

  /* Done! */
  return EXIT_SUCCESS; 
}

/***************************************************************************
 anim_2d_texture_remove
 Removes last added texture from a 2D Animation.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 anim_2d_texture_remove(anim_st_anim *_pst_anim)
{
  uint32 u32_counter;

  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Gets counter */
  u32_counter = anim_2d_texture_counter_get(_pst_anim);

  /* No texture remaining? */
  if(u32_counter == 0)
  {
    /* !!! MSG !!! */

    return EXIT_FAILURE;
  }
  else
  {
    /* Gets real index */
    u32_counter--;

    /* Updates counter */
    anim_2d_texture_counter_decrease(_pst_anim);

    /* Updates texture reference counter */
    structure_struct_counter_decrease((structure_st_struct *)_pst_anim->past_texture[u32_counter]);

    /* Removes the texture & cleans info */
    _pst_anim->past_texture[u32_counter]   = NULL;
    _pst_anim->au32_timestamp[u32_counter]  = 0;
  }

  /* Done! */
  return EXIT_SUCCESS;
}

/***************************************************************************
 anim_2d_texture_clean
 Cleans all referenced textures from a 2D Animation.

 returns: void
 ***************************************************************************/
void anim_2d_texture_clean(anim_st_anim *_pst_anim)
{
  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Until there are no texture left */
  while(anim_2d_texture_remove(_pst_anim) != EXIT_FAILURE);

  /* Done! */
  return;
}

/***************************************************************************
 anim_2d_texture_get
 Texture used by a 2D Animation get accessor, given its index.

 returns: texture_st_texture *
 ***************************************************************************/
texture_st_texture *anim_2d_texture_get(anim_st_anim *_pst_anim, uint32 _u32_index)
{
  uint32 u32_counter;
  texture_st_texture *pst_texture = NULL;

  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Gets counter */
  u32_counter = anim_2d_texture_counter_get(_pst_anim);

  /* Is index valid? */
  if(_u32_index < u32_counter)
  {
    /* Gets texture */
    pst_texture = _pst_anim->past_texture[_u32_index];
  }
  else
  {
    /* !!! MSG !!! */

  }

  return pst_texture;
}

/***************************************************************************
 anim_2d_texture_compute
 Computes active 2D texture given a timestamp.

 returns: uint32 texture index
 ***************************************************************************/
texture_st_texture *anim_2d_texture_compute(anim_st_anim *_pst_anim, uint32 _u32_timestamp)
{
  texture_st_texture *pst_texture = NULL;
  uint32 u32_index;

  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Finds corresponding texture index */
  u32_index = anim_2d_texture_index_find(_pst_anim, _u32_timestamp);

  /* Found? */
  if(u32_index != KUL_UNDEFINED)
  {
    pst_texture = _pst_anim->past_texture[u32_index];
  }

  return pst_texture;
}


/* *** Structure accessors *** */


/***************************************************************************
 anim_2d_texture_counter_get
 2D Animation texture counter get accessor.

 returns: uint32 counter
 ***************************************************************************/
uint32 anim_2d_texture_counter_get(anim_st_anim *_pst_anim)
{
  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Gets counter */
  return((_pst_anim->u32_id_flags & ANIM_KUL_ID_MASK_COUNTER) >> ANIM_KI_ID_SHIFT_COUNTER);
}

/***************************************************************************
 anim_2d_texture_storage_size_get
 2D Animation texture storage size get accessor.

 returns: uint32 storage size
 ***************************************************************************/
uint32 anim_2d_texture_storage_size_get(anim_st_anim *_pst_anim)
{
  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);
  ASSERT(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE);

  /* Gets storage size */
  return((_pst_anim->u32_id_flags & ANIM_KUL_ID_MASK_SIZE) >> ANIM_KI_ID_SHIFT_SIZE);
}  

/***************************************************************************
 anim_duration_get
 Animation duration get accessor.

 returns: uint32 duration
 ***************************************************************************/
uint32 anim_duration_get(anim_st_anim *_pst_anim)
{
  uint32 u32_counter, u32_duration = 0;

  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);

  /* 2D? */
  if(anim_flag_test(_pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE)
  {
    /* Gets texture counter */
    u32_counter = anim_2d_texture_counter_get(_pst_anim);

    /* Is animation non empty? */
    if(u32_counter != 0)
    {
      /* Gets duration */
      u32_duration = _pst_anim->au32_timestamp[u32_counter - 1];
    }
  }
  else
  {
    /* !!! MSG !!! */

    return KUL_UNDEFINED;
  }

  return u32_duration;
}

/***************************************************************************
 anim_flag_test
 Animation flag test accessor.

 returns: bool
 ***************************************************************************/
bool anim_flag_test(anim_st_anim *_pst_anim, uint32 _u32_flag)
{
  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);

  if(_pst_anim->u32_id_flags & _u32_flag)
  {
    return TRUE;
  }

  return FALSE;
}

/***************************************************************************
 anim_flag_set
 Animation flag get/set accessor.

 returns: void
 ***************************************************************************/
void anim_flag_set(anim_st_anim *_pst_anim, uint32 _u32_add_flags, uint32 _u32_remove_flags)
{
  /* Checks */
  ASSERT(anim_su32_flags & ANIM_KUL_FLAG_READY);
  ASSERT(_pst_anim != NULL);

  _pst_anim->u32_id_flags &= ~_u32_remove_flags;
  _pst_anim->u32_id_flags |= _u32_add_flags;

  return;
}

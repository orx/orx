/***************************************************************************
 graphic.c
 Graphic module
 
 begin                : 08/12/2003
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


#include "graph/graphic.h"

#include "anim/animpointer.h"


/*
 * Platform independant defines
 */

#define GRAPHIC_KUL_FLAG_NONE                 0x00000000
#define GRAPHIC_KUL_FLAG_READY                0x00000001
#define GRAPHIC_KUL_FLAG_DEFAULT              0x00000000

#define GRAPHIC_KUL_ID_FLAG_RENDER_DIRTY      0x20000000

#define GRAPHIC_KI_STRUCT_NUMBER              2

#define GRAPHIC_KI_STRUCT_OFFSET_INVALID     -1
#define GRAPHIC_KI_STRUCT_OFFSET_TEXTURE      0
#define GRAPHIC_KI_STRUCT_OFFSET_ANIMPOINTER  1


/*
 * Object structure
 */
struct st_graphic_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Used structures ids : 20 */
  uint32 u32_struct_ids;

  /* Id flags : 24 */
  uint32 u32_id_flags;

  /* Used structures : 32 */
  structure_st_struct *past_struct[GRAPHIC_KI_STRUCT_NUMBER];
};



/*
 * Static members
 */
static uint32 graphic_su32_flags = GRAPHIC_KUL_FLAG_DEFAULT;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 graphic_struct_offset_get
 Gets a structure offset given its id

 returns: requested structure offset
 ***************************************************************************/
inline int32 graphic_struct_offset_get(uint32 _u32_struct_id)
{
  /* Gets structure offset according to id */
  switch(_u32_struct_id)
  {
    /* Texture structure */
    case STRUCTURE_KUL_STRUCT_ID_TEXTURE:
      return GRAPHIC_KI_STRUCT_OFFSET_TEXTURE;

    /* AnimationPointer structure*/
    case STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER:
      return GRAPHIC_KI_STRUCT_OFFSET_ANIMPOINTER;

    default:
      return GRAPHIC_KI_STRUCT_OFFSET_INVALID;
  }
}

/***************************************************************************
 graphic_list_delete
 Deletes all graphics.

 returns: void
 ***************************************************************************/
inline void graphic_list_delete()
{
  graphic_st_graphic *pst_graphic = (graphic_st_graphic *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_GRAPHIC);

  /* Non empty? */
  while(pst_graphic != NULL)
  {
    /* Deletes graphic */
    graphic_delete(pst_graphic);

    /* Gets first graphic */
    pst_graphic = (graphic_st_graphic *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_GRAPHIC);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 graphic_init
 Inits graphic system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 graphic_init()
{
  /* Not already Initialized? */
  if(!(graphic_su32_flags & GRAPHIC_KUL_FLAG_READY))
  {
    /* Inits Flags */
    graphic_su32_flags = GRAPHIC_KUL_FLAG_READY;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 graphic_exit
 Exits from the graphic system.

 returns: void
 ***************************************************************************/
void graphic_exit()
{
  /* Initialized? */
  if(graphic_su32_flags & GRAPHIC_KUL_FLAG_READY)
  {
    graphic_su32_flags &= ~GRAPHIC_KUL_FLAG_READY;

    /* Deletes graphic list */
    graphic_list_delete();
  }

  return;
}

/***************************************************************************
 graphic_create
 Creates a new empty graphic.

 returns: Created graphic.
 ***************************************************************************/
graphic_st_graphic *graphic_create()
{
  graphic_st_graphic *pst_graphic;
  int32 i;

  /* Creates graphic */
  pst_graphic = (graphic_st_graphic *) malloc(sizeof(graphic_st_graphic));

  /* Non null? */
  if(pst_graphic != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_graphic, STRUCTURE_KUL_STRUCT_ID_GRAPHIC) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Frees partially allocated texture */
      free(pst_graphic);

      /* Returns nothing */
      return NULL;
    }

    /* Inits flags */
    pst_graphic->u32_struct_ids = STRUCTURE_KUL_STRUCT_ID_NONE;
    pst_graphic->u32_id_flags = GRAPHIC_KUL_ID_FLAG_NONE;

    /* Cleans structure pointers */
    for(i = 0; i < GRAPHIC_KI_STRUCT_NUMBER; i++)
    {
      pst_graphic->past_struct[i] = NULL;
    }
  }

  return pst_graphic;
}

/***************************************************************************
 graphic_delete
 Deletes a graphic.

 returns: void
 ***************************************************************************/
void graphic_delete(graphic_st_graphic *_pst_graphic)
{
  /* Non null? */
  if(_pst_graphic != NULL)
  {
    /* Cleans members */
    graphic_struct_unlink(_pst_graphic, STRUCTURE_KUL_STRUCT_ID_ALL);

    /* Cleans structure */
    structure_struct_clean((structure_st_struct *)_pst_graphic);

    /* Frees graphic memory */
    free(_pst_graphic);
  }

  return;
}

/***************************************************************************
 graphic_struct_link
 Links a structure to a graphic given.

 returns: void
 ***************************************************************************/
void graphic_struct_link(graphic_st_graphic *_pst_graphic, structure_st_struct *_pst_struct)
{
  uint32 u32_struct_id;
  int32 i_struct_offset;

  /* Non null? */
  if((_pst_graphic != NULL) && (_pst_struct != NULL))
  {
    /* Gets structure id & offset */
    u32_struct_id = structure_struct_id_get(_pst_struct);
    i_struct_offset = graphic_struct_offset_get(u32_struct_id);

    /* Valid? */
    if(i_struct_offset != GRAPHIC_KI_STRUCT_OFFSET_INVALID)
    {
      /* Unlink previous structure if needed */
      graphic_struct_unlink(_pst_graphic, u32_struct_id);

      /* Updates structure reference counter */
      structure_struct_counter_increase(_pst_struct);

      /* Links new structure to graphic */
      _pst_graphic->past_struct[i_struct_offset] = _pst_struct;
      _pst_graphic->u32_struct_ids |= u32_struct_id;

      /* Depends on structure type */
      switch(u32_struct_id)
      {
        /* Texture */
        case STRUCTURE_KUL_STRUCT_ID_TEXTURE:
          /* Updates flag */
          graphic_flag_set(_pst_graphic, GRAPHIC_KUL_ID_FLAG_2D, 0);
          break;

        /* Animpointer */
        case STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER:
          /* Checks current anim */
          if(anim_flag_test(animpointer_anim_get((animpointer_st_animpointer *)_pst_struct), ANIM_KUL_ID_FLAG_2D) != FALSE)
          {
            /* Updates flag */
            graphic_flag_set(_pst_graphic, GRAPHIC_KUL_ID_FLAG_2D | GRAPHIC_KUL_ID_FLAG_ANIM, GRAPHIC_KUL_ID_FLAG_NONE);

            break;
          }

        default:
          /* !!! MSG !!! */

          break;
      }
    }
  }

  return;
}

/***************************************************************************
 graphic_struct_unlink
 Unlinks structures from a graphic given their IDs.
 Ids can be OR'ed.

 returns: void
 ***************************************************************************/
void graphic_struct_unlink(graphic_st_graphic *_pst_graphic, uint32 _u32_struct_id)
{
  structure_st_struct *pst_struct;
  uint32 u32_used_id, ul;
  int32 i_struct_offset;
 
  /* Non null? */
  if(_pst_graphic != NULL)
  {
    /* Gets used struct ids */
    u32_used_id = _u32_struct_id & _pst_graphic->u32_struct_ids;

    /* For each requested structure, do the unlink */
    for(ul = 0x00000001; u32_used_id != 0x0000000; ul <<= 1)
    {
      /* Needs to be processed? */
      if(ul & u32_used_id)
      {
        /* Updates remaining structure ids */
        u32_used_id &= ~ul;

        /* Gets structure offset */
        i_struct_offset = graphic_struct_offset_get(ul);

        /* Decreases structure reference counter */
        pst_struct = _pst_graphic->past_struct[i_struct_offset];
        structure_struct_counter_decrease(pst_struct);

        /* Unlinks structure */
        _pst_graphic->past_struct[i_struct_offset] = NULL;
      }
    }

    /* Updates structures ids */
      _pst_graphic->u32_struct_ids &= ~_u32_struct_id;

    /* Updates flags */
    if(_u32_struct_id & STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER)
    {
      graphic_flag_set(_pst_graphic, GRAPHIC_KUL_ID_FLAG_NONE, GRAPHIC_KUL_ID_FLAG_ANIM);
    }
  }

  return;
}

/***************************************************************************
 graphic_render_status_ok
 Test graphic render status (TRUE : clean / FALSE : dirty)

 returns: TRUE (clean) / FALSE (dirty)
 ***************************************************************************/
inline bool graphic_render_status_ok(graphic_st_graphic *_pst_graphic)
{
  /* Non null? */
  if(_pst_graphic != NULL)
  {
    /* Test render dirty flag */
    if(_pst_graphic->u32_id_flags & GRAPHIC_KUL_ID_FLAG_RENDER_DIRTY)
    {
      return FALSE;
    }
    else
    {
      return TRUE;
    }
  }

  return TRUE;
}

/***************************************************************************
 graphic_render_status_clean
 Cleans all graphics render status

 returns: void
 ***************************************************************************/
void graphic_render_status_clean()
{
  graphic_st_graphic *pst_graphic = (graphic_st_graphic *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_GRAPHIC);

  /* Non empty? */
  while(pst_graphic != NULL)
  {
    /* Removes render dirty flag from all graphic */
    pst_graphic->u32_id_flags &= ~GRAPHIC_KUL_ID_FLAG_RENDER_DIRTY;

    /* Gets next graphic */
    pst_graphic = (graphic_st_graphic *)structure_struct_next_get((structure_st_struct *)pst_graphic);
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 graphic_struct_get
 Gets a structure used by a graphic, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
inline structure_st_struct *graphic_struct_get(graphic_st_graphic *_pst_graphic, uint32 _u32_struct_id)
{
  structure_st_struct *pst_struct = NULL;
  int32 i_struct_offset;

  /* Gets offset */
  i_struct_offset = graphic_struct_offset_get(_u32_struct_id);

  /* Offset is valid? */
  if(i_struct_offset != GRAPHIC_KI_STRUCT_OFFSET_INVALID)
  {
    /* Gets requested structure */
    pst_struct = _pst_graphic->past_struct[i_struct_offset];
  }

  return pst_struct;
}    

/***************************************************************************
 graphic_2d_data_get
 Gets current 2D data.

 returns: pointer to the current data
 ***************************************************************************/
inline texture_st_texture *graphic_2d_data_get(graphic_st_graphic *_pst_graphic)
{
  /* Use an animation? */
  if(_pst_graphic->u32_struct_ids & STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER)
  {
    animpointer_st_animpointer *pst_animpointer;
    anim_st_anim *pst_anim;

    /* Gets animpointer */
    pst_animpointer = (animpointer_st_animpointer *)graphic_struct_get(_pst_graphic, STRUCTURE_KUL_STRUCT_ID_ANIMPOINTER);

    /* Gets current animation */
    pst_anim = animpointer_anim_get(pst_animpointer);

    /* Is animation 2D? */
    if(anim_flag_test(pst_anim, ANIM_KUL_ID_FLAG_2D) != FALSE)
    {
      uint32 u32_timestamp;

      /* Gets timestamp */
      u32_timestamp = animpointer_time_get(pst_animpointer);

      /* returns texture */
      return(anim_2d_texture_compute(pst_anim, u32_timestamp));
    }
    else
    {
      /* !!! MSG !!! */
 
      return NULL;
    }
  }
  /* Use single texture? */
  else if(_pst_graphic->u32_struct_ids & STRUCTURE_KUL_STRUCT_ID_TEXTURE)
  {
    return((texture_st_texture *)graphic_struct_get(_pst_graphic, STRUCTURE_KUL_STRUCT_ID_TEXTURE));
  }

  /* No data */
  /* !!! MSG !!! */

  return NULL;
}

/***************************************************************************
 graphic_2d_size_get
 Gets current 2d data size.

 returns: void
 ***************************************************************************/
inline void graphic_2d_size_get(graphic_st_graphic *_pst_graphic, coord_st_coord *_pst_coord)
{
  texture_st_texture *pst_texture;

  /* Gets texture */
  pst_texture = graphic_2d_data_get(_pst_graphic);

  /* Data found? */
  if(pst_texture != NULL)
  {
    texture_size_get(pst_texture, _pst_coord);
  }
  else
  {
    /* !!! MSG !!! */

    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

/***************************************************************************
 graphic_2d_ref_coord_get
 Gets graphic current 2d data ref coord.

 returns: void
 ***************************************************************************/
inline void graphic_2d_ref_coord_get(graphic_st_graphic *_pst_graphic, coord_st_coord *_pst_coord)
{
  texture_st_texture *pst_texture;

  /* Gets texture */
  pst_texture = graphic_2d_data_get(_pst_graphic);

  /* Data found? */
  if(pst_texture != NULL)
  {
      texture_ref_coord_get(pst_texture, _pst_coord);
  }
  else
  {
    /* No data */
    /* !!! MSG !!! */

    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

/***************************************************************************
 graphic_2d_max_size_get
 Gets maximum size (used for object bounding box).

 returns: void
 ***************************************************************************/
inline void graphic_2d_max_size_get(graphic_st_graphic *_pst_graphic, coord_st_coord *_pst_coord)
{
  texture_st_texture *pst_texture;

  /* Gets texture */
  pst_texture = graphic_2d_data_get(_pst_graphic);

  /* Data found? */
  if(pst_texture != NULL)
  {
    texture_size_get(pst_texture, _pst_coord);
  }
  else
  {
    /* No data */
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 graphic_flag_test
 Graphic flag test accessor.

 returns: bool
 ***************************************************************************/
bool graphic_flag_test(graphic_st_graphic *_pst_graphic, uint32 _u32_flag)
{
  if(_pst_graphic->u32_id_flags & _u32_flag)
  {
    return TRUE;
  }

  return FALSE;
}

/***************************************************************************
 graphic_flag_set
 Graphic flag get/set accessor.

 returns: void
 ***************************************************************************/
void graphic_flag_set(graphic_st_graphic *_pst_graphic, uint32 _u32_add_flags, uint32 _u32_remove_flags)
{
  _pst_graphic->u32_id_flags &= ~_u32_remove_flags;
  _pst_graphic->u32_id_flags |= _u32_add_flags;

  return;
}

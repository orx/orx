/***************************************************************************
 object.c
 object module
 
 begin                : 01/12/2003
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


#include "object/object.h"

#include "object/frame.h"
#include "graph/graphic.h"


/*
 * Platform independant defines
 */

#define OBJECT_KUL_FLAG_NONE      0x00000000
#define OBJECT_KUL_FLAG_READY     0x00000001

#define OBJECT_KI_STRUCT_NUMBER           3

#define OBJECT_KI_STRUCT_OFFSET_INVALID   -1
#define OBJECT_KI_STRUCT_OFFSET_FRAME     0
#define OBJECT_KI_STRUCT_OFFSET_GRAPHIC   1


/*
 * Object structure
 */
struct st_object_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Used structures ids : 20 */
  uint32 u32_struct_ids;

  /* Used structures : 32 */
  structure_st_struct *past_struct[OBJECT_KI_STRUCT_NUMBER];
};



/*
 * Static members
 */
static uint32 object_su32_flags = OBJECT_KUL_FLAG_NONE;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 object_struct_offset_get
 Gets a structure offset given its id

 returns: requested structure offset
 ***************************************************************************/
inline int32 object_struct_offset_get(uint32 _u32_struct_id)
{
  /* Gets structure offset according to id */
  switch(_u32_struct_id)
  {
    /* Frame structure */
    case STRUCTURE_KUL_STRUCT_ID_FRAME:
      return OBJECT_KI_STRUCT_OFFSET_FRAME;

    /* Graphic structure */
    case STRUCTURE_KUL_STRUCT_ID_GRAPHIC:
      return OBJECT_KI_STRUCT_OFFSET_GRAPHIC;

    default:
      return OBJECT_KI_STRUCT_OFFSET_INVALID;
  }
}

/***************************************************************************
 object_list_delete
 Deletes all objects.

 returns: void
 ***************************************************************************/
void object_list_delete()
{
  object_st_object *pst_object = (object_st_object *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_OBJECT);

  /* Non empty? */
  while(pst_object != NULL)
  {
    /* Deletes object */
    object_delete(pst_object);

    /* Gets first object */
    pst_object = (object_st_object *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_OBJECT);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 object_init
 Inits object system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 object_init()
{
  /* Not already Initialized? */
  if(!(object_su32_flags & OBJECT_KUL_FLAG_READY))
  {
    /* Inits Flags */
    object_su32_flags = OBJECT_KUL_FLAG_READY;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 object_exit
 Exits from the object system.

 returns: void
 ***************************************************************************/
void object_exit()
{
  /* Initialized? */
  if(object_su32_flags & OBJECT_KUL_FLAG_READY)
  {
    object_su32_flags &= ~OBJECT_KUL_FLAG_READY;

    /* Deletes object list */
    object_list_delete();
  }

  return;
}

/***************************************************************************
 object_create
 Creates a new empty object.

 returns: Created object.
 ***************************************************************************/
object_st_object *object_create()
{
  object_st_object *pst_object;
  int32 i;

  /* Creates object */
  pst_object = (object_st_object *) malloc(sizeof(object_st_object));

  /* Non null? */
  if(pst_object != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_object, STRUCTURE_KUL_STRUCT_ID_OBJECT) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Fress partially allocated texture */
      free(pst_object);

      /* Returns nothing */
      return NULL;
    }

    /* Inits structure flags */
    pst_object->u32_struct_ids = STRUCTURE_KUL_STRUCT_ID_NONE;

    /* Cleans structure pointers */
    for(i = 0; i < OBJECT_KI_STRUCT_NUMBER; i++)
    {
      pst_object->past_struct[i] = NULL;
    }
  }

  return pst_object;
}

/***************************************************************************
 object_delete
 Deletes an object.

 returns: void
 ***************************************************************************/
void object_delete(object_st_object *_pst_object)
{
  /* Non null? */
  if(_pst_object != NULL)
  {
    /* Cleans members */
    object_struct_unlink(_pst_object, STRUCTURE_KUL_STRUCT_ID_ALL);

    /* Cleans structure */
    structure_struct_clean((structure_st_struct *)_pst_object);

    /* Frees object memory */
    free(_pst_object);
  }

  return;
}

/***************************************************************************
 object_struct_link
 Links a structure to an object given.

 returns: void
 ***************************************************************************/
void object_struct_link(object_st_object *_pst_object, structure_st_struct *_pst_struct)
{
  uint32 u32_struct_id;
  int32 i_struct_offset;

  /* Non null? */
  if((_pst_object != NULL) && (_pst_struct != NULL))
  {
    /* Gets structure id & offset */
    u32_struct_id = structure_struct_id_get(_pst_struct);
    i_struct_offset = object_struct_offset_get(u32_struct_id);

    /* Valid? */
    if(i_struct_offset != OBJECT_KI_STRUCT_OFFSET_INVALID)
    {
      /* Unlink previous structure if needed */
      object_struct_unlink(_pst_object, u32_struct_id);

      /* Updates structure reference counter */
      structure_struct_counter_increase(_pst_struct);

      /* Links new structure to object */
      _pst_object->past_struct[i_struct_offset] = _pst_struct;
      _pst_object->u32_struct_ids |= u32_struct_id;
    }
  }

  return;
}

/***************************************************************************
 object_struct_unlink
 Unlinks structures from an object given their IDs.
 Ids can be OR'ed.

 returns: void
 ***************************************************************************/
void object_struct_unlink(object_st_object *_pst_object, uint32 _u32_struct_id)
{
  structure_st_struct *pst_struct;
  uint32 u32_used_id, ul;
  int32 i_struct_offset;
 
  /* Non null? */
  if(_pst_object != NULL)
  {
    /* Gets used struct ids */
    u32_used_id = _u32_struct_id & _pst_object->u32_struct_ids;

    /* For each requested structure, do the unlink */
    for(ul = 0x00000001; u32_used_id != 0x0000000; ul <<= 1)
    {
      /* Needs to be processed? */
      if(ul & u32_used_id)
      {
        /* Updates remaining structure ids */
        u32_used_id &= ~ul;

        /* Gets structure offset */
        i_struct_offset = object_struct_offset_get(ul);

        /* Decreases structure reference counter */
        pst_struct = _pst_object->past_struct[i_struct_offset];
        structure_struct_counter_decrease(pst_struct);

        /* Unlinks structure */
        _pst_object->past_struct[i_struct_offset] = NULL;
      }
    }

    /* Updates structures ids */
      _pst_object->u32_struct_ids &= ~_u32_struct_id;
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 object_struct_get
 Gets a structure used by an object, given its structure ID.

 returns: pointer to the requested structure (must be cast correctly)
 ***************************************************************************/
inline structure_st_struct *object_struct_get(object_st_object *_pst_object, uint32 _u32_struct_id)
{
  structure_st_struct *pst_struct = NULL;
  int32 i_struct_offset;

  /* Gets offset */
  i_struct_offset = object_struct_offset_get(_u32_struct_id);

  /* Offset is valid? */
  if(i_struct_offset != OBJECT_KI_STRUCT_OFFSET_INVALID)
  {
    /* Gets requested structure */
    pst_struct = _pst_object->past_struct[i_struct_offset];
  }

  return pst_struct;
}    


/* *** render handling *** */


/***************************************************************************
 object_render_status_ok
 Test object render status (TRUE : clean / FALSE : dirty)

 returns: TRUE (clean) / FALSE (dirty)
 ***************************************************************************/
extern bool object_render_status_ok(object_st_object *_pst_object)
{
  frame_st_frame *pst_frame;
  graphic_st_graphic *pst_graphic;

  /* Non null? */
  if(_pst_object != NULL)
  {
    /* Tests frame render status */
    pst_frame = (frame_st_frame *)object_struct_get(_pst_object, STRUCTURE_KUL_STRUCT_ID_FRAME);

    if(frame_render_status_ok(pst_frame) == FALSE)
    {
      return FALSE;
    }

    /* Tests graphic render status */
    pst_graphic = (graphic_st_graphic *)object_struct_get(_pst_object, STRUCTURE_KUL_STRUCT_ID_GRAPHIC);

    if(graphic_render_status_ok(pst_graphic) == FALSE)
    {
      return FALSE;
    }
  }

  return TRUE;
}

/***************************************************************************
 texture.c
 texture module
 
 begin                : 07/12/2003
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


#include "graph/texture.h"

#include "graph/graph.h"
#include "object/structure.h"


/*
 * Platform independant defines
 */

#define TEXTURE_KUL_FLAG_NONE         0x00000000
#define TEXTURE_KUL_FLAG_READY        0x00000001

#define TEXTURE_KUL_ID_FLAG_NONE      0x00000000
#define TEXTURE_KUL_ID_FLAG_BITMAP    0x00000010
#define TEXTURE_KUL_ID_FLAG_REF_COORD 0x00000100
#define TEXTURE_KUL_ID_FLAG_SIZE      0x00000200

/*
 * Texture structure
 */
struct st_texture_t
{
  /* Public structure, first structure member : 16 */
  structure_st_struct st_struct;

  /* Internal id flags : 20 */
  uint32 u32_id_flags;

  /* Reference coord : 36 */
  coord_st_coord st_ref_coord;

  /* Size coord : 52 */
  coord_st_coord st_size;

  /* Data : 56 */
  void *pst_data;

  /* 8 extra bytes of padding : 64 */
  uint8 auc_unused[8];
};


/*
 * Static members
 */
static uint32 texture_su32_flags = TEXTURE_KUL_FLAG_NONE;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 texture_list_delete
 Deletes all textures.

 returns: void
 ***************************************************************************/
void texture_list_delete()
{
  texture_st_texture *pst_texture = (texture_st_texture *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_TEXTURE);

  /* Non empty? */
  while(pst_texture != NULL)
  {
    /* Deletes object */
    texture_delete(pst_texture);

    /* Gets first object */
    pst_texture = (texture_st_texture *)structure_struct_first_get(STRUCTURE_KUL_STRUCT_ID_TEXTURE);
  }

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/


/***************************************************************************
 texture_init
 Inits texture system.

 returns: EXIT_SUCCESS/EXIT_FAILURE
 ***************************************************************************/
uint32 texture_init()
{
  /* Not already Initialized? */
  if(!(texture_su32_flags & TEXTURE_KUL_FLAG_READY))
  {
    /* Inits Flags */
    texture_su32_flags = TEXTURE_KUL_FLAG_READY;

    return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/***************************************************************************
 texture_exit
 Exits from the texture system.

 returns: void
 ***************************************************************************/
void texture_exit()
{
  /* Initialized? */
  if(texture_su32_flags & TEXTURE_KUL_FLAG_READY)
  {
    texture_su32_flags &= ~TEXTURE_KUL_FLAG_READY;

    /* Deletes texture list */
    texture_list_delete();
  }

  return;
}

/***************************************************************************
 texture_create
 Creates a new empty texture.

 returns: Created texture.
 ***************************************************************************/
texture_st_texture *texture_create()
{
  texture_st_texture *pst_texture;

  /* Creates texture */
  pst_texture = (texture_st_texture *) malloc(sizeof(texture_st_texture));

  /* Non null? */
  if(pst_texture != NULL)
  {
    /* Inits structure */
    if(structure_struct_init((structure_st_struct *)pst_texture, STRUCTURE_KUL_STRUCT_ID_TEXTURE) != EXIT_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Fress partially allocated texture */
      free(pst_texture);

      /* Returns nothing */
      return NULL;
    }

    /* Inits texture members */
    pst_texture->u32_id_flags = TEXTURE_KUL_ID_FLAG_NONE;
    coord_set(&(pst_texture->st_ref_coord), 0, 0, 0);
    coord_set(&(pst_texture->st_size), 0, 0, 0);
    pst_texture->pst_data = NULL;
  }

  return(pst_texture);
}

/***************************************************************************
 texture_create_from_bitmap
 Creates a texture from a bitmap.
 !!! Warning : use a different bitmap for each texture. !!!

 returns: Created texture.
 ***************************************************************************/
texture_st_texture *texture_create_from_bitmap(graph_st_bitmap *_pst_bitmap)
{
  texture_st_texture *pst_texture;

  /* Creates an empty texture */
  pst_texture = texture_create();

  /* Assigns given bitmap to it */
  texture_bitmap_link(pst_texture, _pst_bitmap);

  return pst_texture;
}

/***************************************************************************
 texture_delete
 Deletes an texture.

 returns: void
 ***************************************************************************/
void texture_delete(texture_st_texture *_pst_texture)
{
  /* Non null? */
  if(_pst_texture != NULL)
  {
    /* Cleans structure */
    structure_struct_clean((structure_st_struct *)_pst_texture);

    /* Cleans bitmap reference */
    texture_bitmap_unlink(_pst_texture);

    /* Frees texture memory */
    free(_pst_texture);
  }

  return;
}

/***************************************************************************
 texture_bitmap_link
 Links a bitmap to a texture.

 returns: void
 ***************************************************************************/
void texture_bitmap_link(texture_st_texture *_pst_texture, graph_st_bitmap *_pst_bitmap)
{
  int32 i_x, i_y;

  /* Non null? */
  if((_pst_texture != NULL) && (_pst_bitmap != NULL))
  {
    /* Updates flags */
    _pst_texture->u32_id_flags |= TEXTURE_KUL_ID_FLAG_BITMAP | TEXTURE_KUL_ID_FLAG_SIZE;

    /* References bitmap */
    _pst_texture->pst_data = (void *)_pst_bitmap;

    /* Gets bitmap size */
    graph_bitmap_size_get(_pst_bitmap, &i_x, &i_y);

    /* Copy bitmap size (Z size is null) */
    coord_set(&(_pst_texture->st_size), i_x, i_y, 0);
  }
  else
  {
    /* Bad parameters */
    /* !!! MSG !!! */
  }

  return;
}


/***************************************************************************
 texture_bitmap_unlink
 Unlinks a bitmap from a texture.
 !!! Warning : it deletes it. !!!

 returns: void
 ***************************************************************************/
void texture_bitmap_unlink(texture_st_texture *_pst_texture)
{
  /* Non null? */
  if(_pst_texture != NULL)
  {
    /* Has bitmap */
    if(_pst_texture->u32_id_flags & TEXTURE_KUL_ID_FLAG_BITMAP)
    {
      /* Updates flags */
      _pst_texture->u32_id_flags &= ~(TEXTURE_KUL_ID_FLAG_BITMAP | TEXTURE_KUL_ID_FLAG_SIZE);

      /* Deletes bitmap */
      graph_bitmap_delete((graph_st_bitmap *) (_pst_texture->pst_data));
    }
  }

  return;
}


/* *** Structure accessors *** */


/***************************************************************************
 texture_bitmap_get
 Gets corresponding bitmap.

 returns: bitmap
 ***************************************************************************/
inline graph_st_bitmap *texture_bitmap_get(texture_st_texture *_pst_texture)
{
  /* Has bitmap? */
  if(_pst_texture->u32_id_flags & TEXTURE_KUL_ID_FLAG_BITMAP)
  {
    return((graph_st_bitmap *)_pst_texture->pst_data);
  }

  return NULL;
}

/***************************************************************************
 texture_ref_coord_set
 Sets reference coordinates (used for rendering purpose).

 returns: void
 ***************************************************************************/
inline void texture_ref_coord_set(texture_st_texture *_pst_texture, coord_st_coord *_pst_coord)
{
  /* Updates */
  _pst_texture->u32_id_flags |= TEXTURE_KUL_ID_FLAG_REF_COORD;
  coord_copy(&(_pst_texture->st_ref_coord), _pst_coord);

  return;
}

/***************************************************************************
 texture_ref_coord_get
 Gets reference coordinates (used for rendering purpose).

 returns: void
 ***************************************************************************/
inline void texture_ref_coord_get(texture_st_texture *_pst_texture, coord_st_coord *_pst_coord)
{
  /* Has reference coordinates? */
  if(_pst_texture->u32_id_flags & TEXTURE_KUL_ID_FLAG_REF_COORD)
  {
    /* Copy coord */
    coord_copy(_pst_coord, &(_pst_texture->st_ref_coord));
  }
  else
  {
    /* Sets coord to (0, 0, 0) */
    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

/***************************************************************************
 texture_size_get
 Gets size.

 returns: void
 ***************************************************************************/
inline void texture_size_get(texture_st_texture *_pst_texture, coord_st_coord *_pst_coord)
{
  /* Has size? */
  if(_pst_texture->u32_id_flags & TEXTURE_KUL_ID_FLAG_SIZE)
  {
    coord_copy(_pst_coord, &(_pst_texture->st_size));
  }
  else
  {
    /* Sets size to (0, 0, 0) */
    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

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


#include "graph/orxTexture.h"

#include "graph/graph.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/*
 * Platform independant defines
 */

#define TEXTURE_KU32_FLAG_NONE         0x00000000
#define TEXTURE_KU32_FLAG_READY        0x00000001

#define TEXTURE_KU32_ID_FLAG_NONE      0x00000000
#define TEXTURE_KU32_ID_FLAG_BITMAP    0x00000010
#define TEXTURE_KU32_ID_FLAG_REF_COORD 0x00000100
#define TEXTURE_KU32_ID_FLAG_SIZE      0x00000200

/*
 * Texture structure
 */
struct st_texture_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Reference coord : 32 */
  orxVEC st_ref_coord;

  /* Size coord : 48 */
  orxVEC vSize;

  /* Internal id flags : 52 */
  orxU32 u32IDFlags;

  /* Data : 56 */
  orxVOID *pstData;

  /* 8 extra bytes of padding : 64 */
  orxU8 au8Unused[8];
};


/*
 * Static members
 */
static orxU32 texture_su32Flags = TEXTURE_KU32_FLAG_NONE;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 texture_list_delete
 Deletes all textures.

 returns: orxVOID
 ***************************************************************************/
orxVOID texture_list_delete()
{
  orxTEXTURE *pstTexture = (orxTEXTURE *)orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE);

  /* Non empty? */
  while(pstTexture != orxNULL)
  {
    /* Deletes object */
    texture_delete(pstTexture);

    /* Gets first object */
    pstTexture = (orxTEXTURE *)orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE);
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

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxU32 texture_init()
{
  /* Not already Initialized? */
  if(!(texture_su32Flags & TEXTURE_KU32_FLAG_READY))
  {
    /* Inits Flags */
    texture_su32Flags = TEXTURE_KU32_FLAG_READY;

    return orxSTATUS_SUCCESS;
  }

  return orxSTATUS_FAILED;
}

/***************************************************************************
 texture_exit
 Exits from the texture system.

 returns: orxVOID
 ***************************************************************************/
orxVOID texture_exit()
{
  /* Initialized? */
  if(texture_su32Flags & TEXTURE_KU32_FLAG_READY)
  {
    texture_su32Flags &= ~TEXTURE_KU32_FLAG_READY;

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
orxTEXTURE *texture_create()
{
  orxTEXTURE *pstTexture;

  /* Creates texture */
  pstTexture = (orxTEXTURE *) orxMemory_Allocate(sizeof(orxTEXTURE), orxMEMORY_TYPE_MAIN);

  /* Non null? */
  if(pstTexture != orxNULL)
  {
    /* Inits structure */
    if(orxStructure_Setup((orxSTRUCTURE *)pstTexture, orxSTRUCTURE_ID_TEXTURE) != orxSTATUS_SUCCESS)
    {
      /* !!! MSG !!! */

      /* Fress partially allocated texture */
      orxMemory_Free(pstTexture);

      /* Returns nothing */
      return orxNULL;
    }

    /* Inits texture members */
    pstTexture->u32IDFlags = TEXTURE_KU32_ID_FLAG_NONE;
    coord_set(&(pstTexture->st_ref_coord), 0, 0, 0);
    coord_set(&(pstTexture->vSize), 0, 0, 0);
    pstTexture->pstData = orxNULL;
  }

  return(pstTexture);
}

/***************************************************************************
 texture_create_from_bitmap
 Creates a texture from a bitmap.
 !!! Warning : use a different bitmap for each texture. !!!

 returns: Created texture.
 ***************************************************************************/
orxTEXTURE *texture_create_from_bitmap(graph_st_bitmap *_pstBitmap)
{
  orxTEXTURE *pstTexture;

  /* Creates an empty texture */
  pstTexture = texture_create();

  /* Assigns given bitmap to it */
  texture_bitmap_link(pstTexture, _pstBitmap);

  return pstTexture;
}

/***************************************************************************
 texture_delete
 Deletes an texture.

 returns: orxVOID
 ***************************************************************************/
orxVOID texture_delete(orxTEXTURE *_pstTexture)
{
  /* Non null? */
  if(_pstTexture != orxNULL)
  {
    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstTexture);

    /* Cleans bitmap reference */
    texture_bitmap_unlink(_pstTexture);

    /* Frees texture memory */
    orxMemory_Free(_pstTexture);
  }

  return;
}

/***************************************************************************
 texture_bitmap_link
 Links a bitmap to a texture.

 returns: orxVOID
 ***************************************************************************/
orxVOID texture_bitmap_link(orxTEXTURE *_pstTexture, graph_st_bitmap *_pstBitmap)
{
  orxS32 i_x, i_y;

  /* Non null? */
  if((_pstTexture != orxNULL) && (_pstBitmap != orxNULL))
  {
    /* Updates flags */
    _pstTexture->u32IDFlags |= TEXTURE_KU32_ID_FLAG_BITMAP | TEXTURE_KU32_ID_FLAG_SIZE;

    /* References bitmap */
    _pstTexture->pstData = (orxVOID *)_pstBitmap;

    /* Gets bitmap size */
    graph_bitmap_size_get(_pstBitmap, &i_x, &i_y);

    /* Copy bitmap size (Z size is null) */
    coord_set(&(_pstTexture->vSize), i_x, i_y, 0);
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

 returns: orxVOID
 ***************************************************************************/
orxVOID texture_bitmap_unlink(orxTEXTURE *_pstTexture)
{
  /* Non null? */
  if(_pstTexture != orxNULL)
  {
    /* Has bitmap */
    if(_pstTexture->u32IDFlags & TEXTURE_KU32_ID_FLAG_BITMAP)
    {
      /* Updates flags */
      _pstTexture->u32IDFlags &= ~(TEXTURE_KU32_ID_FLAG_BITMAP | TEXTURE_KU32_ID_FLAG_SIZE);

      /* Deletes bitmap */
      graph_bitmap_delete((graph_st_bitmap *) (_pstTexture->pstData));
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
inline graph_st_bitmap *texture_bitmap_get(orxTEXTURE *_pstTexture)
{
  /* Has bitmap? */
  if(_pstTexture->u32IDFlags & TEXTURE_KU32_ID_FLAG_BITMAP)
  {
    return((graph_st_bitmap *)_pstTexture->pstData);
  }

  return orxNULL;
}

/***************************************************************************
 texture_ref_coord_set
 Sets reference coordinates (used for rendering purpose).

 returns: orxVOID
 ***************************************************************************/
inline orxVOID texture_ref_coord_set(orxTEXTURE *_pstTexture, orxVEC *_pst_coord)
{
  /* Updates */
  _pstTexture->u32IDFlags |= TEXTURE_KU32_ID_FLAG_REF_COORD;
  coord_copy(&(_pstTexture->st_ref_coord), _pst_coord);

  return;
}

/***************************************************************************
 texture_ref_coord_get
 Gets reference coordinates (used for rendering purpose).

 returns: orxVOID
 ***************************************************************************/
inline orxVOID texture_ref_coord_get(orxTEXTURE *_pstTexture, orxVEC *_pst_coord)
{
  /* Has reference coordinates? */
  if(_pstTexture->u32IDFlags & TEXTURE_KU32_ID_FLAG_REF_COORD)
  {
    /* Copy coord */
    coord_copy(_pst_coord, &(_pstTexture->st_ref_coord));
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

 returns: orxVOID
 ***************************************************************************/
inline orxVOID texture_size_get(orxTEXTURE *_pstTexture, orxVEC *_pst_coord)
{
  /* Has size? */
  if(_pstTexture->u32IDFlags & TEXTURE_KU32_ID_FLAG_SIZE)
  {
    coord_copy(_pst_coord, &(_pstTexture->vSize));
  }
  else
  {
    /* Sets size to (0, 0, 0) */
    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

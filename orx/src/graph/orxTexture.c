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

#define orxTEXTURE_KU32_FLAG_NONE               0x00000000
#define orxTEXTURE_KU32_FLAG_READY              0x00000001

#define orxTEXTURE_KU32_ID_FLAG_NONE            0x00000000
#define orxTEXTURE_KU32_ID_FLAG_BITMAP          0x00000010
#define orxTEXTURE_KU32_ID_FLAG_REF_COORD       0x00000100
#define orxTEXTURE_KU32_ID_FLAG_SIZE            0x00000200

/*
 * Texture structure
 */
struct __orxTEXTURE_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Reference point : 32 */
  orxVEC vRefPoint;

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
 * Static structure
 */
typedef struct __orxTEXTURE_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

} orxTEXTURE_STATIC;

/*
 * Static data
 */
orxSTATIC orxTEXTURE_STATIC sstTexture;



/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxTexture_DeleteAll
 Deletes all textures.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTexture_DeleteAll()
{
  orxTEXTURE *pstTexture;
  
  /* Gets first texture */
  pstTexture = (orxTEXTURE *)orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE);

  /* Non empty? */
  while(pstTexture != orxNULL)
  {
    /* Deletes texture */
    orxTexture_Delete(pstTexture);

    /* Gets first texture */
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
 orxTexture_Init
 Inits texture system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTexture_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not already Initialized? */
  if(!(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstTexture, 0, sizeof(orxTEXTURE_STATIC));

    /* Registers structure type */
    eResult = orxStructure_RegisterStorageType(orxSTRUCTURE_ID_TEXTURE, orxSTRUCTURE_STORAGE_TYPE_LINKLIST);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_FAILED;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstTexture.u32Flags = orxTEXTURE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTexture_Exit
 Exits from the texture system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTexture_Exit()
{
  /* Initialized? */
  if(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY)
  {
    sstTexture.u32Flags &= ~orxTEXTURE_KU32_FLAG_READY;

    /* Deletes texture list */
    orxTexture_DeleteAll();
  }

  return;
}

/***************************************************************************
 orxTexture_Create
 Creates a new empty texture.

 returns: Created texture.
 ***************************************************************************/
orxTEXTURE *orxTexture_Create()
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
    pstTexture->u32IDFlags = orxTEXTURE_KU32_ID_FLAG_NONE;
    coord_set(&(pstTexture->vRefPoint), 0, 0, 0);
    coord_set(&(pstTexture->vSize), 0, 0, 0);
    pstTexture->pstData = orxNULL;
  }

  return(pstTexture);
}

/***************************************************************************
 orxTexture_CreateFromBitmap
 Creates a texture from a bitmap.

 returns: Created texture.
 ***************************************************************************/
orxTEXTURE *orxTexture_CreateFromBitmap(orxCONST orxSTRING _zBitmapFileName)
{
  orxTEXTURE *pstTexture;

  /* Creates an empty texture */
  pstTexture = orxTexture_Create();

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    orxBITMAP *pstBitmap;
    
    /* Loads bitmap */
    pstBitmap = graph_bitmap_load(_zBitmapFileName);

    /* Assigns given bitmap to it */
    orxTexture_LinkBitmap(pstTexture, pstBitmap);
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstTexture;
}

/***************************************************************************
 orxTexture_Delete
 Deletes an texture.

 returns: orxVOID
 ***************************************************************************/
orxSTATUS orxTexture_Delete(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Non null? */
  if(_pstTexture != orxNULL)
  {
    /* Cleans structure */
    orxStructure_Clean((orxSTRUCTURE *)_pstTexture);

    /* Cleans bitmap reference */
    orxTexture_UnlinkBitmap(_pstTexture);

    /* Frees texture memory */
    orxMemory_Free(_pstTexture);
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTexture_LinkBitmap
 Links a bitmap to a texture.

 returns: orxVOID
 ***************************************************************************/
orxSTATUS orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, orxBITMAP *_pstBitmap)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;
  orxU32  u32Width, u32Height;

  /* Non null? */
  if((_pstTexture != orxNULL) && (_pstBitmap != orxNULL))
  {
    /* Updates flags */
    _pstTexture->u32IDFlags |= orxTEXTURE_KU32_ID_FLAG_BITMAP | orxTEXTURE_KU32_ID_FLAG_SIZE;

    /* References bitmap */
    _pstTexture->pstData = (orxVOID *)_pstBitmap;

    /* Gets bitmap size */
    graph_bitmap_size_get(_pstBitmap, &u32Width, &u32Height);

    /* Copy bitmap size (Z size is null) */
    coord_set(&(_pstTexture->vSize), orxU2F(u32Width), orxU2F(u32Height), 0.0f);
  }
  else
  {
    /* Bad parameters */
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 orxTexture_UnlinkBitmap
 Unlinks a bitmap from a texture.
 !!! Warning : it deletes it. !!!

 returns: orxVOID
 ***************************************************************************/
orxSTATUS orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Non null? */
  if(_pstTexture != orxNULL)
  {
    /* Has bitmap */
    if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP)
    {
      /* Updates flags */
      _pstTexture->u32IDFlags &= ~(orxTEXTURE_KU32_ID_FLAG_BITMAP | orxTEXTURE_KU32_ID_FLAG_SIZE);

      /* Deletes bitmap */
      graph_delete((orxBITMAP *) (_pstTexture->pstData));
    }
  }

  /* Done! */
  return eResult;
}


/* *** Structure accessors *** */


/***************************************************************************
 orxTexture_GetBitmap
 Gets corresponding bitmap.

 returns: bitmap
 ***************************************************************************/
orxBITMAP *orxTexture_GetBitmap(orxTEXTURE *_pstTexture)
{
  /* Has bitmap? */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP)
  {
    return((orxBITMAP *)_pstTexture->pstData);
  }

  return orxNULL;
}

/***************************************************************************
 orxTexture_SetRefPoint
 Sets reference coordinates (used for rendering purpose).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTexture_SetRefPoint(orxTEXTURE *_pstTexture, orxVEC *_pst_coord)
{
  /* Updates */
  _pstTexture->u32IDFlags |= orxTEXTURE_KU32_ID_FLAG_REF_COORD;
  coord_copy(&(_pstTexture->vRefPoint), _pst_coord);

  return;
}

/***************************************************************************
 orxTexture_GetRefPoint
 Gets reference coordinates (used for rendering purpose).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTexture_GetRefPoint(orxTEXTURE *_pstTexture, orxVEC *_pst_coord)
{
  /* Has reference coordinates? */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_REF_COORD)
  {
    /* Copy coord */
    coord_copy(_pst_coord, &(_pstTexture->vRefPoint));
  }
  else
  {
    /* Sets coord to (0, 0, 0) */
    coord_set(_pst_coord, 0, 0, 0);
  }

  return;
}

/***************************************************************************
 orxTexture_GetSize
 Gets size.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTexture_GetSize(orxTEXTURE *_pstTexture, orxVEC *_pst_coord)
{
  /* Has size? */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_SIZE)
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

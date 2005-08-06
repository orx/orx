/***************************************************************************
 orxTexture.c
 Texture module
 
 begin                : 07/12/2003
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


#include "display/orxTexture.h"

#include "display/orxDisplay.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/*
 * Platform independant defines
 */

#define orxTEXTURE_KU32_FLAG_NONE               0x00000000
#define orxTEXTURE_KU32_FLAG_READY              0x00000001

#define orxTEXTURE_KU32_ID_FLAG_NONE            0x00000000
#define orxTEXTURE_KU32_ID_FLAG_BITMAP          0x00000010
#define orxTEXTURE_KU32_ID_FLAG_EXTERNAL        0x00000020
#define orxTEXTURE_KU32_ID_FLAG_REF_COORD       0x00000100
#define orxTEXTURE_KU32_ID_FLAG_SIZE            0x00000200


/*
 * Texture structure
 */
struct __orxTEXTURE_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE stStructure;

  /* Self reference counter : 20 */
  orxU32 u32Counter;

  /* Associated bitmap name : 24 */
  orxSTRING zDataName;

  /* Reference point : 40 */
  orxVEC vRefPoint;

  /* Size coord : 56 */
  orxVEC vSize;

  /* Internal id flags : 60 */
  orxU32 u32IDFlags;

  /* Data : 64 */
  orxVOID *pstData;
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
 orxTexture_FindByName
 Finds a texture linked to a specified data.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxTEXTURE *orxTexture_FindByName(orxCONST orxSTRING _zDataName)
{
  orxREGISTER orxTEXTURE *pstTexture;

  /* Gets first texture */
  pstTexture = (orxTEXTURE *)orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE);

  /* Non empty? */
  while(pstTexture != orxNULL)
  {
    /* Has bitmap? */
    if(pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP)
    {
        /* Non external? */
        if(!(pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_EXTERNAL))
        {
            /* Is the specified data? */
            if(pstTexture->zDataName == _zDataName)
            {
                break;
            }
        }
    }

    /* Gets next texture */
    pstTexture = (orxTEXTURE *)orxStructure_GetNext((orxSTRUCTURE *)pstTexture);
  }

  /* Done! */
  return pstTexture;
}

/***************************************************************************
 orxTexture_FindByData
 Finds a texture linked to a specified data.

 returns: orxVOID
 ***************************************************************************/
orxINLINE orxTEXTURE *orxTexture_FindByData(orxVOID *_pstData)
{
  orxREGISTER orxTEXTURE *pstTexture;

  /* Gets first texture */
  pstTexture = (orxTEXTURE *)orxStructure_GetFirst(orxSTRUCTURE_ID_TEXTURE);

  /* Non empty? */
  while(pstTexture != orxNULL)
  {
    /* Has bitmap? */
    if(pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP)
    {
        /* Non external? */
        if(!(pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_EXTERNAL))
        {
            /* Is the specified data? */
            if(pstTexture->pstData == _pstData)
            {
                break;
            }
        }
    }

    /* Gets next texture */
    pstTexture = (orxTEXTURE *)orxStructure_GetNext((orxSTRUCTURE *)pstTexture);
  }

  /* Done! */
  return pstTexture;
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
    orxSTRUCTURE_REGISTER_INFO stRegisterInfo;

    /* Cleans static controller */
    orxMemory_Set(&sstTexture, 0, sizeof(orxTEXTURE));

    /* Registers structure type */
    stRegisterInfo.eStorageType = orxSTRUCTURE_STORAGE_TYPE_LINKLIST;
    stRegisterInfo.u32Size      = sizeof(orxTEXTURE);
    stRegisterInfo.eMemoryType  = orxMEMORY_TYPE_MAIN;
    stRegisterInfo.pfnUpdate    = orxNULL;

    eResult = orxStructure_Register(orxSTRUCTURE_ID_TEXTURE, &stRegisterInfo);

    /* Inits Flags */
    sstTexture.u32Flags = orxTEXTURE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_FAILED;
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
    /* Deletes texture list */
    orxTexture_DeleteAll();

    /* Updates flags */
    sstTexture.u32Flags &= ~orxTEXTURE_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
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

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);

  /* Creates texture */
  pstTexture = (orxTEXTURE *)orxStructure_Create(orxSTRUCTURE_ID_TEXTURE);

  /* Created? */
  if(pstTexture != orxNULL)
  {
    /* !!! INIT? !!! */
  }
  else
  {
    /* !!! MSG !!! */
  }

  return pstTexture;
}

/***************************************************************************
 orxTexture_CreateFromBitmap
 Creates a texture from a bitmap.

 returns: Created texture.
 ***************************************************************************/
orxTEXTURE *orxTexture_CreateFromBitmap(orxCONST orxSTRING _zBitmapFileName)
{
  orxTEXTURE *pstTexture;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_zBitmapFileName != orxNULL);

  /* Search for a texture using this bitmap */
  pstTexture = orxTexture_FindByName(_zBitmapFileName);
  
  /* Found? */
  if(pstTexture != orxNULL)
  {
    /* Increases self reference counter */
    pstTexture->u32Counter++;
  }
  else
  {
    /* Creates an empty texture */
    pstTexture = orxTexture_Create();

    /* Valid? */
    if(pstTexture != orxNULL)
    {
      orxBITMAP *pstBitmap;
    
      /* Loads bitmap */
      pstBitmap = orxDisplay_LoadBitmap(_zBitmapFileName);

      /* Assigns given bitmap to it */
      if((pstBitmap != orxNULL)
      && (orxTexture_LinkBitmap(pstTexture, pstBitmap) == orxSTATUS_SUCCESS))
      {
          /* Inits it */
      }
      else
      {
        /* !!! MSG !!! */

        /* Frees allocated texture */
        orxTexture_Delete(pstTexture);

        /* Not created */
        pstTexture = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
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

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);

  /* Is the last self reference? */
  if(_pstTexture->u32Counter == 1)
  {
    /* Not referenced from outside? */
    if(orxStructure_GetRefCounter((orxSTRUCTURE *)_pstTexture) == 0)
    {
      /* Cleans bitmap reference */
      orxTexture_UnlinkBitmap(_pstTexture);

      /* Deletes structure */
      orxStructure_Delete((orxSTRUCTURE *)_pstTexture);
    }
    else
    {
      /* !!! MSG !!! */

      /* Referenced by others */
      eResult = orxSTATUS_FAILED;
    }
  }
  else
  {
    /* Decreases self reference counter */
    _pstTexture->u32Counter--;    
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

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);
  orxASSERT(_pstBitmap != orxNULL);

  /* Unlink previous bitmap if needed */
  orxTexture_UnlinkBitmap(_pstTexture);

  /* Has no texture now? */
  if(!(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP))
  {
    orxTEXTURE *pstTexture;
    orxVEC vSize;

    /* Search for a texture using this bitmap */
    pstTexture = orxTexture_FindByData(_pstBitmap);

    /* Found? */
    if(pstTexture != orxNULL)
    {
      /* Updates flags */
      _pstTexture->u32IDFlags |= orxTEXTURE_KU32_ID_FLAG_BITMAP | orxTEXTURE_KU32_ID_FLAG_EXTERNAL | orxTEXTURE_KU32_ID_FLAG_SIZE;
 
      /* References external texture */
      _pstTexture->pstData = pstTexture;

      /* Updates external texture self referenced counter */
      pstTexture->u32Counter++;
    }
    else
    {
      /* Updates flags */
      _pstTexture->u32IDFlags |= orxTEXTURE_KU32_ID_FLAG_BITMAP | orxTEXTURE_KU32_ID_FLAG_SIZE;
 
      /* References bitmap */
      _pstTexture->pstData = _pstBitmap;
    }

    /* Gets bitmap size */
    orxDisplay_GetBitmapSize(_pstBitmap, &vSize);

    /* Copy bitmap size (Z size is null) */
    orxVec_Set3(&(_pstTexture->vSize), vSize.fX, vSize.fY, orx2F(0.0f));
  }
  else
  {
    /* !!! MSG !!! */

    /* Already linked */
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 orxTexture_UnlinkBitmap
 Unlinks a bitmap from a texture.
 !!! Warning : it deletes it. !!!

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);

  /* Has bitmap */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP)
  {
    /* External bitmap? */
    if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_EXTERNAL)
    {
      /* Updates flags */
      _pstTexture->u32IDFlags &= ~(orxTEXTURE_KU32_ID_FLAG_BITMAP | orxTEXTURE_KU32_ID_FLAG_EXTERNAL | orxTEXTURE_KU32_ID_FLAG_SIZE);

      /* Decreases external texture self reference counter */
      ((orxTEXTURE *)(_pstTexture->pstData))->u32Counter--;

      /* Cleans data */
      _pstTexture->pstData = orxNULL;
      
    }
    else
    {
      /* Updates flags */
      _pstTexture->u32IDFlags &= ~(orxTEXTURE_KU32_ID_FLAG_BITMAP | orxTEXTURE_KU32_ID_FLAG_SIZE);

      /* Deletes bitmap */
      orxDisplay_DeleteBitmap((orxBITMAP *)(_pstTexture->pstData));

      /* Cleans data */
      _pstTexture->pstData = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* No bitmap to unlink from */
    eResult = orxSTATUS_FAILED;
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
orxCONST orxBITMAP *orxTexture_GetBitmap(orxTEXTURE *_pstTexture)
{
  orxREGISTER orxBITMAP *pstBitmap = orxNULL;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);

  /* Has bitmap? */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_BITMAP)
  {
    /* External bitmap? */
    if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_EXTERNAL)
    {
        pstBitmap = (orxBITMAP *)(((orxTEXTURE *)_pstTexture->pstData)->pstData);
    }
    /* Internal bitmap */
    else
    {
        pstBitmap = (orxBITMAP *)_pstTexture->pstData;
    }
  }

  /* Done! */
  return pstBitmap;
}

/***************************************************************************
 orxTexture_SetRefPoint
 Sets reference coordinates (used for rendering purpose).

 returns: orxVOID
 ***************************************************************************/
orxVOID orxTexture_SetRefPoint(orxTEXTURE *_pstTexture, orxVEC *_pvRefPoint)
{
  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);
  orxASSERT(_pvRefPoint!= orxNULL);

  /* Updates */
  _pstTexture->u32IDFlags |= orxTEXTURE_KU32_ID_FLAG_REF_COORD;
  orxVec_Copy(&(_pstTexture->vRefPoint), _pvRefPoint);

  return;
}

/***************************************************************************
 orxTexture_GetRefPoint
 Gets reference coordinates (used for rendering purpose).

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTexture_GetRefPoint(orxTEXTURE *_pstTexture, orxVEC *_pvRefPoint)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);
  orxASSERT(_pvRefPoint!= orxNULL);

  /* Has reference coordinates? */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_REF_COORD)
  {
    /* Copy coord */
    orxVec_Copy(_pvRefPoint, &(_pstTexture->vRefPoint));
  }
  else
  {
    /* !!! MSG !!! */

    /* No refpoint */
    orxVec_Set3(_pvRefPoint, orx2F(0.0f), orx2F(0.0f), orx2F(0.0f));
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTexture_GetSize
 Gets size.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxTexture_GetSize(orxTEXTURE *_pstTexture, orxVEC *_pvSize)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_FLAG_READY);
  orxASSERT(_pstTexture != orxNULL);
  orxASSERT(_pvSize!= orxNULL);

  /* Has size? */
  if(_pstTexture->u32IDFlags & orxTEXTURE_KU32_ID_FLAG_SIZE)
  {
    /* Gets it */
    orxVec_Copy(_pvSize, &(_pstTexture->vSize));
  }
  else
  {
    /* !!! MSG !!! */

    /* No size */
    orxVec_Set3(_pvSize, orx2F(0.0f), orx2F(0.0f), orx2F(0.0f));
    eResult = orxSTATUS_FAILED;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTexture.c
 Texture module

 begin                : 07/12/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "display/orxTexture.h"

#include "display/orxDisplay.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"


/*
 * Platform independant defines
 */

#define orxTEXTURE_KU32_STATIC_FLAG_NONE        0x00000000

#define orxTEXTURE_KU32_STATIC_FLAG_READY       0x00000001

#define orxTEXTURE_KU32_STATIC_MASK_ALL         0xFFFFFFFF


#define orxTEXTURE_KU32_FLAG_NONE               0x00000000

#define orxTEXTURE_KU32_FLAG_BITMAP             0x00000010
#define orxTEXTURE_KU32_FLAG_EXTERNAL           0x00000020
#define orxTEXTURE_KU32_FLAG_REF_COORD          0x00000100
#define orxTEXTURE_KU32_FLAG_SIZE               0x00000200

#define orxTEXTURE_KU32_MASK_ALL                0xFFFFFFFF

#define orxTEXTURE_KU32_TABLE_SIZE              128

#define orxTEXTURE_KZ_SCREEN_NAME               "-=SCREEN=-"


/*
 * Texture structure
 */
struct __orxTEXTURE_t
{
  /* Public structure, first structure member : 16 */
  orxSTRUCTURE  stStructure;

  /* Self reference counter : 20 */
  orxU32        u32Counter;

  /* Associated bitmap name : 24 */
  orxSTRING     zDataName;

  /* Width : 28 */
  orxFLOAT      fWidth;

  /* Height : 32 */
  orxFLOAT      fHeight;

  /* Data : 36 */
  orxHANDLE     hData;

  /* Padding */
  orxPAD(36)
};

/*
 * Static structure
 */
typedef struct __orxTEXTURE_STATIC_t
{
  orxHASHTABLE *pstTable;                     /**< Bitmap hashtable : 4 */
  orxTEXTURE   *pstScreen;                    /**< Screen texture : 8 */
  orxU32        u32Flags;                     /**< Control flags : 12 */

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
orxSTATIC orxINLINE orxVOID orxTexture_DeleteAll()
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
orxSTATIC orxINLINE orxTEXTURE *orxTexture_FindByName(orxCONST orxSTRING _zDataName)
{
  orxREGISTER orxTEXTURE *pstTexture;

  /* Gets texture from hash table */
  pstTexture = orxHashTable_Get(sstTexture.pstTable, orxString_ToCRC(_zDataName));

  /* Done! */
  return pstTexture;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxTexture_Setup
 Texture module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxTexture_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_HASHTABLE);
  orxModule_AddDependency(orxMODULE_ID_TEXTURE, orxMODULE_ID_DISPLAY);

  return;
}

/***************************************************************************
 orxTexture_Init
 Inits texture system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxTexture_Init()
{
  orxSTATUS eResult;

  /* Not already Initialized? */
  if(!(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstTexture, sizeof(orxTEXTURE_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(TEXTURE, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);

    /* Success? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Creates hash table */
      sstTexture.pstTable = orxHashTable_Create(orxTEXTURE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(sstTexture.pstTable != orxNULL)
      {
        /* Updates flags for screen texture creation */
        sstTexture.u32Flags = orxTEXTURE_KU32_STATIC_FLAG_READY;

        /* Creates screen texture */
        sstTexture.pstScreen = orxTexture_Create();

        /* Valid? */
        if(sstTexture.pstScreen != orxNULL)
        {
          /* Links screen bitmap */
          eResult = orxTexture_LinkBitmap(sstTexture.pstScreen, orxDisplay_GetScreenBitmap(), orxTEXTURE_KZ_SCREEN_NAME);

          /* Failed? */
          if(eResult != orxSTATUS_SUCCESS)
          {
            /* Deletes screen texture */
            orxTexture_Delete(sstTexture.pstScreen);

            /* Deletes hash table */
            orxHashTable_Delete(sstTexture.pstTable);
          }
        }
        else
        {
          /* Deletes hash table */
          orxHashTable_Delete(sstTexture.pstTable);

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult != orxSTATUS_SUCCESS)
  {
    /* !!! MSG !!! */

    /* Updates Flags */
    sstTexture.u32Flags &= ~orxTEXTURE_KU32_STATIC_FLAG_READY;
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
  if(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY)
  {
    /* Unlinks screen texture */
    orxTexture_UnlinkBitmap(sstTexture.pstScreen);

    /* Deletes screen texture */
    orxTexture_Delete(sstTexture.pstScreen);

    /* Deletes texture list */
    orxTexture_DeleteAll();

    /* Deletes hash table */
    orxHashTable_Delete(sstTexture.pstTable);
    sstTexture.pstTable = orxNULL;

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_TEXTURE);

    /* Updates flags */
    sstTexture.u32Flags &= ~orxTEXTURE_KU32_STATIC_FLAG_READY;
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
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);

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
 orxTexture_CreateFromFile
 Creates a texture from a bitmap file.

 returns: Created texture.
 ***************************************************************************/
orxTEXTURE *orxFASTCALL orxTexture_CreateFromFile(orxCONST orxSTRING _zBitmapFileName)
{
  orxTEXTURE *pstTexture;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
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
      && (orxTexture_LinkBitmap(pstTexture, pstBitmap, _zBitmapFileName) == orxSTATUS_SUCCESS))
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
orxSTATUS orxFASTCALL orxTexture_Delete(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Is the last self reference? */
  if(_pstTexture->u32Counter == 0)
  {
    /* Not referenced from outside? */
    if(orxStructure_GetRefCounter(_pstTexture) == 0)
    {
      /* Cleans bitmap reference */
      orxTexture_UnlinkBitmap(_pstTexture);

      /* Deletes structure */
      orxStructure_Delete(_pstTexture);
    }
    else
    {
      /* !!! MSG !!! */

      /* Referenced by others */
      eResult = orxSTATUS_FAILURE;
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
orxSTATUS orxFASTCALL orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zDataName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);
  orxASSERT(_pstBitmap != orxNULL);

  /* Unlink previous bitmap if needed */
  orxTexture_UnlinkBitmap(_pstTexture);

  /* Has no texture now? */
  if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP) == orxFALSE)
  {
    orxTEXTURE *pstTexture;

    /* Search for a texture using this bitmap */
    pstTexture = orxTexture_FindByName(_zDataName);

    /* Found? */
    if(pstTexture != orxNULL)
    {
      /* Checks */
      orxASSERT(_pstBitmap == (orxBITMAP *)pstTexture->hData);

      /* Updates flags */
      orxStructure_SetFlags(_pstTexture, orxStructure_GetFlags(pstTexture, orxTEXTURE_KU32_MASK_ALL) | orxTEXTURE_KU32_FLAG_EXTERNAL, orxTEXTURE_KU32_FLAG_NONE);

      /* References external texture */
      _pstTexture->hData      = (orxHANDLE)pstTexture;

      /* Copies size */
      _pstTexture->fWidth   = pstTexture->fWidth;
      _pstTexture->fHeight  = pstTexture->fHeight;

      /* Updates external texture self referenced counter */
      pstTexture->u32Counter++;
    }
    else
    {
      orxU32 u32Width, u32Height;

       /* Updates flags */
      orxStructure_SetFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP | orxTEXTURE_KU32_FLAG_SIZE, orxTEXTURE_KU32_FLAG_NONE);

      /* References bitmap */
      _pstTexture->hData = (orxHANDLE)_pstBitmap;

      /* Gets bitmap size */
      orxDisplay_GetBitmapSize(_pstBitmap, &u32Width, &u32Height);

      /* Stores it */
      _pstTexture->fWidth   = orxU2F(u32Width);
      _pstTexture->fHeight  = orxU2F(u32Height);
    }

    /* Updates texture name */
    _pstTexture->zDataName = _zDataName;

    /* Adds it to hash table */
    orxHashTable_Add(sstTexture.pstTable, orxString_ToCRC(_zDataName), _pstTexture);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already linked */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxTexture_UnlinkBitmap
 Unlinks a bitmap from a texture.
 !!! Warning : it deletes it. !!!

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Has bitmap */
  if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP) != orxFALSE)
  {
    /* External bitmap? */
    if(orxStructure_TestFlags(_pstTexture, orxTEXTURE_KU32_FLAG_EXTERNAL) != orxFALSE)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstTexture, orxTEXTURE_KU32_FLAG_NONE, (orxTEXTURE_KU32_FLAG_BITMAP | orxTEXTURE_KU32_FLAG_EXTERNAL | orxTEXTURE_KU32_FLAG_SIZE));

      /* Decreases external texture self reference counter */
      ((orxTEXTURE *)(_pstTexture->hData))->u32Counter--;

      /* Cleans data */
      _pstTexture->hData = orxHANDLE_UNDEFINED;
    }
    else
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstTexture, orxTEXTURE_KU32_FLAG_NONE, (orxTEXTURE_KU32_FLAG_BITMAP | orxTEXTURE_KU32_FLAG_SIZE));

      /* Deletes bitmap */
      orxDisplay_DeleteBitmap((orxBITMAP *)(_pstTexture->hData));

      /* Cleans data */
      _pstTexture->hData = orxHANDLE_UNDEFINED;
    }

    /* Removes from hash table */
    orxHashTable_Remove(sstTexture.pstTable, orxString_ToCRC(_pstTexture->zDataName));
  }
  else
  {
    /* !!! MSG !!! */

    /* No bitmap to unlink from */
    eResult = orxSTATUS_FAILURE;
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
orxBITMAP *orxFASTCALL orxTexture_GetBitmap(orxCONST orxTEXTURE *_pstTexture)
{
  orxREGISTER orxBITMAP *pstBitmap = orxNULL;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Has bitmap? */
  if(orxStructure_TestFlags((orxTEXTURE *)_pstTexture, orxTEXTURE_KU32_FLAG_BITMAP) != orxFALSE)
  {
    /* External bitmap? */
    if(orxStructure_TestFlags((orxTEXTURE *)_pstTexture, orxTEXTURE_KU32_FLAG_EXTERNAL) != orxFALSE)
    {
        pstBitmap = (orxBITMAP *)(((orxTEXTURE *)_pstTexture->hData)->hData);
    }
    /* Internal bitmap */
    else
    {
        pstBitmap = (orxBITMAP *)_pstTexture->hData;
    }
  }

  /* Done! */
  return pstBitmap;
}

/***************************************************************************
 orxTexture_GetSize
 Gets size.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxTexture_GetSize(orxCONST orxTEXTURE *_pstTexture, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Has size? */
  if(orxStructure_TestFlags((orxTEXTURE *)_pstTexture, orxTEXTURE_KU32_FLAG_SIZE) != orxFALSE)
  {
    /* Gets size */
    *_pfWidth  = _pstTexture->fWidth;
    *_pfHeight = _pstTexture->fHeight;
  }
  else
  {
    /* !!! MSG !!! */

    /* No size */
    *_pfWidth  = *_pfHeight = orx2F(-1.0f);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets texture name
 * @param[in]   _pstTexture   Concerned texture
 * @return      Texture name / orxNULL
 */
orxSTRING orxFASTCALL orxTexture_GetName(orxCONST orxTEXTURE *_pstTexture)
{
  orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Updates result */
  zResult = _pstTexture->zDataName;

  /* Done! */
  return zResult;
}

/** Sets texture color
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _stColor        Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTexture_SetColor(orxTEXTURE *_pstTexture, orxRGBA _stColor)
{
  orxBITMAP  *pstBitmap;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTexture);

  /* Gets bitmap */
  pstBitmap = orxTexture_GetBitmap(_pstTexture);

  /* Valid? */
  if(pstBitmap != orxNULL)
  {
    /* Updates its color */
    eResult = orxDisplay_SetBitmapColor(pstBitmap, _stColor);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets screen texture
 * @return      Screen texture / orxNULL
 */
orxTEXTURE *orxTexture_GetScreenTexture()
{
  /* Checks */
  orxASSERT(sstTexture.u32Flags & orxTEXTURE_KU32_STATIC_FLAG_READY);

  /* Done! */
  return(sstTexture.pstScreen);
}

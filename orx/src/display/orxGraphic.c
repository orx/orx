/**
 * @file orxGraphic.c
 *
 * Graphic module
 *
 */

 /***************************************************************************
 orxGraphic.c
 Graphic module

 begin                : 08/12/2003
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


#include "display/orxGraphic.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "io/orxFileSystem.h"


/** Module flags
 */

#define orxGRAPHIC_KU32_STATIC_FLAG_NONE      0x00000000

#define orxGRAPHIC_KU32_STATIC_FLAG_READY     0x00000001


#define orxGRAPHIC_KU32_FLAG_INTERNAL         0x10000000  /**< Internal structure handling flag  */

#define orxGRAPHIC_KU32_MASK_ALL              0xFFFFFFFF  /**< All flags */


#define orxGRAPHIC_KC_MARKER_START            '$'
#define orxGRAPHIC_KC_MARKER_WIDTH            'w'
#define orxGRAPHIC_KC_MARKER_HEIGHT           'h'


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Graphic structure
 */
struct __orxGRAPHIC_t
{
  orxSTRUCTURE  stStructure;                /**< Public structure, first structure member : 16 */
  orxSTRUCTURE *pstData;                    /**< Data structure : 20 */
  orxVECTOR     vPivot;                     /**< Pivot : 36 */

  orxPAD(36)
};

/** Static structure
 */
typedef struct __orxGRAPHIC_STATIC_t
{
  orxU32 u32Flags;                          /**< Control flags : 4 */

} orxGRAPHIC_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxGRAPHIC_STATIC sstGraphic;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all graphics
 */
orxSTATIC orxINLINE orxVOID orxGraphic_DeleteAll()
{
  orxREGISTER orxGRAPHIC *pstGraphic;

  /* Gets first graphic */
  pstGraphic = (orxGRAPHIC *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);

  /* Non empty? */
  while(pstGraphic != orxNULL)
  {
    /* Deletes Graphic */
    orxGraphic_Delete(pstGraphic);

    /* Gets first Graphic */
    pstGraphic = (orxGRAPHIC *)orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC);
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Graphic module setup
 */
orxVOID orxGraphic_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_TEXTURE);

  return;
}

/** Inits the Graphic module
 */
orxSTATUS orxGraphic_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if((sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY) == orxGRAPHIC_KU32_STATIC_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstGraphic, sizeof(orxGRAPHIC_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(GRAPHIC, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstGraphic.u32Flags = orxGRAPHIC_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return eResult;
}

/** Exits from the Graphic module
 */
orxVOID orxGraphic_Exit()
{
  /* Initialized? */
  if(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY)
  {
    /* Deletes graphic list */
    orxGraphic_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_GRAPHIC);

    /* Updates flags */
    sstGraphic.u32Flags &= ~orxGRAPHIC_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty graphic
 * @param[in]   _u32Flags                     Graphic flags (2D / ...)
 * @return      Created orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxGraphic_Create(orxU32 _u32Flags)
{
  orxGRAPHIC *pstGraphic;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxGRAPHIC_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates graphic */
  pstGraphic = (orxGRAPHIC *)orxStructure_Create(orxSTRUCTURE_ID_GRAPHIC);

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_ALL);

    /* 2D? */
    if(orxFLAG_TEST(_u32Flags, orxGRAPHIC_KU32_FLAG_2D))
    {
      /* Updates flags */
      orxStructure_SetFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D, orxGRAPHIC_KU32_FLAG_NONE);
    }
  }

  /* Done! */
  return pstGraphic;
}

/** Creates a graphic from file
 * @param[in]   _zBitmapFileName              Bitmap to use as data
 * @param[in]   _u32Flags                     Graphic flags (2D / ...)
 * @ return orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxGraphic_CreateFromFile(orxCONST orxSTRING _zBitmapFileName, orxU32 _u32Flags)
{
  orxGRAPHIC *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxASSERT(_zBitmapFileName != orxNULL);
  orxASSERT((_u32Flags & orxGRAPHIC_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates graphic */
  pstResult = orxGraphic_Create(_u32Flags);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* 2D? */
    if(orxFLAG_TEST(_u32Flags, orxGRAPHIC_KU32_FLAG_2D))
    {
      orxS32      s32FirstMarkerIndex, s32Width, s32Height;
      orxTEXTURE *pstTexture = orxNULL;

      /* Gets marker index */
      s32FirstMarkerIndex = orxString_SearchCharIndex(_zBitmapFileName, orxGRAPHIC_KC_MARKER_START, 0);

      /* Use marker? */
      if(s32FirstMarkerIndex >= 0)
      {
        orxFILESYSTEM_INFO  stFileInfo;
        orxBOOL             bDone;
        orxCHAR             zBaseName[256];

        /* Checks */
        orxASSERT(s32FirstMarkerIndex < 255);

        /* Clears buffer */
        orxMemory_Zero(zBaseName, 256 * sizeof(orxCHAR));

        /* Gets base name */
        orxString_NCopy(zBaseName, _zBitmapFileName, s32FirstMarkerIndex);

        /* Adds wildcard */
        zBaseName[s32FirstMarkerIndex] = '*';

        /* For all matching file */
        for(bDone = (orxFileSystem_FindFirst(zBaseName, &stFileInfo) != orxSTATUS_FAILURE) ? orxFALSE : orxTRUE;
            bDone == orxFALSE;
            bDone = (orxFileSystem_FindNext(&stFileInfo) != orxSTATUS_FAILURE) ? orxFALSE : orxTRUE)
        {
          orxS32   *ps32Value;
          orxSTRING zRemaining;

          /* Height? */
          if(_zBitmapFileName[s32FirstMarkerIndex + 1] == orxGRAPHIC_KC_MARKER_HEIGHT)
          {
            /* Updates value pointer */
            ps32Value = &s32Height;
          }
          /* Width? */
          else if(_zBitmapFileName[s32FirstMarkerIndex + 1] == orxGRAPHIC_KC_MARKER_WIDTH)
          {
            /* Updates value pointer */
            ps32Value = &s32Width;
          }
          else
          {
            /* Updates value pointer */
            ps32Value = orxNULL;
          }

          /* Valid? */
          if(ps32Value != orxNULL)
          {
            /* Gets value */
            if(orxString_ToS32(stFileInfo.zFullName + s32FirstMarkerIndex, 10, ps32Value, &zRemaining) != orxSTATUS_FAILURE)
            {
              orxS32 s32SecondMarkerIndex;

              /* Gets second marker index */
              s32SecondMarkerIndex = orxString_SearchCharIndex(_zBitmapFileName, orxGRAPHIC_KC_MARKER_START, s32FirstMarkerIndex + 1);

              /* Valid? */
              if(s32SecondMarkerIndex >= 0)
              {
                /* Height? */
                if((_zBitmapFileName[s32SecondMarkerIndex + 1] == orxGRAPHIC_KC_MARKER_HEIGHT) && (ps32Value == &s32Width))
                {
                  /* Updates value pointer */
                  ps32Value = &s32Height;
                }
                /* Width? */
                else if((_zBitmapFileName[s32SecondMarkerIndex + 1] == orxGRAPHIC_KC_MARKER_WIDTH) && (ps32Value == &s32Height))
                {
                  /* Updates value pointer */
                  ps32Value = &s32Width;
                }
                else
                {
                  /* Updates value pointer */
                  ps32Value = orxNULL;
                }

                /* Valid? */
                if(ps32Value != orxNULL)
                {
                  /* Gets value */
                  if(orxString_ToS32(zRemaining + (s32SecondMarkerIndex - s32FirstMarkerIndex - 2), 10, ps32Value, &zRemaining) != orxSTATUS_FAILURE)
                  {
                    /* Loads texture */
                    pstTexture = orxTexture_CreateFromFile(stFileInfo.zFullName);

                    /* Valid? */
                    if(pstTexture != orxNULL)
                    {
                      /* Done! */
                      bDone = orxTRUE;
                    }
                  }
                }
              }
            }
          }
        }

        /* Closes search */
        orxFileSystem_FindClose(&stFileInfo);
      }
      else
      {
        /* Loads textures */
        pstTexture = orxTexture_CreateFromFile(_zBitmapFileName);
      }

      /* Valid texture? */
      if(pstTexture != orxNULL)
      {
        /* Links it */
        if(orxGraphic_SetData(pstResult, (orxSTRUCTURE *)pstTexture) != orxSTATUS_FAILURE)
        {
          /* Uses a pivot? */
          if(s32FirstMarkerIndex >= 0)
          {
            orxVECTOR vPivot;

            /* Sets pivot vector */
            orxVector_Set(&vPivot, orxS2F(s32Width), orxS2F(s32Height), orxFLOAT_0);

            /* Updates graphic */
            orxGraphic_SetPivot(pstResult, &vPivot);
          }

          /* Updates status flags */
          orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_INTERNAL | orxGRAPHIC_KU32_FLAG_2D, orxGRAPHIC_KU32_FLAG_NONE);
        }
        else
        {
          /* Deletes structures */
          orxTexture_Delete(pstTexture);
          orxGraphic_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }
      else
      {
        /* !!! MSG !!! */

        /* Deletes structures */
        orxGraphic_Delete(pstResult);
        pstResult = orxNULL;
      }
    }
    else
    {
      /* !!! MSG !!! */
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstResult;
}

/** Deletes a graphic
 * @param[in]   _pstGraphic     Graphic to delete
 */
orxSTATUS orxFASTCALL orxGraphic_Delete(orxGRAPHIC *_pstGraphic)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstGraphic) == 0)
  {
    /* Cleans data */
    orxGraphic_SetData(_pstGraphic, orxNULL);

    /* Deletes structure */
    orxStructure_Delete(_pstGraphic);
  }
  else
  {
    /* !!! MSG !!! */

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets graphic data
 * @param[in]   _pstGraphic     Graphic concerned
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetData(orxGRAPHIC *_pstGraphic, orxSTRUCTURE *_pstData)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Had previously data? */
  if(_pstGraphic->pstData != orxNULL)
  {
    /* Updates structure reference counter */
    orxStructure_DecreaseCounter(_pstGraphic->pstData);

    /* Internally handled? */
    if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_INTERNAL))
    {
      /* 2D data ? */
      if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D))
      {
        /* Deletes it */
        orxTexture_Delete(orxSTRUCTURE_GET_POINTER(_pstGraphic->pstData, TEXTURE));
      }
      else
      {
        /* !!! MSG !!! */

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }

    /* Cleans reference */
    _pstGraphic->pstData = orxNULL;
  }

  /* Valid & sets new data? */
  if((eResult != orxSTATUS_FAILURE) && (_pstData != orxNULL))
  {
    /* Stores it */
    _pstGraphic->pstData = _pstData;

    /* Updates structure reference counter */
    orxStructure_IncreaseCounter(_pstData);

    /* Is data a texture? */
    if(orxSTRUCTURE_GET_POINTER(_pstData, TEXTURE) != orxNULL)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D, orxGRAPHIC_KU32_STATIC_FLAG_NONE);
    }
    else
    {
      /* !!! MSG !!! */

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* !!! TODO : Update internal flags given data type */
  }

  /* Done! */
  return eResult;
}

/** Gets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      OrxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxGraphic_GetData(orxCONST orxGRAPHIC *_pstGraphic)
{
  orxSTRUCTURE *pstStructure;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  pstStructure = _pstGraphic->pstData;

  /* Done! */
  return pstStructure;
}

/** Sets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvPivot        Pivot to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetPivot(orxGRAPHIC *_pstGraphic, orxCONST orxVECTOR *_pvPivot)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvPivot != orxNULL);

  /* Stores pivot */
  orxVector_Copy(&(_pstGraphic->vPivot), _pvPivot);

  /* Done! */
  return eResult;
}

/** Gets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvPivot        Graphic pivot
 * @return      orxPIVOT / orxNULL
 */
orxVECTOR *orxFASTCALL orxGraphic_GetPivot(orxCONST orxGRAPHIC *_pstGraphic, orxVECTOR *_pvPivot)
{
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvPivot != orxNULL);

  /* Copies pivot */
  orxVector_Copy(_pvPivot, &(_pstGraphic->vPivot));

  /* Done! */
  return _pvPivot;
}

/** Gets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pfWidth        Object's width
 * @param[out]  _pfHeight       Object's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_GetSize(orxCONST orxGRAPHIC *_pstGraphic, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pfWidth != orxNULL);
  orxASSERT(_pfHeight != orxNULL);

  /* Valid 2D data? */
  if(orxStructure_TestFlags((orxGRAPHIC *)_pstGraphic, orxGRAPHIC_KU32_FLAG_2D) != orxFALSE)
  {
    /* Gets its size */
    eResult = orxTexture_GetSize(orxSTRUCTURE_GET_POINTER(_pstGraphic->pstData, TEXTURE), _pfWidth, _pfHeight);
  }
  else
  {
    /* No size */
    *_pfWidth  = *_pfHeight = orx2F(-1.0f);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _stColor        Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetColor(orxGRAPHIC *_pstGraphic, orxRGBA _stColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Valid 2D data? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D) != orxFALSE)
  {
    /* Sets color */
    eResult = orxTexture_SetColor(orxSTRUCTURE_GET_POINTER(_pstGraphic->pstData, TEXTURE), _stColor);
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

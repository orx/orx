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
#include "core/orxConfig.h"


/** Module flags
 */

#define orxGRAPHIC_KU32_STATIC_FLAG_NONE      0x00000000  /**< No flags  */

#define orxGRAPHIC_KU32_STATIC_FLAG_READY     0x00000001  /**< Ready flag  */


/** Graphic flags
 */
#define orxGRAPHIC_KU32_FLAG_INTERNAL         0x10000000  /**< Internal structure handling flag  */
#define orxGRAPHIC_KU32_FLAG_HAS_COLOR        0x20000000  /**< Has color flag  */

#define orxGRAPHIC_KU32_MASK_ALL              0xFFFFFFFF  /**< All flags */


/** Misc defines
 */
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_NAME     "Texture"
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_TL       "TextureTL"
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_BR       "TextureBR"
#define orxGRAPHIC_KZ_CONFIG_PIVOT            "Pivot"
#define orxGRAPHIC_KZ_CONFIG_COLOR            "Color"
#define orxGRAPHIC_KZ_CONFIG_ALPHA            "Alpha"
#define orxGRAPHIC_KZ_CONFIG_FLIP             "Flip"

#define orxGRAPHIC_KZ_CENTERED_PIVOT          "centered"
#define orxGRAPHIC_KZ_X                       "x"
#define orxGRAPHIC_KZ_Y                       "y"
#define orxGRAPHIC_KZ_BOTH                    "both"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Graphic structure
 */
struct __orxGRAPHIC_t
{
  orxSTRUCTURE  stStructure;                /**< Public structure, first structure member : 16 */
  orxSTRUCTURE *pstData;                    /**< Data structure : 20 */
  orxVECTOR     vPivot;                     /**< Pivot : 32 */
  orxCOLOR      stColor;                    /**< Color : 48 */
  orxFLOAT      fTop;                       /**< Top coordinate : 52 */
  orxFLOAT      fLeft;                      /**< Left coordinate : 56 */
  orxFLOAT      fWidth;                     /**< Width : 60 */
  orxFLOAT      fHeight;                    /**< Height : 64 */

  orxPAD(64)
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
  pstGraphic = orxGRAPHIC(orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC));

  /* Non empty? */
  while(pstGraphic != orxNULL)
  {
    /* Deletes Graphic */
    orxGraphic_Delete(pstGraphic);

    /* Gets first Graphic */
    pstGraphic = orxGRAPHIC(orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC));
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
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_CONFIG);
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
  pstGraphic = orxGRAPHIC(orxStructure_Create(orxSTRUCTURE_ID_GRAPHIC));

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_ALL);

    /* 2D? */
    if(orxFLAG_TEST(_u32Flags, orxGRAPHIC_KU32_FLAG_2D))
    {
      /* Clears its color */
      orxGraphic_ClearColor(pstGraphic);

      /* Updates flags */
      orxStructure_SetFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D, orxGRAPHIC_KU32_FLAG_NONE);
    }
  }

  /* Done! */
  return pstGraphic;
}

/** Creates a graphic from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxGraphic_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxGRAPHIC *pstResult;
  orxSTRING   zPreviousSection;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (*_zConfigID != *orxSTRING_EMPTY));

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates graphic */
    pstResult = orxGraphic_Create(orxGRAPHIC_KU32_FLAG_2D);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxSTRING zTextureName;

      /* Gets texture name */
      zTextureName = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_TEXTURE_NAME);

      /* Valid? */
      if((zTextureName != orxNULL) && (*zTextureName != *orxSTRING_EMPTY))
      {
        orxTEXTURE *pstTexture;

        /* Creates textures */
        pstTexture = orxTexture_CreateFromFile(zTextureName);

        /* Valid? */
        if(pstTexture != orxNULL)
        {
          /* Links it */
          if(orxGraphic_SetData(pstResult, (orxSTRUCTURE *)pstTexture) != orxSTATUS_FAILURE)
          {
            orxVECTOR vPivot;
            orxSTRING zFlipping;
            orxU32    u32Flags;

            /* Inits default 2D flags */
            u32Flags = orxGRAPHIC_KU32_FLAG_INTERNAL | orxGRAPHIC_KU32_FLAG_2D;

            /* Has corners? */
            if((orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_TEXTURE_TL) != orxFALSE)
            && (orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_TEXTURE_BR) != orxFALSE))
            {
              orxAABOX stTextureBox;

              /* Gets both corners */
              orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_TL, &(stTextureBox.vTL));
              orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_BR, &(stTextureBox.vBR));

              /* Stores them */
              pstResult->fLeft    = stTextureBox.vTL.fX;
              pstResult->fTop     = stTextureBox.vTL.fY;
              pstResult->fWidth   = stTextureBox.vBR.fX - stTextureBox.vTL.fX;
              pstResult->fHeight  = stTextureBox.vBR.fY - stTextureBox.vTL.fY;
            }
            else
            {
              /* Inits full coordinates */
              pstResult->fLeft    = orxFLOAT_0;
              pstResult->fTop     = orxFLOAT_0;
              pstResult->fWidth   = orxTexture_GetWidth(pstTexture);
              pstResult->fHeight  = orxTexture_GetHeight(pstTexture);
            }

            /* Uses centered pivot? */
            if(orxString_Compare(orxString_LowerCase(orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_PIVOT)), orxGRAPHIC_KZ_CENTERED_PIVOT) == 0)
            {
              orxFLOAT fWidth, fHeight;

              /* Gets object size */
              if(orxGraphic_GetSize(pstResult, &fWidth, &fHeight) != orxSTATUS_FAILURE)
              {
                /* Inits pivot */
                orxVector_Set(&vPivot, orx2F(0.5f) * fWidth, orx2F(0.5f) * fHeight, orxFLOAT_0);
              }
              else
              {
                orxVector_Copy(&vPivot, &orxVECTOR_0);
              }
            }
            /* Gets pivot value */
            else if(orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_PIVOT, &vPivot) == orxNULL)
            {
              orxVector_Copy(&vPivot, &orxVECTOR_0);
            }

            /* Updates its pivot */
            orxGraphic_SetPivot(pstResult, &vPivot);

            /* Gets flipping value */
            zFlipping = orxString_LowerCase(orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_FLIP));

            /* X flipping? */
            if(orxString_Compare(zFlipping, orxGRAPHIC_KZ_X) == 0)
            {
              /* Updates frame flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_FLIP_X;
            }
            /* Y flipping? */
            else if(orxString_Compare(zFlipping, orxGRAPHIC_KZ_Y) == 0)
            {
              /* Updates frame flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_FLIP_Y;
            }
            /* Both flipping? */
            else if(orxString_Compare(zFlipping, orxGRAPHIC_KZ_BOTH) == 0)
            {
              /* Updates frame flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_FLIP_X | orxGRAPHIC_KU32_FLAG_FLIP_Y;
            }

            /* Has color? */
            if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_COLOR) != orxFALSE)
            {
              orxVECTOR vColor;

              /* Gets its value */
              orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_COLOR, &vColor);

              /* Applies it */
              orxColor_SetRGB(&(pstResult->stColor), &vColor);

              /* Updates status */
              orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);
            }

            /* Has alpha? */
            if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_ALPHA) != orxFALSE)
            {
              /* Applies it */
              orxColor_SetAlpha(&(pstResult->stColor), orxConfig_GetFloat(orxGRAPHIC_KZ_CONFIG_ALPHA));

              /* Updates status */
              orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);
            }

            /* Updates status flags */
            orxStructure_SetFlags(pstResult, u32Flags, orxGRAPHIC_KU32_FLAG_NONE);
          }
          else
          {
            /* !!! MSG !!! */

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

          /* Updates result */
          pstResult = orxNULL;
        }
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    pstResult = orxNULL;
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
        orxTexture_Delete(orxTEXTURE(_pstGraphic->pstData));
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
    if(orxTEXTURE(_pstData) != orxNULL)
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
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D) != orxFALSE)
  {
    /* Gets its size */
    *_pfWidth   = _pstGraphic->fWidth;
    *_pfHeight  = _pstGraphic->fHeight;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
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
 * @param[in]   _pstColor       Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetColor(orxGRAPHIC *_pstGraphic, orxCONST orxCOLOR *_pstColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pstColor != orxNULL);

  /* Stores color */
  orxColor_Copy(&(_pstGraphic->stColor), _pstColor);

  /* Updates its flag */
  orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);

  /* Done! */
  return eResult;
}

/** Clears graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_ClearColor(orxGRAPHIC *_pstGraphic)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates its flag */
  orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_FLAG_HAS_COLOR);

  /* Restores default color */
  orxColor_SetRGBA(&(_pstGraphic->stColor), orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF));

  /* Done! */
  return eResult;
}

/** Graphic has color accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxGraphic_HasColor(orxCONST orxGRAPHIC *_pstGraphic)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  bResult = orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_COLOR);

  /* Done! */
  return bResult;
}

/** Gets graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pstColor       Object's color
 * @return      orxCOLOR / orxNULL
 */
orxCOLOR *orxFASTCALL orxGraphic_GetColor(orxCONST orxGRAPHIC *_pstGraphic, orxCOLOR *_pstColor)
{
  orxCOLOR *pstResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pstColor != orxNULL);

  /* Has color? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_COLOR))
  {
    /* Copies color */
    orxColor_Copy(_pstColor, &(_pstGraphic->stColor));

    /* Updates result */
    pstResult = _pstColor;
  }
  else
  {
    /* !!! MSG !!! */

    /* Clears result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets graphic top
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      Top coordinate
 */
orxFLOAT orxFASTCALL orxGraphic_GetTop(orxCONST orxGRAPHIC *_pstGraphic)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  fResult = _pstGraphic->fTop;

  /* Done! */
  return fResult;
}

/** Gets graphic left
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      Left coordinate
 */
orxFLOAT orxFASTCALL orxGraphic_GetLeft(orxCONST orxGRAPHIC *_pstGraphic)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  fResult = _pstGraphic->fLeft;

  /* Done! */
  return fResult;
}

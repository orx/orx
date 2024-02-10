/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxGraphic.c
 * @date 08/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "display/orxGraphic.h"

#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxLocale.h"
#include "debug/orxDebug.h"
#include "display/orxText.h"
#include "display/orxTexture.h"
#include "memory/orxMemory.h"
#include "object/orxObject.h"


/** Module flags
 */
#define orxGRAPHIC_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags  */

#define orxGRAPHIC_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag  */


/** Graphic flags
 */
#define orxGRAPHIC_KU32_FLAG_INTERNAL             0x10000000  /**< Internal structure handling flag  */
#define orxGRAPHIC_KU32_FLAG_HAS_COLOR            0x20000000  /**< Has color flag  */
#define orxGRAPHIC_KU32_FLAG_HAS_BLEND_MODE       0x40000000  /**< Has color flag  */
#define orxGRAPHIC_KU32_FLAG_HAS_PIVOT            0x80000000  /**< Has pivot flag  */
#define orxGRAPHIC_KU32_FLAG_RELATIVE_PIVOT       0x01000000  /**< Relative pivot flag */
#define orxGRAPHIC_KU32_FLAG_SMOOTHING_ON         0x02000000  /**< Smoothing on flag  */
#define orxGRAPHIC_KU32_FLAG_SMOOTHING_OFF        0x04000000  /**< Smoothing off flag  */
#define orxGRAPHIC_KU32_FLAG_KEEP_IN_CACHE        0x08000000  /**< Keep in cache flag */

#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_NONE      0x00000000  /**< Blend mode no flags */

#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_ALPHA     0x00100000  /**< Blend mode alpha flag */
#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_MULTIPLY  0x00200000  /**< Blend mode multiply flag */
#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_ADD       0x00400000  /**< Blend mode add flag */
#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_PREMUL    0x00800000  /**< Blend mode premul flag */

#define orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL       0x00F00000  /**< Blend mode mask */

#define orxGRAPHIC_KU32_MASK_ALL                  0xFFFFFFFF  /**< All flags */


/** Misc defines
 */
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_CORNER       "TextureCorner" /**< Kept for retro-compatibility reason */

#define orxGRAPHIC_KZ_CENTERED_PIVOT              "center"
#define orxGRAPHIC_KZ_TRUNCATE_PIVOT              "truncate"
#define orxGRAPHIC_KZ_ROUND_PIVOT                 "round"
#define orxGRAPHIC_KZ_TOP_PIVOT                   "top"
#define orxGRAPHIC_KZ_LEFT_PIVOT                  "left"
#define orxGRAPHIC_KZ_BOTTOM_PIVOT                "bottom"
#define orxGRAPHIC_KZ_RIGHT_PIVOT                 "right"
#define orxGRAPHIC_KZ_X                           "x"
#define orxGRAPHIC_KZ_Y                           "y"
#define orxGRAPHIC_KZ_BOTH                        "both"

#define orxGRAPHIC_KU32_BANK_SIZE                 1024

#define orxGRAPHIC_KC_LOCALE_MARKER               '$'


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Graphic structure
 */
struct __orxGRAPHIC_t
{
  orxSTRUCTURE    stStructure;              /**< Public structure, first structure member : 32 */
  orxSTRUCTURE   *pstData;                  /**< Data structure : 20 */
  orxSTRINGID     stLocaleNameID;           /**< Locale name ID : 28 */
  orxVECTOR       vPivot;                   /**< Pivot : 40 */
  orxCOLOR        stColor;                  /**< Color : 56 */
  orxFLOAT        fTop;                     /**< Top coordinate : 60 */
  orxFLOAT        fLeft;                    /**< Left coordinate : 64 */
  orxFLOAT        fWidth;                   /**< Width : 68 */
  orxFLOAT        fHeight;                  /**< Height : 72 */
  orxFLOAT        fRepeatX;                 /**< X-axis repeat count : 76 */
  orxFLOAT        fRepeatY;                 /**< Y-axis repeat count : 80 */
  const orxSTRING zReference;               /**< Reference : 84 */
  const orxSTRING zDataReference;           /**< Data reference : 88 */
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
static orxGRAPHIC_STATIC sstGraphic;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Sets graphic data
 * @param[in]   _pstGraphic     Graphic concerned
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @param[in]   _bInternal      Internal call
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxINLINE orxSTATUS orxGraphic_SetDataInternal(orxGRAPHIC *_pstGraphic, orxSTRUCTURE *_pstData, orxBOOL _bInternal)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Had previous data? */
  if(_pstGraphic->pstData != orxNULL)
  {
    /* Internally handled? */
    if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_INTERNAL))
    {
      /* Removes its owner */
      orxStructure_SetOwner(_pstGraphic->pstData, orxNULL);

      /* 2D data? */
      if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D))
      {
        /* Deletes it */
        orxTexture_Delete(orxTEXTURE(_pstGraphic->pstData));
      }
      /* Text data? */
      else if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT))
      {
        /* Deletes it */
        orxText_Delete(orxTEXT(_pstGraphic->pstData));
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Non-2d (texture/text) graphics not supported yet.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }

      /* Updates flags */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_FLAG_INTERNAL);
    }
    else
    {
      /* Updates structure reference count */
      orxStructure_DecreaseCount(_pstGraphic->pstData);
    }

    /* Cleans reference */
    _pstGraphic->pstData = orxNULL;
  }

  /* Valid & sets new data? */
  if((eResult != orxSTATUS_FAILURE) && (_pstData != orxNULL))
  {
    /* Is data a texture? */
    if(orxTEXTURE(_pstData) != orxNULL)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D, orxGRAPHIC_KU32_MASK_TYPE);
    }
    /* Is data a text? */
    else if(orxTEXT(_pstData) != orxNULL)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT, orxGRAPHIC_KU32_MASK_TYPE);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Data given is not a texture nor a text.");

      /* Updates flags */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_TYPE);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Stores data */
      _pstGraphic->pstData = _pstData;

      /* Internal call? */
      if(_bInternal != orxFALSE)
      {
        /* Updates its owner */
        orxStructure_SetOwner(_pstData, _pstGraphic);

        /* Updates flags */
        orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_INTERNAL, orxGRAPHIC_KU32_FLAG_NONE);
      }
      else
      {
        /* Updates structure reference count */
        orxStructure_IncreaseCount(_pstData);
      }
    }
  }
  else
  {
    /* Updates flags */
    orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_TYPE);
  }

  /* Done! */
  return eResult;
}

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxGraphic_EventHandler(const orxEVENT *_pstEvent)
{
  orxGRAPHIC *pstGraphic;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((_pstEvent->eType == orxEVENT_TYPE_LOCALE) || (_pstEvent->eType == orxEVENT_TYPE_OBJECT));

  /* Locale? */
  if(_pstEvent->eType == orxEVENT_TYPE_LOCALE)
  {
    orxLOCALE_EVENT_PAYLOAD  *pstPayload;
    orxBOOL                   bTextureGroup, bTextGroup;

    /* Checks */
    orxASSERT(_pstEvent->eID == orxLOCALE_EVENT_SELECT_LANGUAGE);

    /* Gets its payload */
    pstPayload = (orxLOCALE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Updates status */
    bTextureGroup = ((pstPayload->zGroup == orxNULL) || (orxString_Compare(pstPayload->zGroup, orxTEXTURE_KZ_LOCALE_GROUP) == 0)) ? orxTRUE : orxFALSE;
    bTextGroup    = ((pstPayload->zGroup == orxNULL) || (orxString_Compare(pstPayload->zGroup, orxTEXT_KZ_LOCALE_GROUP) == 0))    ? orxTRUE : orxFALSE;

    /* Is it texture or text group? */
    if((bTextureGroup != orxFALSE) || (bTextGroup != orxFALSE))
    {
      orxGRAPHIC *pstGraphic;

      /* For all graphics */
      for(pstGraphic = orxGRAPHIC(orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC));
          pstGraphic != orxNULL;
          pstGraphic = orxGRAPHIC(orxStructure_GetNext(pstGraphic)))
      {
        /* Text group and text data? */
        if((bTextGroup != orxFALSE) && (orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT)))
        {
          /* Updates graphic's size */
          orxGraphic_UpdateSize(pstGraphic);
        }
        /* Texture group (including disabled stasis ones)? */
        else if(bTextureGroup != orxFALSE)
        {
          /* Has locale name ID? */
          if(pstGraphic->stLocaleNameID != 0)
          {
            const orxSTRING zName;
            orxTEXTURE     *pstTexture;

            /* Retrieves name */
            zName = orxLocale_GetString(orxString_GetFromID(pstGraphic->stLocaleNameID), orxTEXTURE_KZ_LOCALE_GROUP);

            /* Valid? */
            if(*zName != orxCHAR_NULL)
            {
              /* Has data reference? */
              if(pstGraphic->zDataReference != orxNULL)
              {
                /* Updates it */
                pstGraphic->zDataReference = orxString_Store(zName);
              }

              /* Has 2D data? */
              if(orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_2D))
              {
                /* Loads texture */
                pstTexture = orxTexture_Load(zName, orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_KEEP_IN_CACHE) ? orxTRUE : orxFALSE);

                /* Valid? */
                if(pstTexture != orxNULL)
                {
                  /* Updates data */
                  orxGraphic_SetDataInternal(pstGraphic, (orxSTRUCTURE *)pstTexture, orxTRUE);
                }
              }
            }
          }
        }
      }
    }
  }
  /* Object */
  else
  {
    /* Gets its graphic */
    pstGraphic = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstEvent->hSender), GRAPHIC);

    /* Has data reference? */
    if((pstGraphic != orxNULL) && (pstGraphic->zDataReference != orxNULL))
    {
      /* Enable? */
      if(_pstEvent->eID == orxOBJECT_EVENT_ENABLE)
      {
        /* Checks */
        orxASSERT(pstGraphic->pstData == orxNULL);

        /* Updates data */
        orxGraphic_SetDataInternal(pstGraphic, (orxSTRUCTURE *)orxTexture_Load(pstGraphic->zDataReference, orxFALSE), orxTRUE);
      }
      /* Disable */
      else
      {
        /* Checks */
        orxASSERT(pstGraphic->pstData != orxNULL);

        /* Updates data */
        orxGraphic_SetDataInternal(pstGraphic, orxNULL, orxTRUE);
      }
    }
  }

  /* Done! */
  return eResult;
}


/** Deletes all graphics
 */
static orxINLINE void orxGraphic_DeleteAll()
{
  orxGRAPHIC *pstGraphic;

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
void orxFASTCALL orxGraphic_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_TEXT);
  orxModule_AddDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_TEXTURE);
  orxModule_AddOptionalDependency(orxMODULE_ID_GRAPHIC, orxMODULE_ID_LOCALE);

  return;
}

/** Inits the Graphic module
 */
orxSTATUS orxFASTCALL orxGraphic_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if((sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY) == orxGRAPHIC_KU32_STATIC_FLAG_NONE)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstGraphic, sizeof(orxGRAPHIC_STATIC));

    /* Registers event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_LOCALE, orxGraphic_EventHandler);
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxGraphic_EventHandler) : orxSTATUS_FAILURE;

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Filters relevant event IDs */
      orxEvent_SetHandlerIDFlags(orxGraphic_EventHandler, orxEVENT_TYPE_LOCALE, orxNULL, orxEVENT_GET_FLAG(orxLOCALE_EVENT_SELECT_LANGUAGE), orxEVENT_KU32_MASK_ID_ALL);
      orxEvent_SetHandlerIDFlags(orxGraphic_EventHandler, orxEVENT_TYPE_OBJECT, orxNULL, orxEVENT_GET_FLAG(orxOBJECT_EVENT_ENABLE) | orxEVENT_GET_FLAG(orxOBJECT_EVENT_DISABLE), orxEVENT_KU32_MASK_ID_ALL);

      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(GRAPHIC, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxGRAPHIC_KU32_BANK_SIZE, orxNULL);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Inits Flags */
        sstGraphic.u32Flags = orxGRAPHIC_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_LOCALE, orxGraphic_EventHandler);
        orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxGraphic_EventHandler);
      }
    }
    else
    {
      /* Removes event handler */
      orxEvent_RemoveHandler(orxEVENT_TYPE_LOCALE, orxGraphic_EventHandler);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to initialize graphic module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Initializing graphic module failed.");

    /* Updates Flags */
    sstGraphic.u32Flags &= ~orxGRAPHIC_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Graphic module
 */
void orxFASTCALL orxGraphic_Exit()
{
  /* Initialized? */
  if(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY)
  {
    /* Deletes graphic list */
    orxGraphic_DeleteAll();

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_LOCALE, orxGraphic_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxGraphic_EventHandler);

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_GRAPHIC);

    /* Updates flags */
    sstGraphic.u32Flags &= ~orxGRAPHIC_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Tried to exit graphic module when it wasn't initialized.");
  }

  return;
}

/** Gets alignment flags from literals
 * @param[in]   _zAlign         Align literals
 * @ return Align flags
 */
orxU32 orxFASTCALL orxGraphic_GetAlignFlags(const orxSTRING _zAlign)
{
  orxCHAR acBuffer[64];
  orxU32  u32Result = orxGRAPHIC_KU32_FLAG_ALIGN_CENTER;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxASSERT(_zAlign != orxNULL);

  /* Gets lower case value */
  acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
  orxString_LowerCase(orxString_NCopy(acBuffer, _zAlign, sizeof(acBuffer) - 1));

  /* Valid? */
  if(*acBuffer != orxCHAR_NULL)
  {
    /* Left? */
    if(orxString_SearchString(acBuffer, orxGRAPHIC_KZ_LEFT_PIVOT) != orxNULL)
    {
      /* Updates alignment flags */
      u32Result |= orxGRAPHIC_KU32_FLAG_ALIGN_LEFT;
    }
    /* Right? */
    else if(orxString_SearchString(acBuffer, orxGRAPHIC_KZ_RIGHT_PIVOT) != orxNULL)
    {
      /* Updates alignment flags */
      u32Result |= orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT;
    }

    /* Top? */
    if(orxString_SearchString(acBuffer, orxGRAPHIC_KZ_TOP_PIVOT) != orxNULL)
    {
      /* Updates alignment flags */
      u32Result |= orxGRAPHIC_KU32_FLAG_ALIGN_TOP;
    }
    /* Bottom? */
    else if(orxString_SearchString(acBuffer, orxGRAPHIC_KZ_BOTTOM_PIVOT) != orxNULL)
    {
      /* Updates alignment flags */
      u32Result |= orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM;
    }

    /* Truncate? */
    if(orxString_SearchString(acBuffer, orxGRAPHIC_KZ_TRUNCATE_PIVOT) != orxNULL)
    {
      /* Updates alignment flags */
      u32Result |= orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE;
    }
    /* Round? */
    else if(orxString_SearchString(acBuffer, orxGRAPHIC_KZ_ROUND_PIVOT) != orxNULL)
    {
      /* Updates alignment flags */
      u32Result |= orxGRAPHIC_KU32_FLAG_ALIGN_ROUND;
    }
  }

  /* Done! */
  return u32Result;
}

/** Aligns a vector inside a box using flags
 * @param[in]   _u32AlignFlags  Align flags
 * @param[in]   _pstBox         Concerned box
 * @param[out]  _pvValue        Storage for the resulting aligned vector
 * @return orxVECTOR
 */
orxVECTOR *orxFASTCALL orxGraphic_AlignVector(orxU32 _u32AlignFlags, const orxAABOX *_pstBox, orxVECTOR *_pvValue)
{
  orxVECTOR *pvResult = _pvValue;

  /* Align left? */
  if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_LEFT))
  {
    /* Updates x position */
    pvResult->fX = _pstBox->vTL.fX;
  }
  /* Align right? */
  else if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT))
  {
    /* Updates x position */
    pvResult->fX = _pstBox->vBR.fX;
  }
  /* Align center */
  else
  {
    /* Updates x position */
    pvResult->fX = orx2F(0.5f) * (_pstBox->vTL.fX + _pstBox->vBR.fX);
  }

  /* Align top? */
  if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_TOP))
  {
    /* Updates y position */
    pvResult->fY = _pstBox->vTL.fY;
  }
  /* Align bottom? */
  else if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM))
  {
    /* Updates y position */
    pvResult->fY = _pstBox->vBR.fY;
  }
  /* Align center */
  else
  {
    /* Updates y position */
    pvResult->fY = orx2F(0.5f) * (_pstBox->vTL.fY + _pstBox->vBR.fY);
  }

  /* Truncate? */
  if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE))
  {
    /* Updates position */
    orxVector_Floor(pvResult, pvResult);
  }
  /* Round? */
  else if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_ROUND))
  {
    /* Updates position */
    orxVector_Round(pvResult, pvResult);
  }

  /* Clears Z component */
  pvResult->fZ = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

/** Creates an empty graphic
 * @return      Created orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxGraphic_Create()
{
  orxGRAPHIC *pstGraphic;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);

  /* Creates graphic */
  pstGraphic = orxGRAPHIC(orxStructure_Create(orxSTRUCTURE_ID_GRAPHIC));

  /* Valid? */
  if(pstGraphic != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_ALL);

    /* Clears its color */
    orxGraphic_ClearColor(pstGraphic);

    /* Clears its blend mode */
    orxGraphic_ClearBlendMode(pstGraphic);

    /* Sets its repeat value to default */
    orxGraphic_SetRepeat(pstGraphic, orxFLOAT_1, orxFLOAT_1);

    /* Increases count */
    orxStructure_IncreaseCount(pstGraphic);
  }

  /* Done! */
  return pstGraphic;
}

/** Creates a graphic from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxGraphic_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxGRAPHIC *pstResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    orxU32 u32Flags = orxGRAPHIC_KU32_FLAG_NONE;

    /* Creates graphic */
    pstResult = orxGraphic_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      const orxSTRING zName;

      /* Gets texture name */
      zName = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_TEXTURE_NAME);

      /* Valid? */
      if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
      {
        orxTEXTURE *pstTexture;

        /* Begins with locale marker? */
        if(*zName == orxGRAPHIC_KC_LOCALE_MARKER)
        {
          /* Updates name */
          zName = zName + 1;

          /* Using locale? */
          if(*zName != orxGRAPHIC_KC_LOCALE_MARKER)
          {
            /* Stores its locale name ID */
            pstResult->stLocaleNameID = orxString_GetID(zName);

            /* Gets its locale value */
            zName = orxLocale_GetString(zName, orxTEXTURE_KZ_LOCALE_GROUP);
          }
        }

        /* Should keep in cache? */
        if(orxConfig_GetBool(orxGRAPHIC_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
        {
          /* Updates status */
          u32Flags |= orxGRAPHIC_KU32_FLAG_KEEP_IN_CACHE;
        }

        /* Loads texture */
        pstTexture = orxTexture_Load(zName, orxFLAG_TEST(u32Flags, orxGRAPHIC_KU32_FLAG_KEEP_IN_CACHE) ? orxTRUE : orxFALSE);

        /* Valid? */
        if(pstTexture != orxNULL)
        {
          /* Stores its data reference */
          pstResult->zDataReference = (orxConfig_GetBool(orxGRAPHIC_KZ_CONFIG_STASIS) != orxFALSE) ? orxString_Store(zName) : orxNULL;

          /* Links it */
          if(orxGraphic_SetDataInternal(pstResult, (orxSTRUCTURE *)pstTexture, orxTRUE) != orxSTATUS_FAILURE)
          {
            orxVECTOR vValue;

            /* Updates size */
            orxGraphic_UpdateSize(pstResult);

            /* Has origin / corner? */
            if((orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_ORIGIN, &vValue) != orxNULL)
            || (orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_CORNER, &vValue) != orxNULL))
            {
              /* Applies it */
              pstResult->fLeft    = vValue.fX;
              pstResult->fTop     = vValue.fY;

              /* Updates size */
              pstResult->fWidth   = orxMAX(orxFLOAT_0, pstResult->fWidth - vValue.fX);
              pstResult->fHeight  = orxMAX(orxFLOAT_0, pstResult->fHeight - vValue.fY);
            }

            /* Has size? */
            if(orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_SIZE, &vValue) != orxNULL)
            {
              /* Applies it */
              pstResult->fWidth   = vValue.fX;
              pstResult->fHeight  = vValue.fY;
            }
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link texture <%s> data to graphic <%s>.", zName, _zConfigID);

            /* Deletes structures */
            orxTexture_Delete(pstTexture);
          }
        }
      }

      /* Still no data? */
      if(pstResult->pstData == orxNULL)
      {
        /* Gets text name */
        zName = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_TEXT_NAME);

        /* Valid? */
        if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
        {
          orxTEXT *pstText;

          /* Creates text */
          pstText = orxText_CreateFromConfig(zName);

          /* Valid? */
          if(pstText != orxNULL)
          {
            /* Links it */
            if(orxGraphic_SetDataInternal(pstResult, (orxSTRUCTURE *)pstText, orxTRUE) != orxSTATUS_FAILURE)
            {
              /* Updates size */
              orxGraphic_UpdateSize(pstResult);
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link text <%s> data to graphic <%s>.", zName, _zConfigID);

              /* Deletes structures */
              orxText_Delete(pstText);
            }
          }
        }
      }

      /* Has data? */
      if(pstResult->pstData != orxNULL)
      {
        const orxSTRING zFlipping;
        orxVECTOR       vPivot;

        /* Gets pivot value */
        if(orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_PIVOT, &vPivot) != orxNULL)
        {
          /* Updates it */
          orxGraphic_SetPivot(pstResult, &vPivot);
        }
        /* Has relative pivot point? */
        else if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_PIVOT) != orxFALSE)
        {
          const orxSTRING zRelativePivot;

          /* Gets it */
          zRelativePivot = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_PIVOT);

          /* Valid? */
          if(*zRelativePivot != orxCHAR_NULL)
          {
            /* Applies it */
            orxGraphic_SetRelativePivot(pstResult, orxGraphic_GetAlignFlags(zRelativePivot));
          }
        }

        /* Gets flipping value */
        zFlipping = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_FLIP);

        /* X flipping? */
        if(orxString_ICompare(zFlipping, orxGRAPHIC_KZ_X) == 0)
        {
          /* Updates frame flags */
          u32Flags |= orxGRAPHIC_KU32_FLAG_FLIP_X;
        }
        /* Y flipping? */
        else if(orxString_ICompare(zFlipping, orxGRAPHIC_KZ_Y) == 0)
        {
          /* Updates frame flags */
          u32Flags |= orxGRAPHIC_KU32_FLAG_FLIP_Y;
        }
        /* Both flipping? */
        else if(orxString_ICompare(zFlipping, orxGRAPHIC_KZ_BOTH) == 0)
        {
          /* Updates frame flags */
          u32Flags |= orxGRAPHIC_KU32_FLAG_FLIP_X | orxGRAPHIC_KU32_FLAG_FLIP_Y;
        }

        /* Has color? */
        if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_COLOR) != orxFALSE)
        {
          /* Gets it */
          if(orxConfig_GetColorVector(orxGRAPHIC_KZ_CONFIG_COLOR, orxCOLORSPACE_COMPONENT, &(pstResult->stColor.vRGB)) != orxNULL)
          {
            /* Normalizes it */
            orxVector_Mulf(&(pstResult->stColor.vRGB), &(pstResult->stColor.vRGB), orxCOLOR_NORMALIZER);

            /* Updates status */
            orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);
          }
        }
        /* Has RGB values? */
        else if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_RGB) != orxFALSE)
        {
          /* Gets its value */
          orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_RGB, &(pstResult->stColor.vRGB));

          /* Updates status */
          orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);
        }
        /* Has HSL values? */
        else if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_HSL) != orxFALSE)
        {
          /* Gets its value */
          orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_HSL, &(pstResult->stColor.vHSL));

          /* Stores its RGB equivalent */
          orxColor_FromHSLToRGB(&(pstResult->stColor), &(pstResult->stColor));

          /* Updates status */
          orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);
        }
        /* Has HSV values? */
        else if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_HSV) != orxFALSE)
        {
          /* Gets its value */
          orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_HSV, &(pstResult->stColor.vHSV));

          /* Stores its RGB equivalent */
          orxColor_FromHSVToRGB(&(pstResult->stColor), &(pstResult->stColor));

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

        /* Should repeat? */
        if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_REPEAT) != orxFALSE)
        {
          orxVECTOR vRepeat;

          /* Gets its value */
          orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_REPEAT, &vRepeat);

          /* Stores it */
          orxGraphic_SetRepeat(pstResult, vRepeat.fX, vRepeat.fY);
        }

        /* Has smoothing value? */
        if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_SMOOTHING) != orxFALSE)
        {
          /* Updates flags */
          u32Flags |= (orxConfig_GetBool(orxGRAPHIC_KZ_CONFIG_SMOOTHING) != orxFALSE) ? orxGRAPHIC_KU32_FLAG_SMOOTHING_ON : orxGRAPHIC_KU32_FLAG_SMOOTHING_OFF;
        }

        /* Has blend mode? */
        if(orxConfig_HasValue(orxGRAPHIC_KZ_CONFIG_BLEND_MODE) != orxFALSE)
        {
          /* Sets blend mode */
          orxGraphic_SetBlendMode(pstResult, orxDisplay_GetBlendModeFromString(orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_BLEND_MODE)));
        }

        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, u32Flags, orxGRAPHIC_KU32_FLAG_NONE);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't get text or texture for graphic <%s>.", _zConfigID);

        /* Deletes structures */
        orxGraphic_Delete(pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't find config section named <%s>.", _zConfigID);

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

  /* Decreases count */
  orxStructure_DecreaseCount(_pstGraphic);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstGraphic) == 0)
  {
    /* Cleans data */
    orxGraphic_SetDataInternal(_pstGraphic, orxNULL, orxFALSE);

    /* Deletes structure */
    orxStructure_Delete(_pstGraphic);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Clones a graphic
 * @param[in]   _pstGraphic     Graphic model to clone
 * @ return orxGRAPHIC / orxNULL
 */
orxGRAPHIC *orxFASTCALL orxGraphic_Clone(const orxGRAPHIC *_pstGraphic)
{
  orxGRAPHIC *pstResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstGraphic != orxNULL)
  {
    /* Creates graphic */
    pstResult = orxGraphic_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Clears its data */
      pstResult->pstData = orxNULL;

      /* Updates its flags */
      orxStructure_SetFlags(pstResult, orxStructure_GetFlags(_pstGraphic, orxGRAPHIC_KU32_MASK_ALL), orxGRAPHIC_KU32_MASK_ALL);

      /* Has texture? */
      if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D) != orxFALSE)
      {
        /* Checks */
        orxSTRUCTURE_ASSERT(orxTEXTURE(_pstGraphic->pstData));

        /* Increases texture's reference count */
        orxStructure_IncreaseCount(orxTEXTURE(_pstGraphic->pstData));

        /* Can't link it? */
        if(orxGraphic_SetDataInternal(pstResult, (orxSTRUCTURE *)orxTEXTURE(_pstGraphic->pstData), orxTRUE) == orxSTATUS_FAILURE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link texture <%s> data to cloned graphic <%s>.", orxTexture_GetName(orxTEXTURE(_pstGraphic->pstData)), _pstGraphic->zReference);

          /* Decreases texture's reference count */
          orxStructure_DecreaseCount(orxTEXTURE(_pstGraphic->pstData));
        }
      }
      /* Has text? */
      else if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT) != orxFALSE)
      {
        orxTEXT *pstText;

        /* Checks */
        orxSTRUCTURE_ASSERT(orxTEXT(_pstGraphic->pstData));

        /* Creates new text */
        pstText = orxText_Create();

        /* Valid? */
        if(pstText != orxNULL)
        {
          orxTEXT *pstModel;

          /* Gets model */
          pstModel = orxTEXT(_pstGraphic->pstData);

          /* Copies its font */
          orxText_SetFont(pstText, orxText_GetFont(pstModel));

          /* Copies its string */
          orxText_SetString(pstText, orxText_GetString(pstModel));

          /* Can't link it? */
          if(orxGraphic_SetDataInternal(pstResult, (orxSTRUCTURE *)pstText, orxTRUE) == orxSTATUS_FAILURE)
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link text <%s> data to cloned graphic <%s>.", orxText_GetName(pstModel), _pstGraphic->zReference);

            /* Deletes text */
            orxText_Delete(pstText);
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't create text data for cloned graphic <%s>.", _pstGraphic->zReference);
        }
      }

      /* Has data? */
      if(pstResult->pstData != orxNULL)
      {
        /* Copies locale name ID */
        pstResult->stLocaleNameID = _pstGraphic->stLocaleNameID;

        /* Copies data reference */
        pstResult->zDataReference = _pstGraphic->zDataReference;

        /* Copies pivot */
        orxVector_Copy(&(pstResult->vPivot), &(_pstGraphic->vPivot));

        /* Copies color */
        orxColor_Copy(&(pstResult->stColor), &(_pstGraphic->stColor));

        /* Copies coordinates */
        pstResult->fTop     = _pstGraphic->fTop;
        pstResult->fLeft    = _pstGraphic->fLeft;
        pstResult->fWidth   = _pstGraphic->fWidth;
        pstResult->fHeight  = _pstGraphic->fHeight;

        /* Copies repeat */
        pstResult->fRepeatX = _pstGraphic->fRepeatX;
        pstResult->fRepeatY = _pstGraphic->fRepeatY;

        /* Copies reference */
        pstResult->zReference = _pstGraphic->zReference;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't clone text or texture from graphic <%s>.", _pstGraphic->zReference);

        /* Deletes structures */
        orxGraphic_Delete(pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets graphic config name
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxGraphic_GetName(const orxGRAPHIC *_pstGraphic)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstGraphic);

  /* Updates result */
  zResult = (_pstGraphic->zReference != orxNULL) ? _pstGraphic->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Sets graphic data
 * @param[in]   _pstGraphic     Graphic concerned
 * @param[in]   _pstData        Data structure to set / orxNULL
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetData(orxGRAPHIC *_pstGraphic, orxSTRUCTURE *_pstData)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  eResult = orxGraphic_SetDataInternal(_pstGraphic, _pstData, orxFALSE);

  /* Done! */
  return eResult;
}

/** Gets graphic data
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      OrxSTRUCTURE / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxGraphic_GetData(const orxGRAPHIC *_pstGraphic)
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

/** Sets graphic flipping
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _bFlipX         Flip it on X axis
 * @param[in]   _bFlipY         Flip it on Y axis
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetFlip(orxGRAPHIC *_pstGraphic, orxBOOL _bFlipX, orxBOOL _bFlipY)
{
  orxU32    u32Flags;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates flags */
  u32Flags  = (_bFlipX != orxFALSE) ? orxGRAPHIC_KU32_FLAG_FLIP_X : orxGRAPHIC_KU32_FLAG_NONE;
  u32Flags |= (_bFlipY != orxFALSE) ? orxGRAPHIC_KU32_FLAG_FLIP_Y : orxGRAPHIC_KU32_FLAG_NONE;

  /* Updates status */
  orxStructure_SetFlags(_pstGraphic, u32Flags, orxGRAPHIC_KU32_MASK_FLIP_BOTH);

  /* Done! */
  return eResult;
}

/** Gets graphic flipping
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pbFlipX        X axis flipping
 * @param[in]   _pbFlipY        Y axis flipping
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_GetFlip(const orxGRAPHIC *_pstGraphic, orxBOOL *_pbFlipX, orxBOOL *_pbFlipY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pbFlipX != orxNULL);
  orxASSERT(_pbFlipY != orxNULL);

  /* Updates flipping mode */
  *_pbFlipX = orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_FLIP_X) ? orxTRUE : orxFALSE;
  *_pbFlipY = orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_FLIP_Y) ? orxTRUE : orxFALSE;

  /* Done! */
  return eResult;
}

/** Sets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvPivot        Pivot to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetPivot(orxGRAPHIC *_pstGraphic, const orxVECTOR *_pvPivot)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Valid pivot? */
  if(_pvPivot != orxNULL)
  {
    /* Stores it */
    orxVector_Copy(&(_pstGraphic->vPivot), _pvPivot);

    /* Updates status */
    orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_PIVOT, orxGRAPHIC_KU32_FLAG_RELATIVE_PIVOT);
  }
  else
  {
    /* Stores it */
    orxVector_Copy(&(_pstGraphic->vPivot), &orxVECTOR_0);

    /* Updates status */
    orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_FLAG_HAS_PIVOT | orxGRAPHIC_KU32_FLAG_RELATIVE_PIVOT);
  }

  /* Done! */
  return eResult;
}

/** Sets graphic relative pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _u32AlignFlags  Alignment flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetRelativePivot(orxGRAPHIC *_pstGraphic, orxU32 _u32AlignFlags)
{
  orxAABOX  stBox;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT((_u32AlignFlags & orxGRAPHIC_KU32_MASK_ALIGN) == _u32AlignFlags);
  orxASSERT(_pstGraphic->fWidth >= orxFLOAT_0);
  orxASSERT(_pstGraphic->fHeight >= orxFLOAT_0);

  /* Valid size? */
  if(orxGraphic_GetSize(_pstGraphic, &(stBox.vBR)) != orxNULL)
  {
    /* Inits box top left corner */
    orxVector_SetAll(&(stBox.vTL), orxFLOAT_0);

    /* Updates pivot */
    orxGraphic_AlignVector(_u32AlignFlags, &stBox, &(_pstGraphic->vPivot));

    /* Updates status */
    orxStructure_SetFlags(_pstGraphic, _u32AlignFlags | orxGRAPHIC_KU32_FLAG_HAS_PIVOT | orxGRAPHIC_KU32_FLAG_RELATIVE_PIVOT, orxGRAPHIC_KU32_MASK_ALIGN);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid size retrieved from graphic.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvPivot        Graphic pivot
 * @return      orxPIVOT / orxNULL
 */
orxVECTOR *orxFASTCALL orxGraphic_GetPivot(const orxGRAPHIC *_pstGraphic, orxVECTOR *_pvPivot)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvPivot != orxNULL);

  /* Has pivot? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_PIVOT) != orxFALSE)
  {
    /* Copies it */
    orxVector_Copy(_pvPivot, &(_pstGraphic->vPivot));

    /* Updates result */
    pvResult = _pvPivot;
  }
  else
  {
    /* Clears it */
    orxVector_Copy(_pvPivot, &orxVECTOR_0);

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Sets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvSize         Size to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetSize(orxGRAPHIC *_pstGraphic, const orxVECTOR *_pvSize)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvSize);
  orxASSERT((_pvSize->fX >= orxFLOAT_0) && ((_pvSize->fY >= orxFLOAT_0) || orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT)));

  /* Has text? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT) != orxFALSE)
  {
    /* Updates its size */
    eResult = orxText_SetSize(orxTEXT(_pstGraphic->pstData), _pvSize->fX, _pvSize->fY, orxNULL);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Updates graphic */
      orxGraphic_UpdateSize(_pstGraphic);
    }
    else
    {
      /* Stores values */
      _pstGraphic->fWidth   = _pvSize->fX;
      _pstGraphic->fHeight  = _pvSize->fY;
    }
  }
  else
  {
    /* Stores values */
    _pstGraphic->fWidth   = _pvSize->fX;
    _pstGraphic->fHeight  = _pvSize->fY;
  }

  /* Done! */
  return eResult;
}

/** Gets graphic size
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvSize         Object's size
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxGraphic_GetSize(const orxGRAPHIC *_pstGraphic, orxVECTOR *_pvSize)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvSize != orxNULL);

  /* Valid 2D or text data? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D | orxGRAPHIC_KU32_FLAG_TEXT) != orxFALSE)
  {
    /* Gets its size */
    orxVector_Set(_pvSize, _pstGraphic->fWidth, _pstGraphic->fHeight, orxFLOAT_0);

    /* Updates result */
    pvResult = _pvSize;
  }
  else
  {
    /* No size */
    orxVector_SetAll(_pvSize, orx2F(-1.0f));

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Sets graphic color
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pstColor       Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetColor(orxGRAPHIC *_pstGraphic, const orxCOLOR *_pstColor)
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

/** Sets graphic repeat (wrap) value
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _fRepeatX       X-axis repeat value
 * @param[in]   _fRepeatY       Y-axis repeat value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetRepeat(orxGRAPHIC *_pstGraphic, orxFLOAT _fRepeatX, orxFLOAT _fRepeatY)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if((_fRepeatX > orxFLOAT_0) && (_fRepeatY > orxFLOAT_0))
  {
    /* Stores values */
    _pstGraphic->fRepeatX = _fRepeatX;
    _pstGraphic->fRepeatY = _fRepeatY;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Invalid repeat values %f & %f.", _fRepeatX, _fRepeatY);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

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
  _pstGraphic->stColor.fAlpha = orxFLOAT_1;
  orxVector_Copy(&(_pstGraphic->stColor.vRGB), &orxVECTOR_WHITE);

  /* Done! */
  return eResult;
}

/** Graphic has color accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxGraphic_HasColor(const orxGRAPHIC *_pstGraphic)
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
orxCOLOR *orxFASTCALL orxGraphic_GetColor(const orxGRAPHIC *_pstGraphic, orxCOLOR *_pstColor)
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
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Color not set on graphic.");

    /* Clears result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets graphic repeat (wrap) values
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pfRepeatX      X-axis repeat value
 * @param[out]  _pfRepeatY      Y-axis repeat value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_GetRepeat(const orxGRAPHIC *_pstGraphic, orxFLOAT *_pfRepeatX, orxFLOAT *_pfRepeatY)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pfRepeatX != orxNULL);
  orxASSERT(_pfRepeatY != orxNULL);

  /* Stores values */
  *_pfRepeatX = _pstGraphic->fRepeatX;
  *_pfRepeatY = _pstGraphic->fRepeatY;

  /* Done! */
  return eResult;
}

/** Sets graphic origin
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _pvOrigin       Origin coordinates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetOrigin(orxGRAPHIC *_pstGraphic, const orxVECTOR *_pvOrigin)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvOrigin);
  orxASSERT((_pvOrigin->fX >= orxFLOAT_0) && (_pvOrigin->fY >= orxFLOAT_0));

  /* Stores values */
  _pstGraphic->fLeft  = _pvOrigin->fX;
  _pstGraphic->fTop   = _pvOrigin->fY;

  /* Done! */
  return eResult;
}

/** Gets graphic origin
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[out]  _pvOrigin       Origin coordinates
 * @return      Origin coordinates
 */
orxVECTOR *orxFASTCALL orxGraphic_GetOrigin(const orxGRAPHIC *_pstGraphic, orxVECTOR *_pvOrigin)
{
  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT(_pvOrigin);

  /* Updates result */
  orxVector_Set(_pvOrigin, _pstGraphic->fLeft, _pstGraphic->fTop, orxFLOAT_0);

  /* Done! */
  return _pvOrigin;
}

/** Updates graphic size (recompute)
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_UpdateSize(orxGRAPHIC *_pstGraphic)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Is data a texture? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_2D))
  {
    /* Checks */
    orxSTRUCTURE_ASSERT(orxTEXTURE(_pstGraphic->pstData));

    /* Updates coordinates */
    orxTexture_GetSize(orxTEXTURE(_pstGraphic->pstData), &(_pstGraphic->fWidth), &(_pstGraphic->fHeight));
  }
  /* Is data a text? */
  else if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT))
  {
    /* Checks */
    orxSTRUCTURE_ASSERT(orxTEXT(_pstGraphic->pstData));

    /* Inits full coordinates */
    orxText_GetSize(orxTEXT(_pstGraphic->pstData), &(_pstGraphic->fWidth), &(_pstGraphic->fHeight));
  }
  else
  {
    /* Failure */
    eResult = orxSTATUS_FAILURE;
  }

  /* Valid and has a relative pivot? */
  if((eResult != orxSTATUS_FAILURE)
  && (orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_RELATIVE_PIVOT)))
  {
    /* Updates relative pivot */
    orxGraphic_SetRelativePivot(_pstGraphic, orxStructure_GetFlags(_pstGraphic, orxGRAPHIC_KU32_MASK_ALIGN));
  }

  /* Done! */
  return eResult;
}

/** Sets graphic smoothing
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _eSmoothing     Smoothing type (enabled, default or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetSmoothing(orxGRAPHIC *_pstGraphic, orxDISPLAY_SMOOTHING _eSmoothing)
{
  orxU32    u32Flags;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Depending on smoothing type */
  switch(_eSmoothing)
  {
    case orxDISPLAY_SMOOTHING_ON:
    {
      /* Updates flags */
      u32Flags = orxGRAPHIC_KU32_FLAG_SMOOTHING_ON;

      break;
    }

    case orxDISPLAY_SMOOTHING_OFF:
    {
      /* Updates flags */
      u32Flags = orxGRAPHIC_KU32_FLAG_SMOOTHING_OFF;

      break;
    }

    default:
    case orxDISPLAY_SMOOTHING_DEFAULT:
    {
      /* Updates flags */
      u32Flags = orxGRAPHIC_KU32_FLAG_NONE;

      break;
    }
  }

  /* Updates status */
  orxStructure_SetFlags(_pstGraphic, u32Flags, orxGRAPHIC_KU32_FLAG_SMOOTHING_ON|orxGRAPHIC_KU32_FLAG_SMOOTHING_OFF);

  /* Done! */
  return eResult;
}

/** Gets graphic smoothing
 * @param[in]   _pstGraphic     Concerned graphic
 * @return Smoothing type (enabled, default or none)
 */
orxDISPLAY_SMOOTHING orxFASTCALL orxGraphic_GetSmoothing(const orxGRAPHIC *_pstGraphic)
{
  orxDISPLAY_SMOOTHING eResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  eResult = orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_SMOOTHING_ON)
            ? orxDISPLAY_SMOOTHING_ON
            : orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_SMOOTHING_OFF)
              ? orxDISPLAY_SMOOTHING_OFF
              : orxDISPLAY_SMOOTHING_DEFAULT;

  /* Done! */
  return eResult;
}

/** Sets graphic blend mode
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _eBlendMode     Blend mode (alpha, multiply, add or none)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetBlendMode(orxGRAPHIC *_pstGraphic, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Depending on blend mode */
  switch(_eBlendMode)
  {
    case orxDISPLAY_BLEND_MODE_ALPHA:
    {
      /* Updates status */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_ALPHA, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

      break;
    }

    case orxDISPLAY_BLEND_MODE_MULTIPLY:
    {
      /* Updates status */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_MULTIPLY, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

      break;
    }

    case orxDISPLAY_BLEND_MODE_ADD:
    {
      /* Updates status */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_ADD, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

      break;
    }

    case orxDISPLAY_BLEND_MODE_PREMUL:
    {
      /* Updates status */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_PREMUL, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

      break;
    }

    case orxDISPLAY_BLEND_MODE_NONE:
    {
      /* Updates status */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_NONE, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

      break;
    }

    default:
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;

      break;
    }
  }

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Updates its flag */
    orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_BLEND_MODE, orxGRAPHIC_KU32_FLAG_NONE);
  }

  /* Done! */
  return eResult;
}

/** Clears graphic blend mode
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_ClearBlendMode(orxGRAPHIC *_pstGraphic)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates its flag */
  orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_NONE, orxGRAPHIC_KU32_FLAG_HAS_BLEND_MODE | orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

  /* Done! */
  return eResult;
}

/** Graphic has blend mode accessor
 * @param[in]   _pstGraphic     Concerned graphic
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxGraphic_HasBlendMode(const orxGRAPHIC *_pstGraphic)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Updates result */
  bResult = orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_BLEND_MODE);

  /* Done! */
  return bResult;
}

/** Gets graphic blend mode
 * @param[in]   _pstGraphic     Concerned graphic
 * @return Blend mode (alpha, multiply, add or none)
 */
orxDISPLAY_BLEND_MODE orxFASTCALL orxGraphic_GetBlendMode(const orxGRAPHIC *_pstGraphic)
{
  orxDISPLAY_BLEND_MODE eResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Has blendmode? */
  if(orxStructure_TestFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_HAS_BLEND_MODE))
  {
    /* Depending on blend flags */
    switch(orxStructure_GetFlags(_pstGraphic, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL))
    {
      case orxGRAPHIC_KU32_FLAG_BLEND_MODE_ALPHA:
      {
        /* Updates result */
        eResult = orxDISPLAY_BLEND_MODE_ALPHA;

        break;
      }

      case orxGRAPHIC_KU32_FLAG_BLEND_MODE_MULTIPLY:
      {
        /* Updates result */
        eResult = orxDISPLAY_BLEND_MODE_MULTIPLY;

        break;
      }

      case orxGRAPHIC_KU32_FLAG_BLEND_MODE_ADD:
      {
        /* Updates result */
        eResult = orxDISPLAY_BLEND_MODE_ADD;

        break;
      }

      case orxGRAPHIC_KU32_FLAG_BLEND_MODE_PREMUL:
      {
        /* Updates result */
        eResult = orxDISPLAY_BLEND_MODE_PREMUL;

        break;
      }

      default:
      {
        /* Updates result */
        eResult = orxDISPLAY_BLEND_MODE_NONE;

        break;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Blend mode not set on graphic.");

    /* Clears result */
    eResult = orxDISPLAY_BLEND_MODE_NONE;
  }

  /* Done! */
  return eResult;
}

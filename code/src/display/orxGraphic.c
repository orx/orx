/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxLocale.h"
#include "display/orxText.h"
#include "display/orxTexture.h"


/** Module flags
 */
#define orxGRAPHIC_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags  */

#define orxGRAPHIC_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag  */


/** Graphic flags
 */
#define orxGRAPHIC_KU32_FLAG_INTERNAL             0x10000000  /**< Internal structure handling flag  */
#define orxGRAPHIC_KU32_FLAG_HAS_COLOR            0x20000000  /**< Has color flag  */
#define orxGRAPHIC_KU32_FLAG_HAS_PIVOT            0x40000000  /**< Has pivot flag  */
#define orxGRAPHIC_KU32_FLAG_RELATIVE_PIVOT       0x80000000  /**< Relative pivot flag */
#define orxGRAPHIC_KU32_FLAG_SMOOTHING_ON         0x01000000  /**< Smoothing on flag  */
#define orxGRAPHIC_KU32_FLAG_SMOOTHING_OFF        0x02000000  /**< Smoothing off flag  */

#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_NONE      0x00000000  /**< Blend mode no flags */

#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_ALPHA     0x00100000  /**< Blend mode alpha flag */
#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_MULTIPLY  0x00200000  /**< Blend mode multiply flag */
#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_ADD       0x00400000  /**< Blend mode add flag */
#define orxGRAPHIC_KU32_FLAG_BLEND_MODE_PREMUL    0x00800000  /**< Blend mode premul flag */

#define orxGRAPHIC_KU32_MASK_ALIGN                0x000003F0  /**< Alignment mask */

#define orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL       0x00F00000  /**< Blend mode mask */

#define orxGRAPHIC_KU32_MASK_ALL                  0xFFFFFFFF  /**< All flags */


/** Misc defines
 */
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_NAME         "Texture"
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_ORIGIN       "TextureOrigin"
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_SIZE         "TextureSize"
#define orxGRAPHIC_KZ_CONFIG_TEXTURE_CORNER       "TextureCorner" /**< Kept for retro-compatibility reason */
#define orxGRAPHIC_KZ_CONFIG_TEXT_NAME            "Text"
#define orxGRAPHIC_KZ_CONFIG_PIVOT                "Pivot"
#define orxGRAPHIC_KZ_CONFIG_COLOR                "Color"
#define orxGRAPHIC_KZ_CONFIG_ALPHA                "Alpha"
#define orxGRAPHIC_KZ_CONFIG_RGB                  "RGB"
#define orxGRAPHIC_KZ_CONFIG_HSL                  "HSL"
#define orxGRAPHIC_KZ_CONFIG_HSV                  "HSV"
#define orxGRAPHIC_KZ_CONFIG_FLIP                 "Flip"
#define orxGRAPHIC_KZ_CONFIG_REPEAT               "Repeat"
#define orxGRAPHIC_KZ_CONFIG_SMOOTHING            "Smoothing"
#define orxGRAPHIC_KZ_CONFIG_BLEND_MODE           "BlendMode"

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


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Graphic structure
 */
struct __orxGRAPHIC_t
{
  orxSTRUCTURE    stStructure;              /**< Public structure, first structure member : 32 */
  orxSTRUCTURE   *pstData;                  /**< Data structure : 20 */
  orxVECTOR       vPivot;                   /**< Pivot : 32 */
  orxCOLOR        stColor;                  /**< Color : 48 */
  orxFLOAT        fTop;                     /**< Top coordinate : 52 */
  orxFLOAT        fLeft;                    /**< Left coordinate : 56 */
  orxFLOAT        fWidth;                   /**< Width : 60 */
  orxFLOAT        fHeight;                  /**< Height : 64 */
  orxFLOAT        fRepeatX;                 /**< X-axis repeat counter : 68 */
  orxFLOAT        fRepeatY;                 /**< Y-axis repeat counter : 72 */
  const orxSTRING zReference;               /**< Reference : 76 */
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

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxGraphic_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_LOCALE);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Select language event */
    case orxLOCALE_EVENT_SELECT_LANGUAGE:
    {
      orxGRAPHIC *pstGraphic;

      /* For all graphics */
      for(pstGraphic = orxGRAPHIC(orxStructure_GetFirst(orxSTRUCTURE_ID_GRAPHIC));
          pstGraphic != orxNULL;
          pstGraphic = orxGRAPHIC(orxStructure_GetNext(pstGraphic)))
      {
        /* Is data a text? */
        if(orxStructure_TestFlags(pstGraphic, orxGRAPHIC_KU32_FLAG_TEXT))
        {
          /* Updates graphic's size */
          orxGraphic_UpdateSize(pstGraphic);
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all graphics
 */
static orxINLINE void orxGraphic_DeleteAll()
{
  register orxGRAPHIC *pstGraphic;

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

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
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
      }
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

    /* Sets its repeat value to default */
    orxGraphic_SetRepeat(pstGraphic, orxFLOAT_1, orxFLOAT_1);

    /* Increases counter */
    orxStructure_IncreaseCounter(pstGraphic);
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
    /* Creates graphic */
    pstResult = orxGraphic_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      const orxSTRING zName;
      orxU32          u32Flags = orxGRAPHIC_KU32_FLAG_NONE;

      /* Gets texture name */
      zName = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_TEXTURE_NAME);

      /* Valid? */
      if((zName != orxNULL) && (zName != orxSTRING_EMPTY))
      {
        orxTEXTURE *pstTexture;

        /* Creates texture */
        pstTexture = orxTexture_CreateFromFile(zName);

        /* Valid? */
        if(pstTexture != orxNULL)
        {
          /* Links it */
          if(orxGraphic_SetData(pstResult, (orxSTRUCTURE *)pstTexture) != orxSTATUS_FAILURE)
          {
            orxVECTOR vTextureSize;

            /* Updates its owner */
            orxStructure_SetOwner(pstTexture, pstResult);

            /* Inits default 2D flags */
            u32Flags = orxGRAPHIC_KU32_FLAG_INTERNAL | orxGRAPHIC_KU32_FLAG_2D;

            /* Has size? */
            if(orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_SIZE, &vTextureSize) != orxNULL)
            {
              orxVECTOR vTextureOrigin;

              /* Has origin? */
              if(orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_ORIGIN, &vTextureOrigin) != orxNULL)
              {
                /* Stores them */
                pstResult->fLeft    = vTextureOrigin.fX;
                pstResult->fTop     = vTextureOrigin.fY;
                pstResult->fWidth   = vTextureSize.fX;
                pstResult->fHeight  = vTextureSize.fY;
              }
              /* Has corner? */
              else if(orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_TEXTURE_CORNER, &vTextureOrigin) != orxNULL)
              {
                /* Stores them */
                pstResult->fLeft    = vTextureOrigin.fX;
                pstResult->fTop     = vTextureOrigin.fY;
                pstResult->fWidth   = vTextureSize.fX;
                pstResult->fHeight  = vTextureSize.fY;
              }
              else
              {
                /* Updates size */
                orxGraphic_UpdateSize(pstResult);
              }
            }
            else
            {
              /* Updates size */
              orxGraphic_UpdateSize(pstResult);
            }
          }
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link texture (%s) data to graphic (%s).", zName, _zConfigID);

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
            if(orxGraphic_SetData(pstResult, (orxSTRUCTURE *)pstText) != orxSTATUS_FAILURE)
            {
              /* Sets its owner */
              orxStructure_SetOwner(pstText, pstResult);

              /* Inits default text flags */
              u32Flags = orxGRAPHIC_KU32_FLAG_INTERNAL | orxGRAPHIC_KU32_FLAG_TEXT;

              /* Updates size */
              orxGraphic_UpdateSize(pstResult);
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't link text (%s) data to graphic (%s).", zName, _zConfigID);

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
          orxCHAR   acBuffer[64];
          orxSTRING zRelativePos;
          orxU32    u32AlignmentFlags = orxGRAPHIC_KU32_FLAG_ALIGN_CENTER;

          /* Gets lower case value */
          acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
          zRelativePos = orxString_LowerCase(orxString_NCopy(acBuffer, orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_PIVOT), sizeof(acBuffer) - 1));

          /* Left? */
          if(orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_LEFT_PIVOT) != orxNULL)
          {
            /* Updates alignment flags */
            u32AlignmentFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_LEFT;
          }
          /* Right? */
          else if(orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_RIGHT_PIVOT) != orxNULL)
          {
            /* Updates alignment flags */
            u32AlignmentFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT;
          }

          /* Top? */
          if(orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_TOP_PIVOT) != orxNULL)
          {
            /* Updates alignment flags */
            u32AlignmentFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_TOP;
          }
          /* Bottom? */
          else if(orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_BOTTOM_PIVOT) != orxNULL)
          {
            /* Updates alignment flags */
            u32AlignmentFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM;
          }

          /* Truncate? */
          if(orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_TRUNCATE_PIVOT) != orxNULL)
          {
            /* Updates alignment flags */
            u32AlignmentFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE;
          }
          /* Round? */
          else if(orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_ROUND_PIVOT) != orxNULL)
          {
            /* Updates alignment flags */
            u32AlignmentFlags |= orxGRAPHIC_KU32_FLAG_ALIGN_ROUND;
          }

          /* Valid? */
          if((u32AlignmentFlags != orxGRAPHIC_KU32_FLAG_ALIGN_CENTER)
          || (orxString_SearchString(zRelativePos, orxGRAPHIC_KZ_CENTERED_PIVOT) != orxNULL))
          {
            /* Applies it */
            orxGraphic_SetRelativePivot(pstResult, u32AlignmentFlags);
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
          orxVECTOR vColor;

          /* Gets its value */
          orxConfig_GetVector(orxGRAPHIC_KZ_CONFIG_COLOR, &vColor);

          /* Normalizes and applies it */
          orxVector_Mulf(&(pstResult->stColor.vRGB), &vColor, orxCOLOR_NORMALIZER);

          /* Updates status */
          orxStructure_SetFlags(pstResult, orxGRAPHIC_KU32_FLAG_HAS_COLOR, orxGRAPHIC_KU32_FLAG_NONE);
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
          const orxSTRING       zBlendMode;
          orxDISPLAY_BLEND_MODE eBlendMode;

          /* Gets blend mode value */
          zBlendMode = orxConfig_GetString(orxGRAPHIC_KZ_CONFIG_BLEND_MODE);
          eBlendMode = orxDisplay_GetBlendModeFromString(zBlendMode);

          /* Depending on blend mode */
          switch(eBlendMode)
          {
            case orxDISPLAY_BLEND_MODE_ALPHA:
            {
              /* Updates flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_BLEND_MODE_ALPHA;

              break;
            }

            case orxDISPLAY_BLEND_MODE_MULTIPLY:
            {
              /* Updates flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_BLEND_MODE_MULTIPLY;

              break;
            }

            case orxDISPLAY_BLEND_MODE_ADD:
            {
              /* Updates flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_BLEND_MODE_ADD;

              break;
            }

            case orxDISPLAY_BLEND_MODE_PREMUL:
            {
              /* Updates flags */
              u32Flags |= orxGRAPHIC_KU32_FLAG_BLEND_MODE_PREMUL;

              break;
            }

            default:
            {
            }
          }
        }
        else
        {
          /* Defaults to alpha */
          u32Flags |= orxGRAPHIC_KU32_FLAG_BLEND_MODE_ALPHA;
        }

        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Protects it */
        orxConfig_ProtectSection(pstResult->zReference, orxTRUE);

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, u32Flags, orxGRAPHIC_KU32_FLAG_NONE);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't get text or texture for graphic (%s).", _zConfigID);

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
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Couldn't find config section named (%s).", _zConfigID);

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

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstGraphic);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstGraphic) == 0)
  {
    /* Cleans data */
    orxGraphic_SetData(_pstGraphic, orxNULL);

    /* Has reference? */
    if(_pstGraphic->zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstGraphic->zReference, orxFALSE);
    }

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
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);

  /* Had previous data? */
  if(_pstGraphic->pstData != orxNULL)
  {
    /* Updates structure reference counter */
    orxStructure_DecreaseCounter(_pstGraphic->pstData);

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
  }
  else
  {
    /* Updates flags */
    orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_NONE, orxGRAPHIC_KU32_MASK_TYPE);
  }

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

/** Sets relative graphic pivot
 * @param[in]   _pstGraphic     Concerned graphic
 * @param[in]   _u32AlignFlags  Alignment flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxGraphic_SetRelativePivot(orxGRAPHIC *_pstGraphic, orxU32 _u32AlignFlags)
{
  orxVECTOR vSize;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstGraphic.u32Flags & orxGRAPHIC_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstGraphic);
  orxASSERT((_u32AlignFlags & orxGRAPHIC_KU32_MASK_ALIGN) == _u32AlignFlags);
  orxASSERT(_pstGraphic->fWidth >= orxFLOAT_0);
  orxASSERT(_pstGraphic->fHeight >= orxFLOAT_0);

  /* Valid size? */
  if(orxGraphic_GetSize(_pstGraphic, &vSize) != orxNULL)
  {
    orxFLOAT  fHeight, fWidth;

    /* Gets graphic size */
    fWidth  = vSize.fX;
    fHeight = vSize.fY;

    /* Pivot left? */
    if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_LEFT))
    {
      /* Updates x position */
      _pstGraphic->vPivot.fX = orxFLOAT_0;
    }
    /* Align right? */
    else if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_RIGHT))
    {
      /* Updates x position */
      _pstGraphic->vPivot.fX = fWidth;
    }
    /* Align center */
    else
    {
      /* Updates x position */
      _pstGraphic->vPivot.fX = orx2F(0.5f) * fWidth;
    }

    /* Align top? */
    if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_TOP))
    {
      /* Updates y position */
      _pstGraphic->vPivot.fY = orxFLOAT_0;
    }
    /* Align bottom? */
    else if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_BOTTOM))
    {
      /* Updates y position */
      _pstGraphic->vPivot.fY = fHeight;
    }
    /* Align center */
    else
    {
      /* Updates y position */
      _pstGraphic->vPivot.fY = orx2F(0.5f) * fHeight;
    }

    /* Truncate? */
    if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_TRUNCATE))
    {
      /* Updates position */
      orxVector_Floor(&(_pstGraphic->vPivot), &(_pstGraphic->vPivot));
    }
    /* Round? */
    else if(orxFLAG_TEST(_u32AlignFlags, orxGRAPHIC_KU32_FLAG_ALIGN_ROUND))
    {
      /* Updates position */
      orxVector_Round(&(_pstGraphic->vPivot), &(_pstGraphic->vPivot));
    }

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
  orxASSERT((_pvSize->fX >= orxFLOAT_0) && (_pvSize->fY >= orxFLOAT_0));

  /* Stores values */
  _pstGraphic->fWidth   = _pvSize->fX;
  _pstGraphic->fHeight  = _pvSize->fY;

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
  if(orxTEXTURE(_pstGraphic->pstData) != orxNULL)
  {
    /* Updates coordinates */
    orxTexture_GetSize(orxTEXTURE(_pstGraphic->pstData), &(_pstGraphic->fWidth), &(_pstGraphic->fHeight));
  }
  /* Is data a text? */
  else if(orxTEXT(_pstGraphic->pstData) != orxNULL)
  {
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

    default:
    {
      /* Updates status */
      orxStructure_SetFlags(_pstGraphic, orxGRAPHIC_KU32_FLAG_BLEND_MODE_NONE, orxGRAPHIC_KU32_MASK_BLEND_MODE_ALL);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;

      break;
    }
  }

  /* Done! */
  return eResult;
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

  /* Done! */
  return eResult;
}

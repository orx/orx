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
 * @file orxViewport.c
 * @date 14/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "render/orxViewport.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "display/orxDisplay.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/** Module flags
 */
#define orxVIEWPORT_KU32_STATIC_FLAG_NONE       0x00000000  /**< No flags */

#define orxVIEWPORT_KU32_STATIC_FLAG_READY      0x00000001  /**< Ready flag */

#define orxVIEWPORT_KU32_STATIC_MASK_ALL        0xFFFFFFFF  /**< All mask */

/** orxVIEWPORT flags / masks
 */
#define orxVIEWPORT_KU32_FLAG_NONE              0x00000000  /**< No flags */

#define orxVIEWPORT_KU32_FLAG_ENABLED           0x00000001  /**< Enabled flag */
#define orxVIEWPORT_KU32_FLAG_CAMERA            0x00000002  /**< Has camera flag */
#define orxVIEWPORT_KU32_FLAG_BACKGROUND_COLOR  0x00000004  /**< Has background color flag */
#define orxVIEWPORT_KU32_FLAG_USE_SCREEN_SIZE   0x00000008  /**< Uses screen size flag */
#define orxVIEWPORT_KU32_FLAG_AUTO_RESIZE       0x00000010  /**< Auto-resize flag */
#define orxVIEWPORT_KU32_FLAG_INTERNAL_TEXTURES 0x10000000  /**< Internal texture handling flag  */
#define orxVIEWPORT_KU32_FLAG_INTERNAL_SHADER   0x20000000  /**< Internal shader pointer handling flag  */
#define orxVIEWPORT_KU32_FLAG_INTERNAL_CAMERA   0x40000000  /**< Internal camera handling flag  */

#define orxVIEWPORT_KU32_FLAG_DEFAULT           0x00000009  /**< Default flags */

#define orxVIEWPORT_KU32_MASK_ALIGN             0xF0000000  /**< Alignment mask */

#define orxVIEWPORT_KU32_MASK_ALL               0xFFFFFFFF  /** All mask */


/** Misc defines
 */
#define orxVIEWPORT_KU32_BANK_SIZE              16          /**< Bank size */

#define orxVIEWPORT_KZ_CONFIG_TEXTURE_NAME      "Texture"
#define orxVIEWPORT_KZ_CONFIG_TEXTURE_LIST_NAME "TextureList"
#define orxVIEWPORT_KZ_CONFIG_POSITION          "Position"
#define orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION "RelativePosition"
#define orxVIEWPORT_KZ_CONFIG_SIZE              "Size"
#define orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE     "RelativeSize"
#define orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR  "BackgroundColor"
#define orxVIEWPORT_KZ_CONFIG_BACKGROUND_ALPHA  "BackgroundAlpha"
#define orxVIEWPORT_KZ_CONFIG_CAMERA            "Camera"
#define orxVIEWPORT_KZ_CONFIG_SHADER_LIST       "ShaderList"
#define orxVIEWPORT_KZ_CONFIG_BLEND_MODE        "BlendMode"
#define orxVIEWPORT_KZ_CONFIG_AUTO_RESIZE       "AutoResize"

#define orxVIEWPORT_KZ_LEFT                     "left"
#define orxVIEWPORT_KZ_RIGHT                    "right"
#define orxVIEWPORT_KZ_TOP                      "top"
#define orxVIEWPORT_KZ_BOTTOM                   "bottom"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Viewport structure
 */
struct __orxVIEWPORT_t
{
  orxSTRUCTURE          stStructure;                                          /**< Public structure, first structure member : 32 */
  orxFLOAT              fX;                                                   /**< X position (top left corner) : 36 */
  orxFLOAT              fY;                                                   /**< Y position (top left corner) : 40 */
  orxFLOAT              fWidth;                                               /**< Width : 44 */
  orxFLOAT              fHeight;                                              /**< Height : 48 */
  orxCOLOR              stBackgroundColor;                                    /**< Background color : 64 */
  orxU32                u32TextureCounter;                                    /**< Associated texture counter : 68 */
  orxCAMERA            *pstCamera;                                            /**< Associated camera : 72 / 76 */
  orxSHADERPOINTER     *pstShaderPointer;                                     /**< Shader pointer : 76 / 84 */
  orxU32                u32TextureOwnerFlags;                                 /**< Texture owner flags : 80 / 88 */
  orxDISPLAY_BLEND_MODE eBlendMode;                                           /**< Blend mode : 84 / 92 */
  const orxSTRING       zReference;                                           /**< Reference : 88 / 100 */
  orxTEXTURE           *apstTextureList[orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER]; /**< Associated texture list : 152 / 228 */
};

/** Static structure
 */
typedef struct __orxVIEWPORT_STATIC_t
{
  orxU32 u32Flags;                                                        /**< Control flags : 4 */

} orxVIEWPORT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxVIEWPORT_STATIC sstViewport;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxViewport_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_DISPLAY);

  /* Depending on event ID */
  switch(_pstEvent->eID)
  {
    /* Set video mode */
    case orxDISPLAY_EVENT_SET_VIDEO_MODE:
    {
      orxVIEWPORT              *pstViewport;
      orxDISPLAY_EVENT_PAYLOAD *pstPayload;
      orxFLOAT                  fWidthRatio, fHeightRatio;

      /* Gets its payload */
      pstPayload = (orxDISPLAY_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Gets new width & height ratios */
      fWidthRatio   = orxU2F(pstPayload->stVideoMode.u32Width) / orxU2F(pstPayload->stVideoMode.u32PreviousWidth);
      fHeightRatio  = orxU2F(pstPayload->stVideoMode.u32Height) / orxU2F(pstPayload->stVideoMode.u32PreviousHeight);

      /* Changed? */
      if((fWidthRatio != orxFLOAT_1) || (fHeightRatio != orxFLOAT_1))
      {
        /* For all viewports */
        for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
            pstViewport != orxNULL;
            pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
        {
          /* Is linked to screen? */
          if(pstViewport->u32TextureCounter == 0)
          {
            /* Updates relative position & dimension */
            pstViewport->fX       = orxMath_Round(pstViewport->fX * fWidthRatio);
            pstViewport->fWidth   = orxMath_Round(pstViewport->fWidth * fWidthRatio);
            pstViewport->fY       = orxMath_Round(pstViewport->fY * fHeightRatio);
            pstViewport->fHeight  = orxMath_Round(pstViewport->fHeight * fHeightRatio);

            /* Sends event */
            orxEVENT_SEND(orxEVENT_TYPE_VIEWPORT, orxVIEWPORT_EVENT_RESIZE, (orxHANDLE)pstViewport, (orxHANDLE)pstViewport, orxNULL);
          }
          /* Auto-resize? */
          else if(orxStructure_TestFlags(pstViewport, orxVIEWPORT_KU32_FLAG_AUTO_RESIZE))
          {
            /* Has owned textures? */
            if(pstViewport->u32TextureOwnerFlags != 0)
            {
              orxU32 i;

              /* Updates relative position & dimension */
              pstViewport->fX       = orxMath_Round(pstViewport->fX * fWidthRatio);
              pstViewport->fWidth   = orxMath_Round(pstViewport->fWidth * fWidthRatio);
              pstViewport->fY       = orxMath_Round(pstViewport->fY * fHeightRatio);
              pstViewport->fHeight  = orxMath_Round(pstViewport->fHeight * fHeightRatio);

              /* For all textures */
              for(i = 0; i < pstViewport->u32TextureCounter; i++)
              {
                /* Is owned? */
                if(pstViewport->u32TextureOwnerFlags & (1 << i))
                {
                  orxBITMAP  *pstBitmap;
                  orxFLOAT    fWidth, fHeight;
                  orxCHAR     acBuffer[256];

                  /* Gets its name */
                  orxString_NPrint(acBuffer, 255, "%s", orxTexture_GetName(pstViewport->apstTextureList[i]));
                  acBuffer[255] = orxCHAR_NULL;

                  /* Gets its linked bitmap */
                  pstBitmap = orxTexture_GetBitmap(pstViewport->apstTextureList[i]);

                  /* Gets its size */
                  orxDisplay_GetBitmapSize(pstBitmap, &fWidth, &fHeight);

                  /* Unlinks it */
                  orxTexture_UnlinkBitmap(pstViewport->apstTextureList[i]);

                  /* Deletes it */
                  orxDisplay_DeleteBitmap(pstBitmap);

                  /* Updates size */
                  fWidth  = orxMath_Round(fWidth * fWidthRatio);
                  fHeight = orxMath_Round(fHeight * fHeightRatio);

                  /* Re-creates it as the right size */
                  pstBitmap = orxDisplay_CreateBitmap(orxF2U(fWidth), orxF2U(fHeight));

                  /* Checks */
                  orxASSERT(pstBitmap != orxNULL);

                  /* Clears it */
                  orxDisplay_ClearBitmap(pstBitmap, orx2RGBA(0, 0, 0, 0));

                  /* Re-links it */
                  (void)orxTexture_LinkBitmap(pstViewport->apstTextureList[i], pstBitmap, acBuffer);
                }
              }

              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_VIEWPORT, orxVIEWPORT_EVENT_RESIZE, (orxHANDLE)pstViewport, (orxHANDLE)pstViewport, orxNULL);
            }
          }
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

/** Deletes all viewports
 */
static orxINLINE void orxViewport_DeleteAll()
{
  orxVIEWPORT *pstViewport;

  /* Gets first viewport */
  pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));

  /* Non empty? */
  while(pstViewport != orxNULL)
  {
    /* Deletes viewport */
    orxViewport_Delete(pstViewport);

    /* Gets first remaining viewport */
    pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Viewport module setup
 */
void orxFASTCALL orxViewport_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_TEXTURE);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_CAMERA);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_SHADERPOINTER);

  return;
}

/** Inits the Viewport module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER <= 32);

  /* Not already Initialized? */
  if(!(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstViewport, sizeof(orxVIEWPORT_STATIC));

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxViewport_EventHandler);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(VIEWPORT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxVIEWPORT_KU32_BANK_SIZE, orxNULL);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Inits Flags */
        sstViewport.u32Flags = orxVIEWPORT_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxViewport_EventHandler);
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to initialize viewport module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Not initialized? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Initializing viewport module failed.");

    /* Updates Flags */
    sstViewport.u32Flags &= ~orxVIEWPORT_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Viewport module
 */
void orxFASTCALL orxViewport_Exit()
{
  /* Initialized? */
  if(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxViewport_EventHandler);

    /* Deletes viewport list */
    orxViewport_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_VIEWPORT);

    /* Updates flags */
    sstViewport.u32Flags &= ~orxVIEWPORT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to exit from viewport module when it wasn't initialized.");
  }

  return;
}

/** Creates a viewport
 * @return      Created orxVIEWPORT / orxNULL
 */
orxVIEWPORT *orxFASTCALL orxViewport_Create()
{
  orxVIEWPORT *pstViewport;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);

  /* Creates viewport */
  pstViewport = orxVIEWPORT(orxStructure_Create(orxSTRUCTURE_ID_VIEWPORT));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Inits viewport flags */
    orxStructure_SetFlags(pstViewport, orxVIEWPORT_KU32_FLAG_DEFAULT, orxVIEWPORT_KU32_FLAG_NONE);

    /* Inits vars */
    pstViewport->fX = pstViewport->fY = orxFLOAT_0;

    /* Sets default size */
    orxViewport_SetRelativeSize(pstViewport, orxFLOAT_1, orxFLOAT_1);

    /* Increases counter */
    orxStructure_IncreaseCounter(pstViewport);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Viewport not created correctly.");

    /* Not created */
    pstViewport = orxNULL;
  }

  /* Done! */
  return pstViewport;
}

/** Creates a viewport from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxVIEWPORT / orxNULL
 */
orxVIEWPORT *orxFASTCALL orxViewport_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxVIEWPORT *pstResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates viewport */
    pstResult = orxViewport_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      const orxSTRING zCameraName;
      orxS32          s32Number;

      /* Has plain size */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_SIZE) != orxFALSE)
      {
        orxVECTOR vSize;

        /* Gets it */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_SIZE, &vSize);

        /* Applies it */
        orxViewport_SetSize(pstResult, vSize.fX, vSize.fY);
      }
      else
      {
        /* Defaults to screen size */
        orxDisplay_GetScreenSize(&(pstResult->fWidth), &(pstResult->fHeight));

        /* Updates flags */
        orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_USE_SCREEN_SIZE, orxVIEWPORT_KU32_FLAG_NONE);
      }

      /* Has plain position */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_POSITION) != orxFALSE)
      {
        orxVECTOR vPos;

        /* Gets it */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_POSITION, &vPos);

        /* Applies it */
        orxViewport_SetPosition(pstResult, vPos.fX, vPos.fY);
      }

      /* Auto resize */
      if((orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_AUTO_RESIZE) == orxFALSE)
      || (orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_AUTO_RESIZE) != orxFALSE))
      {
        /* Updates flags */
        orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_AUTO_RESIZE, orxVIEWPORT_KU32_FLAG_NONE);
      }

      /* *** Textures *** */

      /* Has texture list? */
      if((s32Number = orxConfig_GetListCounter(orxVIEWPORT_KZ_CONFIG_TEXTURE_LIST_NAME)) > 0)
      {
        orxS32      i, s32TextureCounter;
        orxU32      u32OwnerFlags = 0;
        orxTEXTURE *apstTextureList[orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER];

        /* For all entries */
        for(i = 0, s32TextureCounter = 0; i < s32Number; i++)
        {
          const orxSTRING zTextureName;

          /* Gets its name */
          zTextureName = orxConfig_GetListString(orxVIEWPORT_KZ_CONFIG_TEXTURE_LIST_NAME, i);

          /* Valid? */
          if((zTextureName != orxNULL) && (zTextureName != orxSTRING_EMPTY))
          {
            orxTEXTURE *pstTexture;
            orxBOOL     bDisplayLevelEnabled;

            /* Gets display debug level state */
            bDisplayLevelEnabled = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_DISPLAY);

            /* Deactivates display debug level */
            orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, orxFALSE);

            /* Creates texture from file */
            pstTexture = orxTexture_CreateFromFile(zTextureName);

            /* Restores display debug level state */
            orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, bDisplayLevelEnabled);

            /* Not found? */
            if(pstTexture == orxNULL)
            {
              orxBITMAP *pstBitmap;

              /* Creates new bitmap */
              pstBitmap = orxDisplay_CreateBitmap(orxF2U(pstResult->fWidth), orxF2U(pstResult->fHeight));

              /* Valid? */
              if(pstBitmap != orxNULL)
              {
                /* Clears it */
                orxDisplay_ClearBitmap(pstBitmap, orx2RGBA(0, 0, 0, 0));

                /* Creates new texture */
                pstTexture = orxTexture_Create();

                /* Valid? */
                if(pstTexture != orxNULL)
                {
                  /* Links them */
                  if(orxTexture_LinkBitmap(pstTexture, pstBitmap, zTextureName) != orxSTATUS_FAILURE)
                  {
                    /* Updates owner flags */
                    u32OwnerFlags |= 1 << i;
                  }
                  else
                  {
                    /* Deletes texture */
                    orxTexture_Delete(pstTexture);
                    pstTexture = orxNULL;

                    /* Deletes bitmap */
                    orxDisplay_DeleteBitmap(pstBitmap);
                  }
                }
                else
                {
                  /* Deletes bitmap */
                  orxDisplay_DeleteBitmap(pstBitmap);
                }
              }
            }

            /* Valid? */
            if(pstTexture != orxNULL)
            {
              /* Updates its owner */
              orxStructure_SetOwner(pstTexture, pstResult);

              /* Stores it */
              apstTextureList[s32TextureCounter++] = pstTexture;
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't link texture [%s] to viewport [%s], skipping.", zTextureName, _zConfigID);
            }
          }
        }

        /* Has valid textures? */
        if(s32TextureCounter > 0)
        {
          orxS32 j;

          /* Stores them */
          orxViewport_SetTextureList(pstResult, (orxU32)s32TextureCounter, apstTextureList);

          /* For all secondary textures */
          for(j = 1; j < s32TextureCounter; j++)
          {
            orxU32  k;
            orxBOOL bStored = orxFALSE;

            for(k = 1; k < pstResult->u32TextureCounter; k++)
            {
              /* Found? */
              if(pstResult->apstTextureList[k] == apstTextureList[j])
              {
                /* Updates status */
                bStored = orxTRUE;

                break;
              }
            }

            /* Wasn't stored? */
            if(bStored == orxFALSE)
            {
              /* Was owned? */
              if(u32OwnerFlags & (1 << j))
              {
                orxBITMAP *pstBitmap;

                /* Gets linked bitmap */
                pstBitmap = orxTexture_GetBitmap(apstTextureList[j]);

                /* Unlinks it */
                orxTexture_UnlinkBitmap(apstTextureList[j]);

                /* Deletes it */
                orxDisplay_DeleteBitmap(pstBitmap);

                /* Updates owner flags */
                u32OwnerFlags &= ~(1 << j);
              }

              /* Shifts owner flags to remove gap */
              u32OwnerFlags = (u32OwnerFlags & ((1 << j) - 1)) | ((u32OwnerFlags & ~((1 << (j + 1)) - 1)) >> 1);

              /* Deletes it */
              orxTexture_Delete(apstTextureList[j]);
            }
          }

          /* Stores texture owner flags */
          pstResult->u32TextureOwnerFlags = u32OwnerFlags;

          /* Updates status flags */
          orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_INTERNAL_TEXTURES, orxVIEWPORT_KU32_FLAG_NONE);
        }
      }
      else
      {
        const orxSTRING zTextureName;

        /* Gets old-style texture name */
        zTextureName = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_TEXTURE_NAME);

        /* Valid? */
        if((zTextureName != orxNULL) && (zTextureName != orxSTRING_EMPTY))
        {
          orxTEXTURE *pstTexture;
          orxU32      u32OwnerFlags = 0;
          orxBOOL     bDisplayLevelEnabled;

          /* Gets display debug level state */
          bDisplayLevelEnabled = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_DISPLAY);

          /* Deactivates display debug level */
          orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, orxFALSE);

          /* Creates texture from file */
          pstTexture = orxTexture_CreateFromFile(zTextureName);

          /* Restores display debug level state */
          orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, bDisplayLevelEnabled);

          /* Not found? */
          if(pstTexture == orxNULL)
          {
            orxBITMAP *pstBitmap;

            /* Creates new bitmap */
            pstBitmap = orxDisplay_CreateBitmap(orxF2U(pstResult->fWidth), orxF2U(pstResult->fHeight));

            /* Valid? */
            if(pstBitmap != orxNULL)
            {
              /* Clears it */
              orxDisplay_ClearBitmap(pstBitmap, orx2RGBA(0, 0, 0, 0));

              /* Creates new texture */
              pstTexture = orxTexture_Create();

              /* Valid? */
              if(pstTexture != orxNULL)
              {
                /* Links them */
                if(orxTexture_LinkBitmap(pstTexture, pstBitmap, zTextureName) != orxSTATUS_FAILURE)
                {
                  /* Updates  owner flags */
                  u32OwnerFlags = 1;
                }
                else
                {
                  /* Deletes texture */
                  orxTexture_Delete(pstTexture);
                  pstTexture = orxNULL;

                  /* Deletes bitmap */
                  orxDisplay_DeleteBitmap(pstBitmap);
                }
              }
              else
              {
                /* Deletes bitmap */
                orxDisplay_DeleteBitmap(pstBitmap);
              }
            }
          }

          /* Valid? */
          if(pstTexture != orxNULL)
          {
            /* Sets it */
            orxViewport_SetTextureList(pstResult, 1, &pstTexture);

            /* Stores texture owner flags */
            pstResult->u32TextureOwnerFlags = u32OwnerFlags;

            /* Updates its owner */
            orxStructure_SetOwner(pstTexture, pstResult);

            /* Updates status flags */
            orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_INTERNAL_TEXTURES, orxVIEWPORT_KU32_FLAG_NONE);
          }
        }
      }

      /* *** Shader *** */

      /* Has shader? */
      if((s32Number = orxConfig_GetListCounter(orxVIEWPORT_KZ_CONFIG_SHADER_LIST)) > 0)
      {
        orxS32 i;

        /* For all defined shaders */
        for(i = 0; i < s32Number; i++)
        {
          /* Adds it */
          orxViewport_AddShader(pstResult, orxConfig_GetListString(orxVIEWPORT_KZ_CONFIG_SHADER_LIST, i));
        }
      }

      /* Has blend mode? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BLEND_MODE) != orxFALSE)
      {
        const orxSTRING zBlendMode;

        /* Gets blend mode value */
        zBlendMode = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_BLEND_MODE);

        /* Stores it */
        orxViewport_SetBlendMode(pstResult, orxDisplay_GetBlendModeFromString(zBlendMode));
      }
      else
      {
        /* Defaults to none */
        orxViewport_SetBlendMode(pstResult, orxDISPLAY_BLEND_MODE_NONE);
      }

      /* *** Camera *** */

      /* Gets its name */
      zCameraName = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_CAMERA);

      /* Valid? */
      if((zCameraName != orxNULL) && (zCameraName != orxSTRING_EMPTY))
      {
        orxCAMERA *pstCamera;

        /* Creates camera */
        pstCamera = orxCamera_CreateFromConfig(zCameraName);

        /* Valid? */
        if(pstCamera != orxNULL)
        {
          /* Sets it */
          orxViewport_SetCamera(pstResult, pstCamera);

          /* Updates its owner */
          orxStructure_SetOwner(pstCamera, pstResult);

          /* Updates flags */
          orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_INTERNAL_CAMERA, orxVIEWPORT_KU32_FLAG_NONE);
        }
      }

      /* Has background color? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR) != orxFALSE)
      {
        orxCOLOR stColor;

        /* Gets color vector */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR, &(stColor.vRGB));
        orxVector_Mulf(&(stColor.vRGB), &(stColor.vRGB), orxCOLOR_NORMALIZER);
        stColor.fAlpha = (orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BACKGROUND_ALPHA) != orxFALSE) ? orxConfig_GetFloat(orxVIEWPORT_KZ_CONFIG_BACKGROUND_ALPHA) : orxFLOAT_1;

        /* Applies it */
        orxViewport_SetBackgroundColor(pstResult, &stColor);
      }
      else
      {
        /* Clears background color */
        orxViewport_ClearBackgroundColor(pstResult);
      }

      /* Has relative size? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE) != orxFALSE)
      {
        orxVECTOR vRelSize;

        /* Gets it */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE, &vRelSize);

        /* Applies it */
        orxViewport_SetRelativeSize(pstResult, vRelSize.fX, vRelSize.fY);
      }

      /* Has relative position? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION) != orxFALSE)
      {
        orxCHAR   acBuffer[64];
        orxSTRING zRelativePos;
        orxU32    u32AlignmentFlags = orxVIEWPORT_KU32_FLAG_ALIGN_CENTER;

        /* Gets it */
        acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
        zRelativePos = orxString_LowerCase(orxString_NCopy(acBuffer, orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION), sizeof(acBuffer) - 1));

        /* Left? */
        if(orxString_SearchString(zRelativePos, orxVIEWPORT_KZ_LEFT) != orxNULL)
        {
          /* Updates alignment flags */
          u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_LEFT;
        }
        /* Right? */
        else if(orxString_SearchString(zRelativePos, orxVIEWPORT_KZ_RIGHT) != orxNULL)
        {
          /* Updates alignment flags */
          u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT;
        }

        /* Top? */
        if(orxString_SearchString(zRelativePos, orxVIEWPORT_KZ_TOP) != orxNULL)
        {
          /* Updates alignment flags */
          u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_TOP;
        }
        /* Bottom? */
        else if(orxString_SearchString(zRelativePos, orxVIEWPORT_KZ_BOTTOM) != orxNULL)
        {
          /* Updates alignment flags */
          u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM;
        }

        /* Applies it */
        orxViewport_SetRelativePosition(pstResult, u32AlignmentFlags);
      }

      /* Stores its reference key */
      pstResult->zReference = orxConfig_GetCurrentSection();

      /* Protects it */
      orxConfig_ProtectSection(pstResult->zReference, orxTRUE);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Config file does not contain viewport section named %s.", _zConfigID);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Deletes a viewport
 * @param[in]   _pstViewport    Viewport to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_Delete(orxVIEWPORT *_pstViewport)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstViewport);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstViewport) == 0)
  {
    /* Removes camera */
    orxViewport_SetCamera(_pstViewport, orxNULL);

    /* Was linked to textures? */
    if(_pstViewport->u32TextureCounter != 0)
    {
      /* Removes them */
      orxViewport_SetTextureList(_pstViewport, 0, orxNULL);
    }

    /* Had a shader pointer? */
    if(_pstViewport->pstShaderPointer != orxNULL)
    {
      /* Updates its counter */
      orxStructure_DecreaseCounter(_pstViewport->pstShaderPointer);

      /* Was internally allocated? */
      if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL_SHADER) != orxFALSE)
      {
        /* Removes its owner */
        orxStructure_SetOwner(_pstViewport->pstShaderPointer, orxNULL);

        /* Deletes it */
        orxShaderPointer_Delete(_pstViewport->pstShaderPointer);
      }
    }

    /* Has reference? */
    if(_pstViewport->zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstViewport->zReference, orxFALSE);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstViewport);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a viewport alignment
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32AlignFlags  Alignment flags (must be OR'ed)
 */
void orxFASTCALL orxViewport_SetAlignment(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT((_u32AlignFlags & orxVIEWPORT_KU32_MASK_ALIGN) == _u32AlignFlags)

  /* Updates alignement flags */
  orxStructure_SetFlags(_pstViewport, _u32AlignFlags & orxVIEWPORT_KU32_MASK_ALIGN, orxVIEWPORT_KU32_MASK_ALIGN);

  return;
}

/** Sets a viewport texture list
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32TextureNumber Number of textures to associate with the viewport
 * @param[in]   _apstTextureList List of textures to associate with the viewport
 */
void orxFASTCALL orxViewport_SetTextureList(orxVIEWPORT *_pstViewport, orxU32 _u32TextureNumber, orxTEXTURE **_apstTextureList)
{
  orxU32 i;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_u32TextureNumber < orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER);

  /* For all associated textures */
  for(i = 0; i < _pstViewport->u32TextureCounter; i++)
  {
    /* Updates its reference counter */
    orxStructure_DecreaseCounter((_pstViewport->apstTextureList[i]));

    /* Was internally allocated? */
    if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL_TEXTURES) != orxFALSE)
    {
      /* Was bitmap owned? */
      if(_pstViewport->u32TextureOwnerFlags & (1 << i))
      {
        orxBITMAP *pstBitmap;

        /* Gets its linked bitmap */
        pstBitmap = orxTexture_GetBitmap(_pstViewport->apstTextureList[i]);

        /* Unlinks it */
        orxTexture_UnlinkBitmap(_pstViewport->apstTextureList[i]);

        /* Deletes it */
        orxDisplay_DeleteBitmap(pstBitmap);
      }

      /* Removes its owner */
      orxStructure_SetOwner(_pstViewport->apstTextureList[i], orxNULL);

      /* Deletes it */
      orxTexture_Delete(_pstViewport->apstTextureList[i]);
    }
  }

  /* Clears texture owner flags */
  _pstViewport->u32TextureOwnerFlags = 0;

  /* Has new textures? */
  if(_u32TextureNumber != 0)
  {
    orxFLOAT fTextureWidth, fTextureHeight;
    orxU32 u32TextureCounter;

    /* Checks */
    orxASSERT(_apstTextureList != orxNULL);

    /* For all new textures */
    for(i = 0, u32TextureCounter = 0; i < _u32TextureNumber; i++)
    {
      /* Checks */
      orxSTRUCTURE_ASSERT(_apstTextureList[i]);

      /* First texture? */
      if(i == 0)
      {
        /* Stores its size */
        orxTexture_GetSize(_apstTextureList[i], &fTextureWidth, &fTextureHeight);
      }
      else
      {
        orxFLOAT fWidth, fHeight;

        /* Gets its size */
        orxTexture_GetSize(_apstTextureList[i], &fWidth, &fHeight);

        /* Doesn't match first texture? */
        if((fWidth != fTextureWidth) || (fHeight != fTextureHeight))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't associate texture [%s] with viewport: texture size [%u, %u] doesn't match the original one [%u, %u].", orxTexture_GetName(_apstTextureList[i]), orxF2U(fWidth), orxF2U(fHeight), orxF2U(fTextureWidth), orxF2U(fTextureHeight));

          /* Skips that texture */
          continue;
        }
      }

      /* Stores it */
      _pstViewport->apstTextureList[u32TextureCounter++] = _apstTextureList[i];

      /* Updates its reference counter */
      orxStructure_IncreaseCounter(_apstTextureList[i]);
    }

    /* Updates texture counter */
    _pstViewport->u32TextureCounter = u32TextureCounter;
  }
  else
  {
    /* Updates texture counter */
    _pstViewport->u32TextureCounter = _u32TextureNumber;

    /* Deactivates viewport */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_ENABLED);
  }

  /* Done! */
  return;
}

/** Gets a viewport texture list
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32TextureNumber Number of textures to be retrieved
 * @param[out]  _apstTextureList List of textures associated with the viewport
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_GetTextureList(const orxVIEWPORT *_pstViewport, orxU32 _u32TextureNumber, orxTEXTURE **_apstTextureList)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_u32TextureNumber != 0);
  orxASSERT(_apstTextureList != orxNULL);

  /* Has texture? */
  if(_pstViewport->u32TextureCounter != 0)
  {
    orxU32 i;

    /* For all requested textures */
    for(i = 0; i < _u32TextureNumber; i++)
    {
      /* Stores it */
      _apstTextureList[i] = _pstViewport->apstTextureList[i];
    }
  }
  else
  {
    /* Gets screen texture */
    _apstTextureList[0] = orxTexture_GetScreenTexture();
  }

  /* Done! */
  return eResult;
}

/** Gets a viewport texture counter
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Number of textures associated with the viewport
 */
orxU32 orxFASTCALL orxViewport_GetTextureCounter(const orxVIEWPORT *_pstViewport)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates result */
  u32Result = (_pstViewport->u32TextureCounter != 0) ? _pstViewport->u32TextureCounter : 1;

  /* Done! */
  return u32Result;
}

/** Sets a viewport background color
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _pstColor        Color to use for background
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_SetBackgroundColor(orxVIEWPORT *_pstViewport, const orxCOLOR *_pstColor)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pstColor != orxNULL);

  /* Updates background color */
  orxColor_Copy(&(_pstViewport->stBackgroundColor), _pstColor);

  /* Updates its flag */
  orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_BACKGROUND_COLOR, orxVIEWPORT_KU32_FLAG_NONE);

  /* Done! */
  return eResult;
}

/** Clears viewport background color
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_ClearBackgroundColor(orxVIEWPORT *_pstViewport)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates its flag */
  orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_BACKGROUND_COLOR);

  /* Restores default color */
  _pstViewport->stBackgroundColor.fAlpha = orxFLOAT_1;
  orxVector_Copy(&(_pstViewport->stBackgroundColor.vRGB), &orxVECTOR_BLACK);

  /* Done! */
  return eResult;
}

/** Viewport has background color accessor
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxViewport_HasBackgroundColor(const orxVIEWPORT *_pstViewport)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates result */
  bResult = orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_BACKGROUND_COLOR);

  /* Done! */
  return bResult;
}

/** Gets a viewport texture
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pstColor       Viewport's color
 * @return      Current background color
 */
orxCOLOR *orxFASTCALL orxViewport_GetBackgroundColor(const orxVIEWPORT *_pstViewport, orxCOLOR *_pstColor)
{
  orxCOLOR *pstResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pstColor != orxNULL);

  /* Has color? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_BACKGROUND_COLOR))
  {
    /* Copies color */
    orxColor_Copy(_pstColor, &(_pstViewport->stBackgroundColor));

    /* Updates result */
    pstResult = _pstColor;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Background color not set on viewport.");

    /* Clears result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Enables / disables a viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _bEnable        Enable / disable
 */
void orxFASTCALL orxViewport_Enable(orxVIEWPORT *_pstViewport, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_ENABLED, orxVIEWPORT_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is a viewport enabled?
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxViewport_IsEnabled(const orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Tests */
  return(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_ENABLED));
}

/** Sets a viewport camera
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _pstCamera      Associated camera
 */
void orxFASTCALL orxViewport_SetCamera(orxVIEWPORT *_pstViewport, orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has already a camera? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_CAMERA) != orxFALSE)
  {
    /* Updates its reference counter */
    orxStructure_DecreaseCounter((_pstViewport->pstCamera));

    /* Was internally allocated? */
    if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL_CAMERA) != orxFALSE)
    {
      /* Removes its owner */
      orxStructure_SetOwner(_pstViewport->pstCamera, orxNULL);

      /* Deletes it */
      orxCamera_Delete(_pstViewport->pstCamera);
    }

    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_CAMERA | orxVIEWPORT_KU32_FLAG_INTERNAL_CAMERA);
  }

  /* Updates camera pointer */
  _pstViewport->pstCamera = _pstCamera;

  /* Has a new camera? */
  if(_pstCamera != orxNULL)
  {
    /* Updates its reference counter */
    orxStructure_IncreaseCounter(_pstCamera);

    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_CAMERA, orxVIEWPORT_KU32_FLAG_NONE);
  }

  return;
}

/** Gets a viewport camera
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Associated camera / orxNULL
 */
orxCAMERA *orxFASTCALL orxViewport_GetCamera(const orxVIEWPORT *_pstViewport)
{
  orxCAMERA *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has a camera? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_CAMERA) != orxFALSE)
  {
    /* Updates result */
    pstResult = _pstViewport->pstCamera;
  }

  /* Done! */
  return pstResult;
}

/** Adds a shader to a viewport using its config ID
 * @param[in]   _pstViewport      Concerned Viewport
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_AddShader(orxVIEWPORT *_pstViewport, const orxSTRING _zShaderConfigID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT((_zShaderConfigID != orxNULL) && (_zShaderConfigID != orxSTRING_EMPTY));

  /* Is object active? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_ENABLED))
  {
    /* No shader pointer? */
    if(_pstViewport->pstShaderPointer == orxNULL)
    {
      /* Creates one */
      _pstViewport->pstShaderPointer = orxShaderPointer_Create();

      /* Valid? */
      if(_pstViewport->pstShaderPointer != orxNULL)
      {
        /* Updates its counter */
        orxStructure_IncreaseCounter(_pstViewport->pstShaderPointer);

        /* Updates flags */
        orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL_SHADER, orxVIEWPORT_KU32_FLAG_NONE);

        /* Updates its owner */
        orxStructure_SetOwner(_pstViewport->pstShaderPointer, _pstViewport);

        /* Adds shader from config */
        eResult = orxShaderPointer_AddShaderFromConfig(_pstViewport->pstShaderPointer, _zShaderConfigID);
      }
    }
    else
    {
      /* Adds shader from config */
      eResult = orxShaderPointer_AddShaderFromConfig(_pstViewport->pstShaderPointer, _zShaderConfigID);
    }
  }

  /* Done! */
  return eResult;
}

/** Removes a shader using its config ID
 * @param[in]   _pstViewport      Concerned viewport
 * @param[in]   _zShaderConfigID Config ID of the shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_RemoveShader(orxVIEWPORT *_pstViewport, const orxSTRING _zShaderConfigID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Valid? */
  if(_pstViewport->pstShaderPointer != orxNULL)
  {
    /* Removes shader from config */
    eResult = orxShaderPointer_RemoveShaderFromConfig(_pstViewport->pstShaderPointer, _zShaderConfigID);
  }

  /* Done! */
  return eResult;
}

/** Enables a viewport's shader
 * @param[in]   _pstViewport      Concerned viewport
 * @param[in]   _bEnable          Enable / disable
 */
void orxFASTCALL orxViewport_EnableShader(orxVIEWPORT *_pstViewport, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has a shader pointer? */
  if(_pstViewport->pstShaderPointer != orxNULL)
  {
    /* Enables it */
    orxShaderPointer_Enable(_pstViewport->pstShaderPointer, _bEnable);
  }

  /* Done! */
  return;
}

/** Is a viewport's shader enabled?
 * @param[in]   _pstViewport      Concerned viewport
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxViewport_IsShaderEnabled(const orxVIEWPORT *_pstViewport)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has a shader pointer? */
  if(_pstViewport->pstShaderPointer != orxNULL)
  {
    /* Updates result */
    bResult = orxShaderPointer_IsEnabled(_pstViewport->pstShaderPointer);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets a viewport's shader pointer
 * @param[in]   _pstViewport      Concerned viewport
 * @return      orxSHADERPOINTER / orxNULL
 */
const orxSHADERPOINTER *orxFASTCALL orxViewport_GetShaderPointer(const orxVIEWPORT *_pstViewport)
{
  orxSHADERPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates result */
  pstResult = _pstViewport->pstShaderPointer;

  /* Done! */
  return pstResult;
}

/** Sets a viewport blend mode (only used when has active shaders attached)
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _eBlendMode     Blend mode to set
 */
orxSTATUS orxFASTCALL orxViewport_SetBlendMode(orxVIEWPORT *_pstViewport, orxDISPLAY_BLEND_MODE _eBlendMode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT((_eBlendMode == orxDISPLAY_BLEND_MODE_NONE) || (_eBlendMode < orxDISPLAY_BLEND_MODE_NUMBER));

  /* Stores blend mode */
  _pstViewport->eBlendMode = _eBlendMode;

  /* Done! */
  return eResult;
}

/** Gets a viewport blend mode
 * @param[in]   _pstViewport    Concerned viewport
 * @return orxDISPLAY_BLEND_MODE
 */
orxDISPLAY_BLEND_MODE orxFASTCALL orxViewport_GetBlendMode(const orxVIEWPORT *_pstViewport)
{
  orxDISPLAY_BLEND_MODE eResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates result */
  eResult = _pstViewport->eBlendMode;

  /* Done! */
  return eResult;
}

/** Sets a viewport position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fX             X axis position (top left corner)
 * @param[in]   _fY             Y axis position (top left corner)
 */
void orxFASTCALL orxViewport_SetPosition(orxVIEWPORT *_pstViewport, orxFLOAT _fX, orxFLOAT _fY)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates position */
  _pstViewport->fX = _fX;
  _pstViewport->fY = _fY;

  return;
}

/** Sets a viewport relative position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32AlignFlags  Alignment flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_SetRelativePosition(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags)
{
  orxTEXTURE *pstTexture;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT((_u32AlignFlags & orxVIEWPORT_KU32_MASK_ALIGN) == _u32AlignFlags);
  orxASSERT(_pstViewport->fWidth > orxFLOAT_0);
  orxASSERT(_pstViewport->fHeight > orxFLOAT_0);

  /* Gets first associated texture */
  orxViewport_GetTextureList(_pstViewport, 1, &pstTexture);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    orxFLOAT fHeight, fWidth;

    /* Gets texture size */
    orxTexture_GetSize(pstTexture, &fWidth, &fHeight);

    /* Align left? */
    if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_LEFT)
    {
      /* Updates x position */
      _pstViewport->fX = orxFLOAT_0;
    }
    /* Align right? */
    else if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT)
    {
      /* Updates x position */
      _pstViewport->fX = fWidth - _pstViewport->fWidth;
    }
    /* Align center */
    else
    {
      /* Updates x position */
      _pstViewport->fX = orx2F(0.5f) * (fWidth - _pstViewport->fWidth);
    }

    /* Align top? */
    if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_TOP)
    {
      /* Updates y position */
      _pstViewport->fY = orxFLOAT_0;
    }
    /* Align bottom? */
    else if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM)
    {
      /* Updates y position */
      _pstViewport->fY = fHeight - _pstViewport->fHeight;
    }
    /* Align center */
    else
    {
      /* Updates y position */
      _pstViewport->fY = orx2F(0.5f) * (fHeight - _pstViewport->fHeight);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Invalid texture in viewport.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a viewport position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pfX            X axis position (top left corner)
 * @param[out]  _pfY            Y axis position (top left corner)
 */
void orxFASTCALL orxViewport_GetPosition(const orxVIEWPORT *_pstViewport, orxFLOAT *_pfX, orxFLOAT *_pfY)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pfX != orxNULL);
  orxASSERT(_pfY != orxNULL);

  /* Gets position */
  *_pfX = _pstViewport->fX;
  *_pfY = _pstViewport->fY;

  return;
}

/** Sets a viewport size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fW             Width
 * @param[in]   _fH             Height
 */
void orxFASTCALL orxViewport_SetSize(orxVIEWPORT *_pstViewport, orxFLOAT _fW, orxFLOAT _fH)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates size */
  _pstViewport->fWidth  = _fW;
  _pstViewport->fHeight = _fH;

  /* Done! */
  return;
}

/** Sets a viewport relative size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fW             Width (0.0f - 1.0f)
 * @param[in]   _fH             Height (0.0f - 1.0f)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxViewport_SetRelativeSize(orxVIEWPORT *_pstViewport, orxFLOAT _fW, orxFLOAT _fH)
{
  orxTEXTURE *pstTexture;
  orxSTATUS   eResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT((_fW >= orxFLOAT_0) && (_fW <= orxFLOAT_1));
  orxASSERT((_fH >= orxFLOAT_0) && (_fH <= orxFLOAT_1));

  /* Gets first associated texture */
  orxViewport_GetTextureList(_pstViewport, 1, &pstTexture);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    /* Updates viewport size */
    orxTexture_GetSize(pstTexture, &(_pstViewport->fWidth), &(_pstViewport->fHeight));
    _pstViewport->fWidth   *= _fW;
    _pstViewport->fHeight  *= _fH;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Invalid texture in viewport.");

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a viewport size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pfW            Width
 * @param[out]  _pfH            Height
 */
void orxFASTCALL orxViewport_GetSize(const orxVIEWPORT *_pstViewport, orxFLOAT *_pfW, orxFLOAT *_pfH)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pfW != orxNULL);
  orxASSERT(_pfH != orxNULL);

  /* Gets size */
  *_pfW = _pstViewport->fWidth;
  *_pfH = _pstViewport->fHeight;

  return;
}

/** Gets a viewport relative size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _f32W           Relative width
 * @param[out]  _f32H           Relative height
 */
void orxFASTCALL orxViewport_GetRelativeSize(const orxVIEWPORT *_pstViewport, orxFLOAT *_pfW, orxFLOAT *_pfH)
{
  orxTEXTURE *pstTexture;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pfW != orxNULL);
  orxASSERT(_pfH != orxNULL);

  /* Gets first associated texture */
  orxViewport_GetTextureList(_pstViewport, 1, &pstTexture);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    /* Gets relative size */
    orxTexture_GetSize(pstTexture, _pfW, _pfH);
    *_pfW = _pstViewport->fWidth / *_pfW;
    *_pfH = _pstViewport->fHeight / *_pfH;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Invalid texture in viewport.");

    /* Empties result */
    *_pfW = orxFLOAT_0;
    *_pfH = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Gets an axis aligned box of viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pstBox         Output box
 * @return orxAABOX / orxNULL
 */
orxAABOX *orxFASTCALL orxViewport_GetBox(const orxVIEWPORT *_pstViewport, orxAABOX *_pstBox)
{
  orxAABOX *pstResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pstBox != orxNULL);

  /* Updates result */
  pstResult = _pstBox;

  /* Sets its values */
  orxVector_Set(&(pstResult->vTL), _pstViewport->fX, _pstViewport->fY, orxFLOAT_0);
  orxVector_Copy(&(pstResult->vBR), &(pstResult->vTL));
  pstResult->vBR.fX += _pstViewport->fWidth;
  pstResult->vBR.fY += _pstViewport->fHeight;

  /* Done! */
  return pstResult;
}

/** Get viewport correction ratio
 * @param[in]   _pstViewport  Concerned viewport
 * @return      Correction ratio value
 */
orxFLOAT orxFASTCALL orxViewport_GetCorrectionRatio(const orxVIEWPORT *_pstViewport)
{
  orxCAMERA  *pstCamera;
  orxFLOAT    fResult = orxFLOAT_1;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Gets camera */
  pstCamera = orxViewport_GetCamera(_pstViewport);

  /* Valid? */
  if(pstCamera != orxNULL)
  {
    orxFLOAT fCameraWidth, fCameraHeight;
    orxAABOX stFrustum;

    /* Gets camera frustum */
    orxCamera_GetFrustum(pstCamera, &stFrustum);

    /* Gets camera size */
    fCameraWidth  = stFrustum.vBR.fX - stFrustum.vTL.fX;
    fCameraHeight = stFrustum.vBR.fY - stFrustum.vTL.fY;

    /* Updates result */
    fResult = (_pstViewport->fHeight * fCameraWidth) / ( _pstViewport->fWidth * fCameraHeight);
  }

  /* Done! */
  return fResult;
}

/** Gets viewport config name
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxViewport_GetName(const orxVIEWPORT *_pstViewport)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates result */
  zResult = (_pstViewport->zReference != orxNULL) ? _pstViewport->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

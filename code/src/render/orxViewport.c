/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxViewport.c
 * @date 14/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "render/orxViewport.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/** Module flags
 */
#define orxVIEWPORT_KU32_STATIC_FLAG_NONE     0x00000000  /**< No flags */

#define orxVIEWPORT_KU32_STATIC_FLAG_READY    0x00000001  /**< Ready flag */

#define orxVIEWPORT_KU32_STATIC_MASK_ALL      0xFFFFFFFF  /**< All mask */

/** orxVIEWPORT flags / masks
 */
#define orxVIEWPORT_KU32_FLAG_NONE            0x00000000  /**< No flags */

#define orxVIEWPORT_KU32_FLAG_ENABLED         0x00000001  /**< Enabled flag */
#define orxVIEWPORT_KU32_FLAG_CAMERA          0x00000002  /**< Has camera flag */
#define orxVIEWPORT_KU32_FLAG_TEXTURE         0x00000004  /**< Has texture flag */
#define orxVIEWPORT_KU32_FLAG_CLEAR           0x00000008  /**< Clear background before render flag */
#define orxVIEWPORT_KU32_FLAG_INTERNAL        0x10000000  /**< Internal structure handling flag  */

#define orxVIEWPORT_KU32_FLAG_DEFAULT         0x00000009  /**< Default flags */

#define orxVIEWPORT_KU32_MASK_ALIGN           0xF0000000  /**< Alignment mask */

#define orxVIEWPORT_KU32_MASK_ALL             0xFFFFFFFF  /** All mask */


/** Misc defines
 */
#define orxVIEWPORT_KZ_CONFIG_TEXTURE_NAME    "Texture"
#define orxVIEWPORT_KZ_CONFIG_POSITION        "Position"
#define orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION "RelativePosition"
#define orxVIEWPORT_KZ_CONFIG_SIZE            "Size"
#define orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE   "RelativeSize"
#define orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR "BackgroundColor"
#define orxVIEWPORT_KZ_CONFIG_CAMERA          "Camera"
#define orxVIEWPORT_KZ_CONFIG_BACKGROUND_CLEAR "BackgroundClear"

#define orxVIEWPORT_KZ_LEFT                   "left"
#define orxVIEWPORT_KZ_RIGHT                  "right"
#define orxVIEWPORT_KZ_TOP                    "top"
#define orxVIEWPORT_KZ_BOTTOM                 "bottom"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Viewport structure
 */
struct __orxVIEWPORT_t
{
  orxSTRUCTURE  stStructure;                  /**< Public structure, first structure member : 16 */
  orxFLOAT      fX;                           /**< X position (top left corner) : 20 */
  orxFLOAT      fY;                           /**< Y position (top left corner) : 24 */
  orxFLOAT      fWidth;                       /**< Width : 28 */
  orxFLOAT      fHeight;                      /**< Height : 32 */
  orxCAMERA    *pstCamera;                    /**< Associated camera : 36 */
  orxTEXTURE   *pstTexture;                   /**< Associated texture : 40 */
  orxRGBA       stBackgroundColor;            /**< Background color : 48 */

  orxPAD(48)
};


/** Static structure
 */
typedef struct __orxVIEWPORT_STATIC_t
{
  orxU32 u32Flags;                            /**< Control flags : 4 */

} orxVIEWPORT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxVIEWPORT_STATIC sstViewport;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all viewports
 */
orxSTATIC orxINLINE orxVOID orxViewport_DeleteAll()
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
orxVOID orxViewport_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_TEXTURE);
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_CAMERA);

  return;
}

/** Inits the Viewport module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxViewport_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstViewport, sizeof(orxVIEWPORT_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(VIEWPORT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to initialize viewport module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult == orxSTATUS_SUCCESS)
  {
    /* Inits Flags */
    sstViewport.u32Flags = orxVIEWPORT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to register linked list structure while initializing viewport module.");
  }

  /* Done! */
  return eResult;
}

/** Exits from the Viewport module
 */
orxVOID orxViewport_Exit()
{
  /* Initialized? */
  if(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY)
  {
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
orxVIEWPORT *orxViewport_Create()
{
  orxVIEWPORT *pstViewport = orxNULL;

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
orxVIEWPORT *orxFASTCALL orxViewport_CreateFromConfig(orxCONST orxSTRING _zConfigID)
{
  orxVIEWPORT *pstResult;
  orxSTRING   zPreviousSection;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (*_zConfigID != *orxSTRING_EMPTY));

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects section */
  if(orxConfig_SelectSection(_zConfigID) != orxSTATUS_FAILURE)
  {
    /* Creates viewport */
    pstResult = orxViewport_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxSTRING zTextureName, zCameraName;

      /* *** Texture *** */

      /* Gets its name */
      zTextureName = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_TEXTURE_NAME);

      /* Valid? */
      if((zTextureName != orxNULL) && (*zTextureName != *orxSTRING_EMPTY))
      {
        orxTEXTURE *pstTexture;

        /* Creates texture */
        pstTexture = orxTexture_CreateFromFile(zTextureName);

        /* Valid? */
        if(pstTexture != orxNULL)
        {
          /* Sets it */
          orxViewport_SetTexture(pstResult, pstTexture);

          /* Updates status flags */
          orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_INTERNAL, orxVIEWPORT_KU32_FLAG_NONE);
        }
      }

      /* *** Camera *** */

      /* Gets its name */
      zCameraName = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_CAMERA);

      /* Valid? */
      if((zCameraName != orxNULL) && (*zCameraName != *orxSTRING_EMPTY))
      {
        orxCAMERA *pstCamera;

        /* Creates camera */
        pstCamera = orxCamera_CreateFromConfig(zCameraName);

        /* Valid? */
        if(pstCamera != orxNULL)
        {
          /* Sets it */
          orxViewport_SetCamera(pstResult, pstCamera);
        }
      }

      /* Shouldn't clear before rendering? */
      if((orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BACKGROUND_CLEAR) != orxFALSE)
      && (orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_BACKGROUND_CLEAR) == orxFALSE))
      {
        /* Updates background clearing */
        orxViewport_EnableBackgroundClearing(pstResult, orxFALSE);
      }
      else
      {
        /* Updates background clearing */
        orxViewport_EnableBackgroundClearing(pstResult, orxTRUE);
      }

      /* Has background color? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR) != orxFALSE)
      {
        orxVECTOR vColor;
        orxRGBA   stColor;

        /* Gets color vector */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR, &vColor);

        /* Gets RGBA color */
        stColor = orx2RGBA(orxF2U(vColor.fX), orxF2U(vColor.fY), orxF2U(vColor.fZ), 0);

        /* Applies it */
        orxViewport_SetBackgroundColor(pstResult, stColor);
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
      /* Has plain size */
      else if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_SIZE) != orxFALSE)
      {
        orxVECTOR vSize;

        /* Gets it */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_SIZE, &vSize);

        /* Applies it */
        orxViewport_SetSize(pstResult, vSize.fX, vSize.fY);
      }

      /* Has relative position? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION) != orxFALSE)
      {
        orxSTRING zRelativePos;
        orxU32    u32AlignmentFlags = orxVIEWPORT_KU32_FLAG_ALIGN_CENTER;

        /* Gets it */
        zRelativePos = orxString_LowerCase(orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION));

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
      /* Has plain position */
      else if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_POSITION) != orxFALSE)
      {
        orxVECTOR vPos;

        /* Gets it */
        orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_POSITION, &vPos);

        /* Applies it */
        orxViewport_SetPosition(pstResult, vPos.fX, vPos.fY);
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
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

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstViewport) == 0)
  {
    /* Was linked to a camera? */
    if(_pstViewport->pstCamera != orxNULL)
    {
      /* Removes its reference */
      orxStructure_DecreaseCounter((_pstViewport->pstCamera));
    }

    /* Was linked to a texture? */
    if(_pstViewport->pstTexture != orxNULL)
    {
      /* Removes its reference */
      orxStructure_DecreaseCounter((_pstViewport->pstTexture));

      /* Was internally allocated? */
      if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL) != orxFALSE)
      {
        /* Deletes texture */
        orxTexture_Delete(_pstViewport->pstTexture);
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstViewport);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Cannot delete viewport while it is still referenced.");

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
orxVOID orxFASTCALL orxViewport_SetAlignment(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT((_u32AlignFlags & orxVIEWPORT_KU32_MASK_ALIGN) == _u32AlignFlags)

  /* Updates alignement flags */
  orxStructure_SetFlags(_pstViewport, _u32AlignFlags & orxVIEWPORT_KU32_MASK_ALIGN, orxVIEWPORT_KU32_MASK_ALIGN);

  return;
}

/** Sets a viewport texture
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _pstTexture     Texture to associate with the viewport
 */
orxVOID orxFASTCALL orxViewport_SetTexture(orxVIEWPORT *_pstViewport, orxTEXTURE *_pstTexture)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has already a texture? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_TEXTURE) != orxFALSE)
  {
    /* Updates previous texture reference counter */
    orxStructure_DecreaseCounter((_pstViewport->pstTexture));

    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_TEXTURE);
  }

  /* Updates texture pointer */
  _pstViewport->pstTexture = _pstTexture;

  /* Has a new texture? */
  if(_pstTexture != orxNULL)
  {
    /* Updates texture reference counter */
    orxStructure_IncreaseCounter(_pstTexture);

    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_TEXTURE, orxVIEWPORT_KU32_FLAG_NONE);

    /* Reinits relative size & position */
    _pstViewport->fX = _pstViewport->fY = orxFLOAT_0;
    orxViewport_SetRelativeSize(_pstViewport, orxFLOAT_1, orxFLOAT_1);
  }
  else
  {
    /* Deactivates viewport */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_ENABLED);
  }

  return;
}

/** Gets a viewport texture
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Associated orxTEXTURE / orxNULL
 */
orxTEXTURE *orxFASTCALL orxViewport_GetTexture(orxCONST orxVIEWPORT *_pstViewport)
{
  orxTEXTURE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has texture? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_TEXTURE) != orxFALSE)
  {
    /* Updates result */
    pstResult = _pstViewport->pstTexture;
  }
  else
  {
    /* Gets screen texture */
    pstResult = orxTexture_GetScreenTexture();
  }

  /* Done! */
  return pstResult;
}

/** Sets a viewport background color
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _stColor        Color to use for background
 */
orxVOID orxFASTCALL orxViewport_SetBackgroundColor(orxVIEWPORT *_pstViewport, orxRGBA _stColor)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates background color */
  _pstViewport->stBackgroundColor = _stColor;

  return;
}

/** Gets a viewport texture
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Current background color
 */
orxRGBA orxFASTCALL orxViewport_GetBackgroundColor(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Done! */
  return(_pstViewport->stBackgroundColor);
}

/** Enables / disables a viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _bEnable        Enable / disable
 */
orxVOID orxFASTCALL orxViewport_Enable(orxVIEWPORT *_pstViewport, orxBOOL _bEnable)
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
orxBOOL orxFASTCALL orxViewport_IsEnabled(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Tests */
  return(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_ENABLED));
}

/** Enables / disables background clearing for a viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _bEnable        Enable / disable
 */
orxVOID orxFASTCALL orxViewport_EnableBackgroundClearing(orxVIEWPORT *_pstViewport, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_CLEAR, orxVIEWPORT_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_CLEAR);
  }

  return;
}

/** Has a viewport background clearing enabled?
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxViewport_IsBackgroundClearingEnabled(orxCONST orxVIEWPORT *_pstViewport)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Tests */
  return(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_CLEAR));
}

/** Sets a viewport camera
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _pstCamera      Associated camera
 */
orxVOID orxFASTCALL orxViewport_SetCamera(orxVIEWPORT *_pstViewport, orxCAMERA *_pstCamera)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Has already a camera? */
  if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_CAMERA) != orxFALSE)
  {
    /* Updates its reference counter */
    orxStructure_DecreaseCounter((_pstViewport->pstCamera));

    /* Updates flags */
    orxStructure_SetFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_NONE, orxVIEWPORT_KU32_FLAG_CAMERA);
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
orxCAMERA *orxFASTCALL orxViewport_GetCamera(orxCONST orxVIEWPORT *_pstViewport)
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

/** Sets a viewport position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fX             X axis position (top left corner)
 * @param[in]   _fY             Y axis position (top left corner)
 */
orxVOID orxFASTCALL orxViewport_SetPosition(orxVIEWPORT *_pstViewport, orxFLOAT _fX, orxFLOAT _fY)
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

  /* Gets associated texture */
  pstTexture = orxViewport_GetTexture(_pstViewport);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    orxFLOAT fHeight, fWidth;

    /* Gets texture size */
    fWidth  = orxTexture_GetWidth(pstTexture);
    fHeight = orxTexture_GetHeight(pstTexture);

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
orxVOID orxFASTCALL orxViewport_GetPosition(orxCONST orxVIEWPORT *_pstViewport, orxFLOAT *_pfX, orxFLOAT *_pfY)
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
orxVOID orxFASTCALL orxViewport_SetSize(orxVIEWPORT *_pstViewport, orxFLOAT _fW, orxFLOAT _fH)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates size */
  _pstViewport->fWidth  = _fW;
  _pstViewport->fHeight = _fH;

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
  orxASSERT((_fW >= 0.0f));
  orxASSERT((_fH >= 0.0f));

  /* Gets viewport texture */
  pstTexture = orxViewport_GetTexture(_pstViewport);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    /* Updates viewport size */
    _pstViewport->fWidth  = orxTexture_GetWidth(pstTexture) * _fW;
    _pstViewport->fHeight = orxTexture_GetHeight(pstTexture) * _fH;

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
orxVOID orxFASTCALL orxViewport_GetSize(orxCONST orxVIEWPORT *_pstViewport, orxFLOAT *_pfW, orxFLOAT *_pfH)
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
orxVOID orxFASTCALL orxViewport_GetRelativeSize(orxCONST orxVIEWPORT *_pstViewport, orxFLOAT *_pfW, orxFLOAT *_pfH)
{
  orxTEXTURE *pstTexture;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pfW != orxNULL);
  orxASSERT(_pfH != orxNULL);

  /* Gets viewport texture */
  pstTexture = orxViewport_GetTexture(_pstViewport);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    /* Gets relative size */
    *_pfW = _pstViewport->fWidth / orxTexture_GetWidth(pstTexture);
    *_pfH = _pstViewport->fHeight / orxTexture_GetHeight(pstTexture);
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

/** Gets a viewport clipping
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pu32TLX        X coordinate of top left corner
 * @param[out]  _pu32TLY        Y coordinate of top left corner
 * @param[out]  _pu32BRX        X coordinate of bottom right corner
 * @param[out]  _pu32BRY        Y coordinate of bottom right corner
 */
orxVOID orxFASTCALL orxViewport_GetClipping(orxCONST orxVIEWPORT *_pstViewport, orxU32 *_pu32TLX, orxU32 *_pu32TLY, orxU32 *_pu32BRX, orxU32 *_pu32BRY)
{
  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);
  orxASSERT(_pu32TLX != orxNULL);
  orxASSERT(_pu32TLY != orxNULL);
  orxASSERT(_pu32BRX != orxNULL);
  orxASSERT(_pu32BRY != orxNULL);

  /* Gets corners coordinates */
  *_pu32TLX = orxF2U(_pstViewport->fX);
  *_pu32TLY = orxF2U(_pstViewport->fY);
  *_pu32BRX = orxF2U(_pstViewport->fX + _pstViewport->fWidth);
  *_pu32BRY = orxF2U(_pstViewport->fY + _pstViewport->fHeight);

  return;
}

/** Gets an axis aligned box of viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pstBox         Output box
 * @return orxAABOX / orxNULL
 */
orxAABOX *orxFASTCALL orxViewport_GetBox(orxCONST orxVIEWPORT *_pstViewport, orxAABOX *_pstBox)
{
  orxAABOX *pstResult = orxNULL;

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
orxFLOAT orxFASTCALL orxViewport_GetCorrectionRatio(orxCONST orxVIEWPORT *_pstViewport)
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
    fResult = (_pstViewport->fHeight / _pstViewport->fWidth) * (fCameraWidth / fCameraHeight);
  }

  /* Done! */
  return fResult;
}

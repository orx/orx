/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
#include "core/orxCommand.h"
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
#define orxVIEWPORT_KU32_FLAG_AUTO_RESIZE       0x00000008  /**< Auto-resize flag */
#define orxVIEWPORT_KU32_FLAG_FIXED_RATIO       0x00000010  /**< Fixed ratio flag */
#define orxVIEWPORT_KU32_FLAG_REFERENCED        0x00000020  /**< Referenced flag */
#define orxVIEWPORT_KU32_FLAG_INTERNAL_TEXTURES 0x00100000  /**< Internal texture handling flag  */
#define orxVIEWPORT_KU32_FLAG_INTERNAL_SHADER   0x00200000  /**< Internal shader pointer handling flag  */
#define orxVIEWPORT_KU32_FLAG_INTERNAL_CAMERA   0x00400000  /**< Internal camera handling flag  */

#define orxVIEWPORT_KU32_FLAG_DEFAULT           0x00000001  /**< Default flags */

#define orxVIEWPORT_KU32_MASK_ALIGN             0xF0000000  /**< Alignment mask */

#define orxVIEWPORT_KU32_MASK_ALL               0xFFFFFFFF  /** All mask */


/** Misc defines
 */
#define orxVIEWPORT_KU32_REFERENCE_TABLE_SIZE   16          /**< Reference table size */
#define orxVIEWPORT_KU32_BANK_SIZE              16          /**< Bank size */

#define orxVIEWPORT_KZ_CONFIG_TEXTURE           "Texture"
#define orxVIEWPORT_KZ_CONFIG_TEXTURE_LIST      "TextureList"
#define orxVIEWPORT_KZ_CONFIG_POSITION          "Position"
#define orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION "RelativePosition"
#define orxVIEWPORT_KZ_CONFIG_SIZE              "Size"
#define orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE     "RelativeSize"
#define orxVIEWPORT_KZ_CONFIG_USE_RELATIVE_SIZE "UseRelativeSize"
#define orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR  "BackgroundColor"
#define orxVIEWPORT_KZ_CONFIG_BACKGROUND_ALPHA  "BackgroundAlpha"
#define orxVIEWPORT_KZ_CONFIG_CAMERA            "Camera"
#define orxVIEWPORT_KZ_CONFIG_FIXED_RATIO       "FixedRatio"
#define orxVIEWPORT_KZ_CONFIG_SHADER_LIST       "ShaderList"
#define orxVIEWPORT_KZ_CONFIG_BLEND_MODE        "BlendMode"
#define orxVIEWPORT_KZ_CONFIG_AUTO_RESIZE       "AutoResize"
#define orxVIEWPORT_KZ_CONFIG_KEEP_IN_CACHE     "KeepInCache"
#define orxVIEWPORT_KZ_CONFIG_NO_DEBUG          "NoDebug"

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
  orxSTRUCTURE          stStructure;                                          /**< Public structure, first structure member : 40 */
  orxFLOAT              fX;                                                   /**< X position (top left corner) : 44 */
  orxFLOAT              fY;                                                   /**< Y position (top left corner) : 48 */
  orxFLOAT              fWidth;                                               /**< Width : 52 */
  orxFLOAT              fHeight;                                              /**< Height : 56 */
  orxCOLOR              stBackgroundColor;                                    /**< Background color : 72 */
  orxCAMERA            *pstCamera;                                            /**< Associated camera : 80 */
  orxSHADERPOINTER     *pstShaderPointer;                                     /**< Shader pointer : 88 */
  orxFLOAT              fFixedRatio;                                          /**< Fixed ratio : 92 */
  orxU32                u32TextureCount;                                      /**< Associated texture count : 96 */
  orxU32                u32TextureOwnerFlags;                                 /**< Texture owner flags : 100 */
  orxDISPLAY_BLEND_MODE eBlendMode;                                           /**< Blend mode : 104 */
  const orxSTRING       zReference;                                           /**< Reference : 112 */
  orxFLOAT              fRealX;                                               /**< X position (top left corner) : 116 */
  orxFLOAT              fRealY;                                               /**< Y position (top left corner) : 120 */
  orxFLOAT              fRealWidth;                                           /**< Width : 124 */
  orxFLOAT              fRealHeight;                                          /**< Height : 128 */
  orxTEXTURE           *apstTextureList[orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER]; /**< Associated texture list : 192 */
};

/** Static structure
 */
typedef struct __orxVIEWPORT_STATIC_t
{
  orxU32                u32Flags;                                             /**< Control flags : 4 */
  orxHASHTABLE         *pstReferenceTable;                                    /**< Table to avoid viewport duplication when creating through config file : 8 */

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

/** Semi-private, internal-use only forward declarations
 */
orxVECTOR *orxFASTCALL orxConfig_ToVector(const orxSTRING _zValue, orxVECTOR *_pvVector);

/** Command: Create
 */
void orxFASTCALL orxViewport_CommandCreate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Creates viewport */
  pstViewport = orxViewport_CreateFromConfig(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->u64Value = (pstViewport != orxNULL) ? orxStructure_GetGUID(pstViewport) : orxU64_UNDEFINED;

  /* Done! */
  return;
}

/** Command: Delete
 */
void orxFASTCALL orxViewport_CommandDelete(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Deletes it */
    orxViewport_Delete(pstViewport);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetID
 */
void orxFASTCALL orxViewport_CommandGetID(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetPosition
 */
void orxFASTCALL orxViewport_CommandSetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Sets its position */
    orxViewport_SetPosition(pstViewport, _astArgList[1].vValue.fX, _astArgList[1].vValue.fY);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetRelativePosition
 */
void orxFASTCALL orxViewport_CommandSetRelativePosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    orxCHAR         acBuffer[64];
    const orxSTRING zRelativePos;
    orxU32          u32AlignmentFlags = orxVIEWPORT_KU32_FLAG_ALIGN_CENTER;

    /* Gets its relative position */
    orxString_LowerCase(orxString_NCopy(acBuffer, _astArgList[1].zValue, sizeof(acBuffer) - 1));
    acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
    zRelativePos = orxString_SkipWhiteSpaces(orxString_LowerCase(acBuffer));

    /* Not empty? */
    if(*zRelativePos != orxCHAR_NULL)
    {
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
      orxViewport_SetRelativePosition(pstViewport, u32AlignmentFlags);
    }

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetSize
 */
void orxFASTCALL orxViewport_CommandSetSize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Sets its size */
    orxViewport_SetSize(pstViewport, _astArgList[1].vValue.fX, _astArgList[1].vValue.fY);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetRelativeSize
 */
void orxFASTCALL orxViewport_CommandSetRelativeSize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Sets its relative size */
    orxViewport_SetRelativeSize(pstViewport, _astArgList[1].vValue.fX, _astArgList[1].vValue.fY);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetPosition
 */
void orxFASTCALL orxViewport_CommandGetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Gets its position */
    orxViewport_GetPosition(pstViewport, &(_pstResult->vValue.fX), &(_pstResult->vValue.fY));
    _pstResult->vValue.fZ = orxFLOAT_0;
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: GetSize
 */
void orxFASTCALL orxViewport_CommandGetSize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Gets its size */
    orxViewport_GetSize(pstViewport, &(_pstResult->vValue.fX), &(_pstResult->vValue.fY));
    _pstResult->vValue.fZ = orxFLOAT_0;
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: GetRelativeSize
 */
void orxFASTCALL orxViewport_CommandGetRelativeSize(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Gets its relative size */
    orxViewport_GetRelativeSize(pstViewport, &(_pstResult->vValue.fX), &(_pstResult->vValue.fY));
    _pstResult->vValue.fZ = orxFLOAT_0;
  }
  else
  {
    /* Updates result */
    orxVector_Copy(&(_pstResult->vValue), &orxVECTOR_0);
  }

  /* Done! */
  return;
}

/** Command: Get
 */
void orxFASTCALL orxViewport_CommandGet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxViewport_Get(_astArgList[0].zValue);

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Updates result */
    _pstResult->u64Value = orxStructure_GetGUID(pstViewport);
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetName
 */
void orxFASTCALL orxViewport_CommandGetName(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Updates result */
  _pstResult->zValue = (pstViewport != orxNULL) ? orxViewport_GetName(pstViewport) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: Enable
 */
void orxFASTCALL orxViewport_CommandEnable(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Updates it */
    orxViewport_Enable(pstViewport, (_u32ArgNumber < 2) || (_astArgList[1].bValue != orxFALSE) ? orxTRUE : orxFALSE);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: IsEnabled
 */
void orxFASTCALL orxViewport_CommandIsEnabled(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Updates result */
    _pstResult->bValue = orxViewport_IsEnabled(pstViewport);
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: SetCamera
 */
void orxFASTCALL orxViewport_CommandSetCamera(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    orxCAMERA *pstCamera = orxNULL;

    /* Has camera? */
    if(_u32ArgNumber > 1)
    {
      /* Gets camera */
      pstCamera = orxCAMERA(orxStructure_Get(_astArgList[1].u64Value));
    }

    /* Updates viewport */
    orxViewport_SetCamera(pstViewport, pstCamera);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: GetCamera
 */
void orxFASTCALL orxViewport_CommandGetCamera(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    orxCAMERA *pstCamera;

    /* Gets camera */
    pstCamera = orxViewport_GetCamera(pstViewport);

    /* Updates result */
    _pstResult->u64Value = (pstCamera != orxNULL) ? orxStructure_GetGUID(pstCamera) : orxU64_UNDEFINED;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: SetBlendMode
 */
void orxFASTCALL orxViewport_CommandSetBlendMode(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Sets blend mode */
    orxViewport_SetBlendMode(pstViewport, orxDisplay_GetBlendModeFromString(_astArgList[1].zValue));

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: AddShader
 */
void orxFASTCALL orxViewport_CommandAddShader(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Adds shader */
    orxViewport_AddShader(pstViewport, _astArgList[1].zValue);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: RemoveShader
 */
void orxFASTCALL orxViewport_CommandRemoveShader(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Removes shader */
    orxViewport_RemoveShader(pstViewport, _astArgList[1].zValue);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: EnableShader
 */
void orxFASTCALL orxViewport_CommandEnableShader(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Enables shader */
    orxViewport_EnableShader(pstViewport, (_u32ArgNumber < 2) || (_astArgList[1].bValue != orxFALSE) ? orxTRUE : orxFALSE);

    /* Updates result */
    _pstResult->u64Value = _astArgList[0].u64Value;
  }
  else
  {
    /* Updates result */
    _pstResult->u64Value = orxU64_UNDEFINED;
  }

  /* Done! */
  return;
}

/** Command: IsShaderEnabled
 */
void orxFASTCALL orxViewport_CommandIsShaderEnabled(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Updates result */
    _pstResult->bValue = orxViewport_IsShaderEnabled(pstViewport);
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: GetCorrectionRatio
 */
void orxFASTCALL orxViewport_CommandGetCorrectionRatio(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxVIEWPORT *pstViewport;

  /* Gets viewport */
  pstViewport = orxVIEWPORT(orxStructure_Get(_astArgList[0].u64Value));

  /* Valid? */
  if(pstViewport != orxNULL)
  {
    /* Updates result */
    _pstResult->fValue = orxViewport_GetCorrectionRatio(pstViewport);
  }
  else
  {
    /* Updates result */
    _pstResult->fValue = orxFLOAT_0;
  }

  /* Done! */
  return;
}

/** Registers all the viewports commands
 */
static orxINLINE void orxViewport_RegisterCommands()
{
  /* Command: Create */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, Create, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Name", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: Delete */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, Delete, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: GetID */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetID, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, SetPosition, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Position", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: SetRelativePosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, SetRelativePosition, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"RelativePosition", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: SetSize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, SetSize, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Size", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: SetRelativeSize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, SetRelativeSize, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"RelativeSize", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: GetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetPosition, "Position", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetSize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetSize, "Size", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetRelativeSize */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetRelativeSize, "Size", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Get */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, Get, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Name", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetName */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetName, "Name", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: Enable */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, Enable, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Enable = true", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: IsEnabled */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, IsEnabled, "Enabled?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetCamera */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, SetCamera, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Camera = <void>", orxCOMMAND_VAR_TYPE_U64});
  /* Command: GetCamera */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetCamera, "Camera", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: SetBlendMode */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, SetBlendMode, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"BlendMode", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: AddShader */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, AddShader, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Shader", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: RemoveShader */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, RemoveShader, "Viewport", orxCOMMAND_VAR_TYPE_U64, 2, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Shader", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: EnableShader */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, EnableShader, "Viewport", orxCOMMAND_VAR_TYPE_U64, 1, 1, {"Viewport", orxCOMMAND_VAR_TYPE_U64}, {"Enable = true", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: IsShaderEnabled */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, IsShaderEnabled, "IsEnabled?", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});

  /* Command: GetCorrectionRatio */
  orxCOMMAND_REGISTER_CORE_COMMAND(Viewport, GetCorrectionRatio, "Ratio", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Viewport", orxCOMMAND_VAR_TYPE_U64});
}

/** Unregisters all the viewports commands
 */
static orxINLINE void orxViewport_UnregisterCommands()
{
  /* Command: Create */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, Create);
  /* Command: Delete */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, Delete);

  /* Command: GetID */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetID);

  /* Command: SetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, SetPosition);
  /* Command: SetRelativePosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, SetRelativePosition);
  /* Command: SetSize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, SetSize);
  /* Command: SetRelativeSize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, SetRelativeSize);
  /* Command: GetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetPosition);
  /* Command: GetSize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetSize);
  /* Command: GetRelativeSize */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetRelativeSize);

  /* Command: Get */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, Get);
  /* Command: GetName */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetName);

  /* Command: Enable */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, Enable);
  /* Command: IsEnabled */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, IsEnabled);

  /* Command: SetCamera */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, SetCamera);
  /* Command: GetCamera */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetCamera);

  /* Command: SetBlendMode */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, SetBlendMode);

  /* Command: AddShader */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, AddShader);
  /* Command: RemoveShader */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, RemoveShader);
  /* Command: EnableShader */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, EnableShader);
  /* Command: IsShaderEnabled */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, IsShaderEnabled);

  /* Command: GetCorrectionRatio */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Viewport, GetCorrectionRatio);
}

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
        orxVIEWPORT *pstViewport;

        /* For all viewports */
        for(pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT));
            pstViewport != orxNULL;
            pstViewport = orxVIEWPORT(orxStructure_GetNext(pstViewport)))
        {
          /* Is linked to screen? */
          if(pstViewport->u32TextureCount == 0)
          {
            /* Updates relative position & dimension */
            orxViewport_SetSize(pstViewport, pstViewport->fRealWidth * fWidthRatio, pstViewport->fRealHeight * fHeightRatio);
            orxViewport_SetPosition(pstViewport, pstViewport->fRealX * fWidthRatio, pstViewport->fRealY * fHeightRatio);

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
              orxViewport_SetSize(pstViewport, pstViewport->fRealWidth * fWidthRatio, pstViewport->fRealHeight * fHeightRatio);
              orxViewport_SetPosition(pstViewport, pstViewport->fRealX * fWidthRatio, pstViewport->fRealY * fHeightRatio);

              /* For all textures */
              for(i = 0; i < pstViewport->u32TextureCount; i++)
              {
                /* Is owned? */
                if(pstViewport->u32TextureOwnerFlags & (1 << i))
                {
                  orxBITMAP  *pstBitmap;
                  orxCHAR     acBuffer[256];

                  /* Gets its name */
                  acBuffer[orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s", orxTexture_GetName(pstViewport->apstTextureList[i]))] = orxCHAR_NULL;

                  /* Unlinks its bitmap */
                  orxTexture_UnlinkBitmap(pstViewport->apstTextureList[i]);

                  /* Re-creates it as the right size */
                  pstBitmap = orxDisplay_CreateBitmap(orxF2U(pstViewport->fWidth), orxF2U(pstViewport->fHeight));

                  /* Checks */
                  orxASSERT(pstBitmap != orxNULL);

                  /* Clears it */
                  orxDisplay_ClearBitmap(pstBitmap, orx2RGBA(0, 0, 0, 0));

                  /* Re-links it */
                  orxTexture_LinkBitmap(pstViewport->apstTextureList[i], pstBitmap, acBuffer, orxTRUE);
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
  orxModule_AddDependency(orxMODULE_ID_VIEWPORT, orxMODULE_ID_COMMAND);

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

    /* Creates reference table */
    sstViewport.pstReferenceTable = orxHashTable_Create(orxVIEWPORT_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstViewport.pstReferenceTable != orxNULL)
    {
      /* Adds event handler */
      eResult = orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxViewport_EventHandler);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Filters relevant event IDs */
        orxEvent_SetHandlerIDFlags(orxViewport_EventHandler, orxEVENT_TYPE_DISPLAY, orxNULL, orxEVENT_GET_FLAG(orxDISPLAY_EVENT_SET_VIDEO_MODE), orxEVENT_KU32_MASK_ID_ALL);

        /* Registers structure type */
        eResult = orxSTRUCTURE_REGISTER(VIEWPORT, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxVIEWPORT_KU32_BANK_SIZE, orxNULL);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Registers commands */
          orxViewport_RegisterCommands();
        }
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

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    sstViewport.u32Flags = orxVIEWPORT_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Has reference table? */
    if(sstViewport.pstReferenceTable != orxNULL)
    {
      /* Deletes it */
      orxHashTable_Delete(sstViewport.pstReferenceTable);
      sstViewport.pstReferenceTable = orxNULL;
    }

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxViewport_EventHandler);

    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Initializing viewport module failed.");
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
    /* Unregisters commands */
    orxViewport_UnregisterCommands();

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxViewport_EventHandler);

    /* Deletes viewport list */
    orxViewport_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_VIEWPORT);

    /* Deletes reference table */
    orxHashTable_Delete(sstViewport.pstReferenceTable);
    sstViewport.pstReferenceTable = orxNULL;

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
    pstViewport->fX = pstViewport->fY = pstViewport->fRealX = pstViewport->fRealY = orxFLOAT_0;

    /* Sets default size */
    orxViewport_SetRelativeSize(pstViewport, orxFLOAT_1, orxFLOAT_1);

    /* Increases count */
    orxStructure_IncreaseCount(pstViewport);
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
      orxVECTOR       vSize;
      const orxSTRING zCameraName;
      orxS32          s32Number;
      orxBOOL         bFixedSize = orxFALSE, bFixedPosition = orxFALSE;

      /* No debug? */
      if(orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_NO_DEBUG) != orxFALSE)
      {
        /* Updates flags */
        orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_NO_DEBUG, orxVIEWPORT_KU32_FLAG_NONE);
      }

      /* Has plain size */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_SIZE) != orxFALSE)
      {
        /* Gets it */
        if(orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_SIZE, &vSize) != orxNULL)
        {
          /* Don't use relative size? */
          if(orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_USE_RELATIVE_SIZE) == orxFALSE)
          {
            /* Applies it */
            orxViewport_SetSize(pstResult, vSize.fX, vSize.fY);
          }

          /* Updates status */
          bFixedSize = orxTRUE;
        }
      }

      /* *** Textures *** */

      /* Has texture list? */
      if((s32Number = orxConfig_GetListCount(orxVIEWPORT_KZ_CONFIG_TEXTURE_LIST)) > 0)
      {
        orxS32      i, s32TextureCount;
        orxU32      u32OwnerFlags = 0;
        orxTEXTURE *apstTextureList[orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER];

        /* For all entries */
        for(i = 0, s32TextureCount = 0; i < s32Number; i++)
        {
          const orxSTRING zTextureName;

          /* Gets its name */
          zTextureName = orxConfig_GetListString(orxVIEWPORT_KZ_CONFIG_TEXTURE_LIST, i);

          /* Valid? */
          if((zTextureName != orxNULL) && (zTextureName != orxSTRING_EMPTY))
          {
            orxTEXTURE *pstTexture;
            orxBOOL     bDisplayLevelEnabled;

            /* Gets display debug level state */
            bDisplayLevelEnabled = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_DISPLAY);

            /* Deactivates display debug level */
            orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_DISPLAY, orxFALSE);

            /* Loads texture */
            pstTexture = orxTexture_Load(zTextureName, orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_KEEP_IN_CACHE));

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
                  if(orxTexture_LinkBitmap(pstTexture, pstBitmap, zTextureName, orxTRUE) != orxSTATUS_FAILURE)
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
              apstTextureList[s32TextureCount++] = pstTexture;
            }
            else
            {
              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_DISPLAY, "Can't link texture [%s] to viewport [%s], skipping.", zTextureName, _zConfigID);
            }
          }
        }

        /* Has valid textures? */
        if(s32TextureCount > 0)
        {
          orxS32 j;

          /* Stores them */
          orxViewport_SetTextureList(pstResult, (orxU32)s32TextureCount, apstTextureList);

          /* For all secondary textures */
          for(j = 1; j < s32TextureCount; j++)
          {
            orxU32  k;
            orxBOOL bStored = orxFALSE;

            for(k = 1; k < pstResult->u32TextureCount; k++)
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
        zTextureName = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_TEXTURE);

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

          /* Loads texture */
          pstTexture = orxTexture_Load(zTextureName, orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_KEEP_IN_CACHE));

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
                if(orxTexture_LinkBitmap(pstTexture, pstBitmap, zTextureName, orxTRUE) != orxSTATUS_FAILURE)
                {
                  /* Updates owner flags */
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
      if((s32Number = orxConfig_GetListCount(orxVIEWPORT_KZ_CONFIG_SHADER_LIST)) > 0)
      {
        orxS32 i;

        /* For all defined shaders */
        for(i = 0; i < s32Number; i++)
        {
          const orxSTRING zShader;

          /* Gets its name */
          zShader = orxConfig_GetListString(orxVIEWPORT_KZ_CONFIG_SHADER_LIST, i);

          /* Valid? */
          if(zShader != orxSTRING_EMPTY)
          {
            /* Adds it */
            orxViewport_AddShader(pstResult, zShader);
          }
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
      else
      {
        orxFLOAT fRatio;

        /* Gets fixed ratio */
        fRatio = orxConfig_GetFloat(orxVIEWPORT_KZ_CONFIG_FIXED_RATIO);

        /* Valid? */
        if(fRatio > orxFLOAT_0)
        {
          /* Stores it */
          pstResult->fFixedRatio = fRatio;

          /* Updates flags */
          orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_FIXED_RATIO, orxVIEWPORT_KU32_FLAG_NONE);
        }
      }

      /* Has fixed size? */
      if(bFixedSize != orxFALSE)
      {
        /* Use relative size? */
        if(orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_USE_RELATIVE_SIZE) != orxFALSE)
        {
          /* Applies it */
          orxViewport_SetRelativeSize(pstResult, vSize.fX, vSize.fY);
        }
      }

      /* Has relative size? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE) != orxFALSE)
      {
        /* No fixed size? */
        if(bFixedSize == orxFALSE)
        {
          orxVECTOR vRelSize;

          /* Gets it */
          if(orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_RELATIVE_SIZE, &vRelSize) != orxNULL)
          {
            /* Applies it */
            orxViewport_SetRelativeSize(pstResult, vRelSize.fX, vRelSize.fY);
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Viewport [%s]: Ignoring RelativeSize as Size was also defined.", _zConfigID);
        }
      }

      /* Has plain position */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_POSITION) != orxFALSE)
      {
        orxVECTOR vPos;

        /* Gets it */
        if(orxConfig_GetVector(orxVIEWPORT_KZ_CONFIG_POSITION, &vPos) != orxNULL)
        {
          /* Applies it */
          orxViewport_SetPosition(pstResult, vPos.fX, vPos.fY);

          /* Updates status */
          bFixedPosition = orxTRUE;
        }
        else
        {
          orxCHAR         acBuffer[64];
          const orxSTRING zPos;
          orxU32          u32AlignmentFlags = orxVIEWPORT_KU32_FLAG_ALIGN_CENTER;

          /* Gets it */
          orxString_NCopy(acBuffer, orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_POSITION), sizeof(acBuffer) - 1);
          acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
          zPos = orxString_SkipWhiteSpaces(orxString_LowerCase(acBuffer));

          /* Not empty? */
          if(*zPos != orxCHAR_NULL)
          {
            /* Left? */
            if(orxString_SearchString(zPos, orxVIEWPORT_KZ_LEFT) != orxNULL)
            {
              /* Updates alignment flags */
              u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_LEFT;
            }
            /* Right? */
            else if(orxString_SearchString(zPos, orxVIEWPORT_KZ_RIGHT) != orxNULL)
            {
              /* Updates alignment flags */
              u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT;
            }

            /* Top? */
            if(orxString_SearchString(zPos, orxVIEWPORT_KZ_TOP) != orxNULL)
            {
              /* Updates alignment flags */
              u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_TOP;
            }
            /* Bottom? */
            else if(orxString_SearchString(zPos, orxVIEWPORT_KZ_BOTTOM) != orxNULL)
            {
              /* Updates alignment flags */
              u32AlignmentFlags |= orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM;
            }

            /* Applies it */
            orxViewport_SetRelativePosition(pstResult, u32AlignmentFlags);

            /* Updates status */
            bFixedPosition = orxTRUE;
          }
        }
      }

      /* Auto resize */
      if(((orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_AUTO_RESIZE) == orxFALSE)
       && (bFixedSize == orxFALSE))
      || (orxConfig_GetBool(orxVIEWPORT_KZ_CONFIG_AUTO_RESIZE) != orxFALSE))
      {
        /* Updates flags */
        orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_AUTO_RESIZE, orxVIEWPORT_KU32_FLAG_NONE);
      }

      /* Has relative position? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION) != orxFALSE)
      {
        /* No fixed position? */
        if(bFixedPosition == orxFALSE)
        {
          orxCHAR         acBuffer[64];
          const orxSTRING zRelativePos;
          orxU32          u32AlignmentFlags = orxVIEWPORT_KU32_FLAG_ALIGN_CENTER;

          /* Gets it */
          orxString_NCopy(acBuffer, orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_RELATIVE_POSITION), sizeof(acBuffer) - 1);
          acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
          zRelativePos = orxString_SkipWhiteSpaces(orxString_LowerCase(acBuffer));

          /* Not empty? */
          if(*zRelativePos != orxCHAR_NULL)
          {
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
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Viewport [%s]: Ignoring RelativePosition as Position was also defined.", _zConfigID);
        }
      }

      /* Has background color? */
      if(orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR) != orxFALSE)
      {
        orxCOLOR        stColor;
        const orxSTRING zColor;

        /* Gets literal color */
        zColor = orxConfig_GetString(orxVIEWPORT_KZ_CONFIG_BACKGROUND_COLOR);

        /* Not a vector value? */
        if(orxConfig_ToVector(zColor, &(stColor.vRGB)) == orxNULL)
        {
          /* Pushes color section */
          orxConfig_PushSection(orxCOLOR_KZ_CONFIG_SECTION);

          /* Retrieves its value */
          orxConfig_GetVector(zColor, &(stColor.vRGB));

          /* Pops config section */
          orxConfig_PopSection();
        }

        /* Normalizes it */
        orxVector_Mulf(&(stColor.vRGB), &(stColor.vRGB), orxCOLOR_NORMALIZER);

        /* Gets alpha value */
        stColor.fAlpha = (orxConfig_HasValue(orxVIEWPORT_KZ_CONFIG_BACKGROUND_ALPHA) != orxFALSE) ? orxConfig_GetFloat(orxVIEWPORT_KZ_CONFIG_BACKGROUND_ALPHA) : orxFLOAT_1;

        /* Applies it */
        orxViewport_SetBackgroundColor(pstResult, &stColor);
      }
      else
      {
        /* Clears background color */
        orxViewport_ClearBackgroundColor(pstResult);
      }

      /* Stores its reference key */
      pstResult->zReference = orxConfig_GetCurrentSection();

      /* Adds it to reference table */
      orxHashTable_Add(sstViewport.pstReferenceTable, orxString_Hash(pstResult->zReference), pstResult);

      /* Updates status flags */
      orxStructure_SetFlags(pstResult, orxVIEWPORT_KU32_FLAG_REFERENCED, orxVIEWPORT_KU32_FLAG_NONE);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Couldn't find config section named (%s).", _zConfigID);

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

  /* Decreases count */
  orxStructure_DecreaseCount(_pstViewport);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstViewport) == 0)
  {
    /* Removes camera */
    orxViewport_SetCamera(_pstViewport, orxNULL);

    /* Was linked to textures? */
    if(_pstViewport->u32TextureCount != 0)
    {
      /* Removes them */
      orxViewport_SetTextureList(_pstViewport, 0, orxNULL);
    }

    /* Had a shader pointer? */
    if(_pstViewport->pstShaderPointer != orxNULL)
    {
      /* Updates its count */
      orxStructure_DecreaseCount(_pstViewport->pstShaderPointer);

      /* Was internally allocated? */
      if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL_SHADER) != orxFALSE)
      {
        /* Removes its owner */
        orxStructure_SetOwner(_pstViewport->pstShaderPointer, orxNULL);

        /* Deletes it */
        orxShaderPointer_Delete(_pstViewport->pstShaderPointer);
      }
    }

    /* Is referenced? */
    if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_REFERENCED) != orxFALSE)
    {
      /* Removes it from reference table */
      orxHashTable_Remove(sstViewport.pstReferenceTable, orxString_Hash(_pstViewport->zReference));
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
  for(i = 0; i < _pstViewport->u32TextureCount; i++)
  {
    /* Updates its reference count */
    orxStructure_DecreaseCount((_pstViewport->apstTextureList[i]));

    /* Was internally allocated? */
    if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_INTERNAL_TEXTURES) != orxFALSE)
    {
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
    orxU32 u32TextureCount;

    /* Checks */
    orxASSERT(_apstTextureList != orxNULL);

    /* For all new textures */
    for(i = 0, u32TextureCount = 0; i < _u32TextureNumber; i++)
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
      _pstViewport->apstTextureList[u32TextureCount++] = _apstTextureList[i];

      /* Updates its reference count */
      orxStructure_IncreaseCount(_apstTextureList[i]);
    }

    /* Updates texture count */
    _pstViewport->u32TextureCount = u32TextureCount;
  }
  else
  {
    /* Updates texture count */
    _pstViewport->u32TextureCount = _u32TextureNumber;

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
  if(_pstViewport->u32TextureCount != 0)
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

/** Gets a viewport texture count
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Number of textures associated with the viewport
 */
orxU32 orxFASTCALL orxViewport_GetTextureCount(const orxVIEWPORT *_pstViewport)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstViewport);

  /* Updates result */
  u32Result = (_pstViewport->u32TextureCount != 0) ? _pstViewport->u32TextureCount : 1;

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
    /* Updates its reference count */
    orxStructure_DecreaseCount((_pstViewport->pstCamera));

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
    /* Updates its reference count */
    orxStructure_IncreaseCount(_pstCamera);

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
        /* Updates its count */
        orxStructure_IncreaseCount(_pstViewport->pstShaderPointer);

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
  _pstViewport->fX      = orxMath_Round(_fX);
  _pstViewport->fY      = orxMath_Round(_fY);
  _pstViewport->fRealX  = _fX;
  _pstViewport->fRealY  = _fY;

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
      _pstViewport->fRealX = orxFLOAT_0;
    }
    /* Align right? */
    else if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT)
    {
      /* Updates x position */
      _pstViewport->fRealX = fWidth - _pstViewport->fRealWidth;
    }
    /* Align center */
    else
    {
      /* Updates x position */
      _pstViewport->fRealX = orx2F(0.5f) * (fWidth - _pstViewport->fRealWidth);
    }

    /* Align top? */
    if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_TOP)
    {
      /* Updates y position */
      _pstViewport->fRealY = orxFLOAT_0;
    }
    /* Align bottom? */
    else if(_u32AlignFlags & orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM)
    {
      /* Updates y position */
      _pstViewport->fRealY = fHeight - _pstViewport->fRealHeight;
    }
    /* Align center */
    else
    {
      /* Updates y position */
      _pstViewport->fRealY = orx2F(0.5f) * (fHeight - _pstViewport->fRealHeight);
    }

    /* Updates rounded values */
    _pstViewport->fX = orxMath_Round(_pstViewport->fRealX);
    _pstViewport->fY = orxMath_Round(_pstViewport->fRealY);

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
  _pstViewport->fWidth      = orxMath_Round(_fW);
  _pstViewport->fHeight     = orxMath_Round(_fH);
  _pstViewport->fRealWidth  = _fW;
  _pstViewport->fRealHeight = _fH;

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
  orxASSERT(_fW >= orxFLOAT_0);
  orxASSERT(_fH >= orxFLOAT_0);

  /* Gets first associated texture */
  orxViewport_GetTextureList(_pstViewport, 1, &pstTexture);

  /* Valid? */
  if(pstTexture != orxNULL)
  {
    /* Updates viewport size */
    orxTexture_GetSize(pstTexture, &(_pstViewport->fRealWidth), &(_pstViewport->fRealHeight));
    _pstViewport->fRealWidth   *= _fW;
    _pstViewport->fRealHeight  *= _fH;

    /* Updates rounded values */
    _pstViewport->fWidth  = orxMath_Round(_pstViewport->fRealWidth);
    _pstViewport->fHeight = orxMath_Round(_pstViewport->fRealHeight);

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
  else
  {
    /* Has fixed ratio? */
    if(orxStructure_TestFlags(_pstViewport, orxVIEWPORT_KU32_FLAG_FIXED_RATIO))
    {
      /* Updates result */
      fResult = (_pstViewport->fHeight / _pstViewport->fWidth) * _pstViewport->fFixedRatio;
    }
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

/** Gets viewport given its name
 * @param[in]   _zName          Viewport name
 * @return      orxVIEWPORT / orxNULL
 */
orxVIEWPORT *orxFASTCALL orxViewport_Get(const orxSTRING _zName)
{
  orxVIEWPORT *pstResult;

  /* Checks */
  orxASSERT(sstViewport.u32Flags & orxVIEWPORT_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Updates result */
  pstResult = (orxVIEWPORT *)orxHashTable_Get(sstViewport.pstReferenceTable, orxString_Hash(_zName));

  /* Done! */
  return pstResult;
}

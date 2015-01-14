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
 * @file orxViewport.h
 * @date 14/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxViewport
 *
 * Viewport module
 * Allows to creates and handle viewports.
 * Viewports are structures associated to cameras and used for rendering.
 *
 * @{
 */


#ifndef _orxVIEWPORT_H_
#define _orxVIEWPORT_H_

#include "orxInclude.h"
#include "core/orxClock.h"
#include "render/orxCamera.h"
#include "render/orxShaderPointer.h"
#include "display/orxDisplay.h"
#include "display/orxTexture.h"


/** Viewport flags
 */
#define orxVIEWPORT_KU32_FLAG_ALIGN_CENTER    0x00000000  /**< Center vertical / horizontal alignment */
#define orxVIEWPORT_KU32_FLAG_ALIGN_LEFT      0x10000000  /**< Left horizontal alignment */
#define orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT     0x20000000  /**< Right horizontal alignment */
#define orxVIEWPORT_KU32_FLAG_ALIGN_TOP       0x40000000  /**< Top vertical alignment */
#define orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM    0x80000000  /**< Bottom vertical alignment */


/** Misc defined
 */
#define orxVIEWPORT_KU32_MAX_TEXTURE_NUMBER   16


/** Internal Viewport structure */
typedef struct __orxVIEWPORT_t                orxVIEWPORT;


/** Event enum
 */
typedef enum __orxVIEWPORT_EVENT_t
{
  orxVIEWPORT_EVENT_RESIZE = 0,              /**< Event sent when a viewport has been resized */

  orxVIEWPORT_EVENT_NUMBER,

  orxVIEWPORT_EVENT_NONE = orxENUM_NONE

} orxVIEWPORT_EVENT;


/** Viewport module setup
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_Setup();

/** Inits the viewport module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_Init();

/** Exits from the viewport module
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_Exit();


/** Creates a viewport
 * @return      Created orxVIEWPORT / orxNULL
 */
extern orxDLLAPI orxVIEWPORT *orxFASTCALL     orxViewport_Create();

/** Creates a viewport from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxVIEWPORT / orxNULL
 */
extern orxDLLAPI orxVIEWPORT *orxFASTCALL     orxViewport_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a viewport
 * @param[in]   _pstViewport    Viewport to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_Delete(orxVIEWPORT *_pstViewport);


/** Sets a viewport alignment
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32AlignFlags  Alignment flags (must be OR'ed)
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_SetAlignment(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags);


/** Sets a viewport texture list
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32TextureNumber Number of textures to associate with the viewport
 * @param[in]   _apstTextureList List of textures to associate with the viewport
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_SetTextureList(orxVIEWPORT *_pstViewport, orxU32 _u32TextureNumber, orxTEXTURE **_apstTextureList);

/** Gets a viewport texture list
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32TextureNumber Number of textures to be retrieved
 * @param[out]  _apstTextureList List of textures associated with the viewport
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_GetTextureList(const orxVIEWPORT *_pstViewport, orxU32 _u32TextureNumber, orxTEXTURE **_apstTextureList);

/** Gets a viewport texture counter
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Number of textures associated with the viewport
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxViewport_GetTextureCounter(const orxVIEWPORT *_pstViewport);



/** Sets a viewport background color
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _pstColor        Color to use for background
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_SetBackgroundColor(orxVIEWPORT *_pstViewport, const orxCOLOR *_pstColor);

/** Clears viewport background color
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_ClearBackgroundColor(orxVIEWPORT *_pstViewport);

/** Viewport has background color accessor
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxViewport_HasBackgroundColor(const orxVIEWPORT *_pstViewport);

/** Gets a viewport background color
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pstColor       Viewport's color
 * @return      Current background color
 */
extern orxDLLAPI orxCOLOR *orxFASTCALL        orxViewport_GetBackgroundColor(const orxVIEWPORT *_pstViewport, orxCOLOR *_pstColor);


/** Enables / disables a viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_Enable(orxVIEWPORT *_pstViewport, orxBOOL _bEnable);

/** Is a viewport enabled?
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxViewport_IsEnabled(const orxVIEWPORT *_pstViewport);


/** Sets a viewport camera
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _pstCamera      Associated camera
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_SetCamera(orxVIEWPORT *_pstViewport, orxCAMERA *_pstCamera);

/** Gets a viewport camera
 * @param[in]   _pstViewport    Concerned viewport
 * @return      Associated camera / orxNULL
 */
extern orxDLLAPI orxCAMERA *orxFASTCALL       orxViewport_GetCamera(const orxVIEWPORT *_pstViewport);


/** Adds a shader to a viewport using its config ID
 * @param[in]   _pstViewport      Concerned viewport
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_AddShader(orxVIEWPORT *_pstViewport, const orxSTRING _zShaderConfigID);

/** Removes a shader using its config ID
 * @param[in]   _pstViewport      Concerned viewport
 * @param[in]   _zShaderConfigID Config ID of the shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_RemoveShader(orxVIEWPORT *_pstViewport, const orxSTRING _zShaderConfigID);

/** Enables a viewport's shader
 * @param[in]   _pstViewport      Concerned viewport
 * @param[in]   _bEnable          Enable / disable
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_EnableShader(orxVIEWPORT *_pstViewport, orxBOOL _bEnable);

/** Is a viewport's shader enabled?
 * @param[in]   _pstViewport      Concerned viewport
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxViewport_IsShaderEnabled(const orxVIEWPORT *_pstViewport);

/** Gets a viewport's shader pointer
 * @param[in]   _pstViewport      Concerned viewport
 * @return      orxSHADERPOINTER / orxNULL
 */
extern orxDLLAPI const orxSHADERPOINTER *orxFASTCALL orxViewport_GetShaderPointer(const orxVIEWPORT *_pstViewport);


/** Sets a viewport blend mode (only used when has active shaders attached)
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _eBlendMode     Blend mode to set
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_SetBlendMode(orxVIEWPORT *_pstViewport, orxDISPLAY_BLEND_MODE _eBlendMode);

/** Gets a viewport blend mode
 * @param[in]   _pstViewport    Concerned viewport
 * @return orxDISPLAY_BLEND_MODE
 */
extern orxDLLAPI orxDISPLAY_BLEND_MODE orxFASTCALL orxViewport_GetBlendMode(const orxVIEWPORT *_pstViewport);


/** Sets a viewport position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fX             X axis position (top left corner)
 * @param[in]   _fY             Y axis position (top left corner)
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_SetPosition(orxVIEWPORT *_pstViewport, orxFLOAT _fX, orxFLOAT _fY);

/** Sets a viewport relative position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _u32AlignFlags  Alignment flags
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_SetRelativePosition(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags);

/** Gets a viewport position
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pfX            X axis position (top left corner)
 * @param[out]  _pfY            Y axis position (top left corner)
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_GetPosition(const orxVIEWPORT *_pstViewport, orxFLOAT *_pfX, orxFLOAT *_pfY);


/** Sets a viewport size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fWidth         Width
 * @param[in]   _fHeight        Height
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_SetSize(orxVIEWPORT *_pstViewport, orxFLOAT _fWidth, orxFLOAT _fHeight);

/** Sets a viewport relative size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[in]   _fWidth         Relative width (0.0f - 1.0f)
 * @param[in]   _fHeight        Relative height (0.0f - 1.0f)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxViewport_SetRelativeSize(orxVIEWPORT *_pstViewport, orxFLOAT _fWidth, orxFLOAT _fHeight);

/** Gets a viewport size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pfWidth        Width
 * @param[out]  _pfHeight       Height
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_GetSize(const orxVIEWPORT *_pstViewport, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);

/** Gets a viewport relative size
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pfWidth        Relative width
 * @param[out]  _pfHeight       Relative height
 */
extern orxDLLAPI void orxFASTCALL             orxViewport_GetRelativeSize(const orxVIEWPORT *_pstViewport, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);

/** Gets an axis aligned box of viewport
 * @param[in]   _pstViewport    Concerned viewport
 * @param[out]  _pstBox         Output box
 * @return orxAABOX / orxNULL
 */
extern orxDLLAPI orxAABOX *orxFASTCALL        orxViewport_GetBox(const orxVIEWPORT *_pstViewport, orxAABOX *_pstBox);

/** Get viewport correction ratio
 * @param[in]   _pstViewport  Concerned viewport
 * @return      Correction ratio value
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxViewport_GetCorrectionRatio(const orxVIEWPORT *_pstViewport);

/** Gets viewport config name
 * @param[in]   _pstViewport    Concerned viewport
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxViewport_GetName(const orxVIEWPORT *_pstViewport);

#endif /* _orxVIEWPORT_H_ */

/** @} */

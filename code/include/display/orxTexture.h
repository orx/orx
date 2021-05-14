/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2021 Orx-Project
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
 * @file orxTexture.h
 * @date 07/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxTexture
 *
 * Texture module
 * Module that handles textures
 *
 * @{
 */


#ifndef _orxTEXTURE_H_
#define _orxTEXTURE_H_

#include "orxInclude.h"

#include "display/orxDisplay.h"
#include "math/orxMath.h"


/** Defines
 */
#define orxTEXTURE_KZ_RESOURCE_GROUP      "Texture"

#define orxTEXTURE_KZ_SCREEN              "screen"
#define orxTEXTURE_KZ_PIXEL               "pixel"


/** Event enum
 */
typedef enum __orxTEXTURE_EVENT_t
{
  orxTEXTURE_EVENT_CREATE = 0,
  orxTEXTURE_EVENT_DELETE,
  orxTEXTURE_EVENT_LOAD,

  orxTEXTURE_EVENT_NUMBER,

  orxTEXTURE_EVENT_NONE = orxENUM_NONE

} orxTEXTURE_EVENT;


/** Internal texture structure */
typedef struct __orxTEXTURE_t             orxTEXTURE;


/** Setups the texture module
 */
extern orxDLLAPI void orxFASTCALL         orxTexture_Setup();

/** Inits the texture module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_Init();

/** Exits from the texture module
 */
extern orxDLLAPI void orxFASTCALL         orxTexture_Exit();


/** Creates an empty texture
 * @return      orxTEXTURE / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL  orxTexture_Create();

/** Creates a texture from a bitmap file
 * @param[in]   _zFileName      Name of the bitmap
 * @param[in]   _bKeepInCache   Should be kept in cache after no more references exist?
 * @return      orxTEXTURE / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL  orxTexture_CreateFromFile(const orxSTRING _zFileName, orxBOOL _bKeepInCache);

/** Deletes a texture (and its referenced bitmap)
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_Delete(orxTEXTURE *_pstTexture);

/** Clears cache (if any texture is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_ClearCache();

/** Links a bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _pstBitmap      Bitmap to link
 * @param[in]   _zDataName      Name associated with the bitmap (usually filename)
 * @param[in]   _bTransferOwnership If set to true, the texture will become the bitmap's owner and will have it deleted upon its own deletion
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, const orxBITMAP *_pstBitmap, const orxSTRING _zDataName, orxBOOL _bTransferOwnership);

/** Unlinks (and deletes if not used anymore) a bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture);


/** Gets texture bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxBITMAP / orxNULL
 */
extern orxDLLAPI orxBITMAP *orxFASTCALL   orxTexture_GetBitmap(const orxTEXTURE *_pstTexture);

/** Gets texture size
 * @param[in]   _pstTexture     Concerned texture
 * @param[out]  _pfWidth        Texture's width
 * @param[out]  _pfHeight       Texture's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_GetSize(const orxTEXTURE *_pstTexture, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);

/** Gets texture name
 * @param[in]   _pstTexture   Concerned texture
 * @return      Texture name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxTexture_GetName(const orxTEXTURE *_pstTexture);

/** Gets screen texture
 * @return      Screen texture / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL  orxTexture_GetScreenTexture();

/** Gets pending load count
 * @return      Pending load count
 */
extern orxDLLAPI orxU32 orxFASTCALL       orxTexture_GetLoadCount();

#endif /* _orxTEXTURE_H_ */

/** @} */

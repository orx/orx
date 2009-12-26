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
#define orxTEXTURE_KZ_SCREEN_NAME         "-=SCREEN=-"


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
 * @param[in]   _zBitmapFileName  Name of the bitmap
 * @return      orxTEXTURE / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL  orxTexture_CreateFromFile(const orxSTRING _zBitmapFileName);

/** Deletes a texture (and its referenced bitmap)
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_Delete(orxTEXTURE *_pstTexture);

/** Links a bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _pstBitmap      Bitmap to link
 * @param[in]   _zDataName      Name associated with the bitmap (usually filename)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, const orxBITMAP *_pstBitmap, const orxSTRING _zDataName);

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
 * @return      Texture name / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxTexture_GetName(const orxTEXTURE *_pstTexture);

/** Sets texture color
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _pstColor       Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_SetColor(orxTEXTURE *_pstTexture, const orxCOLOR *_pstColor);

/** Gets screen texture
 * @return      Screen texture / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL  orxTexture_GetScreenTexture();

#endif /* _orxTEXTURE_H_ */

/** @} */

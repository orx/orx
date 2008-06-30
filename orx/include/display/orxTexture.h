/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxTexture.h
 * @date 07/12/2003
 * @author (C) Arcallians
 */

/**
 * @addtogroup Display
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


/** Internal texture structure */
typedef struct __orxTEXTURE_t             orxTEXTURE;


/** Setups the texture module
 */
extern orxDLLAPI orxVOID                  orxTexture_Setup();

/** Inits the texture module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                orxTexture_Init();

/** Exits from the texture module
 */
extern orxDLLAPI orxVOID                  orxTexture_Exit();


/** Creates an empty texture
 * @return      orxTEXTURE / orxNULL
 */
extern orxDLLAPI orxTEXTURE *             orxTexture_Create();

/** Creates a texture from a bitmap file
 * @param[in]   _zBitmapFileName  Name of the bitmap
 * @return      orxTEXTURE / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL  orxTexture_CreateFromFile(orxCONST orxSTRING _zBitmapFileName);

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
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, orxCONST orxBITMAP *_pstBitmap, orxCONST orxSTRING _zDataName);

/** Unlinks (and deletes if not used anymore) a bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture);


/** Gets texture bitmap
 * @param[in]   _pstTexture     Concerned texture
 * @return      orxBITMAP / orxNULL
 */
extern orxDLLAPI orxBITMAP *orxFASTCALL   orxTexture_GetBitmap(orxCONST orxTEXTURE *_pstTexture);

/** Gets texture width
 * @param[in]   _pstTexture     Concerned texture
 * @return      Texture's width
 */
extern orxDLLAPI orxFLOAT orxFASTCALL     orxTexture_GetWidth(orxCONST orxTEXTURE *_pstTexture);

/** Gets texture height
 * @param[in]   _pstTexture     Concerned texture
 * @return      Texture's height
 */
extern orxDLLAPI orxFLOAT orxFASTCALL     orxTexture_GetHeight(orxCONST orxTEXTURE *_pstTexture);

/** Gets texture name
 * @param[in]   _pstTexture   Concerned texture
 * @return      Texture name / orxNULL
 */
extern orxDLLAPI orxSTRING orxFASTCALL    orxTexture_GetName(orxCONST orxTEXTURE *_pstTexture);

/** Sets texture color
 * @param[in]   _pstTexture     Concerned texture
 * @param[in]   _stColor        Color to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxTexture_SetColor(orxTEXTURE *_pstTexture, orxRGBA _stColor);

/** Gets screen texture
 * @return      Screen texture / orxNULL
 */
extern orxDLLAPI orxTEXTURE *             orxTexture_GetScreenTexture();

#endif /* _orxTEXTURE_H_ */

/** @} */

/** 
 * \file orxTexture.h
 * 
 * Texture Module.
 * Allows to creates and handle textures.
 * They thus can be referenced by other structures.
 * Textures are 2D structures.
 * 
 * \todo
 * Add external texture linking
 * Add bitmap name (ID) storing for later retrieving
 */


/***************************************************************************
 orxTextures.h
 Texture module
 
 begin                : 07/12/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxTEXTURE_H_
#define _orxTEXTURE_H_

#include "orxInclude.h"

#include "display/orxDisplay.h"
#include "math/orxMath.h"


/** Internal texture structure. */
typedef struct __orxTEXTURE_t           orxTEXTURE;


/** Texture module setup */
extern orxDLLAPI orxVOID                orxTexture_Setup();
/** Inits the texture system. */
extern orxDLLAPI orxSTATUS              orxTexture_Init();
/** Exits from the texture system. */
extern orxDLLAPI orxVOID                orxTexture_Exit();

/** Creates an empty texture. */
extern orxDLLAPI orxTEXTURE *           orxTexture_Create();
/** Creates a texture from a bitmap. */
extern orxDLLAPI orxTEXTURE *           orxTexture_CreateFromBitmap(orxCONST orxSTRING _zBitmapFileName);
/** Deletes a texture (Warning : it deletes referenced bitmap too!). */
extern orxDLLAPI orxSTATUS              orxTexture_Delete(orxTEXTURE *_pstTexture);

/** Links a bitmap to a texture (Warning : use a different bitmap for each texture). */
extern orxDLLAPI orxSTATUS              orxTexture_LinkBitmap(orxTEXTURE *_pstTexture, orxBITMAP *_pstBitmap);
/** Unlinks a bitmap from a texture (Warning : it deletes it). */
extern orxDLLAPI orxSTATUS              orxTexture_UnlinkBitmap(orxTEXTURE *_pstTexture);


/** !!! Texture accessors !!! */


/** Corresponding bitmap get accessor. */
extern orxDLLAPI orxCONST orxBITMAP *   orxTexture_GetBitmap(orxTEXTURE *_pstTexture);

/** Texture size get accessor. */
extern orxDLLAPI orxSTATUS              orxTexture_GetSize(orxTEXTURE *_pstTexture, orxVECTOR *_pvSize);

/** Reference coordinates set accessor (used for rendering purpose). */
extern orxDLLAPI orxVOID                orxTexture_SetRefPoint(orxTEXTURE *_pstTexture, orxVECTOR *_pvRefPoint);
/** Reference coordinates get accessor (used for rendering purpose). */
extern orxDLLAPI orxSTATUS              orxTexture_GetRefPoint(orxTEXTURE *_pstTexture, orxVECTOR *_pvRefPoint);


#endif /* _orxTEXTURE_H_ */

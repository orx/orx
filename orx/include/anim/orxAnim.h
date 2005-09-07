/**
 * \file orxAnim.h
 * 
 * Animation (Data) Module.
 * Allows to creates and handle Animations data.
 * It consists of a structure containing data for a single animation
 * and functions for handling and accessing them.
 * Animations are structures.
 * They thus can be referenced by Animation Sets (animset) Module.
 * 
 * \todo
 * Optimizations
 */


/***************************************************************************
 orxAnim.h
 Animation Data module
 
 begin                : 11/02/2004
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


#ifndef _orxANIM_H_
#define _orxANIM_H_

#include "orxInclude.h"

#include "display/orxTexture.h"
#include "object/orxStructure.h"


/** Animation ID Flags. */
#define orxANIM_KU32_ID_FLAG_2D               0x00010000  /**< 2D type animation ID flag */

/** Animation defines. */
#define orxANIM_KS32_TEXTURE_MAX_NUMBER       256         /**< Maximum number of texture for an animation structure */


/** Internal Animation structure. */
typedef struct __orxANIM_t  orxANIM;


/** Inits the Animation system. */
extern orxDLLAPI orxSTATUS                    orxAnim_Init();
/** Exits from the Animation system. */
extern orxDLLAPI orxVOID                      orxAnim_Exit();

/** Creates an empty Animation, given its id type and storage size (<= orxANIM_KS32_TEXTURE_MAX_NUMBER). */
extern orxDLLAPI orxANIM *                    orxAnim_Create(orxU32 _u32IDFlag, orxU32 _u32Size);
/** Deletes an Animation. */
extern orxDLLAPI orxSTATUS                    orxAnim_Delete(orxANIM *_pstAnim);

/** Adds a texture to a 2D Animation. */
extern orxDLLAPI orxSTATUS                    orxAnim_AddTexture(orxANIM *_pstAnim, orxTEXTURE *_pstTexture, orxU32 _u32Time);
/** Removes last added texture from a 2D Animation.*/
extern orxDLLAPI orxSTATUS                    orxAnim_RemoveTexture(orxANIM *_pstAnim);

/** Removes all referenced textures from a 2D Animation. */
extern orxDLLAPI orxVOID                      orxAnim_RemoveAllTextures(orxANIM *_pstAnim);

/** Computes active 2D texture given a timestamp. */
extern orxDLLAPI orxTEXTURE *                 orxAnim_ComputeTexture(orxANIM *_pstAnim, orxU32 _u32Timestamp);


/** !!! Warning : Animation accessors don't test parameter validity !!! */


/** Animation 2D texture get accessor, given its index. */
extern orxDLLAPI orxTEXTURE *                 orxAnim_GetTexture(orxANIM *_pstAnim, orxU32 _u32Index);

/** Animation 2D texture storage size get accessor. */
extern orxDLLAPI orxU32                       orxAnim_GetTextureStorageSize(orxANIM *_pstAnim);
/** Animation 2D texture counter get accessor. */
extern orxDLLAPI orxU32                       orxAnim_GetTextureCounter(orxANIM *_pstAnim);

/** Animation length get accessor. */
extern orxDLLAPI orxU32                       orxAnim_GetLength(orxANIM *_pstAnim);

/** Animation flag test accessor. */
extern orxDLLAPI orxBOOL                      orxAnim_TestFlags(orxANIM *_pstAnim, orxU32 _u32Flag);
/** Animation flag get/set accessor. */
extern orxDLLAPI orxVOID                      orxAnim_SetFlags(orxANIM *_pstAnim, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);


#endif /* _orxANIM_H_ */

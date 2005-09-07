/**
 * \file orxAnimPointer.h
 * 
 * Animation (Pointer) Module.
 * Allows to creates and handle Animation Set Pointers.
 * It consists of a structure containing pointers and counters referenced to an Animation Set.
 * It also contains functions for handling and accessing animations of the referenced Animation Set.
 * Animation Pointers are structures.
 * They thus can be referenced by Graphics.
 * 
 * \todo
 * Everything :)
 * X Add/Remove "update-all function" clock registering in orxAnimPointer_Init / orxAnimPointer_Exit.
 * V Add current anim, timestamp, dest anim, animset ref, link_table ref in structure.
 * V Duplicate link table on creation if animset is non link-static.
 * V Lock animset & update ref counter upon creation, unlock it upon destruction if updated ref counter is 0.
 * V Optimized animpointer update : will only happen if graphic is displayed on screen.
 * - Add clocks registering (using different frequencies) for an animpointer update.
 * - Add optimization with above system, to compute only if graphic is tagged as rendered.
 */


/***************************************************************************
 orxAnimPointer.h
 Animation Pointer module
 
 begin                : 03/03/2004
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


#ifndef _orxANIMPOINTER_H_
#define _orxANIMPOINTER_H_


#include "orxInclude.h"

#include "anim/orxAnimSet.h"


/** AnimationPointer Link Flags. */
#define orxANIMPOINTER_KU32_ID_FLAG_NONE            0x00000000  /**< No flags. */

#define orxANIMPOINTER_KU32_ID_FLAG_PAUSE           0x00010000  /**< Animation link priority flag. */


/** Internal AnimationPointer structure. */
typedef struct __orxANIMPOINTER_t                   orxANIMPOINTER;


/** Inits the AnimationPointer system. */
extern orxDLLAPI orxSTATUS                          orxAnimPointer_Init();
/** Exits from the AnimationPointer system. */
extern orxDLLAPI orxVOID                            orxAnimPointer_Exit();

/** Creates an empty AnimationPointer, given its AnimationSet reference. */
extern orxDLLAPI orxANIMPOINTER *                   orxAnimPointer_Create(orxANIMSET *_pstAnimset);
/** Deletes an AnimationPointer. */
extern orxDLLAPI orxSTATUS                          orxAnimPointer_Delete(orxANIMPOINTER *_pstAnimpointer);

/** Gets the referenced Animation Set. */
extern orxDLLAPI orxANIMSET *                       orxAnimPointer_GetAnimSet(orxCONST orxANIMPOINTER *_pstAnimpointer);


/** !!! Warning : AnimationPointer accessors don't test parameter validity !!! */


/** AnimationPointer current Animation get accessor. */
extern orxDLLAPI orxANIM *                          orxAnimPointer_GetAnim(orxCONST orxANIMPOINTER *_pstAnimpointer);
/** AnimationPointer current Animation Time get accessor. */
extern orxDLLAPI orxTIME                            orxAnimPointer_GetTime(orxCONST orxANIMPOINTER *_pstAnimpointer);

/** AnimationPointer current Animation set accessor. */
extern orxDLLAPI orxSTATUS                          orxAnimPointer_SetAnim(orxANIMPOINTER *_pstAnimpointer, orxHANDLE _hAnimHandle);
/** AnimationPointer current Animation Time set accessor. */
extern orxDLLAPI orxSTATUS                          orxAnimPointer_SetTime(orxANIMPOINTER *_pstAnimpointer, orxTIME _stTime);

/** AnimationPointer Frequency get accessor. */
extern orxDLLAPI orxFLOAT                           orxAnimPointer_GetFrequency(orxCONST orxANIMPOINTER *_pstAnimpointer);
/** AnimationPointer Frequency set accessor. */
extern orxDLLAPI orxSTATUS                          orxAnimPointer_SetFrequency(orxANIMPOINTER *_pstAnimpointer, orxFLOAT _fFrequency);

/** AnimationPointer flag test accessor. */
extern orxDLLAPI orxBOOL                            orxAnimPointer_TestFlags(orxCONST orxANIMPOINTER *_pstAnimpointer, orxU32 _u32Flag);
/** AnimationPointer flag get/set accessor. */
extern orxDLLAPI orxVOID                            orxAnimPointer_SetFlags(orxANIMPOINTER *_pstAnimpointer, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);


#endif /* _orxANIMPOINTER_H_ */

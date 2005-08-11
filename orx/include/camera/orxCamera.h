/** 
 * \file orxCamera.h
 * 
 * Camera Module.
 * Allows to creates and handle cameras.
 * Camera are structures used to render graphic (2D/3D) objects.
 * They thus can be referenced by other structures.
 * 
 * \todo
 * Optimize view list update.
 * Add 3d system (Not planned yet).
 */


/***************************************************************************
 orxCamera.h
 Texture module
 
 begin                : 10/12/2003
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


#ifndef _orxCAMERA_H_
#define _orxCAMERA_H_

#include "orxInclude.h"

#include "math/orxMath.h"
#include "object/orxFrame.h"
#include "object/orxObject.h"


/** Camera ID Flags. */

#define orxCAMERA_KU32_ID_FLAG_NONE     0x00000000

#define orxCAMERA_KU32_ID_FLAG_2D       0x00000010


/** Internal camera structure. */
typedef struct __orxCAMERA_t            orxCAMERA;

/** Public camera view list structure. */
typedef struct __orxCAMERA_VIEW_LIST_t  orxCAMERA_VIEW_LIST;


/** Inits the camera system. */
extern orxSTATUS                        orxCamera_Init();
/** Exits from the camera system. */
extern orxVOID                          orxCamera_Exit();

/** Creates an empty camera. */
extern orxCAMERA                       *orxCamera_Create();
/** Deletes a camera. */
extern orxSTATUS                        orxCamera_Delete(orxCAMERA *_pstCamera);

/** Updates camera view list. */
extern orxVOID                          orxCamera_UpdateViewList(orxCAMERA *_pstCamera);

/** !!! Warning : Camera accessors don't test parameter validity !!! */

/** Gets camera first view list. */
extern orxCAMERA_VIEW_LIST             *orxCamera_GetViewListFirstCell(orxCAMERA *_pstCamera);
/** Gets camera next view list. */
extern orxCAMERA_VIEW_LIST             *orxCamera_GetViewListNextCell(orxCAMERA *_pstCamera);

/** Gets camera view list number. */
extern orxS32                           orxCamera_GetViewListSize(orxCAMERA *_pstCamera);
/** Gets view list screen frame. */
extern orxFRAME                        *orxCamera_GetViewListFrame(orxCAMERA_VIEW_LIST *_pstViewList);
/** Gets view list object. */
extern orxOBJECT                       *orxCamera_GetViewListObject(orxCAMERA_VIEW_LIST *_pstViewList);

/** Camera position set accessor. */
extern orxVOID                          orxCamera_SetPosition(orxCAMERA *_pstCamera, orxVEC *_pvPosition);
/** Camera size set accessor. */
extern orxVOID                          orxCamera_SetSize(orxCAMERA *_pstCamera, orxVEC *_pvSize);
/** Camera rotation set accessor. */
extern orxVOID                          orxCamera_SetRotation(orxCAMERA *_pstCamera, orxFLOAT _fRotation);
/** Camera zoom set accessor. */
extern orxVOID                          orxCamera_SetZoom(orxCAMERA *_pstCamera, orxFLOAT _fZoom);

/** Camera link set accessor. */
extern orxVOID                          orxCamera_SetTarget(orxCAMERA *_pstCamera, orxOBJECT *_pstObject);
/** Camera limit set accessor (Upper left & Bottom right corners positions). */
extern orxVOID                          orxCamera_SetLimits(orxCAMERA *_pstCamera, orxVEC *_pvUL, orxVEC *_pvBR);
/** Camera limit reset accessor (Removes all position limits). */
extern orxVOID                          orxCamera_RemoveLimits(orxCAMERA *_pstCamera);

/** Camera on screen position set accessor. */
extern orxVOID                          orxCamera_SetOnScreenPosition(orxCAMERA *_pstCamera, orxVEC *_pvPosition);

/** Camera position get accessor. */
extern orxVOID                          orxCamera_GetPosition(orxCAMERA *_pstCamera, orxVEC *_pvPosition);
/** Camera size get accessor. */
extern orxVOID                          orxCamera_GetSize(orxCAMERA *_pstCamera, orxVEC *_pvSize);
/** Camera rotation get accessor. */
extern orxFLOAT                         orxCamera_GetRotation(orxCAMERA *_pstCamera);
/** Camera zoom get accessor. */
extern orxFLOAT                         orxCamera_GetZoom(orxCAMERA *_pstCamera);
/** Camera link get accessor. */

extern orxOBJECT                       *orxCamera_GetTarget(orxCAMERA *_pstCamera);
/** Camera limit get accessor (Upper left & Bottom right corners positions). */
extern orxVOID                          orxCamera_GetLimits(orxCAMERA *_pstCamera, orxVEC *_pvUL, orxVEC *_pvBR);
/** Camera on screen position get accessor. */
extern orxVOID                          orxCamera_GetOnScreenPosition(orxCAMERA *_pstCamera, orxVEC *_pvPosition);

/** Camera flag test accessor. */
extern orxBOOL                          orxCamera_TestFlags(orxCAMERA *_pstCamera, orxU32 _u32Flag);
/** Camera flag get/set accessor. */
extern orxVOID                          orxCamera_SetFlags(orxCAMERA *_pstCamera, orxU32 _u32AddFlags, orxU32 _u32RemoveFlags);


#endif /* _orxCAMERA_H_ */

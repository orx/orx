/** 
 * \file orxViewport.h
 * 
 * Viewport Module.
 * Allows to creates and handle viewports.
 * Viewports are structures associated to cameras and used for rendering.
 * They thus can be referenced by other structures.
 * 
 */


/***************************************************************************
 orxViewport.h
 Viewport module
 
 begin                : 14/12/2003
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


#ifndef _orxVIEWPORT_H_
#define _orxVIEWPORT_H_

#include "orxInclude.h"

#include "camera/orxCamera.h"
#include "display/orxTexture.h"


/** Viewport alingment defines */
#define orxVIEWPORT_KU32_FLAG_ALIGN_CENTER    0x00000000
#define orxVIEWPORT_KU32_FLAG_ALIGN_LEFT      0x10000000
#define orxVIEWPORT_KU32_FLAG_ALIGN_RIGHT     0x20000000
#define orxVIEWPORT_KU32_FLAG_ALIGN_TOP       0x40000000
#define orxVIEWPORT_KU32_FLAG_ALIGN_BOTTOM    0x80000000


/** Internal viewport structure. */
typedef struct __orxVIEWPORT_t                orxVIEWPORT;

/** Viewport system setup */
extern orxDLLAPI orxVOID                      orxViewport_Setup();
/** Inits the viewport system. */
extern orxDLLAPI orxSTATUS                    orxViewport_Init();
/** Ends the viewport system. */
extern orxDLLAPI orxVOID                      orxViewport_Exit();

/** Creates an empty viewport. */
extern orxDLLAPI orxVIEWPORT *                orxViewport_Create();
/** Deletes a viewport. */
extern orxDLLAPI orxSTATUS                    orxViewport_Delete(orxVIEWPORT *_pstViewport);


/** *** Viewport accessors *** */

/** Viewport alignment set accessor (flags must be OR'ed). */
extern orxDLLAPI orxVOID                      orxViewport_SetAlignment(orxVIEWPORT *_pstViewport, orxU32 _u32AlignFlags);

/** Viewport on screen camera position get accessor. */
extern orxDLLAPI orxVOID                      orxViewport_GetCameraOnScreenPosition(orxVIEWPORT *_pstViewport, orxVECTOR *_pvPosition);

/** Viewport surface set accessor. */
extern orxDLLAPI orxVOID                      orxViewport_SetSurface(orxVIEWPORT *_pstViewport, orxTEXTURE *_pstSurface);
/** Viewport surface get accessor. */
extern orxDLLAPI orxTEXTURE *                 orxViewport_GetSurface(orxVIEWPORT *_pstViewport);

/** Viewport enable accessor. */
extern orxDLLAPI orxVOID                      orxViewport_Enable(orxVIEWPORT *_pstViewport, orxBOOL _bEnable);
/** Viewport is enabled accessor. */
extern orxDLLAPI orxBOOL                      orxViewport_IsEnabled(orxVIEWPORT *_pstViewport);

/** Viewport clip get accessor. */
extern orxDLLAPI orxVOID                      orxViewport_GetClip(orxVIEWPORT * _pstViewport, orxVECTOR *_pvTL, orxVECTOR *_pvBR);

/** Viewport camera set accessor. */
extern orxDLLAPI orxVOID                      orxViewport_SetCamera(orxVIEWPORT *_pstViewport, orxCAMERA *_pstCamera);
/** Viewport position set accessor. */
extern orxDLLAPI orxVOID                      orxViewport_SetPosition(orxVIEWPORT *_pstViewport, orxVECTOR *_pvPosition);
/** Viewport size set accessor. */
extern orxDLLAPI orxVOID                      orxViewport_SetSize(orxVIEWPORT *_pstViewport, orxVECTOR *_pvSize);

/** Viewport camera get accessor. */
extern orxDLLAPI orxCAMERA *                  orxViewport_GetCamera(orxVIEWPORT *_pstViewport);
/** Viewport position get accessor. */
extern orxDLLAPI orxVOID                      orxViewport_GetPosition(orxVIEWPORT *_pstViewport, orxVECTOR *_pvPosition);
/** Viewport size get accessor. */
extern orxDLLAPI orxVOID                      orxViewport_GetSize(orxVIEWPORT *_pstViewport, orxVECTOR *_pvSize);


#endif /* _orxVIEWPORT_H_ */

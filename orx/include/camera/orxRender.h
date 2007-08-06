/** 
 * \file orxRender.h
 * 
 * Render Module.
 * Renders visible objects on screen, using active renders.
 * 
 * \todo
 * Optimize viewport list handling.
 */


/***************************************************************************
 orxRender.h
 Render module
 
 begin                : 15/12/2003
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


#ifndef _orxRENDER_H_
#define _orxRENDER_H_


#include "orxInclude.h"

#include "camera/orxViewport.h"


/** Render system setup */
extern orxDLLAPI orxVOID                  orxRender_Setup();
/** Inits the render system. */
extern orxDLLAPI orxSTATUS                orxRender_Init();
/** Ends the render system. */
extern orxDLLAPI orxVOID                  orxRender_Exit();

/** Renders all viewports. */
extern orxDLLAPI orxVOID orxFASTCALL      orxRender_RenderAllViewports(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext);
/** Rendres given viewport. */
extern orxDLLAPI orxVOID orxFASTCALL      orxRender_RenderViewport(orxCONST orxVIEWPORT *_pstViewport);


#endif /* _orxRENDER_H_ */

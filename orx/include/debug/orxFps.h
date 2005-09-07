/** 
 * \file orxFps.h
 * 
 * FPS Module.
 * Allows to compute & display FPS.
 * 
 * \todo
 * Evertyhing. :)
 */


/***************************************************************************
 orxFps.h
 FPS module
 
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


#ifndef _orxFPS_H_
#define _orxFPS_H_

#include "orxInclude.h"

/** Inits the fps system. */
extern orxDLLAPI orxSTATUS                        orxFps_Init();
/** Exits from the fps system. */
extern orxDLLAPI orxVOID                          orxFps_Exit();

/** Increases fps counter */
extern orxDLLAPI orxVOID                          orxFps_IncreaseFrameCounter();

/** Gets fps counter */
extern orxDLLAPI orxU32                           orxFps_GetFPS();


#endif /* _orxFPS_H_ */

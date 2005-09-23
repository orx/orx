/** 
 * \file orxFPS.h
 * 
 * FPS Module.
 * Allows to compute & display FPS.
 * 
 * \todo
 * Evertyhing. :)
 */


/***************************************************************************
 orxFPS.h
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


/** FPS module setup */
extern orxDLLAPI orxVOID                          orxFPS_Setup();
/** Inits the FPS system. */
extern orxDLLAPI orxSTATUS                        orxFPS_Init();
/** Exits from the FPS system. */
extern orxDLLAPI orxVOID                          orxFPS_Exit();

/** Increases FPS counter */
extern orxDLLAPI orxVOID                          orxFPS_IncreaseFrameCounter();

/** Gets FPS counter */
extern orxDLLAPI orxU32                           orxFPS_GetFPS();


#endif /* _orxFPS_H_ */

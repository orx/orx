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
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxRENDER_H_
#define _orxRENDER_H_


#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#include "object/orxObject.h"


/** Misc defines
 */

#define orxRENDER_KZ_CONFIG_SECTION   "Render"
#define orxRENDER_KZ_CONFIG_SHOW_FPS  "ShowFPS"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Render module setup */
extern orxDLLAPI orxVOID                orxRender_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

extern orxDLLAPI orxSTATUS  orxRender_Init();
extern orxDLLAPI orxVOID    orxRender_Exit();
extern orxDLLAPI orxSTATUS  orxRender_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition);

#endif /* _orxRENDER_H_ */

/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

/**
 * @file orxRender.h
 * @date 15/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxRender
 * 
 * Render plugin module
 * Renders visible objects on screen, using active cameras/viewports.
 *
 * @{
 */


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

/** Render module setup
 */
extern orxDLLAPI orxVOID                orxRender_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the render module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS  orxRender_Init();

/** Exits from the render module
 */
extern orxDLLAPI orxVOID    orxRender_Exit();

/** Get a world position given a screen one (absolute picking)
 * @param[in]   _pvScreenPosition                     Concerned screen position
 * @param[out]  _pvWorldPosition                      Concerned callback
 * @return      orxVECTOR if found, orxNULL otherwise
 */
extern orxDLLAPI orxVECTOR *orxRender_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition);

#endif /* _orxRENDER_H_ */

/** @} */

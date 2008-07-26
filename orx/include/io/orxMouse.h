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
 * @file orxMouse.h
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup IO
 * 
 * Mouse plugin module
 * Plugin module that handles mouse-like peripherals
 *
 * @{
 */


#ifndef _orxMOUSE_H_
#define _orxMOUSE_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/** Button enum
 */
typedef enum __orxMOUSE_BUTTON_t
{
  orxMOUSE_BUTTON_LEFT = 0,
  orxMOUSE_BUTTON_RIGHT,
  orxMOUSE_BUTTON_MIDDLE,

  orxMOUSE_BUTTON_NUMBER,

  orxMOUSE_BUTTON_NONE = orxENUM_NONE

} orxMOUSE_BUTTON;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Mouse module setup
 */
extern orxDLLAPI orxVOID                              orxMouse_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the mouse module
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxMouse_Init();

/** Exits from the mouse module
 */
extern orxDLLAPI orxVOID orxMouse_Exit();

/** Gets mouse on screen position
 * @param[out] _ps32x   X coordinates
 * @param[out] _ps32y   Y coordinates
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxMouse_GetPosition(orxS32 *_s32X, orxS32 *_s32Y);

/** Is mouse button pressed?
 * @param _eButton      Mouse button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxMouse_IsButtonPressed(orxMOUSE_BUTTON _eButton);

#endif /* _orxMOUSE_H_ */

/** @} */

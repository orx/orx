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
 * @addtogroup orxMouse
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
#include "math/orxVector.h"


/** Button enum
 */
typedef enum __orxMOUSE_BUTTON_t
{
  orxMOUSE_BUTTON_LEFT = 0,
  orxMOUSE_BUTTON_RIGHT,
  orxMOUSE_BUTTON_MIDDLE,
  orxMOUSE_BUTTON_EXTRA_1,
  orxMOUSE_BUTTON_EXTRA_2,
  orxMOUSE_BUTTON_WHEEL_UP,
  orxMOUSE_BUTTON_WHEEL_DOWN,
  orxMOUSE_BUTTON_NUMBER,

  orxMOUSE_BUTTON_NONE = orxENUM_NONE

} orxMOUSE_BUTTON;


/** Event enum
 */
typedef enum __orxMOUSE_EVENT_t
{
  orxMOUSE_EVENT_BUTTON_PRESSED = 0,
  orxMOUSE_EVENT_BUTTON_RELEASED,

  orxMOUSE_EVENT_NUMBER,

  orxMOUSE_EVENT_NONE = orxENUM_NONE

} orxMOUSE_EVENT;


/** Mouse event payload
 */
typedef struct __orxMOUSE_EVENT_PAYLOAD_t
{
  orxMOUSE_BUTTON eButton;        /**< Mouse button : 4 */

} orxMOUSE_EVENT_PAYLOAD;


#define orxMOUSE_KZ_CONFIG_SECTION      "Mouse"
#define orxMOUSE_KZ_CONFIG_SHOW_CURSOR  "ShowCursor"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Mouse module setup
 */
extern orxDLLAPI void orxFASTCALL             orxMouse_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the mouse module
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxMouse_Init();

/** Exits from the mouse module
 */
extern orxDLLAPI void orxFASTCALL             orxMouse_Exit();

/** Sets mouse position
* @param[in] _pvPosition  Mouse position
* @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
extern orxDLLAPI orxSTATUS orxFASTCALL        orxMouse_SetPosition(const orxVECTOR *_pvPosition);

/** Gets mouse position
 * @param[out] _pvPosition  Mouse position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxMouse_GetPosition(orxVECTOR *_pvPosition);

/** Is mouse button pressed?
 * @param[in] _eButton          Mouse button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxMouse_IsButtonPressed(orxMOUSE_BUTTON _eButton);

/** Gets mouse move delta (since last call)
 * @param[out] _pvMoveDelta Mouse move delta
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxMouse_GetMoveDelta(orxVECTOR *_pvMoveDelta);

/** Gets mouse wheel delta (since last call)
 * @return Mouse wheel delta
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxMouse_GetWheelDelta();

/** Shows mouse cursor
* @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
*/
extern orxDLLAPI orxSTATUS orxFASTCALL        orxMouse_ShowCursor(orxBOOL _bShow);

/** Gets button literal name
 * @param[in] _eButton          Concerned button
 * @return Button's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxMouse_GetButtonName(orxMOUSE_BUTTON _eButton);

#endif /* _orxMOUSE_H_ */

/** @} */

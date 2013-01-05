/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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

typedef enum __orxMOUSE_AXIS_t
{
  orxMOUSE_AXIS_X = 0,
  orxMOUSE_AXIS_Y,
  orxMOUSE_AXIS_NUMBER,

  orxMOUSE_AXIS_NONE = orxENUM_NONE

} orxMOUSE_AXIS;

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

/** Gets axis literal name
 * @param[in] _eAxis            Concerned axis
 * @return Axis's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxMouse_GetAxisName(orxMOUSE_AXIS _eAxis);

#endif /* _orxMOUSE_H_ */

/** @} */

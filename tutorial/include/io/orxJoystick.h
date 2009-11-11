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
 * @file orxJoystick.h
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxJoystick
 *
 * Joystick plugin module
 * Plugin module that handles joystick-like peripherals
 *
 * @{
 */


#ifndef _orxJOYSTICK_H_
#define _orxJOYSTICK_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/** Button enum
 */
typedef enum __orxJOYSTICK_BUTTON_t
{
  #ifdef __orxGP2X__

    orxJOYSTICK_BUTTON_1 = 0,
    orxJOYSTICK_BUTTON_2,
    orxJOYSTICK_BUTTON_3,
    orxJOYSTICK_BUTTON_4,
    orxJOYSTICK_BUTTON_5,
    orxJOYSTICK_BUTTON_6,
    orxJOYSTICK_BUTTON_7,
    orxJOYSTICK_BUTTON_8,
    orxJOYSTICK_BUTTON_9,
    orxJOYSTICK_BUTTON_10,
    orxJOYSTICK_BUTTON_11,
    orxJOYSTICK_BUTTON_12,
    orxJOYSTICK_BUTTON_13,
    orxJOYSTICK_BUTTON_14,
    orxJOYSTICK_BUTTON_15,
    orxJOYSTICK_BUTTON_16,

  #else /* __orxGP2X__ */

    orxJOYSTICK_BUTTON_1 = 0,
    orxJOYSTICK_BUTTON_2,
    orxJOYSTICK_BUTTON_3,
    orxJOYSTICK_BUTTON_4,
    orxJOYSTICK_BUTTON_5,
    orxJOYSTICK_BUTTON_6,
    orxJOYSTICK_BUTTON_7,
    orxJOYSTICK_BUTTON_8,
    orxJOYSTICK_BUTTON_9,
    orxJOYSTICK_BUTTON_10,
    orxJOYSTICK_BUTTON_11,
    orxJOYSTICK_BUTTON_12,
    orxJOYSTICK_BUTTON_13,
    orxJOYSTICK_BUTTON_14,
    orxJOYSTICK_BUTTON_15,
    orxJOYSTICK_BUTTON_16,

  #endif /* __orxGP2X__ */

  orxJOYSTICK_BUTTON_NUMBER,

  orxJOYSTICK_BUTTON_NONE = orxENUM_NONE

} orxJOYSTICK_BUTTON;


/** Axis enum
 */
typedef enum __orxJOYSTICK_AXIS_t
{
  #ifdef __orxGP2X__

    orxJOYSTICK_AXIS_X = 0,
    orxJOYSTICK_AXIS_Y,
    orxJOYSTICK_AXIS_Z,
    orxJOYSTICK_AXIS_R,
    orxJOYSTICK_AXIS_U,
    orxJOYSTICK_AXIS_V,
    orxJOYSTICK_AXIS_POV,

  #else /* __orxGP2X__ */

    orxJOYSTICK_AXIS_X = 0,
    orxJOYSTICK_AXIS_Y,
    orxJOYSTICK_AXIS_Z,
    orxJOYSTICK_AXIS_R,
    orxJOYSTICK_AXIS_U,
    orxJOYSTICK_AXIS_V,
    orxJOYSTICK_AXIS_POV,

  #endif /* __orxGP2X__ */

  orxJOYSTICK_AXIS_NUMBER,

  orxJOYSTICK_AXIS_NONE = orxENUM_NONE

} orxJOYSTICK_AXIS;


/** Event enum
 */
typedef enum __orxJOYSTICK_EVENT_t
{
  orxJOYSTICK_EVENT_BUTTON_PRESSED = 0,
  orxJOYSTICK_EVENT_BUTTON_RELEASED,

  orxJOYSTICK_EVENT_NUMBER,

  orxJOYSTICK_EVENT_NONE = orxENUM_NONE

} orxJOYSTICK_EVENT;


/** Joystick event payload
 */
typedef struct __orxJOYSTICK_EVENT_PAYLOAD_t
{
  orxU32              u32JoystickID;  /**< Joystick ID : 4 */
  orxJOYSTICK_BUTTON  eButton;        /**< Joystick button : 8 */

} orxJOYSTICK_EVENT_PAYLOAD;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** JOYSTICK module setup
 */
extern orxDLLAPI void orxFASTCALL             orxJoystick_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the joystick module
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxJoystick_Init();

/** Exits from the joystick module
 */
extern orxDLLAPI void orxFASTCALL             orxJoystick_Exit();

/** Gets JOYSTICK on screen position
 * @param[in] _u32ID        ID of the joystick to test
 * @param[in] _eAxis        Joystick axis to check
 * @return Value of the axis
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxJoystick_GetAxisValue(orxU32 _u32ID, orxJOYSTICK_AXIS _eAxis);

/** Is joystick button pressed?
 * @param[in] _u32ID        ID of the joystick to test
 * @param[in] _eButton      Joystick button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxJoystick_IsButtonPressed(orxU32 _u32ID, orxJOYSTICK_BUTTON _eButton);

/** Gets axis literal name
 * @param[in] _eAxis        Concerned axis
 * @return Axis's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxJoystick_GetAxisName(orxJOYSTICK_AXIS _eAxis);

/** Gets button literal name
 * @param[in] _eButton      Concerned button
 * @return Button's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxJoystick_GetButtonName(orxJOYSTICK_BUTTON _eButton);

#endif /* _orxJOYSTICK_H_ */

/** @} */

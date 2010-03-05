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
  orxJOYSTICK_BUTTON_1_1 = 0,
  orxJOYSTICK_BUTTON_2_1,
  orxJOYSTICK_BUTTON_3_1,
  orxJOYSTICK_BUTTON_4_1,
  orxJOYSTICK_BUTTON_5_1,
  orxJOYSTICK_BUTTON_6_1,
  orxJOYSTICK_BUTTON_7_1,
  orxJOYSTICK_BUTTON_8_1,
  orxJOYSTICK_BUTTON_9_1,
  orxJOYSTICK_BUTTON_10_1,
  orxJOYSTICK_BUTTON_11_1,
  orxJOYSTICK_BUTTON_12_1,
  orxJOYSTICK_BUTTON_13_1,
  orxJOYSTICK_BUTTON_14_1,
  orxJOYSTICK_BUTTON_15_1,
  orxJOYSTICK_BUTTON_16_1,

  orxJOYSTICK_BUTTON_SINGLE_NUMBER,
  
  orxJOYSTICK_BUTTON_1_2 = orxJOYSTICK_BUTTON_SINGLE_NUMBER,
  orxJOYSTICK_BUTTON_2_2,
  orxJOYSTICK_BUTTON_3_2,
  orxJOYSTICK_BUTTON_4_2,
  orxJOYSTICK_BUTTON_5_2,
  orxJOYSTICK_BUTTON_6_2,
  orxJOYSTICK_BUTTON_7_2,
  orxJOYSTICK_BUTTON_8_2,
  orxJOYSTICK_BUTTON_9_2,
  orxJOYSTICK_BUTTON_10_2,
  orxJOYSTICK_BUTTON_11_2,
  orxJOYSTICK_BUTTON_12_2,
  orxJOYSTICK_BUTTON_13_2,
  orxJOYSTICK_BUTTON_14_2,
  orxJOYSTICK_BUTTON_15_2,
  orxJOYSTICK_BUTTON_16_2,

  orxJOYSTICK_BUTTON_1_3,
  orxJOYSTICK_BUTTON_2_3,
  orxJOYSTICK_BUTTON_3_3,
  orxJOYSTICK_BUTTON_4_3,
  orxJOYSTICK_BUTTON_5_3,
  orxJOYSTICK_BUTTON_6_3,
  orxJOYSTICK_BUTTON_7_3,
  orxJOYSTICK_BUTTON_8_3,
  orxJOYSTICK_BUTTON_9_3,
  orxJOYSTICK_BUTTON_10_3,
  orxJOYSTICK_BUTTON_11_3,
  orxJOYSTICK_BUTTON_12_3,
  orxJOYSTICK_BUTTON_13_3,
  orxJOYSTICK_BUTTON_14_3,
  orxJOYSTICK_BUTTON_15_3,
  orxJOYSTICK_BUTTON_16_3,

  orxJOYSTICK_BUTTON_1_4,
  orxJOYSTICK_BUTTON_2_4,
  orxJOYSTICK_BUTTON_3_4,
  orxJOYSTICK_BUTTON_4_4,
  orxJOYSTICK_BUTTON_5_4,
  orxJOYSTICK_BUTTON_6_4,
  orxJOYSTICK_BUTTON_7_4,
  orxJOYSTICK_BUTTON_8_4,
  orxJOYSTICK_BUTTON_9_4,
  orxJOYSTICK_BUTTON_10_4,
  orxJOYSTICK_BUTTON_11_4,
  orxJOYSTICK_BUTTON_12_4,
  orxJOYSTICK_BUTTON_13_4,
  orxJOYSTICK_BUTTON_14_4,
  orxJOYSTICK_BUTTON_15_4,
  orxJOYSTICK_BUTTON_16_4,

  orxJOYSTICK_BUTTON_NUMBER,

  orxJOYSTICK_BUTTON_NONE = orxENUM_NONE

} orxJOYSTICK_BUTTON;


/** Axis enum
 */
typedef enum __orxJOYSTICK_AXIS_t
{
  orxJOYSTICK_AXIS_X_1 = 0,
  orxJOYSTICK_AXIS_Y_1,
  orxJOYSTICK_AXIS_Z_1,
  orxJOYSTICK_AXIS_R_1,
  orxJOYSTICK_AXIS_U_1,
  orxJOYSTICK_AXIS_V_1,
  orxJOYSTICK_AXIS_POV_1,

  orxJOYSTICK_AXIS_SINGLE_NUMBER,

  orxJOYSTICK_AXIS_X_2 = orxJOYSTICK_AXIS_SINGLE_NUMBER,
  orxJOYSTICK_AXIS_Y_2,
  orxJOYSTICK_AXIS_Z_2,
  orxJOYSTICK_AXIS_R_2,
  orxJOYSTICK_AXIS_U_2,
  orxJOYSTICK_AXIS_V_2,
  orxJOYSTICK_AXIS_POV_2,
  
  orxJOYSTICK_AXIS_X_3,
  orxJOYSTICK_AXIS_Y_3,
  orxJOYSTICK_AXIS_Z_3,
  orxJOYSTICK_AXIS_R_3,
  orxJOYSTICK_AXIS_U_3,
  orxJOYSTICK_AXIS_V_3,
  orxJOYSTICK_AXIS_POV_3,

  orxJOYSTICK_AXIS_X_4,
  orxJOYSTICK_AXIS_Y_4,
  orxJOYSTICK_AXIS_Z_4,
  orxJOYSTICK_AXIS_R_4,
  orxJOYSTICK_AXIS_U_4,
  orxJOYSTICK_AXIS_V_4,
  orxJOYSTICK_AXIS_POV_4,

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

/** Gets joystick axis value
 * @param[in] _eAxis        Joystick axis to check
 * @return Value of the axis
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxJoystick_GetAxisValue(orxJOYSTICK_AXIS _eAxis);

/** Is joystick button pressed?
 * @param[in] _eButton      Joystick button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON _eButton);

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

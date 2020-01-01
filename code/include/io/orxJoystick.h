/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2020 Orx-Project
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


/** Helpers
 */
#define orxJOYSTICK_GET_AXIS(AXIS, ID)          ((((orxU32)AXIS) % orxJOYSTICK_AXIS_SINGLE_NUMBER) + ((ID - 1) * orxJOYSTICK_AXIS_SINGLE_NUMBER))
#define orxJOYSTICK_GET_BUTTON(BUTTON, ID)      ((((orxU32)BUTTON) % orxJOYSTICK_BUTTON_SINGLE_NUMBER) + ((ID - 1) * orxJOYSTICK_BUTTON_SINGLE_NUMBER))

#define orxJOYSTICK_GET_ID_FROM_AXIS(AXIS)      ((((orxU32)AXIS) / orxJOYSTICK_AXIS_SINGLE_NUMBER) + 1)
#define orxJOYSTICK_GET_ID_FROM_BUTTON(BUTTON)  ((((orxU32)BUTTON) / orxJOYSTICK_BUTTON_SINGLE_NUMBER) + 1)

#define orxJOYSTICK_DECLARE_BUTTON_ENUM(ID) \
  orxJOYSTICK_BUTTON_A_##ID,                \
  orxJOYSTICK_BUTTON_B_##ID,                \
  orxJOYSTICK_BUTTON_X_##ID,                \
  orxJOYSTICK_BUTTON_Y_##ID,                \
  orxJOYSTICK_BUTTON_LBUMPER_##ID,          \
  orxJOYSTICK_BUTTON_RBUMPER_##ID,          \
  orxJOYSTICK_BUTTON_BACK_##ID,             \
  orxJOYSTICK_BUTTON_START_##ID,            \
  orxJOYSTICK_BUTTON_GUIDE_##ID,            \
  orxJOYSTICK_BUTTON_LTHUMB_##ID,           \
  orxJOYSTICK_BUTTON_RTHUMB_##ID,           \
  orxJOYSTICK_BUTTON_UP_##ID,               \
  orxJOYSTICK_BUTTON_RIGHT_##ID,            \
  orxJOYSTICK_BUTTON_DOWN_##ID,             \
  orxJOYSTICK_BUTTON_LEFT_##ID,             \
  orxJOYSTICK_BUTTON_1_##ID,                \
  orxJOYSTICK_BUTTON_2_##ID,                \
  orxJOYSTICK_BUTTON_3_##ID,                \
  orxJOYSTICK_BUTTON_4_##ID,                \
  orxJOYSTICK_BUTTON_5_##ID,                \
  orxJOYSTICK_BUTTON_6_##ID,                \
  orxJOYSTICK_BUTTON_7_##ID,                \
  orxJOYSTICK_BUTTON_8_##ID,                \
  orxJOYSTICK_BUTTON_9_##ID,                \
  orxJOYSTICK_BUTTON_10_##ID,               \
  orxJOYSTICK_BUTTON_11_##ID,               \
  orxJOYSTICK_BUTTON_12_##ID,               \
  orxJOYSTICK_BUTTON_13_##ID,               \
  orxJOYSTICK_BUTTON_14_##ID,               \
  orxJOYSTICK_BUTTON_15_##ID,               \
  orxJOYSTICK_BUTTON_16_##ID

#define orxJOYSTICK_DECLARE_AXIS_ENUM(ID)   \
  orxJOYSTICK_AXIS_LX_##ID,                 \
  orxJOYSTICK_AXIS_LY_##ID,                 \
  orxJOYSTICK_AXIS_RX_##ID,                 \
  orxJOYSTICK_AXIS_RY_##ID,                 \
  orxJOYSTICK_AXIS_LTRIGGER_##ID,           \
  orxJOYSTICK_AXIS_RTRIGGER_##ID

/** Button enum
 */
typedef enum __orxJOYSTICK_BUTTON_t
{
  orxJOYSTICK_BUTTON_A_1 = 0,
  orxJOYSTICK_BUTTON_B_1,
  orxJOYSTICK_BUTTON_X_1,
  orxJOYSTICK_BUTTON_Y_1,
  orxJOYSTICK_BUTTON_LBUMPER_1,
  orxJOYSTICK_BUTTON_RBUMPER_1,
  orxJOYSTICK_BUTTON_BACK_1,
  orxJOYSTICK_BUTTON_START_1,
  orxJOYSTICK_BUTTON_GUIDE_1,
  orxJOYSTICK_BUTTON_LTHUMB_1,
  orxJOYSTICK_BUTTON_RTHUMB_1,
  orxJOYSTICK_BUTTON_UP_1,
  orxJOYSTICK_BUTTON_RIGHT_1,
  orxJOYSTICK_BUTTON_DOWN_1,
  orxJOYSTICK_BUTTON_LEFT_1,
  orxJOYSTICK_BUTTON_1_1,
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

  orxJOYSTICK_BUTTON_A_2 = orxJOYSTICK_BUTTON_SINGLE_NUMBER,
  orxJOYSTICK_BUTTON_B_2,
  orxJOYSTICK_BUTTON_X_2,
  orxJOYSTICK_BUTTON_Y_2,
  orxJOYSTICK_BUTTON_LBUMPER_2,
  orxJOYSTICK_BUTTON_RBUMPER_2,
  orxJOYSTICK_BUTTON_BACK_2,
  orxJOYSTICK_BUTTON_START_2,
  orxJOYSTICK_BUTTON_GUIDE_2,
  orxJOYSTICK_BUTTON_LTHUMB_2,
  orxJOYSTICK_BUTTON_RTHUMB_2,
  orxJOYSTICK_BUTTON_UP_2,
  orxJOYSTICK_BUTTON_RIGHT_2,
  orxJOYSTICK_BUTTON_DOWN_2,
  orxJOYSTICK_BUTTON_LEFT_2,
  orxJOYSTICK_BUTTON_1_2,
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

  orxJOYSTICK_DECLARE_BUTTON_ENUM(3),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(4),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(5),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(6),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(7),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(8),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(9),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(10),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(11),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(12),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(13),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(14),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(15),
  orxJOYSTICK_DECLARE_BUTTON_ENUM(16),

  orxJOYSTICK_BUTTON_NUMBER,

  orxJOYSTICK_BUTTON_NONE = orxENUM_NONE

} orxJOYSTICK_BUTTON;

/** Axis enum
 */
typedef enum __orxJOYSTICK_AXIS_t
{
  orxJOYSTICK_AXIS_LX_1 = 0,
  orxJOYSTICK_AXIS_LY_1,
  orxJOYSTICK_AXIS_RX_1,
  orxJOYSTICK_AXIS_RY_1,
  orxJOYSTICK_AXIS_LTRIGGER_1,
  orxJOYSTICK_AXIS_RTRIGGER_1,

  orxJOYSTICK_AXIS_SINGLE_NUMBER,

  orxJOYSTICK_AXIS_LX_2 = orxJOYSTICK_AXIS_SINGLE_NUMBER,
  orxJOYSTICK_AXIS_LY_2,
  orxJOYSTICK_AXIS_RX_2,
  orxJOYSTICK_AXIS_RY_2,
  orxJOYSTICK_AXIS_LTRIGGER_2,
  orxJOYSTICK_AXIS_RTRIGGER_2,

  orxJOYSTICK_DECLARE_AXIS_ENUM(3),
  orxJOYSTICK_DECLARE_AXIS_ENUM(4),
  orxJOYSTICK_DECLARE_AXIS_ENUM(5),
  orxJOYSTICK_DECLARE_AXIS_ENUM(6),
  orxJOYSTICK_DECLARE_AXIS_ENUM(7),
  orxJOYSTICK_DECLARE_AXIS_ENUM(8),
  orxJOYSTICK_DECLARE_AXIS_ENUM(9),
  orxJOYSTICK_DECLARE_AXIS_ENUM(10),
  orxJOYSTICK_DECLARE_AXIS_ENUM(11),
  orxJOYSTICK_DECLARE_AXIS_ENUM(12),
  orxJOYSTICK_DECLARE_AXIS_ENUM(13),
  orxJOYSTICK_DECLARE_AXIS_ENUM(14),
  orxJOYSTICK_DECLARE_AXIS_ENUM(15),
  orxJOYSTICK_DECLARE_AXIS_ENUM(16),

  orxJOYSTICK_AXIS_NUMBER,

  orxJOYSTICK_AXIS_NONE = orxENUM_NONE

} orxJOYSTICK_AXIS;

#undef orxJOYSTICK_DECLARE_BUTTON_ENUM
#undef orxJOYSTICK_DECLARE_AXIS_ENUM

#define orxJOYSTICK_KU32_MIN_ID               1
#define orxJOYSTICK_KU32_MAX_ID               (orxJOYSTICK_BUTTON_NUMBER / orxJOYSTICK_BUTTON_SINGLE_NUMBER)


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
 * @return orxTRUE if pressed / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxJoystick_IsButtonPressed(orxJOYSTICK_BUTTON _eButton);

/** Gets button literal name
 * @param[in] _eButton      Concerned button
 * @return Button's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxJoystick_GetButtonName(orxJOYSTICK_BUTTON _eButton);

/** Gets axis literal name
 * @param[in] _eAxis        Concerned axis
 * @return Axis's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxJoystick_GetAxisName(orxJOYSTICK_AXIS _eAxis);

/** Is joystick connected?
 * @param[in] _u32ID        ID of the joystick, 1-based index
 * @return orxTRUE if connected / orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxJoystick_IsConnected(orxU32 _u32ID);

#endif /* _orxJOYSTICK_H_ */

/** @} */

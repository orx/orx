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
 * @file orxPlugin_Physics.h
 * @date 24/03/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all IDs of the physics plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_PHYSICS_H_
#define _orxPLUGIN_PHYSICS_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_t
{
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_EXIT,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_GRAVITY,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_GRAVITY,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_CREATE_BODY,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_DELETE_BODY,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_CREATE_BODY_PART,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_DELETE_BODY_PART,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_CREATE_BODY_JOINT,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_DELETE_BODY_JOINT,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_ROTATION,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_SPEED,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_ANGULAR_VELOCITY,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_CUSTOM_GRAVITY,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_POSITION,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_ROTATION,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_SPEED,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_ANGULAR_VELOCITY,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_CUSTOM_GRAVITY,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_MASS,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_MASS_CENTER,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_LINEAR_DAMPING,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_ANGULAR_DAMPING,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_LINEAR_DAMPING,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_ANGULAR_DAMPING,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_APPLY_TORQUE,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_APPLY_FORCE,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_APPLY_IMPULSE,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_PART_SELF_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_PART_CHECK_MASK,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_PART_SELF_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_PART_CHECK_MASK,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_IS_PART_SOLID,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_PART_SOLID,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_ENABLE_MOTOR,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_JOINT_MOTOR_SPEED,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_SET_JOINT_MAX_MOTOR_TORQUE,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_RAYCAST,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_ENABLE_SIMULATION,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_PHYSICS;

#endif /* _orxPLUGIN_PHYSICS_H_ */

/** @} */

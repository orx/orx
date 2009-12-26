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
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_GET_MASS_CENTER,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_APPLY_TORQUE,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_APPLY_FORCE,
  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_APPLY_IMPULSE,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_RAYCAST,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_ENABLE_SIMULATION,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_PHYSICS_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_PHYSICS;

#endif /* _orxPLUGIN_PHYSICS_H_ */

/** @} */

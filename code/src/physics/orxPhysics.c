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
 * @file orxPhysics.c
 * @date 24/03/2008
 * @author iarwain@orx-project.org
 *
 */


#include "physics/orxPhysics.h"

#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Render module setup
 */
void orxFASTCALL orxPhysics_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_EVENT);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Exit, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetGravity, orxSTATUS, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetGravity, orxVECTOR *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBody, orxPHYSICS_BODY *, const orxHANDLE, const orxBODY_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBody, void, orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBodyPart, orxPHYSICS_BODY_PART *, orxPHYSICS_BODY *, const orxBODY_PART_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBodyPart, void, orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPosition, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetRotation, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetSpeed, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetAngularVelocity, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetCustomGravity, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPosition, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetRotation, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetSpeed, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetAngularVelocity, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetCustomGravity, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetMassCenter, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyTorque, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyForce, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyImpulse, orxSTATUS, orxPHYSICS_BODY *, const  orxVECTOR *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Raycast, orxHANDLE, const orxVECTOR *, const orxVECTOR *, orxU16, orxU16, orxVECTOR *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_EnableSimulation, void, orxBOOL);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PHYSICS)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, INIT, orxPhysics_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, EXIT, orxPhysics_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_GRAVITY, orxPhysics_SetGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_GRAVITY, orxPhysics_GetGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY, orxPhysics_CreateBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY, orxPhysics_DeleteBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY_PART, orxPhysics_CreateBodyPart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY_PART, orxPhysics_DeleteBodyPart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_POSITION, orxPhysics_SetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ROTATION, orxPhysics_SetRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_SPEED, orxPhysics_SetSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ANGULAR_VELOCITY, orxPhysics_SetAngularVelocity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_CUSTOM_GRAVITY, orxPhysics_SetCustomGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_POSITION, orxPhysics_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ROTATION, orxPhysics_GetRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_SPEED, orxPhysics_GetSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ANGULAR_VELOCITY, orxPhysics_GetAngularVelocity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_CUSTOM_GRAVITY, orxPhysics_GetCustomGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_MASS_CENTER, orxPhysics_GetMassCenter)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_TORQUE, orxPhysics_ApplyTorque)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_FORCE, orxPhysics_ApplyForce)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_IMPULSE, orxPhysics_ApplyImpulse)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, RAYCAST, orxPhysics_Raycast)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, ENABLE_SIMULATION, orxPhysics_EnableSimulation)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(PHYSICS)


/* *** Core function implementations *** */

orxSTATUS orxFASTCALL orxPhysics_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Init)();
}

void orxFASTCALL orxPhysics_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Exit)();
}

orxSTATUS orxFASTCALL orxPhysics_SetGravity(const orxVECTOR *_pvGravity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetGravity)(_pvGravity);
}

orxVECTOR *orxFASTCALL orxPhysics_GetGravity(orxVECTOR *_pvGravity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetGravity)(_pvGravity);
}

orxPHYSICS_BODY *orxFASTCALL orxPhysics_CreateBody(const orxHANDLE _hUserData, const orxBODY_DEF *_pstBodyDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreateBody)(_hUserData, _pstBodyDef);
}

void orxFASTCALL orxPhysics_DeleteBody(orxPHYSICS_BODY *_pstBody)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeleteBody)(_pstBody);
}

orxPHYSICS_BODY_PART *orxFASTCALL orxPhysics_CreateBodyPart(orxPHYSICS_BODY *_pstBody, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreateBodyPart)(_pstBody, _pstBodyPartDef);
}

void orxFASTCALL orxPhysics_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeleteBodyPart)(_pstBodyPart);
}

orxSTATUS orxFASTCALL orxPhysics_SetPosition(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPosition)(_pstBody, _pvPosition);
}

orxSTATUS orxFASTCALL orxPhysics_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetRotation)(_pstBody, _fRotation);
}

orxSTATUS orxFASTCALL orxPhysics_SetSpeed(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvSpeed)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetSpeed)(_pstBody, _pvSpeed);
}

orxSTATUS orxFASTCALL orxPhysics_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetAngularVelocity)(_pstBody, _fVelocity);
}

orxSTATUS orxFASTCALL orxPhysics_SetCustomGravity(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvCustomGravity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetCustomGravity)(_pstBody, _pvCustomGravity);
}

orxVECTOR *orxFASTCALL orxPhysics_GetPosition(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetPosition)(_pstBody, _pvPosition);
}

orxFLOAT orxFASTCALL orxPhysics_GetRotation(const orxPHYSICS_BODY *_pstBody)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetRotation)(_pstBody);
}

orxVECTOR *orxFASTCALL orxPhysics_GetSpeed(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetSpeed)(_pstBody, _pvSpeed);
}

orxFLOAT orxFASTCALL orxPhysics_GetAngularVelocity(const orxPHYSICS_BODY *_pstBody)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetAngularVelocity)(_pstBody);
}

orxVECTOR *orxFASTCALL orxPhysics_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetCustomGravity)(_pstBody, _pvCustomGravity);
}

orxVECTOR *orxFASTCALL orxPhysics_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetMassCenter)(_pstBody, _pvMassCenter);
}

orxSTATUS orxFASTCALL orxPhysics_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_ApplyTorque)(_pstBody, _fTorque);
}

orxSTATUS orxFASTCALL orxPhysics_ApplyForce(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_ApplyForce)(_pstBody, _pvForce, _pvPoint);
}

orxSTATUS orxFASTCALL orxPhysics_ApplyImpulse(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_ApplyImpulse)(_pstBody, _pvImpulse, _pvPoint);
}

orxHANDLE orxFASTCALL orxPhysics_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Raycast)(_pvStart, _pvEnd, _u16SelfFlags, _u16CheckMask, _pvContact, _pvNormal);
}

void orxFASTCALL orxPhysics_EnableSimulation(orxBOOL _bEnable)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_EnableSimulation)(_bEnable);
}

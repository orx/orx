/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_PROFILER);
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
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBodyPart, orxPHYSICS_BODY_PART *, orxPHYSICS_BODY *, const orxHANDLE, const orxBODY_PART_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBodyPart, void, orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBodyJoint, orxPHYSICS_BODY_JOINT *, orxPHYSICS_BODY *, orxPHYSICS_BODY *, const orxHANDLE, const orxBODY_JOINT_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBodyJoint, void, orxPHYSICS_BODY_JOINT *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetJointMotorSpeed, void, orxPHYSICS_BODY_JOINT *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetJointMaxMotorTorque, void, orxPHYSICS_BODY_JOINT *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_EnableMotor, void, orxPHYSICS_BODY_JOINT *, orxBOOL);
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
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetMass, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetMassCenter, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetLinearDamping, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetAngularDamping, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetLinearDamping, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetAngularDamping, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyTorque, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyForce, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyImpulse, orxSTATUS, orxPHYSICS_BODY *, const  orxVECTOR *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartSelfFlags, orxSTATUS, orxPHYSICS_BODY_PART *, orxU16);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartCheckMask, orxSTATUS, orxPHYSICS_BODY_PART *, orxU16);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPartSelfFlags, orxU16, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPartCheckMask, orxU16, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_IsPartSolid, orxBOOL, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartSolid, orxSTATUS, orxPHYSICS_BODY_PART *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Raycast, orxHANDLE, const orxVECTOR *, const orxVECTOR *, orxU16, orxU16, orxBOOL, orxVECTOR *, orxVECTOR *);
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
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY_JOINT, orxPhysics_CreateBodyJoint)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY_JOINT, orxPhysics_DeleteBodyJoint)
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
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_MASS, orxPhysics_GetMass)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_MASS_CENTER, orxPhysics_GetMassCenter)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_LINEAR_DAMPING, orxPhysics_SetLinearDamping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ANGULAR_DAMPING, orxPhysics_SetAngularDamping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_LINEAR_DAMPING, orxPhysics_GetLinearDamping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ANGULAR_DAMPING, orxPhysics_GetAngularDamping)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_TORQUE, orxPhysics_ApplyTorque)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_FORCE, orxPhysics_ApplyForce)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_IMPULSE, orxPhysics_ApplyImpulse)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_SELF_FLAGS, orxPhysics_SetPartSelfFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_CHECK_MASK, orxPhysics_SetPartCheckMask)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_PART_SELF_FLAGS, orxPhysics_GetPartSelfFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_PART_CHECK_MASK, orxPhysics_GetPartCheckMask)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, IS_PART_SOLID, orxPhysics_IsPartSolid)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_SOLID, orxPhysics_SetPartSolid)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, ENABLE_MOTOR, orxPhysics_EnableMotor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_JOINT_MOTOR_SPEED, orxPhysics_SetJointMotorSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_JOINT_MAX_MOTOR_TORQUE, orxPhysics_SetJointMaxMotorTorque)
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

orxPHYSICS_BODY_PART *orxFASTCALL orxPhysics_CreateBodyPart(orxPHYSICS_BODY *_pstBody, const orxHANDLE _hUserData, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreateBodyPart)(_pstBody, _hUserData, _pstBodyPartDef);
}

void orxFASTCALL orxPhysics_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeleteBodyPart)(_pstBodyPart);
}

orxPHYSICS_BODY_JOINT *orxFASTCALL orxPhysics_CreateBodyJoint(orxPHYSICS_BODY *_pstSrcBody, orxPHYSICS_BODY *_pstDstBody, const orxHANDLE _hUserData, const orxBODY_JOINT_DEF *_pstBodyJointDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreateBodyJoint)(_pstSrcBody, _pstDstBody, _hUserData, _pstBodyJointDef);
}

void orxFASTCALL orxPhysics_DeleteBodyJoint(orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeleteBodyJoint)(_pstBodyJoint);
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

orxFLOAT orxFASTCALL orxPhysics_GetMass(const orxPHYSICS_BODY *_pstBody)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetMass)(_pstBody);
}

orxVECTOR *orxFASTCALL orxPhysics_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetMassCenter)(_pstBody, _pvMassCenter);
}

orxSTATUS orxFASTCALL orxPhysics_SetLinearDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetLinearDamping)(_pstBody, _fDamping);
}

orxSTATUS orxFASTCALL orxPhysics_SetAngularDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetAngularDamping)(_pstBody, _fDamping);
}

orxFLOAT orxFASTCALL orxPhysics_GetLinearDamping(const orxPHYSICS_BODY *_pstBody)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetLinearDamping)(_pstBody);
}

orxFLOAT orxFASTCALL orxPhysics_GetAngularDamping(const orxPHYSICS_BODY *_pstBody)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetAngularDamping)(_pstBody);
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

orxSTATUS orxFASTCALL orxPhysics_SetPartSelfFlags(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16SelfFlags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartSelfFlags)(_pstBodyPart, _u16SelfFlags);
}

orxSTATUS orxFASTCALL orxPhysics_SetPartCheckMask(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16CheckMask)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartCheckMask)(_pstBodyPart, _u16CheckMask);
}

orxU16 orxFASTCALL orxPhysics_GetPartSelfFlags(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetPartSelfFlags)(_pstBodyPart);
}

orxU16 orxFASTCALL orxPhysics_GetPartCheckMask(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
   return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetPartCheckMask)(_pstBodyPart);
}

orxBOOL orxFASTCALL orxPhysics_IsPartSolid(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_IsPartSolid)(_pstBodyPart);
}

orxSTATUS orxFASTCALL orxPhysics_SetPartSolid(orxPHYSICS_BODY_PART *_pstBodyPart, orxBOOL _bSolid)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartSolid)(_pstBodyPart, _bSolid);
}

void orxFASTCALL orxPhysics_SetJointMotorSpeed(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetJointMotorSpeed)(_pstBodyJoint, _fSpeed);
}

void orxFASTCALL orxPhysics_SetJointMaxMotorTorque(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetJointMaxMotorTorque)(_pstBodyJoint, _fMaxTorque);
}

void orxFASTCALL orxPhysics_EnableMotor(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxBOOL _bEnable)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_EnableMotor)(_pstBodyJoint, _bEnable);
}

orxHANDLE orxFASTCALL orxPhysics_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Raycast)(_pvStart, _pvEnd, _u16SelfFlags, _u16CheckMask, _bEarlyExit, _pvContact, _pvNormal);
}

void orxFASTCALL orxPhysics_EnableSimulation(orxBOOL _bEnable)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_EnableSimulation)(_bEnable);
}

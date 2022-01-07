/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @date 13/03/2021
 * @author iarwain@orx-project.org
 *
 * Dummy physics plugin implementation
 *
 */


#include "orxPluginAPI.h"

orxSTATUS orxFASTCALL orxPhysics_Dummy_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Not available on this platform!");

  /* Done! */
  return eResult;
}

void orxFASTCALL orxPhysics_Dummy_Exit()
{
  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetGravity(const orxVECTOR *_pvGravity)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetGravity(orxVECTOR *_pvGravity)
{
  /* Done! */
  return orxNULL;
}

orxPHYSICS_BODY *orxFASTCALL orxPhysics_Dummy_CreateBody(const orxHANDLE _hUserData, const orxBODY_DEF *_pstBodyDef)
{
  /* Done! */
  return orxNULL;
}

void orxFASTCALL orxPhysics_Dummy_DeleteBody(orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return;
}

orxPHYSICS_BODY_PART *orxFASTCALL orxPhysics_Dummy_CreatePart(orxPHYSICS_BODY *_pstBody, const orxHANDLE _hUserData, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  /* Done! */
  return orxNULL;
}

void orxFASTCALL orxPhysics_Dummy_DeletePart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return;
}

orxPHYSICS_BODY_JOINT *orxFASTCALL orxPhysics_Dummy_CreateJoint(orxPHYSICS_BODY *_pstSrcBody, orxPHYSICS_BODY *_pstDstBody, const orxHANDLE _hUserData, const orxBODY_JOINT_DEF *_pstBodyJointDef)
{
  /* Done! */
  return orxNULL;
}

void orxFASTCALL orxPhysics_Dummy_DeleteJoint(orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPosition(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetSpeed(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvSpeed)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetCustomGravity(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvCustomGravity)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetFixedRotation(orxPHYSICS_BODY *_pstBody, orxBOOL _bFixed)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetDynamic(orxPHYSICS_BODY *_pstBody, orxBOOL _bDynamic)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetAllowMoving(orxPHYSICS_BODY *_pstBody, orxBOOL _bAllowMoving)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetPosition(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxNULL;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetRotation(const orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return orxFLOAT_0;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetSpeed(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed)
{
  /* Done! */
  return orxNULL;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetSpeedAtWorldPosition(const orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition, orxVECTOR *_pvSpeed)
{
  /* Done! */
  return orxNULL;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetAngularVelocity(const orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return orxFLOAT_0;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity)
{
  /* Done! */
  return orxNULL;
}

orxBOOL orxFASTCALL orxPhysics_Dummy_IsFixedRotation(const orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return orxFALSE;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetMass(const orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return orxFLOAT_0;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter)
{
  /* Done! */
  return orxNULL;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetLinearDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetAngularDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetLinearDamping(const orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return orxFLOAT_0;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetAngularDamping(const orxPHYSICS_BODY *_pstBody)
{
  /* Done! */
  return orxFLOAT_0;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_ApplyForce(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_ApplyImpulse(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPartSelfFlags(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16SelfFlags)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPartCheckMask(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16CheckMask)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxU16 orxFASTCALL orxPhysics_Dummy_GetPartSelfFlags(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return 0;
}

orxU16 orxFASTCALL orxPhysics_Dummy_GetPartCheckMask(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return 0;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPartSolid(orxPHYSICS_BODY_PART *_pstBodyPart, orxBOOL _bSolid)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxBOOL orxFASTCALL orxPhysics_Dummy_IsPartSolid(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return orxFALSE;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPartFriction(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fFriction)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetPartFriction(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return orxFLOAT_0;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPartRestitution(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fRestitution)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetPartRestitution(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return orxFLOAT_0;
}

orxSTATUS orxFASTCALL orxPhysics_Dummy_SetPartDensity(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fDensity)
{
  /* Done! */
  return orxSTATUS_FAILURE;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetPartDensity(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  /* Done! */
  return orxFLOAT_0;
}

orxBOOL orxFASTCALL orxPhysics_Dummy_IsInsidePart(const orxPHYSICS_BODY_PART *_pstBodyPart, const orxVECTOR *_pvPosition)
{
  /* Done! */
  return orxFALSE;
}

void orxFASTCALL orxPhysics_Dummy_EnableMotor(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxBOOL _bEnable)
{
  /* Done! */
  return;
}

void orxFASTCALL orxPhysics_Dummy_SetJointMotorSpeed(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed)
{
  /* Done! */
  return;
}

void orxFASTCALL orxPhysics_Dummy_SetJointMaxMotorTorque(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque)
{
  /* Done! */
  return;
}

orxVECTOR *orxFASTCALL orxPhysics_Dummy_GetJointReactionForce(const orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxVECTOR *_pvForce)
{
  /* Done! */
  return orxNULL;
}

orxFLOAT orxFASTCALL orxPhysics_Dummy_GetJointReactionTorque(const orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  /* Done! */
  return orxFLOAT_0;
}

orxHANDLE orxFASTCALL orxPhysics_Dummy_Raycast(const orxVECTOR *_pvBegin, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  /* Done! */
  return orxHANDLE_UNDEFINED;
}

orxU32 orxFASTCALL orxPhysics_Dummy_BoxPick(const orxAABOX *_pstBox, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxHANDLE _ahUserDataList[], orxU32 _u32Number)
{
  /* Done! */
  return orxU32_UNDEFINED;
}

void orxFASTCALL orxPhysics_Dummy_EnableSimulation(orxBOOL _bEnable)
{
  /* Done! */
  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(PHYSICS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_Init, PHYSICS, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_Exit, PHYSICS, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetGravity, PHYSICS, SET_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetGravity, PHYSICS, GET_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_CreateBody, PHYSICS, CREATE_BODY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_DeleteBody, PHYSICS, DELETE_BODY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_CreatePart, PHYSICS, CREATE_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_DeletePart, PHYSICS, DELETE_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_CreateJoint, PHYSICS, CREATE_JOINT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_DeleteJoint, PHYSICS, DELETE_JOINT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPosition, PHYSICS, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetRotation, PHYSICS, SET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetSpeed, PHYSICS, SET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetAngularVelocity, PHYSICS, SET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetCustomGravity, PHYSICS, SET_CUSTOM_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetFixedRotation, PHYSICS, SET_FIXED_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetDynamic, PHYSICS, SET_DYNAMIC);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetAllowMoving, PHYSICS, SET_ALLOW_MOVING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetPosition, PHYSICS, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetRotation, PHYSICS, GET_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetSpeed, PHYSICS, GET_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetSpeedAtWorldPosition, PHYSICS, GET_SPEED_AT_WORLD_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetAngularVelocity, PHYSICS, GET_ANGULAR_VELOCITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetCustomGravity, PHYSICS, GET_CUSTOM_GRAVITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_IsFixedRotation, PHYSICS, IS_FIXED_ROTATION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetMass, PHYSICS, GET_MASS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetMassCenter, PHYSICS, GET_MASS_CENTER);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetLinearDamping, PHYSICS, SET_LINEAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetAngularDamping, PHYSICS, SET_ANGULAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetLinearDamping, PHYSICS, GET_LINEAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetAngularDamping, PHYSICS, GET_ANGULAR_DAMPING);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_ApplyTorque, PHYSICS, APPLY_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_ApplyForce, PHYSICS, APPLY_FORCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_ApplyImpulse, PHYSICS, APPLY_IMPULSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPartSelfFlags, PHYSICS, SET_PART_SELF_FLAGS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPartCheckMask, PHYSICS, SET_PART_CHECK_MASK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetPartSelfFlags, PHYSICS, GET_PART_SELF_FLAGS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetPartCheckMask, PHYSICS, GET_PART_CHECK_MASK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPartSolid, PHYSICS, SET_PART_SOLID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_IsPartSolid, PHYSICS, IS_PART_SOLID);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPartFriction, PHYSICS, SET_PART_FRICTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetPartFriction, PHYSICS, GET_PART_FRICTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPartRestitution, PHYSICS, SET_PART_RESTITUTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetPartRestitution, PHYSICS, GET_PART_RESTITUTION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetPartDensity, PHYSICS, SET_PART_DENSITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetPartDensity, PHYSICS, GET_PART_DENSITY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_IsInsidePart, PHYSICS, IS_INSIDE_PART);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_EnableMotor, PHYSICS, ENABLE_MOTOR);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetJointMotorSpeed, PHYSICS, SET_JOINT_MOTOR_SPEED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_SetJointMaxMotorTorque, PHYSICS, SET_JOINT_MAX_MOTOR_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetJointReactionForce, PHYSICS, GET_JOINT_REACTION_FORCE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_GetJointReactionTorque, PHYSICS, GET_JOINT_REACTION_TORQUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_Raycast, PHYSICS, RAYCAST);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_BoxPick, PHYSICS, BOX_PICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Dummy_EnableSimulation, PHYSICS, ENABLE_SIMULATION);
orxPLUGIN_USER_CORE_FUNCTION_END();

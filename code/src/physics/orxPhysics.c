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
 * @date 24/03/2008
 * @author iarwain@orx-project.org
 *
 */


#include "physics/orxPhysics.h"

#include "plugin/orxPluginCore.h"

#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

  #include <malloc.h>

#endif /* __orxMSVC__ */


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 */
static orxSTATUS orxFASTCALL orxPhysics_EventHandler(const orxEVENT *_pstEvent)
{
  static orxSTRING szCollisionFlagList = orxNULL;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Reload start? */
  if(_pstEvent->eID == orxCONFIG_EVENT_RELOAD_START)
  {
    /* No collision flag list stored? */
    if(szCollisionFlagList == orxNULL)
    {
      /* Pushes config section */
      orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);

      /* Stores it */
      szCollisionFlagList = orxConfig_DuplicateRawValue(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST);

      /* Pops config section */
      orxConfig_PopSection();
    }
  }
  /* Reload stop? */
  else if(_pstEvent->eID == orxCONFIG_EVENT_RELOAD_STOP)
  {
    /* Has stored collision flag list? */
    if(szCollisionFlagList != orxNULL)
    {
      /* Pushes config section */
      orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);

      /* Not defined in config? */
      if(orxConfig_HasValue(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST) == orxFALSE)
      {
        /* Stores it */
        orxConfig_SetString(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST, szCollisionFlagList);
      }

      /* Pops config section */
      orxConfig_PopSection();

      /* Deletes it */
      orxString_Delete(szCollisionFlagList);
      szCollisionFlagList = orxNULL;
    }
  }

  /* Done! */
  return eResult;
}


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
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_EVENT);

  return;
}

/** Gets collision flag literal name
 * @param[in] _u32Flag      Concerned collision flag numerical value
 * @return Flag's name
 */
const orxSTRING orxFASTCALL orxPhysics_GetCollisionFlagName(orxU32 _u32Flag)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(_u32Flag != 0);

  /* Is a flag? */
  if(orxMath_IsPowerOfTwo(_u32Flag) != orxFALSE)
  {
    orxU32 u32Index;

    /* Pushes config section */
    orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);

    /* Gets flag index */
    u32Index = orxMath_GetTrailingZeroCount(_u32Flag);

    /* Valid? */
    if(u32Index < (orxU32)orxConfig_GetListCount(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST))
    {
      /* Updates result */
      zResult = orxConfig_GetListString(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST, u32Index);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't get collision flag name for value <%d>: no flag has been defined with this value!", _u32Flag);
    }

    /* Pops config section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't get collision flag name for value <%d>: value needs to be a power of two!", _u32Flag);
  }

  /* Done! */
  return zResult;
}

/** Gets collision flag numerical value
 * @param[in] _zFlag        Concerned collision flag literal name
 * @return Flag's value
 */
orxU32 orxFASTCALL orxPhysics_GetCollisionFlagValue(const orxSTRING _zFlag)
{
  orxU32 u32Result = 0, u32Count;

  /* Checks */
  orxASSERT(_zFlag != orxNULL);

  /* Pushes config section */
  orxConfig_PushSection(orxPHYSICS_KZ_CONFIG_SECTION);

  /* Gets flag list count */
  u32Count = orxConfig_GetListCount(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST);

  {
#ifdef __orxMSVC__

    const orxSTRING *azFlagList = (const orxSTRING *)alloca((u32Count + 1) * sizeof(orxSTRING *));

#else /* __orxMSVC__ */

    const orxSTRING azFlagList[u32Count + 1];

#endif /* __orxMSVC__ */

    orxU32 i;

    /* For all flags */
    for(i = 0; i < u32Count; i++)
    {
      /* Gets it */
      azFlagList[i] = orxConfig_GetListString(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST, i);

      /* Found? */
      if(!orxString_ICompare(_zFlag, azFlagList[i]))
      {
        /* Updates result */
        u32Result = 1 << i;

        break;
      }
    }

    /* Not found? */
    if(u32Result == 0)
    {
      /* Is there room to add the new flag? */
      if(u32Count < 16)
      {
        /* Stores its name */
        azFlagList[u32Count] = _zFlag;

        /* Updates flag list */
        if(orxConfig_SetListString(orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST, azFlagList, u32Count + 1) != orxSTATUS_FAILURE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Adding collision flag <%s> with value <%d>.", _zFlag, 1 << u32Count);

          /* Updates result */
          u32Result = 1 << u32Count;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_PHYSICS, "Can't add collision flag <%s>: too many collision flags already defined!", _zFlag);
      }
    }
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return u32Result;
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
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreatePart, orxPHYSICS_BODY_PART *, orxPHYSICS_BODY *, const orxHANDLE, const orxBODY_PART_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeletePart, void, orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateJoint, orxPHYSICS_BODY_JOINT *, orxPHYSICS_BODY *, orxPHYSICS_BODY *, const orxHANDLE, const orxBODY_JOINT_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteJoint, void, orxPHYSICS_BODY_JOINT *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_EnableMotor, void, orxPHYSICS_BODY_JOINT *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetJointMotorSpeed, void, orxPHYSICS_BODY_JOINT *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetJointMaxMotorTorque, void, orxPHYSICS_BODY_JOINT *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetJointReactionForce, orxVECTOR *, const orxPHYSICS_BODY_JOINT *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetJointReactionTorque, orxFLOAT, const orxPHYSICS_BODY_JOINT *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPosition, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetRotation, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetSpeed, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetAngularVelocity, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetCustomGravity, orxSTATUS, orxPHYSICS_BODY *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetFixedRotation, orxSTATUS, orxPHYSICS_BODY *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetDynamic, orxSTATUS, orxPHYSICS_BODY *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetAllowMoving, orxSTATUS, orxPHYSICS_BODY *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPosition, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetRotation, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetSpeed, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetSpeedAtWorldPosition, orxVECTOR *, const orxPHYSICS_BODY *, const orxVECTOR *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetAngularVelocity, orxFLOAT, const orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetCustomGravity, orxVECTOR *, const orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_IsFixedRotation, orxBOOL, const orxPHYSICS_BODY *);
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
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartSolid, orxSTATUS, orxPHYSICS_BODY_PART *, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_IsPartSolid, orxBOOL, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartFriction, orxSTATUS, orxPHYSICS_BODY_PART *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPartFriction, orxFLOAT, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartRestitution, orxSTATUS, orxPHYSICS_BODY_PART *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPartRestitution, orxFLOAT, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPartDensity, orxSTATUS, orxPHYSICS_BODY_PART *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPartDensity, orxFLOAT, const orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_IsInsidePart, orxBOOL, const orxPHYSICS_BODY_PART *, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Raycast, orxHANDLE, const orxVECTOR *, const orxVECTOR *, orxU16, orxU16, orxBOOL, orxVECTOR *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_BoxPick, orxU32, const orxAABOX *, orxU16, orxU16, orxHANDLE [], orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_EnableSimulation, void, orxBOOL);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PHYSICS)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, INIT, orxPhysics_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, EXIT, orxPhysics_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_GRAVITY, orxPhysics_SetGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_GRAVITY, orxPhysics_GetGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY, orxPhysics_CreateBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY, orxPhysics_DeleteBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_PART, orxPhysics_CreatePart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_PART, orxPhysics_DeletePart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_JOINT, orxPhysics_CreateJoint)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_JOINT, orxPhysics_DeleteJoint)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_POSITION, orxPhysics_SetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ROTATION, orxPhysics_SetRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_SPEED, orxPhysics_SetSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ANGULAR_VELOCITY, orxPhysics_SetAngularVelocity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_CUSTOM_GRAVITY, orxPhysics_SetCustomGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_FIXED_ROTATION, orxPhysics_SetFixedRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_DYNAMIC, orxPhysics_SetDynamic)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ALLOW_MOVING, orxPhysics_SetAllowMoving)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_POSITION, orxPhysics_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ROTATION, orxPhysics_GetRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_SPEED, orxPhysics_GetSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_SPEED_AT_WORLD_POSITION, orxPhysics_GetSpeedAtWorldPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ANGULAR_VELOCITY, orxPhysics_GetAngularVelocity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_CUSTOM_GRAVITY, orxPhysics_GetCustomGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, IS_FIXED_ROTATION, orxPhysics_IsFixedRotation)
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
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_SOLID, orxPhysics_SetPartSolid)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, IS_PART_SOLID, orxPhysics_IsPartSolid)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_FRICTION, orxPhysics_SetPartFriction)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_PART_FRICTION, orxPhysics_GetPartFriction)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_RESTITUTION, orxPhysics_SetPartRestitution)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_PART_RESTITUTION, orxPhysics_GetPartRestitution)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_PART_DENSITY, orxPhysics_SetPartDensity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_PART_DENSITY, orxPhysics_GetPartDensity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, IS_INSIDE_PART, orxPhysics_IsInsidePart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, ENABLE_MOTOR, orxPhysics_EnableMotor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_JOINT_MOTOR_SPEED, orxPhysics_SetJointMotorSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_JOINT_MAX_MOTOR_TORQUE, orxPhysics_SetJointMaxMotorTorque)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_JOINT_REACTION_FORCE, orxPhysics_GetJointReactionForce)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_JOINT_REACTION_TORQUE, orxPhysics_GetJointReactionTorque)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, RAYCAST, orxPhysics_Raycast)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, BOX_PICK, orxPhysics_BoxPick)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, ENABLE_SIMULATION, orxPhysics_EnableSimulation)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(PHYSICS)


/* *** Core function implementations *** */

orxSTATUS orxFASTCALL orxPhysics_Init()
{
  /* Adds event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_CONFIG, orxPhysics_EventHandler);
  orxEvent_SetHandlerIDFlags(orxPhysics_EventHandler, orxEVENT_TYPE_CONFIG, orxNULL, orxEVENT_GET_FLAG(orxCONFIG_EVENT_RELOAD_START) | orxEVENT_GET_FLAG(orxCONFIG_EVENT_RELOAD_STOP), orxEVENT_KU32_MASK_ID_ALL);

  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Init)();
}

void orxFASTCALL orxPhysics_Exit()
{
  /* Removes event handler */
  orxEvent_RemoveHandler(orxEVENT_TYPE_CONFIG, orxPhysics_EventHandler);

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

orxPHYSICS_BODY_PART *orxFASTCALL orxPhysics_CreatePart(orxPHYSICS_BODY *_pstBody, const orxHANDLE _hUserData, const orxBODY_PART_DEF *_pstBodyPartDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreatePart)(_pstBody, _hUserData, _pstBodyPartDef);
}

void orxFASTCALL orxPhysics_DeletePart(orxPHYSICS_BODY_PART *_pstBodyPart)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeletePart)(_pstBodyPart);
}

orxPHYSICS_BODY_JOINT *orxFASTCALL orxPhysics_CreateJoint(orxPHYSICS_BODY *_pstSrcBody, orxPHYSICS_BODY *_pstDstBody, const orxHANDLE _hUserData, const orxBODY_JOINT_DEF *_pstBodyJointDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreateJoint)(_pstSrcBody, _pstDstBody, _hUserData, _pstBodyJointDef);
}

void orxFASTCALL orxPhysics_DeleteJoint(orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeleteJoint)(_pstBodyJoint);
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

orxSTATUS orxFASTCALL orxPhysics_SetFixedRotation(orxPHYSICS_BODY *_pstBody, orxBOOL _bFixed)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetFixedRotation)(_pstBody, _bFixed);
}

orxSTATUS orxFASTCALL orxPhysics_SetDynamic(orxPHYSICS_BODY *_pstBody, orxBOOL _bDynamic)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetDynamic)(_pstBody, _bDynamic);
}

orxSTATUS orxFASTCALL orxPhysics_SetAllowMoving(orxPHYSICS_BODY *_pstBody, orxBOOL _bAllowMoving)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetAllowMoving)(_pstBody, _bAllowMoving);
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

orxVECTOR *orxFASTCALL orxPhysics_GetSpeedAtWorldPosition(const orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition, orxVECTOR *_pvSpeed)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetSpeedAtWorldPosition)(_pstBody, _pvPosition, _pvSpeed);
}

orxFLOAT orxFASTCALL orxPhysics_GetAngularVelocity(const orxPHYSICS_BODY *_pstBody)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetAngularVelocity)(_pstBody);
}

orxVECTOR *orxFASTCALL orxPhysics_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetCustomGravity)(_pstBody, _pvCustomGravity);
}

orxBOOL orxFASTCALL orxPhysics_IsFixedRotation(const orxPHYSICS_BODY *_pstBody)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_IsFixedRotation)(_pstBody);
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

orxSTATUS orxFASTCALL orxPhysics_SetPartSolid(orxPHYSICS_BODY_PART *_pstBodyPart, orxBOOL _bSolid)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartSolid)(_pstBodyPart, _bSolid);
}

orxBOOL orxFASTCALL orxPhysics_IsPartSolid(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_IsPartSolid)(_pstBodyPart);
}

orxSTATUS orxFASTCALL orxPhysics_SetPartFriction(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fFriction)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartFriction)(_pstBodyPart, _fFriction);
}

orxFLOAT orxFASTCALL orxPhysics_GetPartFriction(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetPartFriction)(_pstBodyPart);
}

orxSTATUS orxFASTCALL orxPhysics_SetPartRestitution(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fRestitution)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartRestitution)(_pstBodyPart, _fRestitution);
}

orxFLOAT orxFASTCALL orxPhysics_GetPartRestitution(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetPartRestitution)(_pstBodyPart);
}

orxSTATUS orxFASTCALL orxPhysics_SetPartDensity(orxPHYSICS_BODY_PART *_pstBodyPart, orxFLOAT _fDensity)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetPartDensity)(_pstBodyPart, _fDensity);
}

orxFLOAT orxFASTCALL orxPhysics_GetPartDensity(const orxPHYSICS_BODY_PART *_pstBodyPart)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetPartDensity)(_pstBodyPart);
}

orxBOOL orxFASTCALL orxPhysics_IsInsidePart(const orxPHYSICS_BODY_PART *_pstBodyPart, const orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_IsInsidePart)(_pstBodyPart, _pvPosition);
}

void orxFASTCALL orxPhysics_EnableMotor(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxBOOL _bEnable)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_EnableMotor)(_pstBodyJoint, _bEnable);
}

void orxFASTCALL orxPhysics_SetJointMotorSpeed(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetJointMotorSpeed)(_pstBodyJoint, _fSpeed);
}

void orxFASTCALL orxPhysics_SetJointMaxMotorTorque(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_SetJointMaxMotorTorque)(_pstBodyJoint, _fMaxTorque);
}

orxVECTOR *orxFASTCALL orxPhysics_GetJointReactionForce(const orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxVECTOR *_pvForce)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetJointReactionForce)(_pstBodyJoint, _pvForce);
}

orxFLOAT orxFASTCALL orxPhysics_GetJointReactionTorque(const orxPHYSICS_BODY_JOINT *_pstBodyJoint)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_GetJointReactionTorque)(_pstBodyJoint);
}

orxHANDLE orxFASTCALL orxPhysics_Raycast(const orxVECTOR *_pvBegin, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Raycast)(_pvBegin, _pvEnd, _u16SelfFlags, _u16CheckMask, _bEarlyExit, _pvContact, _pvNormal);
}

orxU32 orxFASTCALL orxPhysics_BoxPick(const orxAABOX *_pstBox, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxHANDLE _ahUserDataList[], orxU32 _u32Number)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_BoxPick)(_pstBox, _u16SelfFlags, _u16CheckMask, _ahUserDataList, _u32Number);
}

void orxFASTCALL orxPhysics_EnableSimulation(orxBOOL _bEnable)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_EnableSimulation)(_bEnable);
}

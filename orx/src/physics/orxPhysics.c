/**
 * @file orxPhysics.c
 * 
 * Physics module
 * 
 */

 /***************************************************************************
 orxPhysics.c
 Physics module
 
 begin                : 24/03/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "physics/orxPhysics.h"

#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Render module setup
 */
orxVOID orxPhysics_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_CLOCK);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PHYSICS)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, INIT, orxPhysics_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, EXIT, orxPhysics_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_GRAVITY, orxPhysics_SetGravity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY, orxPhysics_CreateBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY, orxPhysics_DeleteBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY_PART, orxPhysics_CreateBodyPart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY_PART, orxPhysics_DeleteBodyPart)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_POSITION, orxPhysics_SetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ROTATION, orxPhysics_SetRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_SPEED, orxPhysics_SetSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, SET_ANGULAR_VELOCITY, orxPhysics_SetAngularVelocity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_POSITION, orxPhysics_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ROTATION, orxPhysics_GetRotation)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_SPEED, orxPhysics_GetSpeed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_ANGULAR_VELOCITY, orxPhysics_GetAngularVelocity)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, GET_MASS_CENTER, orxPhysics_GetMassCenter)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_TORQUE, orxPhysics_ApplyTorque)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_FORCE, orxPhysics_ApplyForce)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, APPLY_IMPULSE, orxPhysics_ApplyImpulse)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(PHYSICS)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetGravity, orxSTATUS, orxCONST orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBody, orxPHYSICS_BODY *, orxCONST orxHANDLE, orxCONST orxBODY_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBody, orxVOID, orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBodyPart, orxPHYSICS_BODY_PART *, orxPHYSICS_BODY *, orxCONST orxBODY_PART_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBodyPart, orxVOID, orxPHYSICS_BODY_PART *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetPosition, orxSTATUS, orxPHYSICS_BODY *, orxCONST orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetRotation, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetSpeed, orxSTATUS, orxPHYSICS_BODY *, orxCONST orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_SetAngularVelocity, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetPosition, orxVECTOR *, orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetRotation, orxFLOAT, orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetSpeed, orxVECTOR *, orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetAngularVelocity, orxFLOAT, orxPHYSICS_BODY *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_GetMassCenter, orxVECTOR *, orxPHYSICS_BODY *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyTorque, orxSTATUS, orxPHYSICS_BODY *, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyForce, orxSTATUS, orxPHYSICS_BODY *, orxCONST orxVECTOR *, orxCONST orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_ApplyImpulse, orxSTATUS, orxPHYSICS_BODY *, orxCONST  orxVECTOR *, orxCONST orxVECTOR *);

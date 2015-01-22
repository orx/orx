/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxBody.h
 * @date 10/03/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxBody
 *
 * Body Module
 * Allows to creates and handle physical bodies
 * They are used as container with associated properties
 * Bodies are used by objects
 * They thus can be referenced by objects as structures
 *
 * @{
 */


#ifndef _orxBODY_H_
#define _orxBODY_H_

#include "orxInclude.h"

#include "object/orxStructure.h"
#include "physics/orxPhysics.h"


/** Internal Body structure
 */
typedef struct __orxBODY_t                    orxBODY;

/** Internal Body part structure
 */
typedef struct __orxBODY_PART_t               orxBODY_PART;

/** Internal Body joint structure
 */
typedef struct __orxBODY_JOINT_t              orxBODY_JOINT;


/** Body module setup
 */
extern orxDLLAPI void orxFASTCALL             orxBody_Setup();

/** Inits the Body module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_Init();

/** Exits from the Body module
 */
extern orxDLLAPI void orxFASTCALL             orxBody_Exit();


/** Creates an empty body
 * @param[in]   _pstOwner                     Body's owner used for collision callbacks (usually an orxOBJECT)
 * @param[in]   _pstBodyDef                   Body definition
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_Create(const orxSTRUCTURE *_pstOwner, const orxBODY_DEF *_pstBodyDef);

/** Creates a body from config
 * @param[in]   _pstOwner                     Body's owner used for collision callbacks (usually an orxOBJECT)
 * @param[in]   _zConfigID                    Body config ID
 * @return      Created orxGRAPHIC / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_CreateFromConfig(const orxSTRUCTURE *_pstOwner, const orxSTRING _zConfigID);

/** Deletes a body
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_Delete(orxBODY *_pstBody);


/** Tests flags against body definition ones
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxBody_TestDefFlags(const orxBODY *_pstBody, orxU32 _u32Flags);

/** Tests all flags against body definition ones
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Flags       Flags to test
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxBody_TestAllDefFlags(const orxBODY *_pstBody, orxU32 _u32Flags);

/** Gets body definition flags
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Mask        Mask to use for getting flags
 * @return      orxU32
 */
extern orxDLLAPI orxU32 orxFASTCALL           orxBody_GetDefFlags(const orxBODY *_pstBody, orxU32 _u32Mask);


/** Gets a body owner
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxBody_GetOwner(const orxBODY *_pstBody);

/** Adds a part to body
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pstBodyPartDef Body part definition
 * @return      orxBODY_PART / orxNULL
 */
extern orxDLLAPI orxBODY_PART *orxFASTCALL    orxBody_AddPart(orxBODY *_pstBody, const orxBODY_PART_DEF *_pstBodyPartDef);

/** Adds a part to body from config
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _zConfigID      Body part config ID
 * @return      orxBODY_PART / orxNULL
 */
extern orxDLLAPI orxBODY_PART *orxFASTCALL    orxBody_AddPartFromConfig(orxBODY *_pstBody, const orxSTRING _zConfigID);

/** Removes a part using its config ID
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _zConfigID      Config ID of the part to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_RemovePartFromConfig(orxBODY *_pstBody, const orxSTRING _zConfigID);

/** Gets next body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pstBodyPart    Current body part (orxNULL to get the first one)
 * @return      orxBODY_PART / orxNULL
 */
extern orxDLLAPI orxBODY_PART *orxFASTCALL    orxBody_GetNextPart(const orxBODY *_pstBody, const orxBODY_PART *_pstBodyPart);

/** Gets a body part name
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      orxSTRING / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxBody_GetPartName(const orxBODY_PART *_pstBodyPart);

/** Gets a body part definition (matching current part status)
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      orxBODY_PART_DEF / orxNULL
 */
extern orxDLLAPI const orxBODY_PART_DEF *orxFASTCALL orxBody_GetPartDef(const orxBODY_PART *_pstBodyPart);

/** Gets a body part body (ie. owner)
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      orxBODY / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_GetPartBody(const orxBODY_PART *_pstBodyPart);

/** Removes a body part
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_RemovePart(orxBODY_PART *_pstBodyPart);


/** Adds a joint to link two bodies together
 * @param[in]   _pstSrcBody       Concerned source body
 * @param[in]   _pstDstBody       Concerned destination body
 * @param[in]   _pstBodyJointDef  Body joint definition
 * @return      orxBODY_JOINT / orxNULL
 */
extern orxDLLAPI orxBODY_JOINT *orxFASTCALL   orxBody_AddJoint(orxBODY *_pstSrcBody, orxBODY *_pstDstBody, const orxBODY_JOINT_DEF *_pstBodyJointDef);

/** Adds a joint from config to link two bodies together
 * @param[in]   _pstSrcBody     Concerned source body
 * @param[in]   _pstDstBody     Concerned destination body
 * @param[in]   _zConfigID      Body joint config ID
 * @return      orxBODY_JOINT / orxNULL
 */
extern orxDLLAPI orxBODY_JOINT *orxFASTCALL   orxBody_AddJointFromConfig(orxBODY *_pstSrcBody, orxBODY *_pstDstBody, const orxSTRING _zConfigID);

/** Gets next body joint
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pstBodyJoint   Current body joint (orxNULL to get the first one)
 * @return      orxBODY_JOINT / orxNULL
 */
extern orxDLLAPI orxBODY_JOINT *orxFASTCALL   orxBody_GetNextJoint(const orxBODY *_pstBody, const orxBODY_JOINT *_pstBodyJoint);

/** Gets a body joint name
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @return      orxSTRING / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxBody_GetJointName(const orxBODY_JOINT *_pstBodyJoint);

/** Removes a body joint
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_RemoveJoint(orxBODY_JOINT *_pstBodyJoint);


/** Sets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvPosition     Position to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPosition(orxBODY *_pstBody, const orxVECTOR *_pvPosition);

/** Sets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fRotation      Rotation to set (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetRotation(orxBODY *_pstBody, orxFLOAT _fRotation);

/** Sets a body scale
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvScale        Scale to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetScale(orxBODY *_pstBody, const orxVECTOR *_pvScale);

/** Sets a body speed
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvSpeed        Speed to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetSpeed(orxBODY *_pstBody, const orxVECTOR *_pvSpeed);

/** Sets a body angular velocity
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fVelocity      Angular velocity to set (radians/seconds)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetAngularVelocity(orxBODY *_pstBody, orxFLOAT _fVelocity);

/** Sets a body custom gravity
 * @param[in]   _pstBody          Concerned body
 * @param[in]   _pvCustomGravity  Custom gravity to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetCustomGravity(orxBODY *_pstBody, const orxVECTOR *_pvCustomGravity);

/** Sets a body fixed rotation
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _bFixed         Fixed / not fixed
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetFixedRotation(orxBODY *_pstBody, orxBOOL _bFixed);

/** Gets a body position
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvPosition     Position to get
 * @return      Body position / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetPosition(const orxBODY *_pstBody, orxVECTOR *_pvPosition);

/** Gets a body rotation
 * @param[in]   _pstBody        Concerned body
 * @return      Body rotation (radians)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetRotation(const orxBODY *_pstBody);

/** Gets a body speed
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvSpeed        Speed to get
 * @return      Body speed / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetSpeed(const orxBODY *_pstBody, orxVECTOR *_pvSpeed);

/** Gets a body speed at a specified world position
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvPosition     Concerned world position
 * @param[out]  _pvSpeed        Speed to get
 * @return      Body speed / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetSpeedAtWorldPosition(const orxBODY *_pstBody, const orxVECTOR *_pvPosition, orxVECTOR *_pvSpeed);

/** Gets a body angular velocity
 * @param[in]   _pstBody        Concerned body
 * @return      Body angular velocity (radians/seconds)
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetAngularVelocity(const orxBODY *_pstBody);

/** Gets a body custom gravity
 * @param[in]   _pstBody          Concerned body
 * @param[out]  _pvCustomGravity  Custom gravity to get
 * @return      Body custom gravity / orxNULL is object doesn't have any
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetCustomGravity(const orxBODY *_pstBody, orxVECTOR *_pvCustomGravity);

/** Is a body using a fixed rotation
 * @param[in]   _pstBody        Concerned body
 * @return      orxTRUE if fixed rotation, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxBody_IsFixedRotation(const orxBODY *_pstBody);

/** Gets a body mass
 * @param[in]   _pstBody        Concerned body
 * @return      Body mass
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetMass(const orxBODY *_pstBody);

/** Gets a body center of mass (object space)
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvMassCenter   Mass center to get
 * @return      Mass center / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetMassCenter(const orxBODY *_pstBody, orxVECTOR *_pvMassCenter);


/** Sets a body linear damping
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fDamping       Linear damping to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetLinearDamping(orxBODY *_pstBody, orxFLOAT _fDamping);

/** Sets a body angular damping
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fDamping       Angular damping to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetAngularDamping(orxBODY *_pstBody, orxFLOAT _fDamping);

/** Gets a body linear damping
 * @param[in]   _pstBody        Concerned body
 * @return      Body's linear damping
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetLinearDamping(const orxBODY *_pstBody);

/** Gets a body angular damping
 * @param[in]   _pstBody        Concerned body
 * @return      Body's angular damping
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetAngularDamping(const orxBODY *_pstBody);


/** Applies a torque
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _fTorque        Torque to apply
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_ApplyTorque(orxBODY *_pstBody, orxFLOAT _fTorque);

/** Applies a force
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvForce        Force to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the force will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_ApplyForce(orxBODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint);

/** Applies an impulse
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _pvImpulse      Impulse to apply
 * @param[in]   _pvPoint        Point (world coordinates) where the impulse will be applied, if orxNULL, center of mass will be used
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_ApplyImpulse(orxBODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint);


/** Sets self flags of a body part
 * @param[in]   _pstBodyPart    Concerned body part
 * @param[in]   _u16SelfFlags   Self flags to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPartSelfFlags(orxBODY_PART *_pstBodyPart, orxU16 _u16SelfFlags);

/** Sets check mask of a body part
 * @param[in]   _pstBodyPart    Concerned body part
 * @param[in]   _u16CheckMask   Check mask to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPartCheckMask(orxBODY_PART *_pstBodyPart, orxU16 _u16CheckMask);

/** Gets self flags of a body part
 * @param[in]   _pstBodyPart    Concerned body part
 * @return Self flags of the body part
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxBody_GetPartSelfFlags(const orxBODY_PART *_pstBodyPart);

/** Gets check mask of a body part
 * @param[in]   _pstBodyPart    Concerned body part
 * @return Check mask of the body part
 */
extern orxDLLAPI orxU16 orxFASTCALL           orxBody_GetPartCheckMask(const orxBODY_PART *_pstBodyPart);

/** Is a body part solid?
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxBody_IsPartSolid(const orxBODY_PART *_pstBodyPart);

/** Sets a body part solid
 * @param[in]   _pstBodyPart    Concerned body part
 * @param[in]   _bSolid         Solid or sensor?
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPartSolid(orxBODY_PART *_pstBodyPart, orxBOOL _bSolid);


/** Enables a (revolute) body joint motor
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @param[in]   _bEnable        Enable / Disable
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_EnableMotor(orxBODY_JOINT *_pstBodyJoint, orxBOOL _bEnable);

/** Sets a (revolute) body joint motor speed
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @param[in]   _fSpeed         Speed
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetJointMotorSpeed(orxBODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed);

/** Sets a (revolute) body joint maximum motor torque
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @param[in]   _fMaxTorque     Maximum motor torque
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetJointMaxMotorTorque(orxBODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque);


/** Gets the reaction force on the attached body at the joint anchor
 * @param[in]   _pstBodyJoint                         Concerned body joint
 * @param[out]  _pvForce                              Reaction force
 * @return      Reaction force in Newtons
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetJointReactionForce(const orxBODY_JOINT *_pstBodyJoint, orxVECTOR *_pvForce);

/** Gets the reaction torque on the attached body
 * @param[in]   _pstBodyJoint                         Concerned body joint
 * @return      Reaction torque
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxBody_GetJointReactionTorque(const orxBODY_JOINT *_pstBodyJoint);


/** Issues a raycast to test for potential bodies in the way
 * @param[in]   _pvStart        Start of raycast
 * @param[in]   _pvEnd          End of raycast
 * @param[in]   _u16SelfFlags   Selfs flags used for filtering (0xFFFF for no filtering)
 * @param[in]   _u16CheckMask   Check mask used for filtering (0xFFFF for no filtering)
 * @param[in]   _bEarlyExit     Should stop as soon as an object has been hit (which might not be the closest)
 * @param[in]   _pvContact      If non-null and a contact is found it will be stored here
 * @param[in]   _pvNormal       If non-null and a contact is found, its normal will be stored here
 * @return Colliding orxBODY / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal);

#endif /* _orxBODY_H_ */

/** @} */

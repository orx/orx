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


/** Misc defines
 */
#define orxBODY_KU32_PART_MAX_NUMBER          8


/** Internal Body structure
 */
typedef struct __orxBODY_t                    orxBODY;

/** Internal Body part structure
 */
typedef struct __orxBODY_PART_t               orxBODY_PART;


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

/** Gets a body owner
 * @param[in]   _pstBody        Concerned body
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL    orxBody_GetOwner(const orxBODY *_pstBody);

/** Adds a part to body
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_PART_MAX_NUMBER)
 * @param[in]   _pstBodyPartDef Body part definition
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_AddPart(orxBODY *_pstBody, orxU32 _u32Index, const orxBODY_PART_DEF *_pstBodyPartDef);

/** Adds a part to body from config
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_PART_MAX_NUMBER)
 * @param[in]   _zConfigID      Body part config ID
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_AddPartFromConfig(orxBODY *_pstBody, orxU32 _u32Index, const orxSTRING _zConfigID);

/** Gets a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Body part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxPHYSICS_BODY_PART / orxNULL
 */
extern orxDLLAPI orxPHYSICS_BODY_PART *orxFASTCALL orxBody_GetPart(const orxBODY *_pstBody, orxU32 _u32Index);

/** Gets a body part name
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTRING / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxBody_GetPartName(const orxBODY *_pstBody, orxU32 _u32Index);

/** Removes a body part
 * @param[in]   _pstBody        Concerned body
 * @param[in]   _u32Index       Part index (should be less than orxBODY_KU32_DATA_MAX_NUMBER)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_RemovePart(orxBODY *_pstBody, orxU32 _u32Index);


/** Sets a body template
 * @param[in]   _pstBodyTemplate  Body template to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetTemplate(const orxBODY_DEF *_pstBodyTemplate);

/** Sets a body part template
 * @param[in]   _pstBodyPartTemplate  Body part template to set / orxNULL to remove it
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxBody_SetPartTemplate(const orxBODY_PART_DEF *_pstBodyPartTemplate);

/** Gets the body template
 * @param[out]  _pstBodyTemplate  Body template to get
 * @return      orxBODY_DEF / orxNULL
 */
extern orxDLLAPI orxBODY_DEF *orxFASTCALL     orxBody_GetTemplate(orxBODY_DEF *_pstBodyTemplate);

/** Gets the body part template
 * @param[out]  _pstBodyPartTemplate  Body part template to get
 * @return      orxBODY_PART_DEF / orxNULL
 */
extern orxDLLAPI orxBODY_PART_DEF *orxFASTCALL orxBody_GetPartTemplate(orxBODY_PART_DEF *_pstBodyPartTemplate);

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
 * @param[out]   _pvSpeed       Speed to get
 * @return      Body speed / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetSpeed(const orxBODY *_pstBody, orxVECTOR *_pvSpeed);

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

/** Gets a body center of mass
 * @param[in]   _pstBody        Concerned body
 * @param[out]  _pvMassCenter   Mass center to get
 * @return      Mass center / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxBody_GetMassCenter(const orxBODY *_pstBody, orxVECTOR *_pvMassCenter);


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


/** Issues a raycast to test for potential bodies in the way
 * @param[in]   _pvStart        Start of raycast
 * @param[in]   _pvEnd          End of raycast
 * @param[in]   _u16SelfFlags   Selfs flags used for filtering (0xFFFF for no filtering)
 * @param[in]   _u16CheckMask   Check mask used for filtering (0xFFFF for no filtering)
 * @param[in]   _pvContact      If non-null and a contact is found it will be stored here
 * @param[in]   _pvNormal       If non-null and a contact is found, its normal will be stored here
 * @return Colliding orxBODY / orxNULL
 */
extern orxDLLAPI orxBODY *orxFASTCALL         orxBody_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxVECTOR *_pvContact, orxVECTOR *_pvNormal);


/** Applies physics simulation result to the Body
 * @param[in]   _pstBody                      Concerned body
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI void orxFASTCALL             orxBody_ApplySimulationResult(orxBODY *_pstBody);

#endif /* _orxBODY_H_ */

/** @} */

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
 * @file orxPhysics.h
 * @date 24/03/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPhysics
 * 
 * Physics plugin module
 * Module that handles physics interactions
 *
 * @{
 */


#ifndef _orxPHYSICS_H_
#define _orxPHYSICS_H_


#include "orxInclude.h"
#include "plugin/orxPluginCore.h"
#include "math/orxAABox.h"


/** Body definition flags
 */
#define orxBODY_DEF_KU32_FLAG_NONE                0x00000000  /**< No flags */

#define orxBODY_DEF_KU32_FLAG_2D                  0x00000001  /**< 2D type body def flag */
#define orxBODY_DEF_KU32_FLAG_DYNAMIC             0x00000002  /**< Dynamic type body def flag */
#define orxBODY_DEF_KU32_FLAG_HIGH_SPEED          0x00000004  /**< High speed type body def flag */
#define orxBODY_DEF_KU32_FLAG_FIXED_ROTATION      0x00000008  /**< Body can't be rotated by physics */
#define orxBODY_DEF_KU32_FLAG_CAN_SLIDE           0x00000010  /**< Body is allowed to slide */

#define orxBODY_DEF_KU32_MASK_ALL                 0xFFFFFFFF  /**< Body def all mask */

/** Body part definition flags
 */
#define orxBODY_PART_DEF_KU32_FLAG_NONE           0x00000000  /**< No flags */

#define orxBODY_PART_DEF_KU32_FLAG_SPHERE         0x00000001  /**< Sphere body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_BOX            0x00000002  /**< Box body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_MESH           0x00000004  /**< Mesh body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_SOLID          0x00000010  /**< Solid body part def flag */

#define orxBODY_PART_DEF_KU32_MASK_ALL            0xFFFFFFFF  /**< Body part def all mask */


/** Misc defines
 */
#define orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER  8


/** Body definition
 */
typedef struct __orxBODY_DEF_t
{
  orxVECTOR vPosition;                      /**< Position : 12 */
  orxFLOAT  fRotation;                      /**< Rotation : 16 */
  orxFLOAT  fInertia;                       /**< Inertia : 20 */
  orxFLOAT  fMass;                          /**< Mass : 24 */
  orxFLOAT  fLinearDamping;                 /**< Linear damping : 28 */
  orxFLOAT  fAngularDamping;                /**< Angular damping : 32 */
  orxU32    u32Flags;                       /**< Control flags : 36 */

} orxBODY_DEF;

/** Shape definition
 */
typedef struct __orxBODY_PART_DEF_t
{
  orxVECTOR vScale;                         /**< Scale : 12 */
  orxFLOAT  fFriction;                      /**< Friction : 16 */
  orxFLOAT  fRestitution;                   /**< Restitution : 20 */
  orxFLOAT  fDensity;                       /**< Density : 24 */
  orxU16    u16SelfFlags;                   /**< Self defining flags : 26 */
  orxU16    u16CheckMask;                   /**< Check mask : 28 */
  orxU32    u32Flags;                       /**< Control flags : 32 */

  union
  {
    struct
    {
      orxVECTOR vCenter;                    /**< Sphere center : 44 */
      orxFLOAT  fRadius;                    /**< Sphere radius : 48 */
    } stSphere;                             /**< Sphere : 48 */

    struct
    {
      orxAABOX  stBox;                      /**< Axis aligned Box : 56 */
    } stAABox;                              /**< Box : 56 */

    struct
    {
      orxU32    u32VertexCounter;           /**< Mesh vertex counter : 36 */
      orxVECTOR avVertices[orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER]; /**< Mesh vertices : 132 */

    } stMesh;

  };                                        /**< Shape : 132 */

} orxBODY_PART_DEF;

/** Event enum
 */
typedef enum __orxPHYSICS_EVENT_t
{
  orxPHYSICS_EVENT_CONTACT_ADD = 0,
  orxPHYSICS_EVENT_CONTACT_REMOVE,
  orxPHYSICS_EVENT_OUT_OF_WORLD,

  orxPHYSICS_EVENT_NUMBER,

  orxPHYSICS_EVENT_NONE = orxENUM_NONE

} orxPHYSICS_EVENT;

/** Contact event payload
 */
typedef struct __orxPHYSICS_EVENT_PAYLOAD_t
{
  orxVECTOR vPosition;                    /**< Contact position : 12 */
  orxVECTOR vNormal;                      /**< Contact normal : 24 */
  orxSTRING zSenderPartName;              /**< Sender part name : 28 */
  orxSTRING zRecipientPartName;           /**< Recipient part name : 32 */

} orxPHYSICS_EVENT_PAYLOAD;

/** Internal physics body structure
 */
typedef struct __orxPHYSICS_BODY_t        orxPHYSICS_BODY;

/** Internal physics shape structure
 */
typedef struct __orxPHYSICS_BODY_PART_t   orxPHYSICS_BODY_PART;


/** Config defines
 */
#define orxPHYSICS_KZ_CONFIG_SECTION      "Physics"
#define orxPHYSICS_KZ_CONFIG_GRAVITY      "Gravity"
#define orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP  "AllowSleep"
#define orxPHYSICS_KZ_CONFIG_WORLD_LOWER  "WorldLowerBound"
#define orxPHYSICS_KZ_CONFIG_WORLD_UPPER  "WorldUpperBound"
#define orxPHYSICS_KZ_CONFIG_ITERATIONS   "IterationsPerStep"
#define orxPHYSICS_KZ_CONFIG_FREQUENCY    "SimulationFrequency"
#define orxPHYSICS_KZ_CONFIG_RATIO        "DimensionRatio"
#define orxPHYSICS_KZ_CONFIG_FIXED_DT     "FixedDT"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Physics module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the physics module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_Init();

/** Exits from the physics module
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_Exit();


/** Sets physics gravity
 * @param[in]   _pvGravity                            Gravity to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetGravity(const orxVECTOR *_pvGravity);

/** Gets physics gravity
 * @param[in]   _pvGravity                            Gravity to get
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetGravity(orxVECTOR *_pvGravity);


/** Creates a physical body
 * @param[in]   _hUserData                            User data to associate with this physical body
 * @param[in]   _pstBodyDef                           Physical body definition
 * @return orxPHYSICS_BODY / orxNULL
 */
extern orxDLLAPI orxPHYSICS_BODY *orxFASTCALL         orxPhysics_CreateBody(const orxHANDLE _hUserData, const orxBODY_DEF *_pstBodyDef);

/** Deletes a physical body
 * @param[in]   _pstBody                              Concerned physical body
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_DeleteBody(orxPHYSICS_BODY *_pstBody);

/** Creates a part for a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pstBodyPartDef                       Physical body part definition
 * @return orxPHYSICS_BODY_PART / orxNULL
 */
extern orxDLLAPI orxPHYSICS_BODY_PART *orxFASTCALL    orxPhysics_CreateBodyPart(orxPHYSICS_BODY *_pstBody, const orxBODY_PART_DEF *_pstBodyPartDef);

/** Deletes a physical body part
 * @param[in]   _pstBodyPart                          Concerned physical body part
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart);


/** Sets the position of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvPosition                           Position to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetPosition(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvPosition);

/** Sets the rotation of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _fRotation                            Rotation (radians) to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation);

/** Sets the speed of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvSpeed                              Speed to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetSpeed(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvSpeed);

/** Sets the angular velocity of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _fVelocity                            Angular velocity (radians/seconds) to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity);

/** Sets the custom gravity of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvCustomGravity                      Custom gravity multiplier to set / orxNULL to remove it
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetCustomGravity(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvCustomGravity);

/** Gets the position of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[out]  _pvPosition                           Position to get
 * @return Position of the physical body
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetPosition(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition);

/** Gets the rotation of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @return Rotation (radians) of the physical body
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxPhysics_GetRotation(const orxPHYSICS_BODY *_pstBody);

/** Gets the speed of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[out]  _pvSpeed                              Speed to get
 * @return Speed of the physical body
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetSpeed(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed);

/** Gets the angular velocity of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @return Angular velocity (radians/seconds) of the physical body
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxPhysics_GetAngularVelocity(const orxPHYSICS_BODY *_pstBody);

/** Gets the custom gravity of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[out]  _pvCustomGravity                      Custom gravity to get
 * @return      Physical body custom gravity / orxNULL is object doesn't have any
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity);

/** Gets the center of mass of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[out]  _pvMassCenter                         Center of mass to get
 * @return Center of mass of the physical body
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter);


/** Applies a torque to a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _fTorque                              Torque to apply
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque);

/** Applies a force to a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvForce                              Force to apply
 * @param[in]   _pvPoint                              Point of application (if null, center of mass will be used)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_ApplyForce(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint);

/** Applies an impulse to a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvImpulse                            Impulse to apply
 * @param[in]   _pvPoint                              Point of application (if null, center of mass will be used)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_ApplyImpulse(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint);


/** Issues a raycast to test for potential physics bodies in the way
 * @param[in]   _pvStart                              Start of raycast
 * @param[in]   _pvEnd                                End of raycast
 * @param[in]   _u16SelfFlags                         Selfs flags used for filtering (0xFFFF for no filtering)
 * @param[in]   _u16CheckMask                         Check mask used for filtering (0xFFFF for no filtering)
 * @param[in]   _pvContact                            If non-null and a contact is found it will be stored here
 * @param[in]   _pvNormal                             If non-null and a contact is found, its normal will be stored here
 * @return Colliding body's user data / orxHANDLE_UNDEFINED
 */
extern orxDLLAPI orxHANDLE orxFASTCALL                orxPhysics_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxVECTOR *_pvContact, orxVECTOR *_pvNormal);


/** Enables/disables physics simulation
 * @param[in]   _bEnable                              Enable / disable
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_EnableSimulation(orxBOOL _bEnable);

#endif /* _orxPHYSICS_H_ */

/** @} */

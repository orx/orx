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
#define orxBODY_DEF_KU32_FLAG_NONE                    0x00000000  /**< No flags */

#define orxBODY_DEF_KU32_FLAG_2D                      0x00000001  /**< 2D type body def flag */
#define orxBODY_DEF_KU32_FLAG_DYNAMIC                 0x00000002  /**< Dynamic type body def flag */
#define orxBODY_DEF_KU32_FLAG_HIGH_SPEED              0x00000004  /**< High speed type body def flag */
#define orxBODY_DEF_KU32_FLAG_FIXED_ROTATION          0x00000008  /**< Body can't be rotated by physics */
#define orxBODY_DEF_KU32_FLAG_CAN_SLIDE               0x00000010  /**< Body is allowed to slide */
#define orxBODY_DEF_KU32_FLAG_CAN_MOVE                0x00000020  /**< Static body is allowed to move by user direct access */
#define orxBODY_DEF_KU32_FLAG_ALLOW_SLEEP             0x00000040  /**< Allow sleep body def flag */

#define orxBODY_DEF_KU32_MASK_ALL                     0xFFFFFFFF  /**< Body def all mask */

/** Body part definition flags
 */
#define orxBODY_PART_DEF_KU32_FLAG_NONE               0x00000000  /**< No flags */

#define orxBODY_PART_DEF_KU32_FLAG_SPHERE             0x00000001  /**< Sphere body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_BOX                0x00000002  /**< Box body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_MESH               0x00000004  /**< Mesh body part def flag */

#define orxBODY_PART_DEF_KU32_MASK_TYPE               0x00000007 /**< Type body part def mask */

#define orxBODY_PART_DEF_KU32_FLAG_SOLID              0x10000000  /**< Solid body part def flag */

#define orxBODY_PART_DEF_KU32_MASK_ALL                0xFFFFFFFF  /**< Body part def all mask */

/** Body joint definition flags
 */
#define orxBODY_JOINT_DEF_KU32_FLAG_NONE              0x00000000  /**< No flags */

#define orxBODY_JOINT_DEF_KU32_FLAG_REVOLUTE          0x00000001  /**< Revolute body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_PRISMATIC         0x00000002  /**< Prismatic body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_SPRING            0x00000004  /**< Spring body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_ROPE              0x00000008  /**< Rope body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_PULLEY            0x00000010  /**< Pulley body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_SUSPENSION        0x00000020  /**< Suspension body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_WELD              0x00000040  /**< Weld body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_FRICTION          0x00000080  /**< Friction body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_GEAR              0x00000100  /**< Gear body joint def flag */

#define orxBODY_JOINT_DEF_KU32_MASK_TYPE              0x000000FF  /**< Type body joint def mask */

#define orxBODY_JOINT_DEF_KU32_FLAG_COLLIDE           0x10000000  /**< Collide body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_ROTATION_LIMIT    0x20000000  /**< Rotation  limit body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_MOTOR             0x40000000  /**< Motor body joint def flag */
#define orxBODY_JOINT_DEF_KU32_FLAG_TRANSLATION_LIMIT 0x80000000  /**< Translation limit def flag */

#define orxBODY_JOINT_DEF_KU32_MASK_ALL               0xFFFFFFFF  /**< Body joint def all mask */


/** Misc defines
 */
#define orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER      8


/** Body definition
 */
typedef struct __orxBODY_DEF_t
{
  orxVECTOR vPosition;                                /**< Position : 12 */
  orxFLOAT  fRotation;                                /**< Rotation : 16 */
  orxFLOAT  fInertia;                                 /**< Inertia : 20 */
  orxFLOAT  fMass;                                    /**< Mass : 24 */
  orxFLOAT  fLinearDamping;                           /**< Linear damping : 28 */
  orxFLOAT  fAngularDamping;                          /**< Angular damping : 32 */
  orxU32    u32Flags;                                 /**< Control flags : 36 */

} orxBODY_DEF;

/** Part definition
 */
typedef struct __orxBODY_PART_DEF_t
{
  orxVECTOR vScale;                                   /**< Scale : 12 */
  orxFLOAT  fFriction;                                /**< Friction : 16 */
  orxFLOAT  fRestitution;                             /**< Restitution : 20 */
  orxFLOAT  fDensity;                                 /**< Density : 24 */
  orxU16    u16SelfFlags;                             /**< Self defining flags : 26 */
  orxU16    u16CheckMask;                             /**< Check mask : 28 */
  orxU32    u32Flags;                                 /**< Control flags : 32 */

  union
  {
    struct
    {
      orxVECTOR vCenter;                              /**< Sphere center : 44 */
      orxFLOAT  fRadius;                              /**< Sphere radius : 48 */
    } stSphere;                                       /**< Sphere : 48 */

    struct
    {
      orxAABOX  stBox;                                /**< Axis aligned Box : 56 */
    } stAABox;                                        /**< Box : 56 */

    struct
    {
      orxU32    u32VertexCounter;                     /**< Mesh vertex counter : 36 */
      orxVECTOR avVertices[orxBODY_PART_DEF_KU32_MESH_VERTEX_NUMBER]; /**< Mesh vertices : 132 */

    } stMesh;

  };                                                  /**< Part : 132 */

} orxBODY_PART_DEF;

/** Joint definition
 */
typedef struct __orxBODY_JOINT_DEF_t
{
  orxVECTOR     vSrcScale;                            /**< Source scale : 12 */
  orxVECTOR     vDstScale;                            /**< Destination scale : 24 */
  orxVECTOR     vSrcAnchor;                           /**< Source body anchor : 36 */
  orxVECTOR     vDstAnchor;                           /**< Destination body anchor : 48 */
  orxU32        u32Flags;                             /**< Control flags : 52 */

  union
  {
    struct
    {
      orxFLOAT  fDefaultRotation;                     /**< Default rotation : 56 */
      orxFLOAT  fMinRotation;                         /**< Min rotation : 60 */
      orxFLOAT  fMaxRotation;                         /**< Max rotation : 64 */
      orxFLOAT  fMotorSpeed;                          /**< Motor speed : 68 */
      orxFLOAT  fMaxMotorTorque;                      /**< Max motor torque : 72 */

    } stRevolute;                                     /**< Revolute : 72 */

    struct
    {
      orxFLOAT  fDefaultRotation;                     /**< Default rotation : 56 */
      orxVECTOR vTranslationAxis;                     /**< Translation axis : 68 */
      orxFLOAT  fMinTranslation;                      /**< Min translation : 72 */
      orxFLOAT  fMaxTranslation;                      /**< Max translation : 76 */
      orxFLOAT  fMotorSpeed;                          /**< Motor speed : 80 */
      orxFLOAT  fMaxMotorForce;                       /**< Max motor force : 84 */

    } stPrismatic;                                    /**< Prismatic : 84 */

    struct
    {
      orxFLOAT  fLength;                              /**< Length : 56 */
      orxFLOAT  fFrequency;                           /**< Frequency : 60 */
      orxFLOAT  fDamping;                             /**< Damping : 64 */

    } stSpring;                                       /**< Spring : 64 */

    struct
    {
      orxFLOAT  fLength;                              /**< Length : 56 */

    } stRope;                                         /**< Rope : 56 */

    struct
    {
      orxFLOAT  fLengthRatio;                         /**< Length ratio : 56 */
      orxVECTOR vSrcGroundAnchor;                     /**< Source ground anchor : 68 */
      orxVECTOR vDstGroundAnchor;                     /**< Destination ground anchor : 80 */
      orxFLOAT  fSrcLength;                           /**< Source length : 84 */
      orxFLOAT  fMaxSrcLength;                        /**< Max source length : 88 */
      orxFLOAT  fDstLength;                           /**< Destination length : 92 */
      orxFLOAT  fMaxDstLength;                        /**< Max destination length : 96 */

    } stPulley;                                       /**< Pulley : 96 */

    struct
    {
      orxVECTOR vTranslationAxis;                     /**< Translation axis : 64 */
      orxFLOAT  fMinTranslation;                      /**< Min translation : 68 */
      orxFLOAT  fMaxTranslation;                      /**< Max translation : 72 */
      orxFLOAT  fMotorSpeed;                          /**< Motor speed : 76 */
      orxFLOAT  fMaxMotorForce;                       /**< Max motor force : 80 */

    } stSuspension;                                   /**< Suspension : 80 */

    struct
    {
      orxFLOAT  fDefaultRotation;                     /**< Default rotation : 56 */

    } stWeld;                                         /**< Weld : 56 */

    struct
    {
      orxFLOAT  fMaxForce;                            /**< Max force : 56 */
      orxFLOAT  fMaxTorque;                           /**< Max torque : 60 */

    } stFriction;                                     /**< Friction : 60 */

    struct
    {
      const orxSTRING zSrcJointName;                  /**< Source joint name : 56 */
      const orxSTRING zDstJointName;                  /**< Destination joint name : 60 */
      orxFLOAT        fJointRatio;                    /**< Joint ratio : 64 */

    } stGear;                                         /**< Gear : 64 */

  };                                                  /**< Joint : 96 */

} orxBODY_JOINT_DEF;


/** Event enum
 */
typedef enum __orxPHYSICS_EVENT_t
{
  orxPHYSICS_EVENT_CONTACT_ADD = 0,
  orxPHYSICS_EVENT_CONTACT_REMOVE,

  orxPHYSICS_EVENT_NUMBER,

  orxPHYSICS_EVENT_NONE = orxENUM_NONE

} orxPHYSICS_EVENT;

/** Event payload
 */
typedef struct __orxPHYSICS_EVENT_PAYLOAD_t
{
  orxVECTOR       vPosition;                          /**< Contact position : 12 */
  orxVECTOR       vNormal;                            /**< Contact normal : 24 */
  const orxSTRING zSenderPartName;                    /**< Sender part name : 28 */
  const orxSTRING zRecipientPartName;                 /**< Recipient part name : 32 */

} orxPHYSICS_EVENT_PAYLOAD;

/** Internal physics body structure
 */
typedef struct __orxPHYSICS_BODY_t                    orxPHYSICS_BODY;

/** Internal physics part structure
 */
typedef struct __orxPHYSICS_BODY_PART_t               orxPHYSICS_BODY_PART;

/** Internal physics joint structure
 */
typedef struct __orxPHYSICS_BODY_JOINT_t              orxPHYSICS_BODY_JOINT;


/** Config defines
 */
#define orxPHYSICS_KZ_CONFIG_SECTION                  "Physics"
#define orxPHYSICS_KZ_CONFIG_GRAVITY                  "Gravity"
#define orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP              "AllowSleep"
#define orxPHYSICS_KZ_CONFIG_ITERATIONS               "IterationsPerStep"
#define orxPHYSICS_KZ_CONFIG_RATIO                    "DimensionRatio"
#define orxPHYSICS_KZ_CONFIG_SHOW_DEBUG               "ShowDebug"
#define orxPHYSICS_KZ_CONFIG_COLLISION_FLAG_LIST      "CollisionFlagList"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Physics module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_Setup();


/** Gets collision flag literal name
 * @param[in] _u32Flag      Concerned collision flag numerical value
 * @return Flag's name
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxPhysics_GetCollisionFlagName(orxU32 _u32Flag);

/** Gets collision flag numerical value
 * @param[in] _zFlag        Concerned collision flag literal name
 * @return Flag's value
 */
extern orxDLLAPI orxU32 orxFASTCALL                   orxPhysics_GetCollisionFlagValue(const orxSTRING _zFlag);


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
 * @param[in]   _hUserData                            User data to associate with this physical body part
 * @param[in]   _pstBodyPartDef                       Physical body part definition
 * @return orxPHYSICS_BODY_PART / orxNULL
 */
extern orxDLLAPI orxPHYSICS_BODY_PART *orxFASTCALL    orxPhysics_CreateBodyPart(orxPHYSICS_BODY *_pstBody, const orxHANDLE _hUserData, const orxBODY_PART_DEF *_pstBodyPartDef);

/** Deletes a physical body part
 * @param[in]   _pstBodyPart                          Concerned physical body part
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart);


/** Creates a joint to link two physical bodies together
 * @param[in]   _pstSrcBody                           Concerned source body
 * @param[in]   _pstDstBody                           Concerned destination body
 * @param[in]   _hUserData                            User data to associate with this physical body part
 * @param[in]   _pstBodyJointDef                      Physical body joint definition
 * @return orxPHYSICS_BODY_JOINT / orxNULL
 */
extern orxDLLAPI orxPHYSICS_BODY_JOINT *orxFASTCALL   orxPhysics_CreateBodyJoint(orxPHYSICS_BODY *_pstSrcBody, orxPHYSICS_BODY *_pstDstBody, const orxHANDLE _hUserData, const orxBODY_JOINT_DEF *_pstBodyJointDef);

/** Deletes a physical body joint
 * @param[in]   _pstBodyJoint                         Concerned physical body joint
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_DeleteBodyJoint(orxPHYSICS_BODY_JOINT *_pstBodyJoint);


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
 * @return Physical body custom gravity / orxNULL is object doesn't have any
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetCustomGravity(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvCustomGravity);

/** Gets the mass of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @return Mass of the physical body
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxPhysics_GetMass(const orxPHYSICS_BODY *_pstBody);

/** Gets the center of mass of a physical body (object space but scale isn't accounted for)
 * @param[in]   _pstBody                              Concerned physical body
 * @param[out]  _pvMassCenter                         Center of mass to get
 * @return Center of mass of the physical body
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxPhysics_GetMassCenter(const orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter);


/** Sets linear damping of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _fDamping                             Linear damping to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetLinearDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping);

/** Sets angular damping of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _fDamping                             Angular damping to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetAngularDamping(orxPHYSICS_BODY *_pstBody, orxFLOAT _fDamping);

/** Gets linear damping of a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @return Linear damping of the physical body
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxPhysics_GetLinearDamping(const orxPHYSICS_BODY *_pstBody);

/** Gets angular damping of a physical body
 * @param[in]   _pstBody        Concerned body
 * @return Angular damping of the physical body
 */
extern orxDLLAPI orxFLOAT orxFASTCALL                 orxPhysics_GetAngularDamping(const orxPHYSICS_BODY *_pstBody);


/** Applies a torque to a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _fTorque                              Torque to apply
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque);

/** Applies a force to a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvForce                              Force to apply
 * @param[in]   _pvPoint                              Point of application (world coordinates) (if null, center of mass will be used)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_ApplyForce(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvForce, const orxVECTOR *_pvPoint);

/** Applies an impulse to a physical body
 * @param[in]   _pstBody                              Concerned physical body
 * @param[in]   _pvImpulse                            Impulse to apply
 * @param[in]   _pvPoint                              Point of application (world coordinates) (if null, center of mass will be used)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_ApplyImpulse(orxPHYSICS_BODY *_pstBody, const orxVECTOR *_pvImpulse, const orxVECTOR *_pvPoint);


/** Sets self flags of a physical body part
 * @param[in]   _pstBodyPart                          Concerned physical body part
 * @param[in]   _u16SelfFlags                         Self flags to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetPartSelfFlags(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16SelfFlags);

/** Sets check mask of a physical body part
 * @param[in]   _pstBodyPart                          Concerned physical body part
 * @param[in]   _u16CheckMask                         Check mask to set
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetPartCheckMask(orxPHYSICS_BODY_PART *_pstBodyPart, orxU16 _u16CheckMask);

/** Gets self flags of a physical body part
 * @param[in]   _pstBodyPart                          Concerned physical body part
 * @return Self flags of the physical body part
 */
extern orxDLLAPI orxU16 orxFASTCALL                   orxPhysics_GetPartSelfFlags(const orxPHYSICS_BODY_PART *_pstBodyPart);

/** Gets check mask of a physical body part
 * @param[in]   _pstBodyPart                          Concerned physical body part
 * @return Check mask of the physical body part
 */
extern orxDLLAPI orxU16 orxFASTCALL                   orxPhysics_GetPartCheckMask(const orxPHYSICS_BODY_PART *_pstBodyPart);

/** Is a physical body part solid?
 * @param[in]   _pstBodyPart    Concerned body part
 * @return      orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxPhysics_IsPartSolid(const orxPHYSICS_BODY_PART *_pstBodyPart);

/** Sets a physical body part solid
 * @param[in]   _pstBodyPart    Concerned body part
 * @param[in]   _bSolid         Solid or sensor?
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxPhysics_SetPartSolid(orxPHYSICS_BODY_PART *_pstBodyPart, orxBOOL _bSolid);


/** Enables a (revolute) body joint motor
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @param[in]   _bEnable        Enable / Disable
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_EnableMotor(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxBOOL _bEnable);

/** Sets a (revolute) body joint motor speed
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @param[in]   _fSpeed         Speed
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_SetJointMotorSpeed(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fSpeed);

/** Sets a (revolute) body joint maximum motor torque
 * @param[in]   _pstBodyJoint   Concerned body joint
 * @param[in]   _fMaxTorque     Maximum motor torque
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_SetJointMaxMotorTorque(orxPHYSICS_BODY_JOINT *_pstBodyJoint, orxFLOAT _fMaxTorque);


/** Issues a raycast to test for potential physics bodies in the way
 * @param[in]   _pvStart                              Start of raycast
 * @param[in]   _pvEnd                                End of raycast
 * @param[in]   _u16SelfFlags                         Selfs flags used for filtering (0xFFFF for no filtering)
 * @param[in]   _u16CheckMask                         Check mask used for filtering (0xFFFF for no filtering)
 * @param[in]   _bEarlyExit     Should stop as soon as an object has been hit (which might not be the closest)
 * @param[in]   _pvContact                            If non-null and a contact is found it will be stored here
 * @param[in]   _pvNormal                             If non-null and a contact is found, its normal will be stored here
 * @return Colliding body's user data / orxHANDLE_UNDEFINED
 */
extern orxDLLAPI orxHANDLE orxFASTCALL                orxPhysics_Raycast(const orxVECTOR *_pvStart, const orxVECTOR *_pvEnd, orxU16 _u16SelfFlags, orxU16 _u16CheckMask, orxBOOL _bEarlyExit, orxVECTOR *_pvContact, orxVECTOR *_pvNormal);


/** Enables/disables physics simulation
 * @param[in]   _bEnable                              Enable / disable
 */
extern orxDLLAPI void orxFASTCALL                     orxPhysics_EnableSimulation(orxBOOL _bEnable);

#endif /* _orxPHYSICS_H_ */

/** @} */

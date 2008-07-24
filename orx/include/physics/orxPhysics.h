/** 
 * \file orxPhysics.h
 * 
 * Physics module
 * Handles physics & collision requests.
 * 
 * \todo
 */


/***************************************************************************
 orxPhysics.h
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


#ifndef _orxPHYSICS_H_
#define _orxPHYSICS_H_


#include "orxInclude.h"
#include "plugin/orxPluginCore.h"
#include "math/orxVector.h"


/** Body definition flags
 */
#define orxBODY_DEF_KU32_FLAG_NONE          0x00000000  /**< No flags */

#define orxBODY_DEF_KU32_FLAG_2D            0x00000001  /**< 2D type body def flag */
#define orxBODY_DEF_KU32_FLAG_DYNAMIC       0x00000002  /**< Dynamic type body def flag */
#define orxBODY_DEF_KU32_FLAG_HIGH_SPEED    0x00000004  /**< High speed type body def flag */
#define orxBODY_DEF_KU32_FLAG_FIXED_ROTATION 0x00000008 /**< Body can't be rotated by physics */

#define orxBODY_DEF_KU32_MASK_ALL           0xFFFFFFFF  /**< Body def all mask */

/** Body part definition flags
 */
#define orxBODY_PART_DEF_KU32_FLAG_NONE     0x00000000  /**< No flags */

#define orxBODY_PART_DEF_KU32_FLAG_BOX      0x00000001  /**< Box body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_SPHERE   0x00000002  /**< Sphere body part def flag */
#define orxBODY_PART_DEF_KU32_FLAG_SOLID    0x00000010  /**< Solid body part def flag */

#define orxBODY_PART_DEF_KU32_MASK_ALL      0xFFFFFFFF  /**< Body part def all mask */

/** Body definition
 */
typedef struct __orxBODY_DEF_t
{
  orxVECTOR vPosition;                      /**< Position */
  orxFLOAT  fRotation;                      /**< Rotation */
  orxFLOAT  fInertia;                       /**< Inertia */
  orxFLOAT  fMass;                          /**< Mass */
  orxFLOAT  fLinearDamping;                 /**< Linear damping */
  orxFLOAT  fAngularDamping;                /**< Angular damping */
  orxU32    u32Flags;                       /**< Control flags */

} orxBODY_DEF;

/** Shape definition
 */
typedef struct __orxBODY_PART_DEF_t
{
  orxFLOAT  fFriction;                      /**< Friction */
  orxFLOAT  fRestitution;                   /**< Restitution */
  orxFLOAT  fDensity;                       /**< Density */
  orxU16    u16SelfFlags;                   /**< Self defining flags */
  orxU16    u16CheckMask;                   /**< Check mask */
  orxU32    u32Flags;                       /**< Control flags */

  union
  {
    struct
    {
      orxVECTOR vCenter;                    /**< Sphere center */
      orxFLOAT  fRadius;                    /**< Sphere radius */
    } stSphere;

    struct
    {
      orxAABOX  stBox;                      /**< Axis aligned Box */
    } stAABox;
  };

} orxBODY_PART_DEF;

/** Event enum
 */
typedef enum __orxPHYSICS_EVENT_t
{
  orxPHYSICS_EVENT_CONTACT_ADD = 0,
  orxPHYSICS_EVENT_CONTACT_PERSIST,
  orxPHYSICS_EVENT_CONTACT_REMOVE,
  orxPHYSICS_EVENT_OUT_OF_WORLD,

  orxPHYSICS_EVENT_NUMBER,

  orxPHYSICS_EVENT_NONE = orxENUM_NONE

} orxPHYSICS_EVENT;

/** Contact event payload
 */
typedef struct __orxPHYSICS_CONTACT_EVENT_PAYLOAD_t
{
  orxVECTOR vPosition;                    /**< Contact position: 12 */
  orxVECTOR vNormal;                      /**< Contact normal: 24 */
  orxFLOAT  fPenetration;                 /**< Penetration: 28 */
  orxU32    u32SourcePartIndex;           /**< Source shape index: 32 */
  orxU32    u32DestinationPartIndex;      /**< Destination shape index: 36 */

} orxPHYSICS_CONTACT_EVENT_PAYLOAD;

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

/** Physics module setup */
extern orxDLLAPI orxVOID            orxPhysics_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

extern orxDLLAPI orxSTATUS orxPhysics_Init();

extern orxDLLAPI orxVOID orxPhysics_Exit();

extern orxDLLAPI orxSTATUS orxPhysics_SetGravity(orxCONST orxVECTOR *_pvGravity);

extern orxDLLAPI orxPHYSICS_BODY *orxPhysics_CreateBody(orxCONST orxHANDLE _hUserData, orxCONST orxBODY_DEF *_pstBodyDef);

extern orxDLLAPI orxVOID orxPhysics_DeleteBody(orxPHYSICS_BODY *_pstBody);

extern orxDLLAPI orxPHYSICS_BODY_PART *orxPhysics_CreateBodyPart(orxPHYSICS_BODY *_pstBody, orxCONST orxBODY_PART_DEF *_pstBodyPartDef);

extern orxDLLAPI orxVOID orxPhysics_DeleteBodyPart(orxPHYSICS_BODY_PART *_pstBodyPart);

extern orxDLLAPI orxSTATUS orxPhysics_SetPosition(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvPosition);

extern orxDLLAPI orxSTATUS orxPhysics_SetRotation(orxPHYSICS_BODY *_pstBody, orxFLOAT _fRotation);

extern orxDLLAPI orxSTATUS orxPhysics_SetSpeed(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvSpeed);

extern orxDLLAPI orxSTATUS orxPhysics_SetAngularVelocity(orxPHYSICS_BODY *_pstBody, orxFLOAT _fVelocity);

extern orxDLLAPI orxVECTOR *orxPhysics_GetPosition(orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvPosition);

extern orxDLLAPI orxFLOAT orxPhysics_GetRotation(orxPHYSICS_BODY *_pstBody);

extern orxDLLAPI orxVECTOR *orxPhysics_GetSpeed(orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvSpeed);

extern orxDLLAPI orxFLOAT orxPhysics_GetAngularVelocity(orxPHYSICS_BODY *_pstBody);

extern orxDLLAPI orxVECTOR *orxPhysics_GetMassCenter(orxPHYSICS_BODY *_pstBody, orxVECTOR *_pvMassCenter);

extern orxDLLAPI orxSTATUS orxPhysics_ApplyTorque(orxPHYSICS_BODY *_pstBody, orxFLOAT _fTorque);

extern orxDLLAPI orxSTATUS orxPhysics_ApplyForce(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvForce, orxCONST orxVECTOR *_pvPoint);

extern orxDLLAPI orxSTATUS orxPhysics_ApplyImpulse(orxPHYSICS_BODY *_pstBody, orxCONST orxVECTOR *_pvImpulse, orxCONST orxVECTOR *_pvPoint);

#endif /* _orxPHYSICS_H_ */

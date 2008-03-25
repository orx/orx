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


/** Physics body flags
 */
#define orxBODY_DEF_KU32_FLAG_NONE          0x00000000  /**< No flags */

#define orxBODY_DEF_KU32_FLAG_2D            0x00000001  /**< 2D type body flag */
#define orxBODY_DEF_KU32_FLAG_DYNAMIC       0x00000002  /**< Dynamic type body flag */
#define orxBODY_DEF_KU32_FLAG_HIGH_SPEED    0x00000004  /**< High speed type body flag */
#define orxBODY_DEF_KU32_FLAG_NO_ROTATION   0x00000008  /**< Body can't be rotated by physics */

#define orxBODY_DEF_KU32_MASK_ALL           0xFFFFFFFF  /**< Body def all mask */

/** Body definition
 */
typedef struct __orxBODY_DEF_t
{
  orxVECTOR vPosition;                      /**< Position */
  orxFLOAT  fAngle;                         /**< Angle */
  orxFLOAT  fLinearDamping;                 /**< Linear damping */
  orxFLOAT  fAngularDamping;                /**< Angular damping */
  orxU32    u32Flags;                       /**< Control flags */

} orxBODY_DEF;

/** Shape definition
 */
typedef struct __orxVODY_PART_DEF_t
{
  //! TODO

} orxBODY_PART_DEF;


/** Internal physics body structure
 */
typedef struct __orxPHYSICS_BODY_t        orxPHYSICS_BODY;

/** Internal physics shape structure
 */
typedef struct __orxPHYSICS_BODY_PART_t   orxPHYSICS_BODY_PART;


#define orxPHYSICS_KZ_CONFIG_SECTION      "Physics"
#define orxPHYSICS_KZ_CONFIG_GRAVITY      "Gravity"
#define orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP  "AllowSleep"
#define orxPHYSICS_KZ_CONFIG_WORLD_LOWER  "WorldLowerBound"
#define orxPHYSICS_KZ_CONFIG_WORLD_UPPER  "WorldUpperBound"
#define orxPHYSICS_KZ_CONFIG_ITERATIONS   "IterationsPerStep"
#define orxPHYSICS_KZ_CONFIG_FREQUENCY    "SimulationFrequency"
#define orxPHYSICS_KZ_CONFIG_RATIO        "DimensionRatio"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Physics module setup */
extern orxDLLAPI orxVOID            orxPhysics_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxPhysics_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPhysics_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPhysics_CreateBody, orxPHYSICS_BODY *, orxCONST orxBODY_DEF *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPhysics_DeleteBody, orxVOID, orxPHYSICS_BODY *);


orxSTATIC orxINLINE orxSTATUS orxPhysics_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Init)();
}

orxSTATIC orxINLINE orxVOID orxPhysics_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Exit)();
}

orxSTATIC orxINLINE orxPHYSICS_BODY *orxPhysics_CreateBody(orxCONST orxBODY_DEF *_pstBodyDef)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_CreateBody)(_pstBodyDef);
}

orxSTATIC orxINLINE orxVOID orxPhysics_DeleteBody(orxPHYSICS_BODY *_pstBody)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_DeleteBody)(_pstBody);
}


#endif /* _orxPHYSICS_H_ */

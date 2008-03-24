/**
 * @file orxPhysics.cpp
 * 
 * Box2D physics plugin
 * 
 */
 
 /***************************************************************************
 orxPhysics.cpp
 Box2D physics plugin
 
 begin                : 24/03/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

extern "C"
{
  #include "orxInclude.h"

  #include "core/orxConfig.h"
  #include "plugin/orxPluginUser.h"

  #include "physics/orxPhysics.h"
}

#include <Box2D/Box2D.h>


/** Module flags
 */
#define orxPHYSICS_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxPHYSICS_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxPHYSICS_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */


orxSTATIC orxCONST orxU32 su32DefaultIterations = 10;


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxPHYSICS_STATIC_t
{
  orxU32            u32Flags;                   /**< Control flags */

  orxU32            u32Iterations;              /**< Simulation iterations per step */
  
  b2World          *poWorld;                    /**< World */
  b2Body           *poGround;                   /**< Ground */

} orxPHYSICS_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxPHYSICS_STATIC sstPhysics;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

extern "C" orxSTATUS orxPhysics_Box2D_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY))
  {
    orxBOOL   bAllowSleep;
    orxVECTOR vGravity, vLower, vUpper;
    b2AABB    stWorldAABB;
    b2Vec2    vWorldGravity;

    /* Cleans static controller */
    orxMemory_Set(&sstPhysics, 0, sizeof(orxPHYSICS_STATIC));

    /* Gets gravity & allow sleep from config */
    orxConfig_SelectSection(orxPHYSICS_KZ_CONFIG_SECTION);
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_GRAVITY, &vGravity);
    bAllowSleep = orxConfig_GetBool(orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP);

    /* Gets world corners from config */
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_WORLD_LOWER, &vLower);
    orxConfig_GetVector(orxPHYSICS_KZ_CONFIG_WORLD_UPPER, &vUpper);

    /* Inits world AABB */
    stWorldAABB.lowerBound.Set(vLower.fX, vLower.fY);
    stWorldAABB.upperBound.Set(vUpper.fX, vUpper.fY);

    /* Inits world gravity */
    vWorldGravity.Set(vGravity.fX, vGravity.fY);

    /* Creates world */
    sstPhysics.poWorld = new b2World(stWorldAABB, vWorldGravity, bAllowSleep);

    /* Success? */
    if(sstPhysics.poWorld != orxNULL)
    {
      b2BodyDef stGroundDef;

      /* Inits ground definition */
      stGroundDef.position.SetZero();

      /* Creates ground */
      sstPhysics.poGround = sstPhysics.poWorld->CreateStaticBody(&stGroundDef);

      /* Success? */
      if(sstPhysics.poGround != orxNULL)
      {
        orxS32 s32IterationsPerStep;

        /* Gets iteration per step number from config */
        orxConfig_GetS32(orxPHYSICS_KZ_CONFIG_ITERATIONS);

        /* Valid? */
        if(s32IterationsPerStep > 0)
        {
          /* Stores it */
          sstPhysics.u32Iterations = (orxU32)s32IterationsPerStep;
        }
        else
        {
          /* Uses default value */
          sstPhysics.u32Iterations = su32DefaultIterations;
        }

        /* Updates status */
        sstPhysics.u32Flags |= orxPHYSICS_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;  
}

extern "C" orxVOID orxPhysics_Box2D_Exit()
{
  /* Was initialized? */
  if(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY)
  {
    /* Deletes ground */
    sstPhysics.poWorld->DestroyBody(sstPhysics.poGround);

    /* Deletes world */
    delete sstPhysics.poWorld;

    /* Cleans static controller */
    orxMemory_Set(&sstPhysics, 0, sizeof(orxPHYSICS_STATIC));
  }

  return;
}


/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_START(PHYSICS);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Init, PHYSICS, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxPhysics_Box2D_Exit, PHYSICS, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_END();

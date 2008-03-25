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
  #include "core/orxClock.h" 
  #include "plugin/orxPluginUser.h"

  #include "physics/orxPhysics.h"
}

#include <Box2D/Box2D.h>


/** Module flags
 */
#define orxPHYSICS_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxPHYSICS_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxPHYSICS_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */


orxSTATIC orxCONST orxU32   su32DefaultIterations   = 10;
orxSTATIC orxCONST orxFLOAT sfDefaultFrequency      = orx2F(60.0f);
orxSTATIC orxCONST orxFLOAT sfDefaultDimensionRatio = orx2F(0.1f);


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxPHYSICS_STATIC_t
{
  orxU32            u32Flags;                   /**< Control flags */
  orxU32            u32Iterations;              /**< Simulation iterations per step */
  orxFLOAT          fDimensionRatio;            /**< Dimension ratio */
  orxCLOCK         *pstClock;                   /**< Simulation clock */
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

/** Update (callback to register on a clock)
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pstContext     Context sent when registering callback to the clock
 */
orxVOID orxFASTCALL orxPhysics_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* Checks */
  orxASSERT(sstPhysics.u32Flags & orxPHYSICS_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Updates world simulation */
  sstPhysics.poWorld->Step(_pstClockInfo->fDT, (orxU32)_pstContext);

  return;
}

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
        orxFLOAT  fFrequency, fTickSize, fRatio;
        orxS32    s32IterationsPerStep;

        /* Gets dimension ratio */
        orxConfig_GetFloat(orxPHYSICS_KZ_CONFIG_RATIO);

        /* Valid? */
        if(fRatio > orxFLOAT_0)
        {
          /* Stores it */
          sstPhysics.fDimensionRatio = fRatio;
        }
        else
        {
          /* Stores default one */
          sstPhysics.fDimensionRatio = sfDefaultDimensionRatio;
        }

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

        /* Gets frequency */
        fFrequency = orxConfig_GetFloat(orxPHYSICS_KZ_CONFIG_FREQUENCY);

        /* Valid? */
        if(fFrequency <= orxFLOAT_0)
        {
          /* Gets tick size */
          fTickSize = orxFLOAT_1 / fFrequency;
        }
        else
        {
          /* Gets default tick size */
          fTickSize = orxFLOAT_1 / sfDefaultFrequency;
        }

        /* Creates physics clock */
        sstPhysics.pstClock = orxClock_Create(fTickSize, orxCLOCK_TYPE_PHYSICS);

        /* Valid? */
        if(sstPhysics.pstClock != orxNULL)
        {
          /* Registers rendering function */
          eResult = orxClock_Register(sstPhysics.pstClock, orxPhysics_Update, (orxVOID *)sstPhysics.u32Iterations, orxMODULE_ID_PHYSICS);

          /* Valid? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Updates status */
            sstPhysics.u32Flags |= orxPHYSICS_KU32_STATIC_FLAG_READY;
          }
          else
          {
            /* Deletes world */
            delete sstPhysics.poWorld;

            /* Updates result */
            eResult = orxSTATUS_FAILURE;
          }
        }
        else
        {
          /* Deletes world */
          delete sstPhysics.poWorld;

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Deletes world */
        delete sstPhysics.poWorld;

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

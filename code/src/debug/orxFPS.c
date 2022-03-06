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
 * @file orxFPS.c
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "debug/orxFPS.h"

#include "debug/orxDebug.h"
#include "core/orxClock.h"
#include "core/orxConfig.h"
#include "memory/orxMemory.h"
#include "object/orxStructure.h"


/** Module flags
 */
#define orxFPS_KU32_STATIC_FLAG_NONE        0x00000000

#define orxFPS_KU32_STATIC_FLAG_READY       0x00000001

#define orxFPS_KU32_STATIC_MASK_ALL         0xFFFFFFFF


/** Misc defines
 */
#define orxFPS_KU32_CLOCK_FREQUENCY         orxFLOAT_1


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxFPS_STATIC_t
{
  /* Associated clock */
  orxCLOCK *pstClock;

  /* Frame count */
  orxU32 u32FrameCount;

  /* FPS */
  orxU32 u32FPS;

  /* Control flags */
  orxU32 u32Flags;

} orxFPS_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFPS_STATIC sstFPS;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Updates FPS count
 * @param[in] _pstClockInfo       Clock information where this callback has been registered
 * @param[in] _pContext         User defined context
 */
static void orxFASTCALL orxFPS_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Gets FPS value */
  sstFPS.u32FPS = sstFPS.u32FrameCount;

  /* Resets frame count */
  sstFPS.u32FrameCount = 0;

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups FPS module */
void orxFASTCALL orxFPS_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_CONFIG);

  return;
}

/** Inits the FPS module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFPS_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstFPS, sizeof(orxFPS_STATIC));

    /* Creates clock */
    orxConfig_PushSection(orxFPS_KZ_CLOCK_NAME);
    orxConfig_SetFloat(orxCLOCK_KZ_CONFIG_FREQUENCY, orxFPS_KU32_CLOCK_FREQUENCY);
    orxConfig_PopSection();
    sstFPS.pstClock = orxClock_CreateFromConfig(orxFPS_KZ_CLOCK_NAME);

    /* Valid? */
    if(sstFPS.pstClock != orxNULL)
    {
      /* Sets it as its own owner */
      orxStructure_SetOwner(sstFPS.pstClock, sstFPS.pstClock);

      /* Registers callback */
      eResult = orxClock_Register(sstFPS.pstClock, orxFPS_Update, orxNULL, orxMODULE_ID_FPS, orxCLOCK_PRIORITY_NORMAL);

      /* Registered? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Inits Flags */
        sstFPS.u32Flags = orxFPS_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to register clock callback.");

        /* Deletes clock */
        orxClock_Delete(sstFPS.pstClock);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create clock object.");

      /* Not initialized */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize FPS module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the FPS module */
void orxFASTCALL orxFPS_Exit()
{
  /* Initialized? */
  if(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY)
  {
    /* Removes callback */
    orxClock_Unregister(sstFPS.pstClock, orxFPS_Update);

    /* Deletes clock */
    orxClock_Delete(sstFPS.pstClock);

    /* Updates flags */
    sstFPS.u32Flags &= ~orxFPS_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to exit FPS module when it wasn't initialized.");
  }

  return;
}

/** Increases internal frame count */
void orxFASTCALL orxFPS_IncreaseFrameCount()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Updates frame count */
  sstFPS.u32FrameCount++;

  return;
}

/** Gets current FTP value
 * @return orxU32
 */
orxU32 orxFASTCALL orxFPS_GetFPS()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Returns it */
  return sstFPS.u32FPS;
}

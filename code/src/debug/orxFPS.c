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
 * @file orxFPS.c
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "debug/orxFPS.h"

#include "debug/orxDebug.h"
#include "core/orxClock.h"
#include "memory/orxMemory.h"


/** Module flags
 */
#define orxFPS_KU32_STATIC_FLAG_NONE        0x00000000

#define orxFPS_KU32_STATIC_FLAG_READY       0x00000001

#define orxFPS_KU32_STATIC_MASK_ALL         0xFFFFFFFF


/** Misc defines
 */
#define orxFPS_KU32_CLOCK_TICKSIZE          orxFLOAT_1


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxFPS_STATIC_t
{
  /* Associated clock */
  orxCLOCK *pstClock;

  /* Frame counter */
  orxU32 u32FrameCounter;

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
orxSTATIC volatile orxFPS_STATIC sstFPS;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Updates FPS counter
 * @param[in] _pstClockInfo       Clock information where this callback has been registered
 * @param[in] _pstContext         User defined context
 */
orxSTATIC orxVOID orxFASTCALL orxFPS_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Gets FPS value */
  sstFPS.u32FPS = sstFPS.u32FrameCounter;

  /* Resets frame counter */
  sstFPS.u32FrameCounter = 0;

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Setups FPS module */
orxVOID orxFPS_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_CLOCK);

  return;
}

/** Inits the FPS module 
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFPS_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero((orxFPS_STATIC *)&sstFPS, sizeof(orxFPS_STATIC));

    /* Creates clock */
    sstFPS.pstClock = orxClock_Create(orxFPS_KU32_CLOCK_TICKSIZE, orxCLOCK_TYPE_SECOND);

    /* Valid? */
    if(sstFPS.pstClock != orxNULL)
    {
      /* Registers callback */
      eResult = orxClock_Register(sstFPS.pstClock, orxFPS_Update, orxNULL, orxMODULE_ID_FPS, orxCLOCK_FUNCTION_PRIORITY_NORMAL);

      /* Registered? */
      if(eResult == orxSTATUS_SUCCESS)
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
orxVOID orxFPS_Exit()
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

/** Increases internal frame counter */
orxVOID orxFPS_IncreaseFrameCounter()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Updates frame counter */
  sstFPS.u32FrameCounter++;

  return;
}

/** Gets current FTP value
 * @return orxU32
 */
orxU32 orxFPS_GetFPS()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Returns it */
  return sstFPS.u32FPS;
}

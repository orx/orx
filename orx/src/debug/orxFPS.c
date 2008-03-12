/***************************************************************************
 orxFPS.c
 FPS module
 
 begin                : 10/12/2003
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


#include "debug/orxFPS.h"

#include "debug/orxDebug.h"
#include "core/orxClock.h"
#include "memory/orxMemory.h"
#include "msg/msg_fps.h"

/*
 * Platform independant defines
 */

#define orxFPS_KU32_STATIC_FLAG_NONE        0x00000000
#define orxFPS_KU32_STATIC_FLAG_READY       0x00000001

#define orxFPS_KU32_CLOCK_TICKSIZE          orxFLOAT_1

/*
 * Static structure
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


/*
 * Static data
 */
orxSTATIC volatile orxFPS_STATIC sstFPS;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxFPS_Update
 Updates current FPS value.

 returns: orxVOID
 ***************************************************************************/
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
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxFPS_Setup
 FPS module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxFPS_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FPS, orxMODULE_ID_CLOCK);

  return;
}

/***************************************************************************
 orxFPS_Init
 Inits FPS system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFPS_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set((orxFPS_STATIC *)&sstFPS, 0, sizeof(orxFPS_STATIC));

    /* Creates clock */
    sstFPS.pstClock = orxClock_Create(orxFPS_KU32_CLOCK_TICKSIZE, orxCLOCK_TYPE_SECOND);

    /* Valid? */
    if(sstFPS.pstClock != orxNULL)
    {
      /* Registers callback */
      eResult = orxClock_Register(sstFPS.pstClock, orxFPS_Update, orxNULL, orxMODULE_ID_FPS);

      /* Registered? */
      if(eResult == orxSTATUS_SUCCESS)
      {
        /* Inits Flags */
        sstFPS.u32Flags = orxFPS_KU32_STATIC_FLAG_READY;
        
        /* Success */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* !!! MSG !!! */

        /* Deletes clock */
        orxClock_Delete(sstFPS.pstClock);
      }
    }
    else
    {
      /* !!! MSG !!! */

      /* Not initialized */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxFPS_Exit
 Exits from the FPS system.

 returns: orxVOID
 ***************************************************************************/
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
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 orxFPS_IncreaseFrameCounter
 Increases frame counter.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFPS_IncreaseFrameCounter()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Updates frame counter */
  sstFPS.u32FrameCounter++;

  return;
}

/***************************************************************************
 orxFPS_GetFPS
 Gets FPS value/

 returns: orxU32 FPS value
 ***************************************************************************/
orxU32 orxFPS_GetFPS()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_STATIC_FLAG_READY);

  /* Returns it */  
  return sstFPS.u32FPS;
}

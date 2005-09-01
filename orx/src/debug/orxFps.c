/***************************************************************************
 orxFps.c
 fps module
 
 begin                : 10/12/2003
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


#include "debug/orxFps.h"

#include "debug/orxDebug.h"
#include "core/orxClock.h"
#include "memory/orxMemory.h"
#include "msg/msg_fps.h"

/*
 * Platform independant defines
 */

#define orxFPS_KU32_FLAG_NONE               0x00000000
#define orxFPS_KU32_FLAG_READY              0x00000001

#define orxFPS_KU32_CLOCK_TICKSIZE          1000

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
orxSTATIC volatile orxFPS_STATIC sstFps;


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
orxSTATIC orxVOID orxFASTCALL orxFps_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* Checks */
  orxASSERT(sstFps.u32Flags & orxFPS_KU32_FLAG_READY);
  
  /* !!! Only to avoid warnings !!! */
  orxASSERT(_pstClockInfo != orxNULL);
  orxASSERT(_pstContext == NULL);

  /* Gets FPS value */
  sstFps.u32FPS = sstFps.u32FrameCounter;

  /* Resets frame counter */
  sstFps.u32FrameCounter = 0;

  return;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxFps_Init
 Inits fps system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxFps_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;

  /* Init dependencies */
  if ((orxMAIN_INIT_MODULE(Memory) == orxSTATUS_SUCCESS) &&
      (orxMAIN_INIT_MODULE(Clock)  == orxSTATUS_SUCCESS))
  {
    /* Not already Initialized? */
    if(!(sstFps.u32Flags & orxFPS_KU32_FLAG_READY))
    {
      /* Cleans control structure */
      orxMemory_Set((orxFPS_STATIC *)&sstFps, 0, sizeof(orxFPS_STATIC));
  
      /* Creates clock */
      sstFps.pstClock = orxClock_Create(orxFPS_KU32_CLOCK_TICKSIZE, orxCLOCK_TYPE_FPS);
  
      /* Valid? */
      if(sstFps.pstClock != orxNULL)
      {
        /* Registers callback */
        eResult = orxClock_Register(sstFps.pstClock, orxFps_Update, orxNULL);
  
        /* Registered? */
        if(eResult == orxSTATUS_SUCCESS)
        {
          /* Inits Flags */
          sstFps.u32Flags = orxFPS_KU32_FLAG_READY;
          
          /* Success */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* !!! MSG !!! */
  
          /* Deletes clock */
          orxClock_Delete(sstFps.pstClock);
        }
      }
      else
      {
        /* !!! MSG !!! */
  
        /* Not initialized */
        eResult = orxSTATUS_FAILED;
      }
    }
    else
    {
      /* !!! MSG !!! */
  
      /* Already initialized */
      eResult = orxSTATUS_FAILED;
    }
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxFps_Exit
 Exits from the fps system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFps_Exit()
{
  /* Initialized? */
  if(sstFps.u32Flags & orxFPS_KU32_FLAG_READY)
  {
    /* Removes callback */
    orxClock_Unregister(sstFps.pstClock, orxFps_Update);

    /* Deletes clock */
    orxClock_Delete(sstFps.pstClock);

    /* Updates flags */
    sstFps.u32Flags &= ~orxFPS_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Exit dependencies */  
  orxMAIN_EXIT_MODULE(Clock);
  orxMAIN_EXIT_MODULE(Memory);

  return;
}

/***************************************************************************
 orxFps_IncreaseFrameCounter
 Increases frame counter.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFps_IncreaseFrameCounter()
{
  /* Checks */
  orxASSERT(sstFps.u32Flags & orxFPS_KU32_FLAG_READY);

  /* Updates frame counter */
  sstFps.u32FrameCounter++;

  return;
}

/***************************************************************************
 orxFps_GetFPS
 Gets FPS value/

 returns: orxU32 FPS value
 ***************************************************************************/
orxU32 orxFps_GetFPS()
{
  /* Checks */
  orxASSERT(sstFps.u32Flags & orxFPS_KU32_FLAG_READY);

  /* Returns it */  
  return sstFps.u32FPS;
}

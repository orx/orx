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
#include "core/clock.h"
#include "memory/orxMemory.h"

/*
 * Platform independant defines
 */

#define orxFPS_KU32_FLAG_NONE             0x00000000
#define orxFPS_KU32_FLAG_READY            0x00000001

/*
 * Static structure
 */
typedef struct __orxFPS_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;

  /* Frame counter */
  orxU32 u32FrameCounter;

  /* FPS */
  orxU32 u32FPS;

} orxFPS_STATIC;


/*
 * Static data
 */
static volatile orxFPS_STATIC sstFPS;


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
static inline orxVOID orxFps_Update()
{
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
 orxFps_Init
 Inits fps system.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILED
 ***************************************************************************/
orxSTATUS orxFps_Init()
{
  /* Already Initialized? */
  if((sstFPS.u32Flags & orxFPS_KU32_FLAG_READY) != orxFPS_KU32_FLAG_NONE)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Registers fps callback */
  if(clock_cb_function_add(orxFps_Update, 1000) == orxFALSE)
  {
    orxDEBUG_LOG(orxDEBUG_LEVEL_TIMER, MSG_FPS_KZ_INIT_FAILED);
  }

  /* Cleans static controller */
  orxMemory_Set((orxVOID *)&sstFPS, 0, sizeof(orxFPS_STATIC));

  /* Inits Flags */
  sstFPS.u32Flags = orxFPS_KU32_FLAG_READY;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxFps_Exit
 Exits from the fps system.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFps_Exit()
{
  /* Not initialized? */
  if((sstFPS.u32Flags & orxFPS_KU32_FLAG_READY) == orxFPS_KU32_FLAG_NONE)
  {
    /* !!! MSG !!! */

    return;
  }

  /* Removes fps callback */
  clock_cb_function_remove(orxFps_Update, 1000);

  /* Updates flags */
  sstFPS.u32Flags &= ~orxFPS_KU32_FLAG_READY;

  return;
}

/***************************************************************************
 orxFps_IncreaseFrameCounter
 Increases frame counter.

 returns: orxVOID
 ***************************************************************************/
inline orxVOID orxFps_IncreaseFrameCounter()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_FLAG_READY);

  /* Updates frame counter */
  sstFPS.u32FrameCounter++;

  return;
}

/***************************************************************************
 orxFps_GetFPS
 Gets FPS value/

 returns: orxU32 FPS value
 ***************************************************************************/
inline orxU32 orxFps_GetFPS()
{
  /* Checks */
  orxASSERT(sstFPS.u32Flags & orxFPS_KU32_FLAG_READY);

  /* Returns it */  
  return sstFPS.u32FPS;
}

/**
 * @file core/orxEvent.c
 */

/***************************************************************************
 begin                : 01/09/2005
 author               : (C) Arcallians
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "core/orxEvent.h"
#include "memory/orxMemory.h"
#include "utils/orxQueue.h"

/*
 * Event module status flag definition.
 */
#define orxEVENT_KU32_FLAG_NONE              0x00000000
#define orxEVENT_KU32_FLAG_READY             0x00000001

/*
 * Event module state structure.
 */
typedef struct __orxEvent_STATIC_t
{
  /* Control flags */
  orxU32 u32Flags;
} orxEVENT_STATIC;


/*
 * Static data
 */
orxSTATIC orxEVENT_STATIC sstEvent;


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/
 
/** 
 * orxEvent_Init
 **/
orxSTATUS orxEvent_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED;
  
  /* Init dependencies */
  if ((orxMAIN_INIT_MODULE(Queue) == orxSTATUS_SUCCESS))
  {
    /* Not already initialized ? */
    if(!(sstEvent.u32Flags & orxEVENT_KU32_FLAG_READY))
    {
      /* Cleans control structure */
      orxMemory_Set(&sstEvent, 0, sizeof(orxEVENT_STATIC));
    
      /* Inits Flags */
      sstEvent.u32Flags = orxEVENT_KU32_FLAG_READY;
      
      /* Successfull Init */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */
    }
  }

  /* Done! */
  return eResult;
}

/**
 * orxEvent_Exit
 **/
orxVOID orxEvent_Exit()
{
  /* Initialized? */
  if(sstEvent.u32Flags & orxEVENT_KU32_FLAG_READY)
  {
    /* Updates flags */
    sstEvent.u32Flags &= ~orxEVENT_KU32_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Exit dependencies */
  orxMAIN_EXIT_MODULE(Queue);

  return;
}


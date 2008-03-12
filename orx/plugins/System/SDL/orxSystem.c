/**
 * \file orxSystem.c
 */

/***************************************************************************
 begin                : 30/07/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 email                : cursor@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/
 
#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "math/orxMath.h"
#include "memory/orxMemory.h"
#include "core/orxSystem.h"

#include <SDL/SDL.h>

#define orxSYSTEM_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxSYSTEM_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxSYSTEM_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the system plugin module */
} orxSYSTEM_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxSYSTEM_STATIC sstSystem;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Init the system module
 * @return Returns the status of the operation
 */
orxSTATUS orxSystemSDL_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS; /* Init result */

  /* Module not already initialized ? */
  if(!(sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstSystem, 0, sizeof(orxSYSTEM_STATIC));

    /* Hqs SDL_Init already been called ? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) == 0)
    {
      /* No, calls it */
      if(SDL_Init(0)==0)
      {
        /* Init system subsystem */
        if(SDL_InitSubSystem(SDL_INIT_SYSTEMR)==0)
        {
          /* Set module as ready */
          sstSystem.u32Flags = orxSYSTEM_KU32_STATIC_FLAG_READY;
          
          /* Successfull init */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }
  }
  
  /* Done */
  return eResult;
}

/** Exit the system module
 */
orxVOID orxSystemSDL_Exit()
{
  /* Module initialized ? */
  if((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Unitialize SDL System */
    SDL_QuitSubSystem(SDL_INIT_SYSTEMR);
    
    /* All subsystem uninitialized ? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) == 0)
    {
      /* Quit SDL */
      SDL_Quit();
    }
    
    /* Module not ready now */
    sstSystem.u32Flags = orxSYSTEM_KU32_STATIC_FLAG_NONE;
  }
}

/** Gets App Elapsed time.
 * @return Returns the amount of seconds elapsed from the application start.
 */
orxFLOAT orxSystemSDL_GetTime()
{
  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  return(orx2F(0.001f) * orxU2F(SDL_GetTicks()));
}

/** Delay the program for given number of milliseconds.
 * @param[in] _fSystem Number of seconds to wait.
 */
orxVOID orxSystemSDL_Delay(orxFLOAT _fSystem)
{
  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);
  
  SDL_Delay(orxF2U(orx2F(1000.0f) * _fSystem));
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystemSDL_Init, SYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystemSDL_Exit, SYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystemSDL_GetTime, SYSTEM, GET_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystemSDL_Delay, SYSTEM, DELAY);
orxPLUGIN_USER_CORE_FUNCTION_END();

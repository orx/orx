/**
 * \file orxTime.c
 */

/***************************************************************************
 begin                : 30/07/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
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
 
#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
/* #include "plugin/orxPlugin.h" */
#include "core/orxTime.h"

#include <SDL/SDL.h>
#include <time.h>

#define orxTIME_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxTIME_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

typedef struct __orxTIME_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the time plugin module */
} orxTIME_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
orxSTATIC orxTIME_STATIC sstTime;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Init the time module
 * @return Returns the status of the operation
 */
orxSTATUS orxTimeSDL_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILED; /* Init result */

  /* Init dependencies */
  if ((orxDEPEND_INIT(Memory) == orxSTATUS_SUCCESS) /* && 
      (orxMAIN_INIT_MODULE(Plugin) == orxSTATUS_SUCCESS)*/)
  {
  
    /* Module not already initialized ? */
    if (!(sstTime.u32Flags & orxTIME_KU32_FLAG_READY))
    {
      /* Cleans static controller */
      orxMemory_Set(&sstTime, 0, sizeof(orxTIME_STATIC));
    
      /* Is SDL_Init has already been called ? */
      if (SDL_WasInit(SDL_INIT_EVERYTHING) == 0)
      {
        /* No, calls it */
        if (SDL_Init(0)==0)
        {
          /* Init time subsystem */
          if (SDL_InitSubSystem(SDL_INIT_TIMER)==0)
          {
            /* Set module as ready */
            sstTime.u32Flags = orxTIME_KU32_FLAG_READY;
            
            /* Successfull init */
            eResult = orxSTATUS_SUCCESS;
          }
        }
      }
    }
  }
  
  /* Done */
  return eResult;
}

/** Exit the time module
 */
orxVOID orxTimeSDL_Exit()
{
  /* Module initialized ? */
  if ((sstTime.u32Flags & orxTIME_KU32_FLAG_READY) == orxTIME_KU32_FLAG_READY)
  {
    /* Unitialize SDL Time */
    SDL_QuitSubSystem(SDL_INIT_TIMER);
    
    /* All subsystem uninitialized ? */
    if (SDL_WasInit(SDL_INIT_EVERYTHING) == 0)
    {
      /* Quit SDL */
      SDL_Quit();
    }
    
    /* Module not ready now */
    sstTime.u32Flags = orxTIME_KU32_FLAG_NONE;
  }
  
  orxDEPEND_EXIT(Memory);
}

/** Gets App Elapsed time.
 * @return Returns the amount of milliseconds elapsed from the application start.
 */
orxU32 orxTimeSDL_GetTime()
{
  /* Module initialized ? */
  orxASSERT((sstTime.u32Flags & orxTIME_KU32_FLAG_READY) == orxTIME_KU32_FLAG_READY);

  return((orxU32)SDL_GetTicks());
}

/** Gets date.
 * @return Current date.
 */
orxDATE orxTimeSDL_GetDate()
{
  /* Module initialized ? */
  orxASSERT((sstTime.u32Flags & orxTIME_KU32_FLAG_READY) == orxTIME_KU32_FLAG_READY);
  
  return (orxDATE)time(NULL);
}

/** Delay the program for given number of milliseconds.
 * @param[in] _u32Time Number of milliseconds to wait.
 */
orxVOID orxTimeSDL_Delay(orxU32 _u32Time)
{
  /* Module initialized ? */
  orxASSERT((sstTime.u32Flags & orxTIME_KU32_FLAG_READY) == orxTIME_KU32_FLAG_READY);
  
  SDL_Delay((Uint32)_u32Time);
}


/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxSTATIC orxPLUGIN_USER_FUNCTION_INFO sau32Time_Function[orxPLUGIN_FUNCTION_BASE_ID_TIME_NUMBER];

orxVOID orxPlugin_Init(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfos)
{
  orxPLUGIN_USER_FUNCTION_START(sau32Time_Function);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxTimeSDL_Init, TIME, INIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxTimeSDL_Exit, TIME, EXIT);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxTimeSDL_GetTime, TIME, GET_TIME);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxTimeSDL_GetDate, TIME, GET_DATE);
  orxPLUGIN_USER_CORE_FUNCTION_ADD(orxTimeSDL_Delay, TIME, DELAY);
  orxPLUGIN_USER_FUNCTION_END(_ps32Number, _ppstInfos);
  return;
}

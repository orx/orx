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
 * @file orxSystem.c
 * @date 30/07/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 *
 * SDL system plugin implementation
 *
 */


#include "orxInclude.h"

#include "plugin/orxPluginUser.h"
#include "core/orxSystem.h"

#include <SDL/SDL.h>

/** Module flags
 */
#define orxSYSTEM_KU32_STATIC_FLAG_NONE         0x00000000 /**< No flags */

#define orxSYSTEM_KU32_STATIC_FLAG_READY        0x00000001 /**< Ready flag */

#define orxSYSTEM_KU32_STATIC_MASK_ALL          0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSYSTEM_STATIC_t
{
  orxU32 u32Flags;         /**< Flags set by the system plugin module */

} orxSYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxSYSTEM_STATIC sstSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Init the system module
 * @return Returns the status of the operation
 */
orxSTATUS orxSystemSDL_Init()
{
  orxSTATUS eResult; /* Init result */

  /* Was not already initialized? */
  if(!(sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));

    /* Is SDL partly initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
    {
      /* Inits the timer subsystem */
      eResult = (SDL_InitSubSystem(SDL_INIT_TIMER) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }
    else
    {
      /* Inits SDL with timer */
      eResult = (SDL_Init(SDL_INIT_TIMER) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Sets module as ready */
      sstSystem.u32Flags = orxSYSTEM_KU32_STATIC_FLAG_READY;
    }
  }

  /* Done */
  return eResult;
}

/** Exit the system module
 */
orxVOID orxSystemSDL_Exit()
{
  /* Was initialized ? */
  if((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Is timer the only subsystem initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_TIMER)
    {
      /* Exits from SDL */
      SDL_Quit();
    }
    else
    {
      /* Exits from timer subsystem */
      SDL_QuitSubSystem(SDL_INIT_TIMER);
    }

    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));
  }
}

/** Gets App Elapsed time.
 * @return Returns the amount of seconds elapsed from the application start.
 */
orxFLOAT orxSystemSDL_GetTime()
{
  /* Checks */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Done! */
  return(orx2F(0.001f) * orxU2F(SDL_GetTicks()));
}

/** Delay the program for given number of milliseconds.
 * @param[in] _fSystem Number of seconds to wait.
 */
orxVOID orxSystemSDL_Delay(orxFLOAT _fSeconds)
{
  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Delays */
  SDL_Delay(orxF2U(orx2F(1000.0f) * _fSeconds));

  return;
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

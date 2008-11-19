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
 * @file orxSystem.cpp
 * @date 25/10/2007
 * @author iarwain@orx-project.org
 * 
 * SFML system plugin implemention
 *
 * @todo
 */


extern "C"
{
#include "orxInclude.h"

#include "memory/orxMemory.h"
#include "math/orxMath.h"
#include "plugin/orxPluginUser.h"

#include "core/orxSystem.h"
}

#include <SFML/Window.hpp>
#include <ctime>

/** Module flags
 */
#define orxSYSTEM_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxSYSTEM_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxSYSTEM_STATIC_t
{
  orxU32      u32Flags;
  sf::Clock  *poClock;

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


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Init the system module
 * @return Returns the status of the operation
 */
extern "C" orxSTATUS orxSystem_SFML_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;


  /* Was already initialized. */
  if(!(sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));

    /* Inits system clock */
    sstSystem.poClock = new sf::Clock();

    /* Resets it */
    sstSystem.poClock->Reset();

    /* Updates status */
    sstSystem.u32Flags |= orxSYSTEM_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exit the system module
 */
extern "C" orxVOID orxSystem_SFML_Exit()
{
  /* Module initialized ? */
  if((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Deletes clock */
    delete sstSystem.poClock;

    /* Cleans static controller */
    orxMemory_Zero(&sstSystem, sizeof(orxSYSTEM_STATIC));
  }

  return;
}

/** Gets App Elapsed time.
 * @return Returns the amount of seconds elapsed from the application start.
 */
extern "C" orxFLOAT orxSystem_SFML_GetTime()
{
  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  return(orx2F(sstSystem.poClock->GetElapsedTime()));
}

/** Gets real time
 * @return Returns the amount of seconds elapsed from the application start.
 */
extern "C" orxS32 orxSystem_SFML_GetRealTime()
{
  time_t  stTime;
  orxS32  s32Result;

  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Gets time */
  s32Result = (orxS32)time(&stTime);

  /* Done! */
  return s32Result;
}

/** Delay the program for given number of milliseconds.
 * @param[in] _fSeconds Number of seconds to wait.
 */
extern "C" orxVOID orxSystem_SFML_Delay(orxFLOAT _fSeconds)
{
  /* Module initialized ? */
  orxASSERT((sstSystem.u32Flags & orxSYSTEM_KU32_STATIC_FLAG_READY) == orxSYSTEM_KU32_STATIC_FLAG_READY);

  /* Sleeps */
  sf::Sleep(_fSeconds);
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(DISPLAY);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystem_SFML_Init, SYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystem_SFML_Exit, SYSTEM, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystem_SFML_GetTime, SYSTEM, GET_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystem_SFML_GetRealTime, SYSTEM, GET_REAL_TIME);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSystem_SFML_Delay, SYSTEM, DELAY);
orxPLUGIN_USER_CORE_FUNCTION_END();

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
 * @file orxJoystick.c
 * @date 06/01/2009
 * @author iarwain@orx-project.org
 *
 * SDL joystick plugin implementation
 *
 * @todo
 */


#include "orxPluginAPI.h"

#include <SDL.h>


/** Module flags
 */
#define orxJOYSTICK_KU32_STATIC_FLAG_NONE     0x00000000 /**< No flags */

#define orxJOYSTICK_KU32_STATIC_FLAG_READY    0x00000001 /**< Ready flag */

#define orxJOYSTICK_KU32_STATIC_MASK_ALL      0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxJOYSTICK_STATIC_t
{
  orxU32            u32Flags;
  orxU32            u32JoystickNumber;
  SDL_Joystick    **apstJoystickList;

} orxJOYSTICK_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxJOYSTICK_STATIC sstJoystick;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATUS orxFASTCALL orxJoystick_SDL_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));

    /* Is SDL partly initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) != 0)
    {
      /* Inits the joystick subsystem */
      eResult = (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }
    else
    {
      /* Inits SDL with joystick */
      eResult = (SDL_Init(SDL_INIT_JOYSTICK) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    }

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      orxU32 i;

      /* Gets number of joysticks */
      sstJoystick.u32JoystickNumber = (orxU32)SDL_NumJoysticks();

      /* Has any? */
      if(sstJoystick.u32JoystickNumber > 0)
      {
        /* Allocates info memory */
        sstJoystick.apstJoystickList = (SDL_Joystick **)orxMemory_Allocate(sstJoystick.u32JoystickNumber * sizeof(SDL_Joystick *), orxMEMORY_TYPE_MAIN);

        /* For all of them */
        for(i = 0; i < sstJoystick.u32JoystickNumber; i++)
        {
          /* Opens it */
          sstJoystick.apstJoystickList[i] = SDL_JoystickOpen(i);

          /* Failed? */
          if(sstJoystick.apstJoystickList[i] == orxNULL)
          {
            /* Clears info memory */
            orxMemory_Free(sstJoystick.apstJoystickList);
            sstJoystick.apstJoystickList = orxNULL;

            /* Updates result */
            eResult = orxSTATUS_FAILURE;
          }
        }
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates status */
        sstJoystick.u32Flags |= orxJOYSTICK_KU32_STATIC_FLAG_READY;
      }
      else
      {
        /* Is joystick the only subsystem initialized? */
        if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_JOYSTICK)
        {
          /* Exits from SDL */
          SDL_Quit();
        }
        else
        {
          /* Exits from joystick subsystem */
          SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxJoystick_SDL_Exit()
{
  /* Was initialized? */
  if(sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY)
  {
    orxU32 i;

    /* For all opened joysticks */
    for(i = 0; i < sstJoystick.u32JoystickNumber; i++)
    {
      /* Closes it */
      SDL_JoystickClose(sstJoystick.apstJoystickList[i]);
    }

    /* Frees info memory */
    if(sstJoystick.apstJoystickList != orxNULL)
    {
      orxMemory_Free(sstJoystick.apstJoystickList);
    }

    /* Is joystick the only subsystem initialized? */
    if(SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_JOYSTICK)
    {
      /* Exits from SDL */
      SDL_Quit();
    }
    else
    {
      /* Exits from joystick subsystem */
      SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
    }

    /* Cleans static controller */
    orxMemory_Zero(&sstJoystick, sizeof(orxJOYSTICK_STATIC));
  }

  return;
}

orxFLOAT orxFASTCALL orxJoystick_SDL_GetAxisValue(orxU32 _u32ID, orxJOYSTICK_AXIS _eAxis)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Is ID valid? */
  if(_u32ID < sstJoystick.u32JoystickNumber)
  {
    /* Is axis valid? */
    if(_eAxis < SDL_JoystickNumAxes(sstJoystick.apstJoystickList[_u32ID]))
    {
      /* Updates result */
      fResult = orxS2F(SDL_JoystickGetAxis(sstJoystick.apstJoystickList[_u32ID], _eAxis));
    }
    else
    {
      /* Logs message */
      orxLOG("Requested axis <%s> for joystick ID <%ld> is out of range.", orxJoystick_GetAxisName(_eAxis), _u32ID);

      /* Updates result */
      fResult = orxFLOAT_0;
    }
  }
  else
  {
    /* Logs message */
    orxLOG("Requested joystick ID <%ld> is invalid.", _u32ID);

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_SDL_IsButtonPressed(orxU32 _u32ID, orxJOYSTICK_BUTTON _eButton)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Is ID valid? */
  if(_u32ID < sstJoystick.u32JoystickNumber)
  {
    /* Is button valid? */
    if(_eButton < SDL_JoystickNumButtons(sstJoystick.apstJoystickList[_u32ID]))
    {
      /* Updates result */
      bResult = SDL_JoystickGetButton(sstJoystick.apstJoystickList[_u32ID], _eButton) ? orxTRUE : orxFALSE;
    }
    else
    {
      /* Logs message */
      orxLOG("Requested button <%s> for joystick ID <%ld> is out of range.", orxJoystick_GetButtonName(_eButton), _u32ID);

      /* Updates result */
      bResult = orxFALSE;
    }
  }
  else
  {
    /* Logs message */
    orxLOG("Requested joystick ID <%ld> is invalid.", _u32ID);

    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(JOYSTICK);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SDL_Init, JOYSTICK, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SDL_Exit, JOYSTICK, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SDL_GetAxisValue, JOYSTICK, GET_AXIS_VALUE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxJoystick_SDL_IsButtonPressed, JOYSTICK, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_END();

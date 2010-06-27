/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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

#ifdef __orxMAC__
  #error This plugin doesn't work on Mac OS X!
#endif /* __orxMAC__ */

#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of SDL!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of SDL!
  #endif /* __orxMSVC__ */
#endif /* __orxEMBEDDED__ */


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

orxFLOAT orxFASTCALL orxJoystick_SDL_GetAxisValue(orxJOYSTICK_AXIS _eAxis)
{
  orxU32    u32ID;
  orxFLOAT  fResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eAxis < orxJOYSTICK_AXIS_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eAxis / orxJOYSTICK_AXIS_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID < sstJoystick.u32JoystickNumber)
  {
    orxS32 s32Axis;

    /* Gets axis */
    s32Axis = _eAxis % orxJOYSTICK_AXIS_SINGLE_NUMBER;
  
    /* Is axis valid? */
    if(s32Axis < SDL_JoystickNumAxes(sstJoystick.apstJoystickList[u32ID]))
    {
      /* Updates result */
      fResult = orxS2F(SDL_JoystickGetAxis(sstJoystick.apstJoystickList[u32ID], s32Axis));
    }
    else
    {
      /* Logs message */
      orxLOG("Requested axis <%s> (joystick ID <%ld>) is out of range.", orxJoystick_GetAxisName(_eAxis), u32ID);

      /* Updates result */
      fResult = orxFLOAT_0;
    }
  }
  else
  {
    /* Logs message */
    orxLOG("Requested joystick ID <%ld> is out of range.", u32ID);

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

orxBOOL orxFASTCALL orxJoystick_SDL_IsButtonPressed(orxJOYSTICK_BUTTON _eButton)
{
  orxU32  u32ID;
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstJoystick.u32Flags & orxJOYSTICK_KU32_STATIC_FLAG_READY) == orxJOYSTICK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxJOYSTICK_BUTTON_NUMBER);

  /* Gets ID */
  u32ID = (orxU32)_eButton / orxJOYSTICK_BUTTON_SINGLE_NUMBER;

  /* Is ID valid? */
  if(u32ID < sstJoystick.u32JoystickNumber)
  {
    orxS32 s32Button;

    /* Gets button */
    s32Button = (orxU32)_eButton % orxJOYSTICK_BUTTON_SINGLE_NUMBER;

    /* Is button valid? */
    if(s32Button < SDL_JoystickNumButtons(sstJoystick.apstJoystickList[u32ID]))
    {
      /* Updates result */
      bResult = SDL_JoystickGetButton(sstJoystick.apstJoystickList[u32ID], s32Button) ? orxTRUE : orxFALSE;
    }
    else
    {
      /* Logs message */
      orxLOG("Requested button <%s> (joystick ID <%ld>) is out of range.", orxJoystick_GetButtonName(_eButton), u32ID);

      /* Updates result */
      bResult = orxFALSE;
    }
  }
  else
  {
    /* Logs message */
    orxLOG("Requested joystick ID <%ld> is out of range.", u32ID);

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

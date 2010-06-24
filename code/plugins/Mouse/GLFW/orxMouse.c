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
 * @file orxMouse.c
 * @date 24/06/2010
 * @author iarwain@orx-project.org
 *
 * GLFW mouse plugin implementation
 *
 */


#include "orxPluginAPI.h"

#include "GL/glfw.h"


/** Module flags
 */
#define orxMOUSE_KU32_STATIC_FLAG_NONE        0x00000000 /**< No flags */

#define orxMOUSE_KU32_STATIC_FLAG_READY       0x00000001 /**< Ready flag */

#define orxMOUSE_KU32_STATIC_MASK_ALL         0xFFFFFFFF /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxMOUSE_STATIC_t
{
  orxVECTOR   vMouseMove, vMouseBackup;
  orxU32      u32Flags;
  orxFLOAT    fWheelMove, fInternalWheelMove;
  orxBOOL     bClearWheel;
  orxS32      s32WheelPos;

} orxMOUSE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxMOUSE_STATIC sstMouse;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Event handler
 */
static void orxFASTCALL orxMouse_GLFW_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Should clear wheel? */
  if(sstMouse.bClearWheel != orxFALSE)
  {
    /* Clears it */
    sstMouse.fWheelMove   = orxFLOAT_0;
    sstMouse.bClearWheel  = orxFALSE;
  }

  /* Clears internal wheel move */
  sstMouse.fInternalWheelMove = orxFLOAT_0;

  /* Done! */
  return;
}

/** Position callback
 */
static void GLFWCALL orxMouse_GLFW_MousePositionCallback(int _iX, int _iY)
{
  /* Updates mouse move */
  sstMouse.vMouseMove.fX += orxS2F(_iX) - sstMouse.vMouseBackup.fX;
  sstMouse.vMouseMove.fY += orxS2F(_iY) - sstMouse.vMouseBackup.fY;

  /* Stores last mouse position */
  sstMouse.vMouseBackup.fX = orxS2F(_iX);
  sstMouse.vMouseBackup.fY = orxS2F(_iY);

  /* Done! */
  return;
}

/** Wheel callback
 */
static void GLFWCALL orxMouse_GLFW_MouseWheelCallback(int _iWheel)
{
  /* Updates wheel moves */
  sstMouse.fWheelMove         += orxS2F(_iWheel - sstMouse.s32WheelPos);
  sstMouse.fInternalWheelMove += orxS2F(_iWheel - sstMouse.s32WheelPos);

  /* Stores last wheel position */
  sstMouse.s32WheelPos = _iWheel;

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxMouse_GLFW_ShowCursor(orxBOOL _bShow)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Show cursor? */
  if(_bShow != orxFALSE)
  {
    glfwEnable(GLFW_MOUSE_CURSOR);
  }
  else
  {
    //! TODO glfwDisable(GLFW_MOUSE_CURSOR);
  }

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_GLFW_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Was already initialized. */
  if(!(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));

    /* Is GLFW window opened? */
    if(glfwGetWindowParam(GLFW_OPENED) != GL_FALSE)
    {
      orxCLOCK *pstClock;

      /* Gets core clock */
      pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Registers event update function */
        eResult = orxClock_Register(pstClock, orxMouse_GLFW_Update, orxNULL, orxMODULE_ID_MOUSE, orxCLOCK_PRIORITY_LOWEST);
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Registers mouse position callback */
        glfwSetMousePosCallback(orxMouse_GLFW_MousePositionCallback);

        /* Registers mouse wheel callback */
        glfwSetMouseWheelCallback(orxMouse_GLFW_MouseWheelCallback);

        /* Pushes config section */
        orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

        /* Has show cursor value? */
        if(orxConfig_HasValue(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE)
        {
          /* Updates cursor status */
          orxMouse_GLFW_ShowCursor(orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR));
        }

        /* Pops config section */
        orxConfig_PopSection();

        /* Updates status */
        sstMouse.u32Flags |= orxMOUSE_KU32_STATIC_FLAG_READY;
      }
    }
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxMouse_GLFW_Exit()
{
  /* Was initialized? */
  if(sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY)
  {
    //! TODO

    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));
  }

  return;
}

orxSTATUS orxFASTCALL orxMouse_GLFW_SetPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Moves mouse */
  glfwSetMousePos((int)orxF2S(_pvPosition->fX), (int)orxF2S(_pvPosition->fY));

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxMouse_GLFW_GetPosition(orxVECTOR *_pvPosition)
{
  orxS32      s32X, s32Y;
  orxVECTOR  *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets mouse position */
  glfwGetMousePos((int *)&s32X, (int *)&s32Y);

  /* Updates result */
  _pvPosition->fX = orxS2F(s32X);
  _pvPosition->fY = orxS2F(s32Y);
  _pvPosition->fZ = orxFLOAT_0;

  /* Done! */
  return pvResult;
}

orxBOOL orxFASTCALL orxMouse_GLFW_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_eButton < orxMOUSE_BUTTON_NUMBER);

  /* Depending on button */
  switch(_eButton)
  {
    case orxMOUSE_BUTTON_LEFT:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT) != GL_FALSE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_RIGHT:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(GLFW_MOUSE_BUTTON_RIGHT) != GL_FALSE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_MIDDLE:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) != GL_FALSE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_1:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(GLFW_MOUSE_BUTTON_4) != GL_FALSE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_2:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(GLFW_MOUSE_BUTTON_5) != GL_FALSE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_WHEEL_UP:
    {
      /* Updates result */
      bResult = (sstMouse.fInternalWheelMove > orxFLOAT_0) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_WHEEL_DOWN:
    {
      /* Updates result */
      bResult = (sstMouse.fInternalWheelMove < orxFLOAT_0) ? orxTRUE : orxFALSE;
      break;
    }

    default:
    {
      /* Updates result */
      bResult = orxFALSE;
      break;
    }
  }

  /* Done! */
  return bResult;
}

orxVECTOR *orxFASTCALL orxMouse_GLFW_GetMoveDelta(orxVECTOR *_pvMoveDelta)
{
  orxVECTOR *pvResult = _pvMoveDelta;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvMoveDelta != orxNULL);

  /* Updates result */
  orxVector_Copy(_pvMoveDelta, &(sstMouse.vMouseMove));

  /* Clears move */
  orxVector_Copy(&(sstMouse.vMouseMove), &orxVECTOR_0);

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxMouse_GLFW_GetWheelDelta()
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Updates result */
  fResult = sstMouse.fWheelMove;

  /* Clears wheel move on next update */
  sstMouse.bClearWheel = orxTRUE;

  /* Done! */
  return fResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(MOUSE);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_Init, MOUSE, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_Exit, MOUSE, EXIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_SetPosition, MOUSE, SET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_GetPosition, MOUSE, GET_POSITION);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_GetMoveDelta, MOUSE, GET_MOVE_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_GetWheelDelta, MOUSE, GET_WHEEL_DELTA);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_ShowCursor, MOUSE, SHOW_CURSOR);
orxPLUGIN_USER_CORE_FUNCTION_END();

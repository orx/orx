/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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

#include "GLFW/glfw3.h"

#ifndef __orxEMBEDDED__
  #ifdef __orxMSVC__
    #pragma message("!!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!")
  #else /* __orxMSVC__ */
    #warning !!WARNING!! This plugin will only work in non-embedded mode when linked against a *DYNAMIC* version of GLFW!
  #endif /* __orxMSVC__ */
#endif /* __orxEMBEDDED__ */


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
  orxVECTOR   vMouseMove, vMouseBackup, vMouseAcc, vMouseTouch;
  GLFWwindow *pstWindow;
  orxU32      u32Flags;
  orxFLOAT    fWheelMove, fInternalWheelMove;
  orxBOOL     bClearWheel, bClearMove, bButtonPressed, bUpdateCursor;

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

/** Position callback
 */
static void orxMouse_GLFW_CursorPositionCallback(GLFWwindow *_pstWindow, double _dX, double _dY)
{
  /* Updates mouse move */
  sstMouse.vMouseMove.fX += orx2F(_dX) - sstMouse.vMouseBackup.fX + sstMouse.vMouseAcc.fX;
  sstMouse.vMouseMove.fY += orx2F(_dY) - sstMouse.vMouseBackup.fY + sstMouse.vMouseAcc.fY;

  /* Stores last mouse position */
  sstMouse.vMouseBackup.fX = orx2F(_dX);
  sstMouse.vMouseBackup.fY = orx2F(_dY);

  /* Clears mouse accumulator */
  sstMouse.vMouseAcc.fX = sstMouse.vMouseAcc.fY = orxFLOAT_0;

  /* Done! */
  return;
}

/** Wheel callback
 */
static void orxMouse_GLFW_ScrollCallback(GLFWwindow *_pstWindow, double _dX, double _dY)
{
  /* Updates wheel moves */
  sstMouse.fWheelMove         += orx2F(_dY);
  sstMouse.fInternalWheelMove += orx2F(_dY);

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxMouse_GLFW_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_DISPLAY);

  /* Retrieves current window */
  orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED, 0, orxNULL, orxNULL, &(sstMouse.pstWindow));

  /* Registers mouse position callback */
  glfwSetCursorPosCallback(sstMouse.pstWindow, orxMouse_GLFW_CursorPositionCallback);

  /* Registers mouse wheel callback */
  glfwSetScrollCallback(sstMouse.pstWindow, orxMouse_GLFW_ScrollCallback);

  /* Asks for cursor update */
  sstMouse.bUpdateCursor = orxTRUE;

  /* Done! */
  return eResult;
}

/** Update callback
 */
static void orxFASTCALL orxMouse_GLFW_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxMouse_Update");

  /* Should update cursor? */
  if(sstMouse.bUpdateCursor != orxFALSE)
  {
    /* Pushes config section */
    orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

    /* Restores cursor status */
    glfwSetInputMode(sstMouse.pstWindow, GLFW_CURSOR, (orxConfig_GetBool(orxMOUSE_KZ_CONFIG_GRAB) != orxFALSE) ? GLFW_CURSOR_DISABLED : (orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

    /* Pops config section */
    orxConfig_PopSection();

    /* Updates status */
    sstMouse.bUpdateCursor = orxFALSE;
  }

  /* Is left button pressed? */
  if(glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE)
  {
    orxSYSTEM_EVENT_PAYLOAD stPayload;

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
    stPayload.stTouch.dTime     = orxSystem_GetTime();
    stPayload.stTouch.u32ID     = 0;
    stPayload.stTouch.fX        = sstMouse.vMouseBackup.fX;
    stPayload.stTouch.fY        = sstMouse.vMouseBackup.fY;
    stPayload.stTouch.fPressure = orxFLOAT_1;

    /* Wasn't pressed before? */
    if(sstMouse.bButtonPressed == orxFALSE)
    {
      /* Sends touch event */
      orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_BEGIN, orxNULL, orxNULL, &stPayload);

      /* Updates button pressed status */
      sstMouse.bButtonPressed = orxTRUE;

      /* Stores touch position */
      orxVector_Copy(&(sstMouse.vMouseTouch), &(sstMouse.vMouseBackup));
    }
    else
    {
      /* Has moved? */
      if(orxVector_AreEqual(&(sstMouse.vMouseBackup), &(sstMouse.vMouseTouch)) == orxFALSE)
      {
        /* Sends touch event */
        orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_MOVE, orxNULL, orxNULL, &stPayload);

        /* Stores touch position */
        orxVector_Copy(&(sstMouse.vMouseTouch), &(sstMouse.vMouseBackup));
      }
    }
  }
  else
  {
    /* Was previously pressed? */
    if(sstMouse.bButtonPressed != orxFALSE)
    {
      orxSYSTEM_EVENT_PAYLOAD stPayload;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxSYSTEM_EVENT_PAYLOAD));
      stPayload.stTouch.dTime     = orxSystem_GetTime();
      stPayload.stTouch.u32ID     = 0;
      stPayload.stTouch.fX        = sstMouse.vMouseBackup.fX;
      stPayload.stTouch.fY        = sstMouse.vMouseBackup.fY;
      stPayload.stTouch.fPressure = orxFLOAT_0;

      /* Sends touch event */
      orxEVENT_SEND(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_TOUCH_END, orxNULL, orxNULL, &stPayload);

      /* Updates button pressed status */
      sstMouse.bButtonPressed = orxFALSE;

      /* Clears touch position */
      orxVector_Copy(&(sstMouse.vMouseTouch), &orxVECTOR_0);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Clean callback
 */
static void orxFASTCALL orxMouse_GLFW_Clean(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Should clear wheel? */
  if(sstMouse.bClearWheel != orxFALSE)
  {
    /* Clears it */
    sstMouse.fWheelMove   = orxFLOAT_0;
    sstMouse.bClearWheel  = orxFALSE;
  }

  /* Should clear move? */
  if(sstMouse.bClearMove != orxFALSE)
  {
    /* Clears it */
    sstMouse.vMouseMove.fX  =
    sstMouse.vMouseMove.fY  = orxFLOAT_0;
    sstMouse.bClearMove     = orxFALSE;
  }

  /* Clears internal wheel move */
  sstMouse.fInternalWheelMove = orxFLOAT_0;

  /* Done! */
  return;
}

orxSTATUS orxFASTCALL orxMouse_GLFW_ShowCursor(orxBOOL _bShow)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Pushes config section */
  orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

  /* Not grabbed? */
  if(orxConfig_GetBool(orxMOUSE_KZ_CONFIG_GRAB) == orxFALSE)
  {
    /* Updates cursor status */
    glfwSetInputMode(sstMouse.pstWindow, GLFW_CURSOR, (_bShow != orxFALSE) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);
  }

  /* Updates cursor status */
  orxConfig_SetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR, _bShow);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_GLFW_Grab(orxBOOL _bGrab)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

  /* Pushes config section */
  orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

  /* Updates cursor status */
  glfwSetInputMode(sstMouse.pstWindow, GLFW_CURSOR, (_bGrab != orxFALSE) ? GLFW_CURSOR_DISABLED : (orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE) ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_HIDDEN);

  /* Updates grab status */
  orxConfig_SetBool(orxMOUSE_KZ_CONFIG_GRAB, _bGrab);

  /* Pops config section */
  orxConfig_PopSection();

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

    /* Retrieves current window */
    orxEVENT_SEND(orxEVENT_TYPE_FIRST_RESERVED, 0, orxNULL, orxNULL, &(sstMouse.pstWindow));

    /* Success? */
    if(sstMouse.pstWindow != orxNULL)
    {
      orxCLOCK *pstClock;

      /* Gets core clock */
      pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Registers update function */
        eResult = orxClock_Register(pstClock, orxMouse_GLFW_Update, orxNULL, orxMODULE_ID_MOUSE, orxCLOCK_PRIORITY_HIGHER);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Registers clean function */
          eResult = orxClock_Register(pstClock, orxMouse_GLFW_Clean, orxNULL, orxMODULE_ID_MOUSE, orxCLOCK_PRIORITY_LOWER);
        }
        else
        {
          /* Unregisters update function */
          orxClock_Unregister(pstClock, orxMouse_GLFW_Update);
        }
      }

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Updates status */
        sstMouse.u32Flags |= orxMOUSE_KU32_STATIC_FLAG_READY;

        /* Adds event handler */
        orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxMouse_GLFW_EventHandler);
        orxEvent_SetHandlerIDFlags(orxMouse_GLFW_EventHandler, orxEVENT_TYPE_DISPLAY, orxNULL, orxEVENT_GET_FLAG(orxDISPLAY_EVENT_SET_VIDEO_MODE), orxEVENT_KU32_MASK_ID_ALL);

        /* Registers mouse position callback */
        glfwSetCursorPosCallback(sstMouse.pstWindow, orxMouse_GLFW_CursorPositionCallback);

        /* Registers mouse wheel callback */
        glfwSetScrollCallback(sstMouse.pstWindow, orxMouse_GLFW_ScrollCallback);

        /* Pushes config section */
        orxConfig_PushSection(orxMOUSE_KZ_CONFIG_SECTION);

        /* Updates cursor status */
        orxMouse_GLFW_ShowCursor(((orxConfig_HasValue(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) == orxFALSE) || (orxConfig_GetBool(orxMOUSE_KZ_CONFIG_SHOW_CURSOR) != orxFALSE)) ? orxTRUE : orxFALSE);

        /* Pops config section */
        orxConfig_PopSection();
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
    orxCLOCK *pstClock;

    /* Gets core clock */
    pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

    /* Unregisters update function */
    orxClock_Unregister(pstClock, orxMouse_GLFW_Update);

    /* Unregisters clean function */
    orxClock_Unregister(pstClock, orxMouse_GLFW_Clean);

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxMouse_GLFW_EventHandler);

    /* Cleans static controller */
    orxMemory_Zero(&sstMouse, sizeof(orxMOUSE_STATIC));
  }

  return;
}

orxSTATUS orxFASTCALL orxMouse_GLFW_SetPosition(const orxVECTOR *_pvPosition)
{
  orxDOUBLE dX, dY;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);

    /* Gets mouse position */
    glfwGetCursorPos(sstMouse.pstWindow, (double *)&dX, (double *)&dY);

    /* Updates accumulator */
    sstMouse.vMouseAcc.fX += orx2F(dX) - _pvPosition->fX;
    sstMouse.vMouseAcc.fY += orx2F(dY) - _pvPosition->fY;

    /* Moves mouse */
    glfwSetCursorPos(sstMouse.pstWindow, (double)_pvPosition->fX, (double)_pvPosition->fY);

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxMouse_GLFW_GetPosition(orxVECTOR *_pvPosition)
{
  orxDOUBLE dX, dY;
  orxVECTOR  *pvResult = _pvPosition;

  /* Checks */
  orxASSERT((sstMouse.u32Flags & orxMOUSE_KU32_STATIC_FLAG_READY) == orxMOUSE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets mouse position */
  glfwGetCursorPos(sstMouse.pstWindow, (double *)&dX, (double *)&dY);

  /* Updates result */
  _pvPosition->fX = orx2F(dX);
  _pvPosition->fY = orx2F(dY);
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
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_LEFT) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_RIGHT:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_MIDDLE:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_MIDDLE) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_1:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_4) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_2:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_5) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_3:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_6) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_4:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_7) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
      break;
    }

    case orxMOUSE_BUTTON_EXTRA_5:
    {
      /* Updates result */
      bResult = (glfwGetMouseButton(sstMouse.pstWindow, GLFW_MOUSE_BUTTON_8) != GLFW_RELEASE) ? orxTRUE : orxFALSE;
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

  /* Clears mouse move on next update */
  sstMouse.bClearMove = orxTRUE;

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
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_GLFW_Grab, MOUSE, GRAB);
orxPLUGIN_USER_CORE_FUNCTION_END();

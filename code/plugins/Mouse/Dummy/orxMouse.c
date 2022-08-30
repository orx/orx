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
 * @date 31/01/2010
 * @author iarwain@orx-project.org
 *
 * Dummy mouse plugin implementation
 *
 */

#include "orxPluginAPI.h"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATUS orxFASTCALL orxMouse_Dummy_ShowCursor(orxBOOL _bShow)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_Dummy_Grab(orxBOOL _bGrab)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxSTATUS orxFASTCALL orxMouse_Dummy_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return eResult;
}

void orxFASTCALL orxMouse_Dummy_Exit()
{
  return;
}

orxSTATUS orxFASTCALL orxMouse_Dummy_SetPosition(const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return eResult;
}

orxVECTOR *orxFASTCALL orxMouse_Dummy_GetPosition(orxVECTOR *_pvPosition)
{
  orxVECTOR* pvResult = _pvPosition;

  orxVector_Copy(pvResult, &orxVECTOR_0);

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return pvResult;
}

orxBOOL orxFASTCALL orxMouse_Dummy_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  orxBOOL bResult = orxFALSE;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return bResult;
}

orxVECTOR *orxFASTCALL orxMouse_Dummy_GetMoveDelta(orxVECTOR *_pvMoveDelta)
{
  orxVECTOR *pvResult = _pvMoveDelta;

  orxVector_Copy(_pvMoveDelta, &orxVECTOR_0);

  /* Done! */
  return pvResult;
}

orxFLOAT orxFASTCALL orxMouse_Dummy_GetWheelDelta()
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Not available */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

  /* Done! */
  return fResult;
}

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/
orxPLUGIN_USER_CORE_FUNCTION_START(MOUSE)
    ;
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_Init, MOUSE, INIT);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_Exit, MOUSE, EXIT);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_SetPosition, MOUSE, SET_POSITION);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_GetPosition, MOUSE, GET_POSITION);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_IsButtonPressed, MOUSE, IS_BUTTON_PRESSED);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_GetMoveDelta, MOUSE, GET_MOVE_DELTA);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_GetWheelDelta, MOUSE, GET_WHEEL_DELTA);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_ShowCursor, MOUSE, SHOW_CURSOR);
    orxPLUGIN_USER_CORE_FUNCTION_ADD(orxMouse_Dummy_Grab, MOUSE, GRAB);
    orxPLUGIN_USER_CORE_FUNCTION_END()
;

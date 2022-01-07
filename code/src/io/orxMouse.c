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
 * @date 22/11/2003
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxMouse.h"
#include "plugin/orxPluginCore.h"
#include "display/orxDisplay.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "debug/orxDebug.h"


/** Misc defines
 */
#define orxMOUSE_KZ_LITERAL_PREFIX            "MOUSE_"


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Command: GetPosition
 */
void orxFASTCALL orxMouse_CommandGetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  orxMouse_GetPosition(&(_pstResult->vValue));

  /* Done! */
  return;
}

/** Command: SetPosition
 */
void orxFASTCALL orxMouse_CommandSetPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates position */
  orxMouse_SetPosition(&(_astArgList[0].vValue));

  /* Updates result */
  orxMouse_GetPosition(&(_pstResult->vValue));

  /* Done! */
  return;
}

/** Command: ShowCursor
 */
void orxFASTCALL orxMouse_CommandShowCursor(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates cursor */
  orxMouse_ShowCursor(_astArgList[0].bValue);

  /* Updates result */
  _pstResult->bValue = _astArgList[0].bValue;

  /* Done! */
  return;
}

/** Command: Grab
 */
void orxFASTCALL orxMouse_CommandGrab(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates grab */
  orxMouse_Grab(_astArgList[0].bValue);

  /* Updates result */
  _pstResult->bValue = _astArgList[0].bValue;

  /* Done! */
  return;
}

/** Command: SetCursor
 */
void orxFASTCALL orxMouse_CommandSetCursor(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTATUS eResult;

  /* Has arguments? */
  if(_u32ArgNumber != 0)
  {
    /* Updates cursor */
    eResult = orxMouse_SetCursor(_astArgList[0].zValue, (_u32ArgNumber > 1) ? &(_astArgList[1].vValue) : orxNULL);
  }
  else
  {
    /* Clears cursor */
    eResult = orxMouse_SetCursor(orxNULL, orxNULL);
  }

  /* Updates result */
  _pstResult->bValue = (eResult != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Registers all the mouse commands
 */
static orxINLINE void orxMouse_RegisterCommands()
{
  /* Command: GetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Mouse, GetPosition, "Position", orxCOMMAND_VAR_TYPE_VECTOR, 0, 0);
  /* Command: SetPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Mouse, SetPosition, "Position", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"Position", orxCOMMAND_VAR_TYPE_VECTOR});

  /* Command: ShowCursor */
  orxCOMMAND_REGISTER_CORE_COMMAND(Mouse, ShowCursor, "Shown", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Show", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: Grab */
  orxCOMMAND_REGISTER_CORE_COMMAND(Mouse, Grab, "Grabbed", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Grab", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: SetCursor */
  orxCOMMAND_REGISTER_CORE_COMMAND(Mouse, SetCursor, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 2, {"Name = none", orxCOMMAND_VAR_TYPE_STRING}, {"Pivot = (0, 0)", orxCOMMAND_VAR_TYPE_VECTOR});
}

/** Unregisters all the mouse commands
 */
static orxINLINE void orxMouse_UnregisterCommands()
{
  /* Command: GetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Mouse, GetPosition);
  /* Command: SetPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Mouse, SetPosition);
  /* Command: ShowCursor */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Mouse, ShowCursor);
  /* Command: Grab */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Mouse, Grab);
  /* Command: SetCursor */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Mouse, SetCursor);
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Mouse module setup
 */
void orxFASTCALL orxMouse_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_MOUSE, orxMODULE_ID_COMMAND);

  return;
}

/** Gets button literal name
 * @param[in] _eButton          Concerned button
 * @return Button's name
 */
const orxSTRING orxFASTCALL orxMouse_GetButtonName(orxMOUSE_BUTTON _eButton)
{
  const orxSTRING zResult;

#define orxMOUSE_DECLARE_BUTTON_NAME(BUTTON)  case orxMOUSE_BUTTON_##BUTTON: zResult = orxMOUSE_KZ_LITERAL_PREFIX#BUTTON; break

  /* Checks */
  orxASSERT(_eButton < orxMOUSE_BUTTON_NUMBER);

  /* Depending on button */
  switch(_eButton)
  {
    orxMOUSE_DECLARE_BUTTON_NAME(LEFT);
    orxMOUSE_DECLARE_BUTTON_NAME(RIGHT);
    orxMOUSE_DECLARE_BUTTON_NAME(MIDDLE);
    orxMOUSE_DECLARE_BUTTON_NAME(EXTRA_1);
    orxMOUSE_DECLARE_BUTTON_NAME(EXTRA_2);
    orxMOUSE_DECLARE_BUTTON_NAME(EXTRA_3);
    orxMOUSE_DECLARE_BUTTON_NAME(EXTRA_4);
    orxMOUSE_DECLARE_BUTTON_NAME(EXTRA_5);
    orxMOUSE_DECLARE_BUTTON_NAME(WHEEL_UP);
    orxMOUSE_DECLARE_BUTTON_NAME(WHEEL_DOWN);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "No name defined for button #%d.", _eButton);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets axis literal name
 * @param[in] _eAxis            Concerned axis
 * @return Axis's name
 */
const orxSTRING orxFASTCALL orxMouse_GetAxisName(orxMOUSE_AXIS _eAxis)
{
  const orxSTRING zResult;

#define orxMOUSE_DECLARE_AXIS_NAME(AXIS)  case orxMOUSE_AXIS_##AXIS: zResult = orxMOUSE_KZ_LITERAL_PREFIX#AXIS; break

  /* Checks */
  orxASSERT(_eAxis < orxMOUSE_AXIS_NUMBER);

  /* Depending on button */
  switch(_eAxis)
  {
    orxMOUSE_DECLARE_AXIS_NAME(X);
    orxMOUSE_DECLARE_AXIS_NAME(Y);

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "No name defined for axis #%d.", _eAxis);

      /* Updates result */
      zResult = orxSTRING_EMPTY;
    }
  }

  /* Done! */
  return zResult;
}

/** Sets mouse (hardware) cursor
 * @param[in] _zName       Cursor's name can be: a standard name (arrow, ibeam, hand, crosshair, hresize or vresize), a file name or orxNULL to reset the hardware cursor to default
 * @param[in] _pvPivot     Cursor's pivot (aka hotspot), orxNULL will default to (0, 0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMouse_SetCursor(const orxSTRING _zName, const orxVECTOR *_pvPivot)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

#if defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

  /* Logs message */
  orxDEBUG_PRINT(orxDEBUG_LEVEL_MOUSE, "Not available on this platform!");

#else /* __orxIOS__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

  /* Pushes display config section */
  orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);

  /* Has name? */
  if(_zName != orxNULL)
  {
    /* Has pivot? */
    if(_pvPivot != orxNULL)
    {
      orxCHAR acBuffer[128];

      /* Prints name & pivot */
      acBuffer[orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s#(%g,%g)", _zName, _pvPivot->fX, _pvPivot->fY)] = orxCHAR_NULL;

      /* Stores it */
      orxConfig_SetString(orxDISPLAY_KZ_CONFIG_CURSOR, acBuffer);
    }
    else
    {
      /* Stores cursor's name */
      orxConfig_SetString(orxDISPLAY_KZ_CONFIG_CURSOR, _zName);
    }
  }
  else
  {
    /* Clears cursor value */
    orxConfig_ClearValue(orxDISPLAY_KZ_CONFIG_CURSOR);
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Updates display */
  orxDisplay_SetVideoMode(orxNULL);

#endif /* __orxIOS__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Exit, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_SetPosition, orxSTATUS, const orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_GetPosition, orxVECTOR *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_IsButtonPressed, orxBOOL, orxMOUSE_BUTTON);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_GetMoveDelta, orxVECTOR *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_GetWheelDelta, orxFLOAT, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_ShowCursor, orxSTATUS, orxBOOL);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxMouse_Grab, orxSTATUS, orxBOOL);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(MOUSE)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, INIT, orxMouse_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, EXIT, orxMouse_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, SET_POSITION, orxMouse_SetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, GET_POSITION, orxMouse_GetPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, IS_BUTTON_PRESSED, orxMouse_IsButtonPressed)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, GET_MOVE_DELTA, orxMouse_GetMoveDelta)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, GET_WHEEL_DELTA, orxMouse_GetWheelDelta)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, SHOW_CURSOR, orxMouse_ShowCursor)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(MOUSE, GRAB, orxMouse_Grab)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(MOUSE)


/* *** Core function implementations *** */

/** Inits the mouse module
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxMouse_Init()
{
  orxSTATUS eResult;

  /* Inits module */
  eResult = orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_Init)();

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Registers commands */
    orxMouse_RegisterCommands();
  }

  /* Done! */
  return eResult;
}

/** Exits from the mouse module
 */
void orxFASTCALL orxMouse_Exit()
{
  /* Unregisters commands */
  orxMouse_UnregisterCommands();

  /* Exits from module */
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_Exit)();
}

/** Sets mouse position
 * @param[in] _pvPosition  Mouse position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMouse_SetPosition(const orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_SetPosition)(_pvPosition);
}

/** Gets mouse on screen position
 * @param[out] _pvPosition  Mouse position
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxMouse_GetPosition(orxVECTOR *_pvPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_GetPosition)(_pvPosition);
}

/** Is mouse button pressed?
 * @param[in] _eButton          Mouse button to check
 * @return orxTRUE if pressed / orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxMouse_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_IsButtonPressed)(_eButton);
}

/** Gets mouse move delta (since last call)
 * @param[out] _pvMoveDelta Mouse move delta
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxMouse_GetMoveDelta(orxVECTOR *_pvMoveDelta)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_GetMoveDelta)(_pvMoveDelta);
}

/** Gets mouse wheel delta (since last call)
 * @return Mouse wheel delta
 */
orxFLOAT orxFASTCALL orxMouse_GetWheelDelta()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_GetWheelDelta)();
}

/** Shows mouse (hardware) cursor
 * @param[in] _bShow            Show / Hide
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMouse_ShowCursor(orxBOOL _bShow)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_ShowCursor)(_bShow);
}

/** Grabs the mouse
 * @param[in] _bGrab            Grab / Release
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxMouse_Grab(orxBOOL _bGrab)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_Grab)(_bGrab);
}

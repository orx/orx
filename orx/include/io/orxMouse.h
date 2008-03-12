/**
 * \file orxMouse.h
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxMOUSE_H_
#define _orxMOUSE_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/** Button enum
 */
typedef enum __orxMOUSE_BUTTON_t
{
  orxMOUSE_BUTTON_LEFT = 0,
  orxMOUSE_BUTTON_RIGHT,
  orxMOUSE_BUTTON_MIDDLE,

  orxMOUSE_BUTTON_NUMBER,

  orxMOUSE_BUTTON_NONE = orxENUM_NONE

} orxMOUSE_BUTTON;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Mouse module setup */
extern orxDLLAPI orxVOID                              orxMouse_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxMouse_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxMouse_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxMouse_GetPosition, orxSTATUS, orxS32 *, orxS32 *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxMouse_IsButtonPressed, orxBOOL, orxMOUSE_BUTTON);


/** Init the mouse module
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxMouse_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_Init)();
}

/** Exit the mouse module
 */
orxSTATIC orxINLINE orxVOID orxMouse_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_Exit)();
}

/** Gets mouse on screen position
 * @param[out] _ps32x   X coordinates
 * @param[out] _ps32y   Y coordinates
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxINLINE orxSTATUS orxMouse_GetPosition(orxS32 *_s32X, orxS32 *_s32Y)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_GetPosition)(_s32X, _s32Y);
}

/** Is mouse button pressed?
 * @param _eButton      Mouse button to check
 * @return orxTRUE if presse / orxFALSE otherwise
 */
orxSTATIC orxINLINE orxBOOL orxMouse_IsButtonPressed(orxMOUSE_BUTTON _eButton)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxMouse_IsButtonPressed)(_eButton);
}


#endif /* _orxMOUSE_H_ */

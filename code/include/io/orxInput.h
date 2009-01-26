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
 * @file orxInput.h
 * @date 04/11/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxInput
 * 
 * Input module
 * Module that handles generalized inputs (keyboard, mouse, joystick, ...)
 *
 * @{
 */


#ifndef _orxINPUT_H_
#define _orxINPUT_H_


#include "orxInclude.h"

#include "io/orxJoystick.h"
#include "io/orxKeyboard.h"
#include "io/orxMouse.h"


/** Input type enum
 */
typedef enum __orxINPUT_TYPE_t
{
  orxINPUT_TYPE_KEYBOARD_KEY = 0,
  orxINPUT_TYPE_MOUSE_BUTTON,
  orxINPUT_TYPE_JOYSTICK_BUTTON,
  orxINPUT_TYPE_JOYSTICK_AXIS,

  orxINPUT_TYPE_NUMBER,

  orxINPUT_TYPE_NONE = orxENUM_NONE

} orxINPUT_TYPE;

/** Event enum
 */
typedef enum __orxINPUT_EVENT_t
{
  orxINPUT_EVENT_ON = 0,
  orxINPUT_EVENT_OFF,

  orxINPUT_EVENT_NUMBER,

  orxINPUT_EVENT_NONE = orxENUM_NONE

} orxINPUT_EVENT;


/** Input module setup
 */
extern orxDLLAPI orxVOID                orxInput_Setup();

/** Initializes Input module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxInput_Init();

/** Exits from Input module
 */
extern orxDLLAPI orxVOID                orxInput_Exit();


/** Loads inputs from config
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxInput_Load();

/** Saves inputs to config
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_Save(orxCONST orxSTRING _zFileName);


/** Selects current working set
 * @param[in] _zSetName         Set name to select
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_SelectSet(orxCONST orxSTRING _zSetName);

/** Gets current working set
 * @return Current selected set
 */
extern orxDLLAPI orxSTRING              orxInput_GetCurrentSet();


/** Is input active?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxInput_IsActive(orxCONST orxSTRING _zInputName);

/** Gets input value
 * @param[in] _zInputName       Concerned input name
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL   orxInput_GetValue(orxCONST orxSTRING _zInputName);


/** Binds an input to a mouse/joystick button, keyboard key or joystick axis
 * @param[in] _zName            Concerned input name
 * @param[in] _eType            Type of peripheral to bind
 * @param[in] _eID              ID of button/key/axis to bind
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_Bind(orxCONST orxSTRING _zName, orxINPUT_TYPE _eType, orxENUM _eID);

/** Unbinds a mouse/joystick button, keyboard key or joystick axis
 * @param[in] _eType            Type of peripheral to unbind
 * @param[in] _eID              ID of button/key/axis to unbind
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_Unbind(orxINPUT_TYPE _eType, orxENUM _eID);

#endif /*_orxINPUT_H_*/

/** @} */

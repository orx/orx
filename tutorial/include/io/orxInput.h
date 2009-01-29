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


/** Misc defines
 */
#define orxINPUT_KU32_BINDING_NUMBER    2


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

/** Input event payload
 */
typedef struct __orxINPUT_EVENT_PAYLOAD_t
{
  orxSTRING     zSetName;               /**< Set name : 4 */
  orxSTRING     zInputName;             /**< Input name : 8 */
  orxINPUT_TYPE eType;                  /**< Input binding type : 12 */
  orxENUM       eID;                    /**< Input binding ID : 16 */
  orxFLOAT      fValue;                 /**< Input binding value : 20 */

} orxINPUT_EVENT_PAYLOAD;


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
 * @param[in] _zFileName        File name to load, will use current loaded config if orxSTRING_EMPTY/orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxInput_Load(orxCONST orxSTRING _zFileName);

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

/** Has a new active status since this frame?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active status is new, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxInput_HasNewStatus(orxCONST orxSTRING _zInputName);

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

/** Gets the input name to which a mouse/joystick button, keyboard key or joystick axis is bound
 * @param[in] _eType            Type of peripheral to test
 * @param[in] _eID              ID of button/key/axis to test
 * @return orxSTRING input name if bound / orxSTRING_EMPY otherwise
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxInput_GetBoundInput(orxINPUT_TYPE _eType, orxENUM _eID);

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) at a given index
 * @param[in]   _zName           Concerned input name
 * @param[in]   _u32BindingIndex Index of the desired binding
 * @param[out]  _peType          List of binding types (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out]  _peID            List of binding IDs (button/key/axis)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_GetBinding(orxCONST orxSTRING _zName, orxU32 _u32BindingIndex, orxINPUT_TYPE *_peType, orxENUM *_peID);

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) list
 * @param[in] _zName            Concerned input name
 * @param[out] _aeTypeList      List of binding types (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out] _aeIDList        List of binding IDs (button/key/axis)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_GetBindingList(orxCONST orxSTRING _zName, orxINPUT_TYPE _aeTypeList[orxINPUT_KU32_BINDING_NUMBER], orxENUM _aeIDList[orxINPUT_KU32_BINDING_NUMBER]);


/** Gets a binding name
 * @param[in]   _eType          Binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[in]   _eID            Binding ID (ID of button/key/axis to bind)
 * @return orxSTRING (binding's name) if success, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI orxSTRING orxFASTCALL  orxInput_GetBindingName(orxINPUT_TYPE _eType, orxENUM _eID);

/** Gets active binding (current pressed key/button/...) so as to allow on-the-fly user rebinding
 * @param[out]  _peType         Active binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[out]  _peID           Active binding ID (ID of button/key/axis to bind)
 * @return orxSTATUS_SUCCESS if one active binding is found, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxInput_GetActiveBinding(orxINPUT_TYPE *_peType, orxENUM *_peID);

#endif /*_orxINPUT_H_*/

/** @} */

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
#define orxINPUT_KZ_CONFIG_SECTION                    "Input"             /**< Input section name */
#define orxINPUT_KZ_CONFIG_DEFAULT_THRESHOLD          "DefaultThreshold"  /**< Input default threshold */
#define orxINPUT_KZ_CONFIG_DEFAULT_MULTIPLIER         "DefaultMultiplier" /**< Input default multiplier */
#define orxINPUT_KZ_CONFIG_COMBINE_LIST               "CombineList"       /**< Combine input list */
#define orxINPUT_KZ_CONFIG_JOYSTICK_ID_LIST           "JoyIDList"         /**< Joystick ID list */

#define orxINPUT_KU32_BINDING_NUMBER                  16

#define orxINPUT_KZ_INTERNAL_SET_PREFIX               "-="

#define orxINPUT_KC_MODE_PREFIX_POSITIVE              '+'
#define orxINPUT_KC_MODE_PREFIX_NEGATIVE              '-'

#define orxINPUT_GET_FLAG(TYPE)                       ((orxU32)(1U << (orxU32)(TYPE)))
#define orxINPUT_KU32_FLAG_TYPE_NONE                  0x00000000
#define orxINPUT_KU32_MASK_TYPE_ALL                   0x0000FFFF


/** Input type enum
 */
typedef enum __orxINPUT_TYPE_t
{
  orxINPUT_TYPE_KEYBOARD_KEY = 0,
  orxINPUT_TYPE_MOUSE_BUTTON,
  orxINPUT_TYPE_MOUSE_AXIS,
  orxINPUT_TYPE_JOYSTICK_BUTTON,
  orxINPUT_TYPE_JOYSTICK_AXIS,
  orxINPUT_TYPE_EXTERNAL,

  orxINPUT_TYPE_NUMBER,

  orxINPUT_TYPE_NONE = orxENUM_NONE

} orxINPUT_TYPE;

/** Input mode enum
 */
typedef enum __orxINPUT_MODE_t
{
  orxINPUT_MODE_FULL = 0,
  orxINPUT_MODE_POSITIVE,
  orxINPUT_MODE_NEGATIVE,

  orxINPUT_MODE_NUMBER,

  orxINPUT_MODE_NONE = orxENUM_NONE

} orxINPUT_MODE;

/** Event enum
 */
typedef enum __orxINPUT_EVENT_t
{
  orxINPUT_EVENT_ON = 0,
  orxINPUT_EVENT_OFF,
  orxINPUT_EVENT_SELECT_SET,
  orxINPUT_EVENT_REMOVE_SET,

  orxINPUT_EVENT_NUMBER,

  orxINPUT_EVENT_NONE = orxENUM_NONE

} orxINPUT_EVENT;

/** Input event payload
 */
typedef struct __orxINPUT_EVENT_PAYLOAD_t
{
  const orxSTRING zSetName;                               /**< Set name : 4/8 */
  const orxSTRING zInputName;                             /**< Input name : 8/16 */
  orxINPUT_TYPE   aeType[orxINPUT_KU32_BINDING_NUMBER];   /**< Input binding type : 40/48 */
  orxENUM         aeID[orxINPUT_KU32_BINDING_NUMBER];     /**< Input binding ID : 72/80 */
  orxINPUT_MODE   aeMode[orxINPUT_KU32_BINDING_NUMBER];   /**< Input binding Mode : 104/112 */
  orxFLOAT        afValue[orxINPUT_KU32_BINDING_NUMBER];  /**< Input binding value : 136/144 */

} orxINPUT_EVENT_PAYLOAD;


/** Input module setup
 */
extern orxDLLAPI void orxFASTCALL                 orxInput_Setup();

/** Initializes Input module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_Init();

/** Exits from Input module
 */
extern orxDLLAPI void orxFASTCALL                 orxInput_Exit();


/** Loads inputs from config
 * @param[in] _zFileName        File name to load, will use current loaded config if orxSTRING_EMPTY/orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_Load(const orxSTRING _zFileName);

/** Saves inputs to config
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_Save(const orxSTRING _zFileName);


/** Selects (and enables) current working set
 * @param[in] _zSetName         Set name to select
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SelectSet(const orxSTRING _zSetName);

/** Gets current working set
 * @return Current selected set
 */
extern orxDLLAPI const orxSTRING orxFASTCALL      orxInput_GetCurrentSet();

/** Gets next set
 * @param[in]   _zSetName       Concerned set, orxNULL to get the first one
 * @return Set name / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL      orxInput_GetNextSet(const orxSTRING _zSetName);

/** Pushes a set (storing the current one on the stack)
 * @param[in] _zSetName         Set name to push
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_PushSet(const orxSTRING _zSetName);

/** Pops last set from the stack
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_PopSet();

/** Removes a set
 * @param[in] _zSetName         Set name to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_RemoveSet(const orxSTRING _zSetName);


/** Enables/disables working set (without selecting it)
 * @param[in] _zSetName         Set name to enable/disable
 * @param[in] _bEnable          Enable / Disable
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_EnableSet(const orxSTRING _zSetName, orxBOOL _bEnable);

/** Is working set enabled (includes current working set)?
 * @param[in] _zSetName         Set name to check
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxInput_IsSetEnabled(const orxSTRING _zSetName);


/** Clears all input values of a set
 * @param[in] _zSetName         Set name to clear, will use current set if orxSTRING_EMPTY/orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_ClearSet(const orxSTRING _zSetName);


/** Sets current set's type flags, only set types will be polled when updating the set (use orxINPUT_GET_FLAG(TYPE) in order to get the flag that matches a type)
 * @param[in] _u32AddTypeFlags      Type flags to add
 * @param[in] _u32RemoveTypeFlags   Type flags to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SetTypeFlags(orxU32 _u32AddTypeFlags, orxU32 _u32RemoveTypeFlags);


/** Gets next input in current set
 * @param[in] _zInputName       Concerned input, orxNULL to get the first one
 * @return Input name / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL      orxInput_GetNext(const orxSTRING _zInputName);

/** Is input active?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxInput_IsActive(const orxSTRING _zInputName);

/** Has input been activated (this frame)?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if newly activated since last frame, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxInput_HasBeenActivated(const orxSTRING _zInputName);

/** Has input been deactivated (this frame)?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if newly deactivated since last frame, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxInput_HasBeenDeactivated(const orxSTRING _zInputName);

/** Has a new active status since this frame?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active status is new, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxInput_HasNewStatus(const orxSTRING _zInputName);

/** Gets input value
 * @param[in] _zInputName       Concerned input name
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL             orxInput_GetValue(const orxSTRING _zInputName);

/** Sets input value (will take precedence over peripheral inputs only once)
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fValue           Value to set, orxFLOAT_0 to deactivate
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SetValue(const orxSTRING _zInputName, orxFLOAT _fValue);

/** Sets permanent input value (will take precedence over peripheral inputs until reset)
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fValue           Value to set, orxFLOAT_0 to deactivate
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SetPermanentValue(const orxSTRING _zInputName, orxFLOAT _fValue);

/** Resets input value (peripheral inputs will then be used instead of code ones)
 * @param[in] _zInputName       Concerned input name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_ResetValue(const orxSTRING _zInputName);


/** Gets input threshold
 * @param[in] _zInputName       Concerned input name
 * @return Input threshold
 */
extern orxDLLAPI orxFLOAT orxFASTCALL             orxInput_GetThreshold(const orxSTRING _zInputName);

/** Sets input threshold, if not set the default global threshold will be used
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fThreshold       Threshold value (between 0.0f and 1.0f)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SetThreshold(const orxSTRING _zInputName, orxFLOAT _fThreshold);

/** Gets input multiplier
 * @param[in] _zInputName       Concerned input name
 * @return Input multiplier if found, -1.0f otherwise
 */
extern orxDLLAPI orxFLOAT orxFASTCALL             orxInput_GetMultiplier(const orxSTRING _zInputName);

/** Sets input multiplier, if not set the default global multiplier will be used
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fMultiplier      Multiplier value, can be negative
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SetMultiplier(const orxSTRING _zInputName, orxFLOAT _fMultiplier);


/** Sets an input combine mode
 * @param[in] _zName            Concerned input name
 * @param[in] _bCombine         If orxTRUE, all assigned bindings need to be active in order to activate input, otherwise input will be considered active if any of its binding is
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_SetCombineMode(const orxSTRING _zName, orxBOOL _bCombine);

/** Is an input in combine mode?
 * @param[in] _zName            Concerned input name
 * @return orxTRUE if the input is in combine mode, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL              orxInput_IsInCombineMode(const orxSTRING _zName);


/** Binds an input to a mouse/joystick button, keyboard key or joystick axis
 * @param[in] _zName            Concerned input name
 * @param[in] _eType            Type of peripheral to bind
 * @param[in] _eID              ID of button/key/axis to bind
 * @param[in] _eMode            Mode (only used for axis input)
 * @param[in] _s32BindingIndex  Index of the desired binding, if < 0 the oldest binding will be replaced
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_Bind(const orxSTRING _zName, orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode, orxS32 _s32BindingIndex);

/** Unbinds an input
 * @param[in] _zName            Concerned input name
 * @param[in] _s32BindingIndex  Index of the desired binding, if < 0 all the bindings will be removed
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_Unbind(const orxSTRING _zName, orxS32 _s32BindingIndex);

/** Gets the input count to which a mouse/joystick button, keyboard key or joystick axis is bound
 * @param[in] _eType            Type of peripheral to test
 * @param[in] _eID              ID of button/key/axis to test
 * @param[in] _eMode            Mode (only used for axis input)
 * @return Number of bound inputs
 */
extern orxDLLAPI orxU32 orxFASTCALL               orxInput_GetBoundInputCount(orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode);

/** Gets the input name to which a mouse/joystick button, keyboard key or joystick axis is bound (at given index)
 * @param[in] _eType            Type of peripheral to test
 * @param[in] _eID              ID of button/key/axis to test
 * @param[in] _eMode            Mode (only used for axis input)
 * @param[in] _u32InputIndex    Index of the desired input
 * @param[out] _pzName          Input name, mandatory
 * @param[out] _pu32BindingIndex Binding index for this input, ignored if orxNULL
 * @return orxSTATUS_SUCCESS if binding exists / orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_GetBoundInput(orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode, orxU32 _u32InputIndex, const orxSTRING *_pzName, orxU32 *_pu32BindingIndex);

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) at a given index
 * @param[in]   _zName            Concerned input name
 * @param[in]   _u32BindingIndex  Index of the desired binding, should be less than orxINPUT_KU32_BINDING_NUMBER
 * @param[out]  _peType           Binding type (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out]  _peID             Binding ID (button/key/axis)
 * @param[out]  _peMode           Mode (only used for axis inputs)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_GetBinding(const orxSTRING _zName, orxU32 _u32BindingIndex, orxINPUT_TYPE *_peType, orxENUM *_peID, orxINPUT_MODE *_peMode);

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) list
 * @param[in]   _zName          Concerned input name
 * @param[out]  _aeTypeList     List of binding types (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out]  _aeIDList       List of binding IDs (button/key/axis)
 * @param[out]  _aeModeList     List of modes (only used for axis inputs)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_GetBindingList(const orxSTRING _zName, orxINPUT_TYPE _aeTypeList[orxINPUT_KU32_BINDING_NUMBER], orxENUM _aeIDList[orxINPUT_KU32_BINDING_NUMBER], orxINPUT_MODE _aeModeList[orxINPUT_KU32_BINDING_NUMBER]);


/** Gets a binding name, don't keep the result as is as it'll get overridden during the next call to this function
 * @param[in]   _eType          Binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[in]   _eID            Binding ID (ID of button/key/axis to bind)
 * @param[in]   _eMode          Mode (only used for axis input)
 * @return orxSTRING (binding's name) if success, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL      orxInput_GetBindingName(orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode);

/** Gets a binding type and ID from its name
 * @param[in]   _zName          Concerned input name
 * @param[out]  _peType         Binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[out]  _peID           Binding ID (ID of button/key/axis to bind)
 * @param[out]  _peMode         Binding mode (only used for axis input)
 * @return orxSTATUS_SUCCESS if input is valid, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_GetBindingType(const orxSTRING _zName, orxINPUT_TYPE *_peType, orxENUM *_peID, orxINPUT_MODE *_peMode);

/** Gets active binding (current pressed key/button/...) so as to allow on-the-fly user rebinding
 * @param[out]  _peType         Active binding's type (mouse/joystick button, keyboard key or joystick axis)
 * @param[out]  _peID           Active binding's ID (ID of button/key/axis to bind)
 * @param[out]  _pfValue        Active binding's value (optional)
 * @return orxSTATUS_SUCCESS if one active binding is found, orxSTATUS_FAILURE otherwise
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxInput_GetActiveBinding(orxINPUT_TYPE *_peType, orxENUM *_peID, orxFLOAT *_pfValue);

#endif /*_orxINPUT_H_*/

/** @} */

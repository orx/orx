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
 * @file orxInput.c
 * @date 04/11/2008
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxInput.h"

#include "core/orxClock.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxBank.h"
#include "utils/orxHashTable.h"
#include "utils/orxLinkList.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

  #include <malloc.h>

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxINPUT_KU32_STATIC_FLAG_NONE                0x00000000  /**< No flags */

#define orxINPUT_KU32_STATIC_FLAG_READY               0x00000001  /**< Ready flag */
#define orxINPUT_KU32_STATIC_FLAG_CUSTOM_SET_LIST     0x00000002  /**< Custom set list flag */

#define orxINPUT_KU32_STATIC_MASK_ALL                 0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxINPUT_KZ_INPUT_EXTERNAL                    "EXTERNAL"  /**< External input */

#define orxINPUT_KZ_CONFIG_SET_LIST                   "SetList"   /**< Input set list */

#define orxINPUT_KZ_THRESHOLD_FORMAT                  "%sThreshold" /**< Threshold format */
#define orxINPUT_KZ_MULTIPLIER_FORMAT                 "%sMultiplier" /**< Multiplier format */

#define orxINPUT_KU32_SET_BANK_SIZE                   8
#define orxINPUT_KU32_ENTRY_BANK_SIZE                 32

#define orxINPUT_KU32_SET_FLAG_NONE                   0x00000000  /**< No flags */

#define orxINPUT_KU32_SET_FLAG_ENABLED                0x10000000  /**< Enabled flag */

#define orxINPUT_KU32_SET_MASK_ALL                    0xFFFFFFFF  /**< All mask */


#define orxINPUT_KU32_ENTRY_FLAG_NONE                 0x00000000  /**< No flags */

#define orxINPUT_KU32_ENTRY_FLAG_BOUND                0x10000000  /**< Bound flag */
#define orxINPUT_KU32_ENTRY_FLAG_ACTIVE               0x20000000  /**< Active flag */
#define orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS           0x40000000  /**< New status flag */
#define orxINPUT_KU32_ENTRY_FLAG_COMBINE              0x80000000  /**< Combine status flag */
#define orxINPUT_KU32_ENTRY_FLAG_PERMANENT            0x01000000  /**< Permanent flag */
#define orxINPUT_KU32_ENTRY_FLAG_EXTERNAL             0x02000000  /**< External flag */
#define orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL       0x04000000  /**< Reset external flag */
#define orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL        0x08000000  /**< Last external flag */

#define orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING       0x0000000F  /**< Oldest binding mask */
#define orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING  0x000000F0  /**< Last active binding mask */

#define orxINPUT_KU32_ENTRY_MASK_ALL                  0xFFFFFFFF  /**< All mask */

#define orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING 4           /**< Last active binding shift */

#define orxINPUT_KF_DEFAULT_THRESHOLD                 orx2F(0.15f)/**< Default threshold */
#define orxINPUT_KF_DEFAULT_MULTIPLIER                orxFLOAT_1  /**< Default multiplier */

#define orxINPUT_KU32_RESULT_BUFFER_SIZE              64          /**< Result buffer size */

#define orxINPUT_KZ_MODE_FORMAT                       "%c%s"      /**< Mode format */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Input binding structure
 */
typedef struct __orxINPUT_BINDING_t
{
  orxINPUT_TYPE   eType;                                          /**< Input type : 4 */
  orxENUM         eID;                                            /**< Input ID : 8 */
  orxINPUT_MODE   eMode;                                          /**< Input Mode : 12 */
  orxFLOAT        fValue;                                         /**< Value : 16 */

} orxINPUT_BINDING;

/** Input entry structure
 */
typedef struct __orxINPUT_ENTRY_t
{
  orxLINKLIST_NODE  stNode;                                       /**< List node : 12 */
  const orxSTRING   zName;                                        /**< Entry name : 16 */
  orxSTRINGID       stID;                                         /**< Name ID (CRC) : 20 */
  orxU32            u32Status;                                    /**< Entry status : 24 */
  orxFLOAT          fThreshold;                                   /**< Threshold : 28 */
  orxFLOAT          fMultiplier;                                  /**< Multiplier: 32 */
  orxFLOAT          fExternalValue;                               /**< External value : 36 */

  orxINPUT_BINDING  astBindingList[orxINPUT_KU32_BINDING_NUMBER]; /**< Entry binding list : 116 */

} orxINPUT_ENTRY;

/** Input set structure
 */
typedef struct __orxINPUT_SET_t
{
  orxLINKLIST_NODE  stNode;                                       /**< List node : 12 */
  const orxSTRING   zName;                                        /**< Set name : 16 */
  orxBANK          *pstEntryBank;                                 /**< Entry bank : 20 */
  orxLINKLIST       stEntryList;                                  /**< Entry list : 32 */
  orxSTRINGID       stID;                                         /**< Set CRC : 36 */
  orxFLOAT          fDefaultThreshold;                            /**< Default threshold  : 40 */
  orxFLOAT          fDefaultMultiplier;                           /**< Default multiplier : 44 */
  orxU32            u32Flags;                                     /** Flags : 48 */

} orxINPUT_SET;

/** Static structure
 */
typedef struct __orxINPUT_STATIC_t
{
  orxBANK      *pstSetBank;                                       /**< Set bank */
  orxHASHTABLE *pstSetTable;                                      /**< Set table */
  orxINPUT_SET *pstCurrentSet;                                    /**< Current set */
  orxINPUT_SET *pstDefaultSet;                                    /**< Default set */
  orxVECTOR     vMouseMove;                                       /**< Mouse move */
  orxU32        u32Flags;                                         /**< Control flags */
  orxCHAR       acResultBuffer[orxINPUT_KU32_RESULT_BUFFER_SIZE]; /**< Result buffer */

} orxINPUT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxINPUT_STATIC sstInput;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Semi-private, internal-use only forward declarations
 */
orxBOOL orxFASTCALL orxConfig_HasValueNoCheck(const orxSTRING _zKey);

/** Command: Load
 */
void orxFASTCALL orxInput_CommandLoad(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxInput_Load((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxSTRING_EMPTY) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: Save
 */
void orxFASTCALL orxInput_CommandSave(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxInput_Save(_astArgList[0].zValue) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: SelectSet
 */
void orxFASTCALL orxInput_CommandSelectSet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Selects it */
  orxInput_SelectSet(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->zValue = (sstInput.pstCurrentSet != orxNULL) ? sstInput.pstCurrentSet->zName : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: GetCurrentSet
 */
void orxFASTCALL orxInput_CommandGetCurrentSet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = (sstInput.pstCurrentSet != orxNULL) ? sstInput.pstCurrentSet->zName : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: RemoveSet
 */
void orxFASTCALL orxInput_CommandRemoveSet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Removes it */
  _pstResult->bValue = (orxInput_RemoveSet(_astArgList[0].zValue) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: EnableSet
 */
void orxFASTCALL orxInput_CommandEnableSet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Enabled it */
  orxInput_EnableSet(_astArgList[0].zValue, (_u32ArgNumber < 2) || (_astArgList[1].bValue != orxFALSE) ? orxTRUE : orxFALSE);

  /* Updates result */
  _pstResult->zValue = orxInput_IsSetEnabled(_astArgList[0].zValue) ? _astArgList[0].zValue : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: IsSetEnabled
 */
void orxFASTCALL orxInput_CommandIsSetEnabled(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = orxInput_IsSetEnabled(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: ClearSet
 */
void orxFASTCALL orxInput_CommandClearSet(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTATUS eResult;

  /* Clears it */
  eResult = orxInput_ClearSet((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxSTRING_EMPTY);

  /* Updates result */
  _pstResult->zValue = (eResult != orxSTATUS_FAILURE)
                       ? (_u32ArgNumber > 0)
                         ? _astArgList[0].zValue
                         : orxInput_GetCurrentSet()
                       : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: SetValue
 */
void orxFASTCALL orxInput_CommandSetValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Permanently? */
  if((_u32ArgNumber > 2) && (_astArgList[2].bValue != orxFALSE))
  {
    /* Sets it */
    orxInput_SetPermanentValue(_astArgList[0].zValue, _astArgList[1].fValue);
  }
  else
  {
    /* Sets it */
    orxInput_SetValue(_astArgList[0].zValue, _astArgList[1].fValue);
  }

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: ResetValue
 */
void orxFASTCALL orxInput_CommandResetValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Resets it */
  orxInput_ResetValue(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: GetValue
 */
void orxFASTCALL orxInput_CommandGetValue(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->fValue = orxInput_GetValue(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: IsActive
 */
void orxFASTCALL orxInput_CommandIsActive(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = orxInput_IsActive(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: HasNewStatus
 */
void orxFASTCALL orxInput_CommandHasNewStatus(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = orxInput_HasNewStatus(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: HasBeenActivated
 */
void orxFASTCALL orxInput_CommandHasBeenActivated(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = orxInput_HasBeenActivated(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: HasBeenDeactivated
 */
void orxFASTCALL orxInput_CommandHasBeenDeactivated(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = orxInput_HasBeenDeactivated(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Registers all the input commands
 */
static orxINLINE void orxInput_RegisterCommands()
{
  /* Command: Load */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, Load, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 1, {"FileName = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: Save */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, Save, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"FileName", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: SelectSet */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, SelectSet, "Set", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Set", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetCurrentSet */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, GetCurrentSet, "Set", orxCOMMAND_VAR_TYPE_STRING, 0, 0);
  /* Command: RemoveSet */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, RemoveSet, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Set", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: EnableSet */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, EnableSet, "Set", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Set", orxCOMMAND_VAR_TYPE_STRING}, {"Enable = true", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: IsSetEnabled */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, IsSetEnabled, "Enabled?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Set", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: ClearSet */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, ClearSet, "Set", orxCOMMAND_VAR_TYPE_STRING, 0, 1, {"Set = <current>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: SetValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, SetValue, "Input", orxCOMMAND_VAR_TYPE_STRING, 2, 1, {"Input", orxCOMMAND_VAR_TYPE_STRING}, {"Value", orxCOMMAND_VAR_TYPE_FLOAT}, {"Permanent = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: ResetValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, ResetValue, "Input", orxCOMMAND_VAR_TYPE_U64, 1, 0, {"Input", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetValue */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, GetValue, "Value", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Input", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: IsActive */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, IsActive, "Active?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Input", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: HasNewStatus */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, HasNewStatus, "NewStatus?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Input", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: HasBeenActivated */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, HasBeenActivated, "BeenActivated?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Input", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: HasBeenDeactivated */
  orxCOMMAND_REGISTER_CORE_COMMAND(Input, HasBeenDeactivated, "BeenDeactivated?", orxCOMMAND_VAR_TYPE_BOOL, 1, 0, {"Input", orxCOMMAND_VAR_TYPE_STRING});
}

/** Unregisters all the input commands
 */
static orxINLINE void orxInput_UnregisterCommands()
{
  /* Command: Load */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, Load);
  /* Command: Save */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, Save);

  /* Command: SelectSet */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, SelectSet);
  /* Command: GetCurrentSet */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, GetCurrentSet);
  /* Command: GetRemoveSet */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, RemoveSet);

  /* Command: EnableSet */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, EnableSet);
  /* Command: IsSetEnabled */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, IsSetEnabled);

  /* Command: ClearSet */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, ClearSet);

  /* Command: SetValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, SetValue);
  /* Command: ResetValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, ResetValue);
  /* Command: GetValue */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, GetValue);

  /* Command: IsActive */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, IsActive);
  /* Command: HasNewStatus */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, HasNewStatus);
  /* Command: HasBeenActivated */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, HasBeenActivated);
  /* Command: HasBeenDeactivated */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Input, HasBeenDeactivated);
}

static orxINLINE orxFLOAT orxInput_ScaleValue(orxFLOAT _fValue, orxFLOAT _fThreshold, orxFLOAT _fMultiplier)
{
  orxFLOAT fAbs, fResult = orxFLOAT_0;

  /* Gets absolute value */
  fAbs = orxMath_Abs(_fValue);

  /* Above threshold? */
  if(fAbs > _fThreshold)
  {
    orxFLOAT fScale;

    /* Computes scale */
    fScale = _fMultiplier / (orxFLOAT_1 - _fThreshold);

    /* Updates result */
    fResult = (_fValue < orxFLOAT_0) ? fScale * (_fThreshold - fAbs) : fScale * (fAbs - _fThreshold);
  }

  /* Done! */
  return fResult;
}

static orxINLINE orxFLOAT orxInput_GetBindingValue(const orxINPUT_SET *_pstSet, orxINPUT_TYPE _eType, orxENUM _eID)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);

  /* Is type enabled? */
  if((_pstSet == orxNULL) || orxFLAG_TEST(_pstSet->u32Flags, orxINPUT_GET_FLAG(_eType)))
  {
    /* Depending on type */
    switch(_eType)
    {
      case orxINPUT_TYPE_KEYBOARD_KEY:
      {
        /* Updates result */
        fResult = (orxKeyboard_IsKeyPressed((orxKEYBOARD_KEY)_eID) != orxFALSE) ? orxFLOAT_1 : orxFLOAT_0;

        break;
      }

      case orxINPUT_TYPE_MOUSE_BUTTON:
      {
        /* Wheel? */
        switch(_eID)
        {
          case orxMOUSE_BUTTON_WHEEL_UP:
          {
            /* Updates result */
            fResult = orxMouse_GetWheelDelta();
            fResult = orxMAX(fResult, orxFLOAT_0);
            break;
          }

          case orxMOUSE_BUTTON_WHEEL_DOWN:
          {
            /* Updates result */
            fResult = -orxMouse_GetWheelDelta();
            fResult = orxMAX(fResult, orxFLOAT_0);
            break;
          }

          default:
          {
            /* Updates result */
            fResult = (orxMouse_IsButtonPressed((orxMOUSE_BUTTON)_eID) != orxFALSE) ? orxFLOAT_1 : orxFLOAT_0;
            break;
          }
        }

        break;
      }

      case orxINPUT_TYPE_MOUSE_AXIS:
      {
        /* Updates result */
        fResult = (_eID == (orxMOUSE_AXIS_X))
                ? sstInput.vMouseMove.fX
                : (_eID == (orxMOUSE_AXIS_Y))
                ?  sstInput.vMouseMove.fY
                : orxFLOAT_0;

        break;
      }

      case orxINPUT_TYPE_JOYSTICK_BUTTON:
      {
        /* Updates result */
        fResult = (orxJoystick_IsButtonPressed((orxJOYSTICK_BUTTON)_eID) != orxFALSE) ? orxFLOAT_1 : orxFLOAT_0;

        break;
      }

      case orxINPUT_TYPE_JOYSTICK_AXIS:
      {
        /* Updates result */
        fResult = orxJoystick_GetAxisValue((orxJOYSTICK_AXIS)_eID);

        break;
      }

      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input type [%d] is not recognized!", _eType);

        break;
      }
    }
  }

  /* Done! */
  return fResult;
}

static orxINLINE void orxInput_LoadCurrentSet()
{
  orxU64 u64JoyIDs = 0;
  orxU32 eType;
  orxU32 i, u32Count;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(sstInput.pstCurrentSet != orxNULL);
  orxASSERT(orxJOYSTICK_KU32_MAX_ID <= 8 * sizeof(u64JoyIDs));

  /* Pushes its section */
  orxConfig_PushSection(sstInput.pstCurrentSet->zName);

  /* Gets set's default threshold */
  sstInput.pstCurrentSet->fDefaultThreshold = (orxConfig_HasValue(orxINPUT_KZ_CONFIG_DEFAULT_THRESHOLD) != orxFALSE)
                                              ? orxConfig_GetFloat(orxINPUT_KZ_CONFIG_DEFAULT_THRESHOLD)
                                              : (sstInput.pstDefaultSet != orxNULL)
                                                ? sstInput.pstDefaultSet->fDefaultThreshold
                                                : orxINPUT_KF_DEFAULT_THRESHOLD;

  /* Gets set's default multiplier */
  sstInput.pstCurrentSet->fDefaultMultiplier = (orxConfig_HasValue(orxINPUT_KZ_CONFIG_DEFAULT_MULTIPLIER) != orxFALSE)
                                              ? orxConfig_GetFloat(orxINPUT_KZ_CONFIG_DEFAULT_MULTIPLIER)
                                              : (sstInput.pstDefaultSet != orxNULL)
                                                ? sstInput.pstDefaultSet->fDefaultMultiplier
                                                : orxINPUT_KF_DEFAULT_MULTIPLIER;

  /* Has joystick ID list? */
  if(orxConfig_HasValue(orxINPUT_KZ_CONFIG_JOYSTICK_ID_LIST) != orxFALSE)
  {
    /* For all defined joystick IDs */
    for(u32Count = orxConfig_GetListCount(orxINPUT_KZ_CONFIG_JOYSTICK_ID_LIST), i = 0; i < u32Count; i++)
    {
      orxU32 u32JoyID;

      /* Gets it */
      u32JoyID = orxConfig_GetListU32(orxINPUT_KZ_CONFIG_JOYSTICK_ID_LIST, i);

      /* Valid? */
      if((u32JoyID >= orxJOYSTICK_KU32_MIN_ID) && (u32JoyID <= orxJOYSTICK_KU32_MAX_ID))
      {
        /* Updates joystick IDs */
        u64JoyIDs |= ((orxU64)1) << (u32JoyID - 1);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "[%s]: Invalid value <%d> in input set's JoyIDList, valid range is [%u - %u], skipping.", sstInput.pstCurrentSet->zName, u32JoyID, orxJOYSTICK_KU32_MIN_ID, orxJOYSTICK_KU32_MAX_ID);
      }
    }
  }
  else
  {
    /* Uses first four joystick IDs */
    u64JoyIDs = (orxU64)0x000F;
  }

  /* For all input types */
  for(eType = 0; eType < orxINPUT_TYPE_NUMBER; eType++)
  {
    orxU32  eMode;
    orxBOOL bIsJoystick;

    /* Updates status */
    bIsJoystick = ((eType == orxINPUT_TYPE_JOYSTICK_AXIS) || (eType == orxINPUT_TYPE_JOYSTICK_BUTTON)) ? orxTRUE : orxFALSE;

    /* For all modes */
    for(eMode = 0; eMode < orxINPUT_MODE_NUMBER; eMode++)
    {
      orxENUM   eID;
      const orxSTRING zBinding = orxNULL;

      /* For all bindings */
      for(eID = 0; zBinding != orxSTRING_EMPTY; eID++)
      {
        /* Gets binding name */
        zBinding = orxInput_GetBindingName((orxINPUT_TYPE)eType, eID, (orxINPUT_MODE)eMode);

        /* Valid? */
        if(zBinding != orxSTRING_EMPTY)
        {
          orxCHAR acBuffer[64];

          /* Is joystick and not defined? */
          if((bIsJoystick != orxFALSE) && (orxConfig_HasValueNoCheck(zBinding) == orxFALSE))
          {
            orxS32 s32Index, s32NextIndex;
            orxU32 u32JoyID;

            /* Finds last separator */
            for(s32Index = orxString_SearchCharIndex(zBinding, '_', 0);
                (s32Index >= 0) && ((s32NextIndex = orxString_SearchCharIndex(zBinding, '_', s32Index + 1)) > 0);
                s32Index = s32NextIndex)
              ;

            /* Checks */
            orxASSERT((s32Index > 0) && (s32Index < (orxS32)sizeof(acBuffer)));

            /* Gets its ID */
            orxString_ToU32(zBinding + s32Index + 1, &u32JoyID, orxNULL);

            /* Defined? */
            if(u64JoyIDs & (((orxU64)1) << (u32JoyID - 1)))
            {
              /* Uses binding's base name */
              orxString_NCopy(acBuffer, zBinding, s32Index);
              acBuffer[s32Index] = orxCHAR_NULL;
              zBinding = acBuffer;
            }
          }

          /* Has binding? */
          if(orxConfig_HasValueNoCheck(zBinding) != orxFALSE)
          {
            /* For all defined inputs */
            for(u32Count = orxConfig_GetListCount(zBinding), i = 0; i < u32Count; i++)
            {
              const orxSTRING zInput;

              /* Gets bound input */
              zInput = orxConfig_GetListString(zBinding, i);

              /* Valid? */
              if(zInput != orxSTRING_EMPTY)
              {
                orxCHAR acBuffer[128];

                /* Binds it */
                if(orxInput_Bind(zInput, (orxINPUT_TYPE)eType, eID, (orxINPUT_MODE)eMode, -1) != orxSTATUS_FAILURE)
                {
                  /* Gets threshold name */
                  orxString_NPrint(acBuffer, sizeof(acBuffer), orxINPUT_KZ_THRESHOLD_FORMAT, zInput);

                  /* Stores threshold */
                  orxInput_SetThreshold(zInput, (orxConfig_HasValue(acBuffer) != orxFALSE) ? orxConfig_GetFloat(acBuffer) : sstInput.pstCurrentSet->fDefaultThreshold);

                  /* Gets multiplier name */
                  orxString_NPrint(acBuffer, sizeof(acBuffer), orxINPUT_KZ_MULTIPLIER_FORMAT, zInput);

                  /* Stores multiplier */
                  orxInput_SetMultiplier(zInput, (orxConfig_HasValue(acBuffer) != orxFALSE) ? orxConfig_GetFloat(acBuffer) : sstInput.pstCurrentSet->fDefaultMultiplier);
                }
              }
            }
          }
        }
      }
    }
  }

  /* For all defined combines */
  for(i = 0, u32Count = orxConfig_GetListCount(orxINPUT_KZ_CONFIG_COMBINE_LIST); i < u32Count; i++)
  {
    /* Updates its combine mode */
    orxInput_SetCombineMode(orxConfig_GetListString(orxINPUT_KZ_CONFIG_COMBINE_LIST, i), orxTRUE);
  }

  /* Pops previous section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

static orxINLINE void orxInput_UpdateSet(orxINPUT_SET *_pstSet)
{
  orxINPUT_ENTRY *pstEntry;

  /* For all entries */
  for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(_pstSet->stEntryList));
      pstEntry != orxNULL;
      pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
  {
    orxU32  i, u32ActiveIndex = 0;
    orxBOOL bActive = orxFALSE, bStatusSet = orxFALSE, bHasBinding = orxFALSE;

    /* Had external value? */
    if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL))
    {
      /* Updates status */
      orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NONE, orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_PERMANENT);
    }
    /* Need to reset external value? */
    else if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL))
    {
      /* Clears external value */
      pstEntry->fExternalValue = orxFLOAT_0;

      /* Updates status */
      orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_PERMANENT);
    }
    /* Has non permanent external value? */
    else if(orxFLAG_GET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_PERMANENT) == orxINPUT_KU32_ENTRY_FLAG_EXTERNAL)
    {
      /* Marks it for reset */
      orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL, orxINPUT_KU32_ENTRY_FLAG_NONE);
    }

    /* For all bindings */
    for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
    {
      /* Valid? */
      if(pstEntry->astBindingList[i].eType != orxINPUT_TYPE_NONE)
      {
        orxFLOAT fValue, fTestValue;

        /* Updates binding status */
        bHasBinding = orxTRUE;

        /* Gets raw value */
        fValue = orxInput_GetBindingValue(_pstSet, pstEntry->astBindingList[i].eType, pstEntry->astBindingList[i].eID);

        /* Depending on mode */
        switch(pstEntry->astBindingList[i].eMode)
        {
          default:
          case orxINPUT_MODE_FULL:
          {
            /* Uses raw value */
            pstEntry->astBindingList[i].fValue = fValue;

            break;
          }

          case orxINPUT_MODE_POSITIVE:
          {
            /* Stores it if positive */
            pstEntry->astBindingList[i].fValue = (fValue > orxFLOAT_0) ? fValue : orxFLOAT_0;

            break;
          }

          case orxINPUT_MODE_NEGATIVE:
          {
            /* Stores it if negative */
            pstEntry->astBindingList[i].fValue = (fValue < orxFLOAT_0) ? -fValue : orxFLOAT_0;

            break;
          }
        }

        /* Gets test value */
        fTestValue = (orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL)) ? pstEntry->fExternalValue : pstEntry->astBindingList[i].fValue;

        /* Active? */
        if(orxMath_Abs(fTestValue) > pstEntry->fThreshold)
        {
          /* First one? */
          if(bStatusSet == orxFALSE)
          {
            /* Stores active index value */
            u32ActiveIndex = i;

            /* Updates status */
            bActive = orxTRUE;

            /* Updates set status */
            bStatusSet = orxTRUE;
          }
        }
        else
        {
          /* Is in combine mode? */
          if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_COMBINE))
          {
            /* Updates status */
            bActive = orxFALSE;

            /* Updates set status */
            bStatusSet = orxTRUE;
          }
        }
      }
    }

    /* No binding? */
    if(bHasBinding == orxFALSE)
    {
      /* Has external value? */
      if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL))
      {
        /* Updates active status */
        bActive = (pstEntry->fExternalValue != orxFLOAT_0);
      }
    }

    /* Active? */
    if(bActive != orxFALSE)
    {
      /* Was not active and should send events? */
      if(!orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE))
      {
        orxINPUT_EVENT_PAYLOAD stPayload;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxINPUT_EVENT_PAYLOAD));
        stPayload.zSetName    = _pstSet->zName;
        stPayload.zInputName  = pstEntry->zName;

        /* Is in combine mode? */
        if(orxFLAG_GET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_COMBINE | orxINPUT_KU32_ENTRY_FLAG_EXTERNAL) == orxINPUT_KU32_ENTRY_FLAG_COMBINE)
        {
          orxU32 i;

          /* For all bindings */
          for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Updates payload */
            stPayload.aeType[i]   = pstEntry->astBindingList[i].eType;
            stPayload.aeID[i]     = pstEntry->astBindingList[i].eID;
            stPayload.aeMode[i]   = pstEntry->astBindingList[i].eMode;
            stPayload.afValue[i]  = orxInput_ScaleValue(pstEntry->astBindingList[i].fValue, pstEntry->fThreshold, pstEntry->fMultiplier);
          }

          /* Updates status */
          orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS | orxINPUT_KU32_ENTRY_FLAG_ACTIVE | orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING, orxINPUT_KU32_ENTRY_FLAG_NONE);
        }
        else
        {
          orxU32 i;

          /* External value? */
          if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL))
          {
            /* Updates payload values */
            stPayload.aeType[0]   = orxINPUT_TYPE_EXTERNAL;
            stPayload.aeID[0]     = orxENUM_NONE;
            stPayload.aeMode[0]   = orxINPUT_MODE_FULL;
            stPayload.afValue[0]  = pstEntry->fExternalValue;
          }
          else
          {
            /* Updates active binding values */
            stPayload.aeType[0]   = pstEntry->astBindingList[u32ActiveIndex].eType;
            stPayload.aeID[0]     = pstEntry->astBindingList[u32ActiveIndex].eID;
            stPayload.aeMode[0]   = pstEntry->astBindingList[u32ActiveIndex].eMode;
            stPayload.afValue[0]  = orxInput_ScaleValue(pstEntry->astBindingList[u32ActiveIndex].fValue, pstEntry->fThreshold, pstEntry->fMultiplier);
          }

          /* For all unused bindings */
          for(i = 1; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Cleans it */
            stPayload.aeType[i] = orxINPUT_TYPE_NONE;
          }

          /* Updates status */
          orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS | orxINPUT_KU32_ENTRY_FLAG_ACTIVE | (u32ActiveIndex << orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING), orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING);
        }

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_INPUT, orxINPUT_EVENT_ON, orxNULL, orxNULL, &stPayload);
      }
      else
      {
        /* Updates status */
        orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE | (u32ActiveIndex << orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING), orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING | orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS);
      }
    }
    else
    {
      /* Was active and should send events? */
      if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE))
      {
        orxINPUT_EVENT_PAYLOAD stPayload;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxINPUT_EVENT_PAYLOAD));
        stPayload.zSetName    = _pstSet->zName;
        stPayload.zInputName  = pstEntry->zName;

        /* Is in combine mode? */
        if(orxFLAG_GET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_COMBINE | orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL) == orxINPUT_KU32_ENTRY_FLAG_COMBINE)
        {
          orxU32 i;

          /* For all bindings */
          for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Updates payload */
            stPayload.aeType[i]   = pstEntry->astBindingList[i].eType;
            stPayload.aeMode[i]   = pstEntry->astBindingList[i].eMode;
            stPayload.aeID[i]     = pstEntry->astBindingList[i].eID;
            stPayload.afValue[i]  = orxInput_ScaleValue(pstEntry->astBindingList[i].fValue, pstEntry->fThreshold, pstEntry->fMultiplier);;
          }
        }
        else
        {
          orxU32 i;

          /* External value? */
          if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL))
          {
            /* Updates payload values */
            stPayload.aeType[0]   = orxINPUT_TYPE_EXTERNAL;
            stPayload.aeID[0]     = orxENUM_NONE;
            stPayload.aeMode[0]   = orxINPUT_MODE_FULL;
            stPayload.afValue[0]  = pstEntry->fExternalValue;
          }
          else
          {
            orxU32 u32LastActiveIndex;

            /* Gets last active index */
            u32LastActiveIndex = (pstEntry->u32Status & orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING) >> orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING;

            /* Checks */
            orxASSERT(u32LastActiveIndex < orxINPUT_KU32_BINDING_NUMBER);

            /* Updates active binding values */
            stPayload.aeType[0]   = pstEntry->astBindingList[u32LastActiveIndex].eType;
            stPayload.aeID[0]     = pstEntry->astBindingList[u32LastActiveIndex].eID;
            stPayload.aeMode[0]   = pstEntry->astBindingList[u32LastActiveIndex].eMode;
            stPayload.afValue[0]  = orxInput_ScaleValue(pstEntry->astBindingList[u32LastActiveIndex].fValue, pstEntry->fThreshold, pstEntry->fMultiplier);
          }

          /* For all unused bindings */
          for(i = 1; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Cleans it */
            stPayload.aeType[i] = orxINPUT_TYPE_NONE;
          }
        }

        /* Updates status */
        orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS, orxINPUT_KU32_ENTRY_FLAG_ACTIVE);

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_INPUT, orxINPUT_EVENT_OFF, orxNULL, orxNULL, &stPayload);
      }
      else
      {
        /* Updates status */
        orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NONE, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS);
      }
    }
  }
}

/* Save filter callback */
static orxBOOL orxFASTCALL orxInput_SaveCallback(const orxSTRING _zSetName, const orxSTRING _zKeyName, const orxSTRING _zFileName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult = orxFALSE;

  /* Input section? */
  if(orxString_Compare(_zSetName, orxINPUT_KZ_CONFIG_SECTION) == 0)
  {
    /* Updates result */
    bResult = orxTRUE;
  }
  else
  {
    orxINPUT_SET *pstSet;
    orxSTRINGID   stSetID;
    orxU32        u32PrefixLength;

    /* Gets the set ID */
    stSetID = orxString_Hash(_zSetName);

    /* Gets internal prefix length */
    u32PrefixLength = orxString_GetLength(orxINPUT_KZ_INTERNAL_SET_PREFIX);

    /* For all sets */
    for(pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL);
        pstSet != orxNULL;
        pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, pstSet))
    {
      /* Not internal? */
      if(orxString_NCompare(orxINPUT_KZ_INTERNAL_SET_PREFIX, pstSet->zName, u32PrefixLength) != 0)
      {
        /* Found? */
        if(pstSet->stID == stSetID)
        {
          /* Updates result */
          bResult = orxTRUE;
          break;
        }
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Updates inputs
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pContext       Context sent when registering callback to the clock
 */
static void orxFASTCALL orxInput_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxINPUT_SET *pstSet;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxInput_Update");

  /* Updates mouse move */
  orxMouse_GetMoveDelta(&(sstInput.vMouseMove));

  /* For all the sets */
  for(pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL);
      pstSet != orxNULL;
      pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, pstSet))
  {
    /* Is current working set or enabled? */
    if((pstSet == sstInput.pstCurrentSet)
    || orxFLAG_TEST(pstSet->u32Flags, orxINPUT_KU32_SET_FLAG_ENABLED))
    {
      /* Updates it */
      orxInput_UpdateSet(pstSet);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Creates an entry
 * @param[in] _zEntryName       Entry name to create
 * @return orxINPUT_ENTRY
 */
static orxINLINE orxINPUT_ENTRY *orxInput_CreateEntry(const orxSTRING _zEntryName)
{
  orxINPUT_ENTRY *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstInput.pstCurrentSet != orxNULL);
  orxASSERT(_zEntryName != orxNULL);

  /* Valid? */
  if(*_zEntryName != orxCHAR_NULL)
  {
    /* Allocates entry */
    pstResult = (orxINPUT_ENTRY *)orxBank_Allocate(sstInput.pstCurrentSet->pstEntryBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32 i;

      /* Adds it to list */
      orxMemory_Zero(&(pstResult->stNode), sizeof(orxLINKLIST_NODE));
      orxLinkList_AddEnd(&(sstInput.pstCurrentSet->stEntryList), &(pstResult->stNode));

      /* Inits it */
      pstResult->stID       = orxString_GetID(_zEntryName);
      pstResult->zName      = orxString_GetFromID(pstResult->stID);
      pstResult->u32Status  = orxINPUT_KU32_ENTRY_FLAG_NONE;
      pstResult->fThreshold = orxINPUT_KF_DEFAULT_THRESHOLD;
      pstResult->fMultiplier= orxINPUT_KF_DEFAULT_MULTIPLIER;
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        pstResult->astBindingList[i].eType  = orxINPUT_TYPE_NONE;
        pstResult->astBindingList[i].eID    = orxENUM_NONE;
        pstResult->astBindingList[i].eMode  = orxINPUT_MODE_NONE;
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes an entry
 * @param[in] _pstSet           Concerned set
 * @param[in] _pstEntry         Entry to delete
 */
static orxINLINE void orxInput_DeleteEntry(orxINPUT_SET *_pstSet, orxINPUT_ENTRY *_pstEntry)
{
  /* Checks */
  orxASSERT(_pstSet != orxNULL);
  orxASSERT(_pstEntry != orxNULL);

  /* Removes it from list */
  orxLinkList_Remove(&(_pstEntry->stNode));

  /* Deletes it */
  orxBank_Free(_pstSet->pstEntryBank, _pstEntry);

  /* Done! */
  return;
}

/** Creates a set
 * @param[in] _zSetName         Name of the set to create
 * @param[in] _stSetID          ID of the set to create
 */
static orxINLINE orxINPUT_SET *orxInput_CreateSet(orxSTRINGID _stSetID)
{
  orxINPUT_SET *pstResult;

  /* Allocates it */
  pstResult = (orxINPUT_SET *)orxBank_Allocate(sstInput.pstSetBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* Creates its bank */
    pstResult->pstEntryBank = orxBank_Create(orxINPUT_KU32_ENTRY_BANK_SIZE, sizeof(orxINPUT_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstResult->pstEntryBank != orxNULL)
    {
      /* Duplicates its name */
      pstResult->zName = orxString_GetFromID(_stSetID);

      /* Valid? */
      if(pstResult->zName != orxNULL)
      {
        orxSTATUS eResult;

        /* Clears its entry list */
        orxMemory_Zero(&(pstResult->stEntryList), sizeof(orxLINKLIST));

        /* Adds it to table */
        eResult = orxHashTable_Add(sstInput.pstSetTable, _stSetID, pstResult);
        orxASSERT(eResult != orxSTATUS_FAILURE);

        /* Sets its ID */
        pstResult->stID = _stSetID;

        /* Clears its flags */
        pstResult->u32Flags = orxINPUT_KU32_SET_FLAG_NONE | orxINPUT_KU32_MASK_TYPE_ALL;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Can't retrieve input set's name for ID [%u].", _stSetID);

        /* Deletes its bank */
        orxBank_Delete(pstResult->pstEntryBank);

        /* Deletes it */
        orxBank_Free(sstInput.pstSetBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Can't allocate input set for ID [%u].", _stSetID);

      /* Deletes the set */
      orxBank_Free(sstInput.pstSetBank, pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

static orxINLINE orxSTATUS orxInput_SelectSetInternal(const orxSTRING _zSetName, orxBOOL _bForceLoad)
{
  orxINPUT_SET* pstSet;
  orxSTRINGID   stSetID;
  orxBOOL       bLoad;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Updates load status */
  bLoad = _bForceLoad;

  /* Gets the set ID */
  stSetID = orxString_GetID(_zSetName);

  /* Not already selected? */
  if((sstInput.pstCurrentSet == orxNULL)
  || (sstInput.pstCurrentSet->stID != stSetID))
  {
    /* Gets it */
    pstSet = (orxINPUT_SET *)orxHashTable_Get(sstInput.pstSetTable, stSetID);
  }
  else
  {
    /* Updates selection */
    pstSet = sstInput.pstCurrentSet;
  }

  /* Not found? */
  if(pstSet == orxNULL)
  {
    /* Creates it */
    pstSet = orxInput_CreateSet(stSetID);

    /* Success? */
    if(pstSet != orxNULL)
    {
      /* Asks for loading */
      bLoad = orxTRUE;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Can't create input set [%s].", _zSetName);
    }
  }

  /* Has set? */
  if(pstSet != orxNULL)
  {
    /* Selects it */
    sstInput.pstCurrentSet = pstSet;

    /* Should load? */
    if(bLoad != orxFALSE)
    {
      /* Loads it */
      orxInput_LoadCurrentSet();
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Deletes a set
 * @param[in] _pstSet       Set to delete
 */
static orxINLINE void orxInput_DeleteSet(orxINPUT_SET *_pstSet)
{
  /* Checks */
  orxASSERT(_pstSet != orxNULL);

  /* Is the current selected one? */
  if(sstInput.pstCurrentSet == _pstSet)
  {
    /* Deselects it */
    sstInput.pstCurrentSet = orxNULL;
  }

  /* Removes it from table */
  orxHashTable_Remove(sstInput.pstSetTable, _pstSet->stID);

  /* Deletes its entry bank */
  orxBank_Delete(_pstSet->pstEntryBank);

  /* Removes set */
  orxBank_Free(sstInput.pstSetBank, _pstSet);

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxInput_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Add or update? */
  if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
    {
      /* Reloads input */
      orxInput_Load(orxNULL);
    }
  }

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Input module setup
 */
void orxFASTCALL orxInput_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_PROFILER);
  orxModule_AddOptionalDependency(orxMODULE_ID_INPUT, orxMODULE_ID_KEYBOARD);
  orxModule_AddOptionalDependency(orxMODULE_ID_INPUT, orxMODULE_ID_MOUSE);
  orxModule_AddOptionalDependency(orxMODULE_ID_INPUT, orxMODULE_ID_JOYSTICK);

  /* Done! */
  return;
}

/** Inits the input module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstInput, sizeof(orxINPUT_STATIC));

    /* Creates set banks */
    sstInput.pstSetBank = orxBank_Create(orxINPUT_KU32_SET_BANK_SIZE, sizeof(orxINPUT_SET), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if(sstInput.pstSetBank != orxNULL)
    {
      /* Creates set table */
      sstInput.pstSetTable = orxHashTable_Create(orxINPUT_KU32_SET_BANK_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if(sstInput.pstSetTable != orxNULL)
      {
        orxCLOCK *pstClock;

        /* Gets core clock */
        pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

        /* Valid? */
        if(pstClock != orxNULL)
        {
          /* Registers to core clock */
          eResult = orxClock_Register(pstClock, orxInput_Update, orxNULL, orxMODULE_ID_INPUT, orxCLOCK_PRIORITY_HIGH);

          /* Successful? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Updates flags */
            orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY, orxINPUT_KU32_STATIC_FLAG_NONE);

            /* Loads from input */
            orxInput_Load(orxNULL);

            /* Registers commands */
            orxInput_RegisterCommands();

            /* Adds event handler */
            orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxInput_EventHandler);
            orxEvent_SetHandlerIDFlags(orxInput_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
          }
          else
          {
            /* Deletes clock */
            orxClock_Delete(pstClock);

            /* Deletes set table */
            orxHashTable_Delete(sstInput.pstSetTable);

            /* Deletes set bank */
            orxBank_Delete(sstInput.pstSetBank);
          }
        }
        else
        {
          /* Deletes set table */
          orxHashTable_Delete(sstInput.pstSetTable);

          /* Deletes set bank */
          orxBank_Delete(sstInput.pstSetBank);
        }
      }
      else
      {
        /* Deletes set bank */
        orxBank_Delete(sstInput.pstSetBank);
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Tried to initialize input module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the input module
 */
void orxFASTCALL orxInput_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY))
  {
    orxCLOCK     *pstClock;
    orxINPUT_SET *pstSet;

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxInput_EventHandler);

    /* Unregisters commands */
    orxInput_UnregisterCommands();

    /* While there's still a set */
    for(pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL);
        pstSet != orxNULL;
        pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL))
    {
      /* Deletes it */
      orxInput_DeleteSet(pstSet);
    }

    /* Deletes set table */
    orxHashTable_Delete(sstInput.pstSetTable);
    sstInput.pstSetTable = orxNULL;

    /* Clears sets bank */
    orxBank_Delete(sstInput.pstSetBank);
    sstInput.pstSetBank = orxNULL;

    /* Gets core clock */
    pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

    /* Valid? */
    if(pstClock != orxNULL)
    {
      /* Unregisters from core clock */
      orxClock_Unregister(pstClock, orxInput_Update);
    }

    /* Updates flags */
    orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_NONE, orxINPUT_KU32_STATIC_MASK_ALL);
  }

  /* Done! */
  return;
}

/** Loads inputs from config file
 * @param[in] _zFileName        File name to load, will use current loaded config if orxSTRING_EMPTY/orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Load(const orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* Valid name? */
  if((_zFileName != orxNULL) && (*_zFileName != orxCHAR_NULL))
  {
    /* Loads it */
    eResult = orxConfig_Load(_zFileName);
  }

  /* Valid? */
  if(eResult != orxSTATUS_FAILURE)
  {
    orxINPUT_SET *pstPreviousSet, *pstChosenSet = orxNULL;

    /* Stores current set */
    pstPreviousSet = sstInput.pstCurrentSet;

    /* Should load main input? */
    if(orxBank_GetCount(sstInput.pstSetBank) == 0)
    {
      /* Selects and loads it */
      eResult = orxInput_SelectSetInternal(orxINPUT_KZ_CONFIG_SECTION, orxTRUE);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Stores it as default set */
        sstInput.pstDefaultSet = sstInput.pstCurrentSet;
      }
    }
    else
    {
      orxINPUT_SET *pstSet;

      /* For all sets */
      for(pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL);
          pstSet != orxNULL;
          pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, pstSet))
      {
        /* Selects it */
        sstInput.pstCurrentSet = pstSet;

        /* Loads it */
        orxInput_LoadCurrentSet();
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }

    /* Pushes input section */
    orxConfig_PushSection(orxINPUT_KZ_CONFIG_SECTION);

    /* Has list set */
    if(orxConfig_HasValue(orxINPUT_KZ_CONFIG_SET_LIST) != orxFALSE)
    {
      orxU32 i, u32Count;

      /* Updates status */
      orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_CUSTOM_SET_LIST, orxINPUT_KU32_STATIC_FLAG_NONE);

      /* For all defined sets */
      for(i = 0, u32Count = orxConfig_GetListCount(orxINPUT_KZ_CONFIG_SET_LIST); (i < u32Count) && (eResult != orxSTATUS_FAILURE); i++)
      {
        /* Selects and loads it */
        eResult = orxInput_SelectSetInternal(orxConfig_GetListString(orxINPUT_KZ_CONFIG_SET_LIST, i), orxTRUE);

        /* First valid config-defined set? */
        if((i == 0) && (eResult != orxSTATUS_FAILURE))
        {
          /* Chooses it */
          pstChosenSet = sstInput.pstCurrentSet;
        }
      }
    }

    /* Pops previous section */
    orxConfig_PopSection();

    /* Has chosen set? */
    if(pstChosenSet != orxNULL)
    {
      /* Selects it */
      sstInput.pstCurrentSet = pstChosenSet;
    }
    /* Should restore previous set? */
    else if(pstPreviousSet != orxNULL)
    {
      /* Restores it */
      sstInput.pstCurrentSet = pstPreviousSet;
    }
  }

  /* Done! */
  return eResult;
}

/** Saves inputs to input
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Save(const orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zFileName != orxNULL);

  /* Valid? */
  if(*_zFileName != orxCHAR_NULL)
  {
    orxU32            u32Index, u32PrefixLength;
    orxINPUT_SET     *pstSet;
    orxCHAR           acBuffer[128];

#ifdef __orxMSVC__

    const orxSTRING  *azSetNameList = (const orxSTRING *)alloca(orxBank_GetCount(sstInput.pstSetBank) * sizeof(orxSTRING));

#else /* __orxMSVC__ */

    const orxSTRING   azSetNameList[orxBank_GetCount(sstInput.pstSetBank)];

#endif /* __orxMSVC__ */

    /* Gets internal prefix length */
    u32PrefixLength = orxString_GetLength(orxINPUT_KZ_INTERNAL_SET_PREFIX);

    /* Clears input section */
    orxConfig_ClearSection(orxINPUT_KZ_CONFIG_SECTION);

    /* For all sets */
    for(pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL), u32Index = 0;
        pstSet != orxNULL;
        pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, pstSet))
    {
      /* Not an internal input set? */
      if(orxString_NCompare(pstSet->zName, orxINPUT_KZ_INTERNAL_SET_PREFIX, u32PrefixLength) != 0)
      {
        orxINPUT_ENTRY *pstEntry;
        orxU32          u32CombineIndex = 0;

#ifdef __orxMSVC__

        const orxSTRING  *azCombineNameList = (const orxSTRING *)alloca(orxLinkList_GetCount(&(pstSet->stEntryList)) * sizeof(orxSTRING));

#else /* __orxMSVC__ */

        const orxSTRING   azCombineNameList[orxLinkList_GetCount(&(pstSet->stEntryList))];

#endif /* __orxMSVC__ */

        /* Adds it to the set list */
        *(azSetNameList + u32Index++) = pstSet->zName;

        /* Clears and pushes its section */
        orxConfig_ClearSection(pstSet->zName);
        orxConfig_PushSection(pstSet->zName);

        /* Has custom default threshold? */
        if(pstSet->fDefaultThreshold != orxINPUT_KF_DEFAULT_THRESHOLD)
        {
          /* Saves it */
          orxConfig_SetFloat(orxINPUT_KZ_CONFIG_DEFAULT_THRESHOLD, pstSet->fDefaultThreshold);
        }

        /* Has custom default multiplier */
        if(pstSet->fDefaultMultiplier != orxINPUT_KF_DEFAULT_MULTIPLIER)
        {
          /* Saves it */
          orxConfig_SetFloat(orxINPUT_KZ_CONFIG_DEFAULT_MULTIPLIER, pstSet->fDefaultMultiplier);
        }

        /* For all its entries */
        for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(pstSet->stEntryList));
            pstEntry != orxNULL;
            pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
        {
          orxU32 i;

          /* Is in combine mode? */
          if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_COMBINE))
          {
            /* Adds it to the combine list */
            *(azCombineNameList + u32CombineIndex++) = pstEntry->zName;
          }

          /* For all bindings */
          for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Valid? */
            if(pstEntry->astBindingList[i].eType != orxINPUT_TYPE_NONE)
            {
              /* Adds it to config */
              orxConfig_AppendListString(orxInput_GetBindingName(pstEntry->astBindingList[i].eType, pstEntry->astBindingList[i].eID, pstEntry->astBindingList[i].eMode), &pstEntry->zName, 1);
            }
          }

          /* Has a custom threshold? */
          if(pstEntry->fThreshold != pstSet->fDefaultThreshold)
          {
            /* Gets threshold name */
            orxString_NPrint(acBuffer, sizeof(acBuffer), orxINPUT_KZ_THRESHOLD_FORMAT, pstEntry->zName);

            /* Saves it */
            orxConfig_SetFloat(acBuffer, pstEntry->fThreshold);
          }

          /* Has a custom multiplier? */
          if(pstEntry->fMultiplier != pstSet->fDefaultMultiplier)
          {
            /* Gets multiplier name */
            orxString_NPrint(acBuffer, sizeof(acBuffer), orxINPUT_KZ_MULTIPLIER_FORMAT, pstEntry->zName);

            /* Saves it */
            orxConfig_SetFloat(acBuffer, pstEntry->fMultiplier);
          }
        }

        /* Has entries in combine mode? */
        if(u32CombineIndex != 0)
        {
          /* Adds it to config */
          orxConfig_SetListString(orxINPUT_KZ_CONFIG_COMBINE_LIST, azCombineNameList, u32CombineIndex);
        }

        /* Pops config section */
        orxConfig_PopSection();
      }
    }

    /* Has custom set list? */
    if(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_CUSTOM_SET_LIST))
    {
      const orxSTRING *pzSetNameList = azSetNameList;

      /* Skips Input set */
      if((u32Index > 0) && (orxString_Compare(*pzSetNameList, orxINPUT_KZ_CONFIG_SECTION) == 0))
      {
        pzSetNameList++;
        u32Index--;
      }

      /* Pushes input section */
      orxConfig_PushSection(orxINPUT_KZ_CONFIG_SECTION);

      /* Adds set list to config */
      orxConfig_SetListString(orxINPUT_KZ_CONFIG_SET_LIST, pzSetNameList, u32Index);

      /* Pops config section */
      orxConfig_PopSection();
    }

    /* Saves file */
    eResult = orxConfig_Save(_zFileName, orxFALSE, orxInput_SaveCallback);
  }

  /* Done! */
  return eResult;
}

/** Selects current working set
 * @param[in] _zSetnName        Set name to select
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SelectSet(const orxSTRING _zSetName)
{
  orxINPUT_SET *pstPreviousSet;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSetName != orxNULL);

  /* Stores current set */
  pstPreviousSet = sstInput.pstCurrentSet;

  /* Valid? */
  if(*_zSetName != orxCHAR_NULL)
  {
    /* Selects it */
    eResult = orxInput_SelectSetInternal(_zSetName, orxFALSE);
  }
  else
  {
    /* Clears current set */
    sstInput.pstCurrentSet = orxNULL;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* New set? */
    if(sstInput.pstCurrentSet != pstPreviousSet)
    {
      orxINPUT_EVENT_PAYLOAD stPayload;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxINPUT_EVENT_PAYLOAD));
      stPayload.zSetName = (sstInput.pstCurrentSet != orxNULL) ? sstInput.pstCurrentSet->zName : orxSTRING_EMPTY;

      /* Sends it */
      orxEVENT_SEND(orxEVENT_TYPE_INPUT, orxINPUT_EVENT_SELECT_SET, orxNULL, orxNULL, &stPayload);
    }
  }

  /* Done! */
  return eResult;
}

/** Gets current working set
 * @return Current selected set
 */
const orxSTRING orxFASTCALL orxInput_GetCurrentSet()
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* Has selected section? */
  if(sstInput.pstCurrentSet != orxNULL)
  {
    /* Updates result */
    zResult = sstInput.pstCurrentSet->zName;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Gets next set
 * @param[in]   _zSetName       Concerned set, orxNULL to get the first one
 * @return Set name / orxNULL
 */
const orxSTRING orxFASTCALL orxInput_GetNextSet(const orxSTRING _zSetName)
{
  orxINPUT_SET   *pstSet = orxNULL;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* First set? */
  if(_zSetName == orxNULL)
  {
    pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, orxNULL);
  }
  /* Valid name? */
  else if(*_zSetName != orxCHAR_NULL)
  {
    /* Gets it */
    pstSet = (orxINPUT_SET *)orxHashTable_Get(sstInput.pstSetTable, orxString_Hash(_zSetName));

    /* Found? */
    if(pstSet != orxNULL)
    {
      /* Gets next set */
      pstSet = (orxINPUT_SET *)orxBank_GetNext(sstInput.pstSetBank, pstSet);
    }
  }

  /* Updates result */
  zResult = (pstSet != orxNULL) ? pstSet->zName : orxNULL;

  /* Done! */
  return zResult;
}

/** Removes a set
 * @param[in] _zSetName         Set name to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_RemoveSet(const orxSTRING _zSetName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSetName != orxNULL);

  /* Valid? */
  if(*_zSetName != orxCHAR_NULL)
  {
    orxINPUT_SET *pstSet;

    /* For all the sets */
    pstSet = (orxINPUT_SET *)orxHashTable_Get(sstInput.pstSetTable, orxString_Hash(_zSetName));

    /* Found? */
    if(pstSet != orxNULL)
    {
      orxINPUT_EVENT_PAYLOAD stPayload;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxINPUT_EVENT_PAYLOAD));
      stPayload.zSetName = pstSet->zName;

      /* Sends it */
      orxEVENT_SEND(orxEVENT_TYPE_INPUT, orxINPUT_EVENT_REMOVE_SET, orxNULL, orxNULL, &stPayload);

      /* Is currently selected? */
      if(sstInput.pstCurrentSet == pstSet)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input set [%s]: removing currently selected set, no set will be left selected.", pstSet->zName);
      }

      /* Removes it */
      orxInput_DeleteSet(pstSet);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Enables/disables working set (without selecting it)
 * @param[in] _zSetName         Set name to enable/disable
 * @param[in] _bEnable          Enable / Disable
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_EnableSet(const orxSTRING _zSetName, orxBOOL _bEnable)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSetName != orxNULL);

  /* Valid? */
  if(*_zSetName != orxCHAR_NULL)
  {
    orxINPUT_SET *pstPreviousSet;

    /* Stores current set */
    pstPreviousSet = sstInput.pstCurrentSet;

    /* Selects set */
    eResult = orxInput_SelectSetInternal(_zSetName, orxFALSE);

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Should enable it? */
      if(_bEnable != orxFALSE)
      {
        /* Enables it */
        orxFLAG_SET(sstInput.pstCurrentSet->u32Flags, orxINPUT_KU32_SET_FLAG_ENABLED, orxINPUT_KU32_SET_FLAG_NONE);
      }
      else
      {
        /* Disables it */
        orxFLAG_SET(sstInput.pstCurrentSet->u32Flags, orxINPUT_KU32_SET_FLAG_NONE, orxINPUT_KU32_SET_FLAG_ENABLED);
      }
    }

    /* Restores previous set */
    sstInput.pstCurrentSet = pstPreviousSet;
  }

  /* Done! */
  return eResult;
}

/** Is working set enabled (includes current working set)?
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxInput_IsSetEnabled(const orxSTRING _zSetName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSetName != orxNULL);

  /* Valid? */
  if(*_zSetName != orxCHAR_NULL)
  {
    orxINPUT_SET *pstSet;

    /* Gets set */
    pstSet = (orxINPUT_SET *)orxHashTable_Get(sstInput.pstSetTable, orxString_Hash(_zSetName));

    /* Found? */
    if(pstSet != orxNULL)
    {
      /* Updates result */
      bResult = ((pstSet == sstInput.pstCurrentSet) || orxFLAG_TEST(pstSet->u32Flags, orxINPUT_KU32_SET_FLAG_ENABLED)) ? orxTRUE : orxFALSE;
    }
  }

  /* Done! */
  return bResult;
}

/** Clears all input values of a set
 * @param[in] _zSetName         Set name to clear, will use current set if orxSTRING_EMPTY/orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_ClearSet(const orxSTRING _zSetName)
{
  orxINPUT_SET *pstSet;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* Has valid name? */
  if((_zSetName != orxNULL) && (*_zSetName != orxCHAR_NULL))
  {
    /* Gets set */
    pstSet = (orxINPUT_SET *)orxHashTable_Get(sstInput.pstSetTable, orxString_Hash(_zSetName));
  }
  else
  {
    /* Uses current set */
    pstSet = sstInput.pstCurrentSet;
  }

  /* Valid? */
  if(pstSet != orxNULL)
  {
    orxINPUT_ENTRY *pstEntry;

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(pstSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Clears its value */
        pstEntry->astBindingList[i].fValue = orxFLOAT_0;
      }

      /* Updates its status */
      orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NONE, orxINPUT_KU32_ENTRY_FLAG_ACTIVE | orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS | orxINPUT_KU32_ENTRY_FLAG_PERMANENT | orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Sets current set's type flags, only set types will be polled when updating the set (use orxINPUT_GET_FLAG(TYPE) in order to get the flag that matches a type)
 * @param[in] _u32AddTypeFlags      Type flags to add
 * @param[in] _u32RemoveTypeFlags   Type flags to remove
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SetTypeFlags(orxU32 _u32AddTypeFlags, orxU32 _u32RemoveTypeFlags)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if(sstInput.pstCurrentSet != orxNULL)
  {
    /* Updates its ID flags */
    orxFLAG_SET(sstInput.pstCurrentSet->u32Flags, _u32AddTypeFlags & orxINPUT_KU32_MASK_TYPE_ALL, _u32RemoveTypeFlags & orxINPUT_KU32_MASK_TYPE_ALL);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Is input active?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_IsActive(const orxSTRING _zInputName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets its ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        bResult = (orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE)) ? orxTRUE : orxFALSE;

        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Has input been activated (this frame)?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if newly activated since last frame, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_HasBeenActivated(const orxSTRING _zInputName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets its ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        bResult = (orxFLAG_TEST_ALL(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE | orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS)) ? orxTRUE : orxFALSE;

        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Has input been deactivated (this frame)?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if newly deactivated since last frame, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_HasBeenDeactivated(const orxSTRING _zInputName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets its ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        bResult = (orxFLAG_GET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE | orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS) == orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS) ? orxTRUE : orxFALSE;

        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Has a new active status since this frame?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active status is new, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_HasNewStatus(const orxSTRING _zInputName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets its ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        bResult = (orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS)) ? orxTRUE : orxFALSE;

        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Gets input value
 * @param[in] _zInputName       Concerned input name
 * @return orxFLOAT
 */
orxFLOAT orxFASTCALL orxInput_GetValue(const orxSTRING _zInputName)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets its ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        orxU32 i;

        /* External value? */
        if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL))
        {
          /* Updates result */
          fResult = pstEntry->fExternalValue;
        }
        /* Is active? */
        else if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE))
        {
          /* For all bindings */
          for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Valid & active? */
            if((pstEntry->astBindingList[i].eType != orxINPUT_TYPE_NONE)
            && (orxMath_Abs(pstEntry->astBindingList[i].fValue) > pstEntry->fThreshold))
            {
              /* Updates result */
              fResult = pstEntry->astBindingList[i].fValue;
              break;
            }
          }
        }

        /* Updates result */
        fResult = orxInput_ScaleValue(fResult, pstEntry->fThreshold, pstEntry->fMultiplier);

        break;
      }
    }
  }

  /* Done! */
  return fResult;
}

/** Sets input value (will take precedence over peripheral inputs only once)
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fValue           Value to set, orxFLOAT_0 to deactivate
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SetValue(const orxSTRING _zInputName, orxFLOAT _fValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry, *pstSelectedEntry = orxNULL;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates selection */
        pstSelectedEntry = pstEntry;

        break;
      }
    }

    /* Entry not found? */
    if(pstSelectedEntry == orxNULL)
    {
      /* Creates a new entry */
      pstSelectedEntry = orxInput_CreateEntry(_zInputName);
    }

    /* Valid? */
    if(pstSelectedEntry != orxNULL)
    {
      /* Updates its value */
      pstSelectedEntry->fExternalValue = _fValue;

      /* Updates its status */
      orxFLAG_SET(pstSelectedEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL, orxINPUT_KU32_ENTRY_FLAG_PERMANENT | orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Sets permanent input value (will take precedence over peripheral inputs until reset)
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fValue           Value to set, orxFLOAT_0 to deactivate
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SetPermanentValue(const orxSTRING _zInputName, orxFLOAT _fValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry, *pstSelectedEntry = orxNULL;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates selection */
        pstSelectedEntry = pstEntry;

        break;
      }
    }

    /* Entry not found? */
    if(pstSelectedEntry == orxNULL)
    {
      /* Creates a new entry */
      pstSelectedEntry = orxInput_CreateEntry(_zInputName);
    }

    /* Valid? */
    if(pstSelectedEntry != orxNULL)
    {
      /* Updates its value */
      pstSelectedEntry->fExternalValue = _fValue;

      /* Updates its status */
      orxFLAG_SET(pstSelectedEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_PERMANENT, orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_LAST_EXTERNAL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Resets input value (peripheral inputs will then be used instead of code ones)
 * @param[in] _zInputName       Concerned input name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_ResetValue(const orxSTRING _zInputName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Clears its value */
        pstEntry->fExternalValue = orxFLOAT_0;

        /* Updates its status */
        orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_RESET_EXTERNAL, orxINPUT_KU32_ENTRY_FLAG_EXTERNAL | orxINPUT_KU32_ENTRY_FLAG_PERMANENT);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets input threshold
 * @param[in] _zInputName       Concerned input name
 * @return Input threshold
 */
orxFLOAT orxFASTCALL orxInput_GetThreshold(const orxSTRING _zInputName)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        fResult = pstEntry->fThreshold;

        break;
      }
    }
  }

  /* Done! */
  return fResult;
}

/** Sets input threshold, if not set the default global threshold will be used
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fThreshold       Threshold value (between 0.0f and 1.0f)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SetThreshold(const orxSTRING _zInputName, orxFLOAT _fThreshold)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);
  orxASSERT(_fThreshold >= orxFLOAT_0);
  orxASSERT(_fThreshold <= orxFLOAT_1);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Stores threshold */
        pstEntry->fThreshold = _fThreshold;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets input multiplier
 * @param[in] _zInputName       Concerned input name
 * @return Input multiplier if found, -1.0f otherwise
 */
orxFLOAT orxFASTCALL orxInput_GetMultiplier(const orxSTRING _zInputName)
{
  orxFLOAT fResult = -orxFLOAT_1;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        fResult = pstEntry->fMultiplier;

        break;
      }
    }
  }

  /* Done! */
  return fResult;
}

/** Sets input multiplier, if not set the default global multiplier will be used
 * @param[in] _zInputName       Concerned input name
 * @param[in] _fMultiplier      Multiplier value, can be negative
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SetMultiplier(const orxSTRING _zInputName, orxFLOAT _fMultiplier)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zInputName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zInputName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Stores multiplier */
        pstEntry->fMultiplier = _fMultiplier;

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Sets an input combine mode
 * @param[in] _zName            Concerned input name
 * @param[in] _bCombine         If orxTRUE, all assigned bindings need to be active in order to activate input, otherwise input will be considered active if any of its binding is
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_SetCombineMode(const orxSTRING _zName, orxBOOL _bCombine)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Is in combine mode? */
        if(_bCombine != orxFALSE)
        {
          /* Updates its status */
          orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_COMBINE, orxINPUT_KU32_ENTRY_FLAG_NONE);
        }
        else
        {
          /* Updates its status */
          orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NONE, orxINPUT_KU32_ENTRY_FLAG_COMBINE);
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Is an input in combine mode?
 * @param[in] _zName            Concerned input name
 * @return orxTRUE if the input is in combine mode, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_IsInCombineMode(const orxSTRING _zName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        bResult = orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_COMBINE) ? orxTRUE : orxFALSE;

        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Binds an input to a mouse/joystick button, keyboard key or joystick axis
 * @param[in] _zName            Concerned input name
 * @param[in] _eType            Type of peripheral to bind
 * @param[in] _eID              ID of button/key/axis to bind
 * @param[in] _eMode            Mode (only used for axis input)
 * @param[in] _s32BindingIndex  Index of the desired binding, if < 0 the oldest binding will be replaced
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Bind(const orxSTRING _zName, orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode, orxS32 _s32BindingIndex)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT((_eType == orxINPUT_TYPE_NONE) || (_eType < orxINPUT_TYPE_NUMBER));
  orxASSERT((_eMode == orxINPUT_MODE_NONE) || (_eMode < orxINPUT_MODE_NUMBER));
  orxASSERT(_s32BindingIndex < orxINPUT_KU32_BINDING_NUMBER);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zName != orxCHAR_NULL) && (_eType != orxINPUT_TYPE_NONE))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Non-axis type or no mode set? */
    if(((_eType != orxINPUT_TYPE_MOUSE_AXIS) && (_eType != orxINPUT_TYPE_JOYSTICK_AXIS))
    || (_eMode == orxINPUT_MODE_NONE))
    {
      /* Defaults to full */
      _eMode = orxINPUT_MODE_FULL;
    }

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      orxU32 i;

      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        break;
      }

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Is already bound to this? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType) && (pstEntry->astBindingList[i].eMode == _eMode))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input [%s.%s]: <%s> is already bound to input [%s.%s].", sstInput.pstCurrentSet->zName, _zName, orxInput_GetBindingName(_eType, _eID, _eMode), sstInput.pstCurrentSet->zName, pstEntry->zName);
        }
      }
    }

    /* Entry not found? */
    if(pstEntry == orxNULL)
    {
      /* Creates a new entry */
      pstEntry = orxInput_CreateEntry(_zName);
    }

    /* Valid? */
    if(pstEntry != orxNULL)
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Is already bound to entry? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType) && (pstEntry->astBindingList[i].eMode == _eMode))
        {
          /* Updates result */
          eResult = orxSTATUS_SUCCESS;

          break;
        }
      }

      /* Not already bound to it? */
      if(eResult == orxSTATUS_FAILURE)
      {
        orxU32 u32Index;

        /* Gets requested/oldest binding index */
        u32Index = (_s32BindingIndex >= 0) ? (orxU32)_s32BindingIndex : pstEntry->u32Status & orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING;

        /* Checks */
        orxASSERT(u32Index < orxINPUT_KU32_BINDING_NUMBER);

        /* Had a previous binding? */
        if(pstEntry->astBindingList[u32Index].eType != orxINPUT_TYPE_NONE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input [%s.%s]: replacing <%s> with <%s>", sstInput.pstCurrentSet->zName, pstEntry->zName, orxInput_GetBindingName(pstEntry->astBindingList[u32Index].eType, pstEntry->astBindingList[u32Index].eID, pstEntry->astBindingList[u32Index].eMode), orxInput_GetBindingName(_eType, _eID, _eMode));
        }

        /* Updates binding */
        pstEntry->astBindingList[u32Index].eType  = _eType;
        pstEntry->astBindingList[u32Index].eID    = _eID;
        pstEntry->astBindingList[u32Index].eMode  = _eMode;
        pstEntry->astBindingList[u32Index].fValue = orxFLOAT_0;

        /* Gets new oldest index */
        u32Index = (u32Index + 1) % orxINPUT_KU32_BINDING_NUMBER;

        /* Updates status */
        orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_BOUND | u32Index, orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Can't create an input [%s.%s].", sstInput.pstCurrentSet->zName, _zName);
    }
  }

  /* Done! */
  return eResult;
}

/** Unbinds an input
 * @param[in] _zName            Concerned input name
 * @param[in] _s32BindingIndex  Index of the desired binding, if < 0 all the bindings will be removed
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Unbind(const orxSTRING _zName, orxS32 _s32BindingIndex)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_s32BindingIndex < orxINPUT_KU32_BINDING_NUMBER);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        orxU32  u32Index;
        orxBOOL bBound = orxFALSE;

        /* Should unbind all? */
        if(_s32BindingIndex < 0)
        {
          /* For all bindings */
          for(u32Index = 0; u32Index < orxINPUT_KU32_BINDING_NUMBER; u32Index++)
          {
            /* Unbinds it */
            pstEntry->astBindingList[u32Index].eType = orxINPUT_TYPE_NONE;
          }
          u32Index = 0;
        }
        else
        {
          /* Updates binding */
          pstEntry->astBindingList[_s32BindingIndex].eType = orxINPUT_TYPE_NONE;

          /* For all bindings */
          for(u32Index = 0; u32Index < orxINPUT_KU32_BINDING_NUMBER; u32Index++)
          {
            /* Bound? */
            if(pstEntry->astBindingList[u32Index].eType != orxINPUT_TYPE_NONE)
            {
              /* Updates binding status */
              bBound = orxTRUE;
              break;
            }
          }
          u32Index = (orxU32)_s32BindingIndex;
        }

        /* Still bound? */
        if(bBound != orxFALSE)
        {
          /* Updates status */
          orxFLAG_SET(pstEntry->u32Status, u32Index, orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING);
        }
        else
        {
          /* Updates status */
          orxFLAG_SET(pstEntry->u32Status, u32Index, orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING | orxINPUT_KU32_ENTRY_FLAG_BOUND);
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets the input count to which a mouse/joystick button, keyboard key or joystick axis is bound
 * @param[in] _eType            Type of peripheral to test
 * @param[in] _eID              ID of button/key/axis to test
 * @param[in] _eMode            Mode (only used for axis input)
 * @return Number of bound inputs
 */
orxU32 orxFASTCALL orxInput_GetBoundInputCount(orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);

  /* Valid? */
  if(sstInput.pstCurrentSet != orxNULL)
  {
    orxINPUT_ENTRY *pstEntry;

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Found? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType) && (pstEntry->astBindingList[i].eMode == _eMode))
        {
          /* Updates result */
          u32Result++;
        }
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Gets the input name to which a mouse/joystick button, keyboard key or joystick axis is bound (at given index)
 * @param[in] _eType            Type of peripheral to test
 * @param[in] _eID              ID of button/key/axis to test
 * @param[in] _eMode            Mode (only used for axis input)
 * @param[in] _u32InputIndex    Index of the desired input
 * @param[out] _pzName          Input name, mandatory
 * @param[out] _pu32BindingIndex Binding index for this input, ignored if orxNULL
 * @return orxSTATUS_SUCCESS if binding exists / orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetBoundInput(orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode, orxU32 _u32InputIndex, const orxSTRING *_pzName, orxU32 *_pu32BindingIndex)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);
  orxASSERT(_pzName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_u32InputIndex < orxInput_GetBoundInputCount(_eType, _eID, _eMode)))
  {
    orxINPUT_ENTRY *pstEntry;
    orxU32          u32CurrentIndex;

    /* For all entries */
    for(u32CurrentIndex = 0, pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        (eResult == orxSTATUS_FAILURE) && (pstEntry != orxNULL);
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Found? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType) && (pstEntry->astBindingList[i].eMode == _eMode))
        {
          /* Found correct index? */
          if(u32CurrentIndex == _u32InputIndex)
          {
            /* Updates result */
            *_pzName = pstEntry->zName;

            /* Is binding index requested? */
            if(_pu32BindingIndex != orxNULL)
            {
              /* Stores it */
              *_pu32BindingIndex = i;
            }

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;

            break;
          }
          else
          {
            /* Updates current index */
            u32CurrentIndex++;
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) at a given index
 * @param[in]   _zName            Concerned input name
 * @param[in]   _u32BindingIndex  Index of the desired binding, should be less than orxINPUT_KU32_BINDING_NUMBER
 * @param[out]  _peType           Binding type (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out]  _peID             Binding ID (button/key/axis)
 * @param[out]  _peMode           Mode (only used for axis inputs)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetBinding(const orxSTRING _zName, orxU32 _u32BindingIndex, orxINPUT_TYPE *_peType, orxENUM *_peID, orxINPUT_MODE *_peMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_u32BindingIndex < orxINPUT_KU32_BINDING_NUMBER);
  orxASSERT(_peType != orxNULL);
  orxASSERT(_peID != orxNULL);
  orxASSERT(_peMode != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        /* Updates result */
        *_peType  = pstEntry->astBindingList[_u32BindingIndex].eType;
        *_peID    = pstEntry->astBindingList[_u32BindingIndex].eID;
        *_peMode  = pstEntry->astBindingList[_u32BindingIndex].eMode;
        eResult   = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Failed? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Updates result */
    *_peType  = orxINPUT_TYPE_NONE;
    *_peID    = orxENUM_NONE;
    *_peMode  = orxINPUT_MODE_NONE;
  }

  /* Done! */
  return eResult;
}

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) list
 * @param[in]   _zName          Concerned input name
 * @param[out]  _aeTypeList     List of binding types (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out]  _aeIDList       List of binding IDs (button/key/axis)
 * @param[out]  _aeModeList     List of modes (only used for axis inputs)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetBindingList(const orxSTRING _zName, orxINPUT_TYPE _aeTypeList[orxINPUT_KU32_BINDING_NUMBER], orxENUM _aeIDList[orxINPUT_KU32_BINDING_NUMBER], orxINPUT_MODE _aeModeList[orxINPUT_KU32_BINDING_NUMBER])
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_aeTypeList != orxNULL);
  orxASSERT(_aeIDList != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (*_zName != orxCHAR_NULL))
  {
    orxINPUT_ENTRY *pstEntry;
    orxSTRINGID     stEntryID;

    /* Gets entry ID */
    stEntryID = orxString_Hash(_zName);

    /* For all entries */
    for(pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetFirst(&(sstInput.pstCurrentSet->stEntryList));
        pstEntry != orxNULL;
        pstEntry = (orxINPUT_ENTRY *)orxLinkList_GetNext(&(pstEntry->stNode)))
    {
      /* Found? */
      if(pstEntry->stID == stEntryID)
      {
        orxU32 i;

        /* For all bindings */
        for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
        {
          /* Updates result */
          _aeTypeList[i]  = pstEntry->astBindingList[i].eType;
          _aeIDList[i]    = pstEntry->astBindingList[i].eID;
          _aeModeList[i]  = pstEntry->astBindingList[i].eMode;
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;

        break;
      }
    }
  }

  /* Failed? */
  if(eResult == orxSTATUS_FAILURE)
  {
    orxU32 i;

    /* For all bindings */
    for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
    {
      /* Updates result */
      _aeTypeList[i]  = orxINPUT_TYPE_NONE;
      _aeIDList[i]    = orxENUM_NONE;
      _aeModeList[i]  = orxINPUT_MODE_NONE;
    }
  }

  /* Done! */
  return eResult;
}

/** Gets a binding name, don't keep the result as is as it'll get overridden during the next call to this function
 * @param[in]   _eType          Binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[in]   _eID            Binding ID (ID of button/key/axis to bind)
 * @param[in]   _eMode          Mode (only used for axis input)
 * @return orxSTRING (binding's name) if success, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxInput_GetBindingName(orxINPUT_TYPE _eType, orxENUM _eID, orxINPUT_MODE _eMode)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* Depending on type */
  switch(_eType)
  {
    case orxINPUT_TYPE_KEYBOARD_KEY:
    {
      /* Is ID valid? */
      if(_eID < orxKEYBOARD_KEY_NUMBER)
      {
        /* Gets its name */
        zResult = orxKeyboard_GetKeyName((orxKEYBOARD_KEY)_eID);
      }

      break;
    }

    case orxINPUT_TYPE_MOUSE_BUTTON:
    {
      /* Is ID valid? */
      if(_eID < orxMOUSE_BUTTON_NUMBER)
      {
        /* Gets its name */
        zResult = orxMouse_GetButtonName((orxMOUSE_BUTTON)_eID);
      }

      break;
    }

    case orxINPUT_TYPE_MOUSE_AXIS:
    {
      /* Is ID valid? */
      if(_eID < orxMOUSE_AXIS_NUMBER)
      {
        /* Gets its name */
        zResult = orxMouse_GetAxisName((orxMOUSE_AXIS)_eID);
      }

      break;
    }

    case orxINPUT_TYPE_JOYSTICK_BUTTON:
    {
      /* Is ID valid? */
      if(_eID < orxJOYSTICK_BUTTON_NUMBER)
      {
        /* Gets its name */
        zResult = orxJoystick_GetButtonName((orxJOYSTICK_BUTTON)_eID);
      }

      break;
    }

    case orxINPUT_TYPE_JOYSTICK_AXIS:
    {
      /* Is ID valid? */
      if(_eID < orxJOYSTICK_AXIS_NUMBER)
      {
        /* Gets its name */
        zResult = orxJoystick_GetAxisName((orxJOYSTICK_AXIS)_eID);
      }

      break;
    }

    case orxINPUT_TYPE_EXTERNAL:
    {
      /* Updates result */
      zResult = (_eID == orxENUM_NONE) ? (orxSTRING)orxINPUT_KZ_INPUT_EXTERNAL : orxSTRING_EMPTY;

      break;
    }

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input type [%d] is not recognized!", _eType);

      break;
    }
  }

  /* Valid result? */
  if(zResult != orxSTRING_EMPTY)
  {
    /* Depending on mode */
    switch(_eMode)
    {
      case orxINPUT_MODE_FULL:
      {
        /* Nothing to do */
        break;
      }

      case orxINPUT_MODE_POSITIVE:
      {
        /* Updates result */
        orxString_NPrint(sstInput.acResultBuffer, sizeof(sstInput.acResultBuffer), orxINPUT_KZ_MODE_FORMAT, orxINPUT_KC_MODE_PREFIX_POSITIVE, zResult);
        zResult = sstInput.acResultBuffer;

        break;
      }

      case orxINPUT_MODE_NEGATIVE:
      {
        /* Updates result */
        orxString_NPrint(sstInput.acResultBuffer, sizeof(sstInput.acResultBuffer), orxINPUT_KZ_MODE_FORMAT, orxINPUT_KC_MODE_PREFIX_NEGATIVE, zResult);
        zResult = sstInput.acResultBuffer;

        break;
      }

      default:
      {
        if((_eType == orxINPUT_TYPE_MOUSE_AXIS) || (_eType == orxINPUT_TYPE_JOYSTICK_AXIS))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input mode [%d] is not recognized!", _eMode);
        }

        break;
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Gets a binding type and ID from its name
 * @param[in]   _zName          Concerned input name
 * @param[out]  _peType         Binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[out]  _peID           Binding ID (ID of button/key/axis to bind)
 * @param[out]  _peMode         Binding mode (only used for axis input)
 * @return orxSTATUS_SUCCESS if input is valid, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetBindingType(const orxSTRING _zName, orxINPUT_TYPE *_peType, orxENUM *_peID, orxINPUT_MODE *_peMode)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_peType != orxNULL);
  orxASSERT(_peID != orxNULL);
  orxASSERT(_peMode != orxNULL);

  /* Valid name? */
  if(*_zName != orxCHAR_NULL)
  {
    orxSTRINGID stID;

    /* Gets binding ID */
    stID = orxString_Hash(_zName);

    /* Valid? */
    if(stID != orxSTRINGID_UNDEFINED)
    {
      orxU32 eType;

      /* For all input types */
      for(eType = 0; (eResult == orxSTATUS_FAILURE) && (eType < orxINPUT_TYPE_NUMBER); eType++)
      {
        orxU32 eMode;

        /* For all modes */
        for(eMode = 0; eMode < orxINPUT_MODE_NUMBER; eMode++)
        {
          orxENUM   eID;
          const orxSTRING zBinding = orxNULL;

          /* For all bindings */
          for(eID = 0; zBinding != orxSTRING_EMPTY; eID++)
          {
            /* Gets its name */
            zBinding = orxInput_GetBindingName((orxINPUT_TYPE)eType, eID, (orxINPUT_MODE)eMode);

            /* Found? */
            if(orxString_Hash(zBinding) == stID)
            {
              /* Updates result */
              *_peType  = (orxINPUT_TYPE)eType;
              *_peID    = eID;
              *_peMode  = (orxINPUT_MODE)eMode;
              eResult   = orxSTATUS_SUCCESS;

              break;
            }
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets active binding (current pressed key/button/...) so as to allow on-the-fly user rebinding
 * @param[out]  _peType         Active binding's type (mouse/joystick button, keyboard key or joystick axis)
 * @param[out]  _peID           Active binding's ID (ID of button/key/axis to bind)
 * @param[out]  _pfValue        Active binding's value (optional)
 * @return orxSTATUS_SUCCESS if one active binding is found, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetActiveBinding(orxINPUT_TYPE *_peType, orxENUM *_peID, orxFLOAT *_pfValue)
{
  orxU32    eType;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_peType != orxNULL);
  orxASSERT(_peID != orxNULL);

  /* For all input types */
  for(eType = 0; (eResult == orxSTATUS_FAILURE) && (eType < orxINPUT_TYPE_NUMBER); eType++)
  {
    orxENUM         eID;
    const orxSTRING zBinding = orxNULL;

    /* For all bindings */
    for(eID = 0; zBinding != orxSTRING_EMPTY; eID++)
    {
      /* Gets binding name (full mode) */
      zBinding = orxInput_GetBindingName((orxINPUT_TYPE)eType, eID, orxINPUT_MODE_FULL);

      /* Valid? */
      if(zBinding != orxSTRING_EMPTY)
      {
        orxBOOL   bActive;
        orxFLOAT  fValue;

        /* Gets binding's value */
        fValue = orxInput_GetBindingValue(sstInput.pstCurrentSet, (orxINPUT_TYPE)eType, eID);

        /* Updates active status */
        bActive = (orxMath_Abs(fValue) > sstInput.pstCurrentSet->fDefaultThreshold) ? orxTRUE : orxFALSE;

        /* Active? */
        if(bActive != orxFALSE)
        {
          /* Updates result */
          *_peType  = (orxINPUT_TYPE)eType;
          *_peID    = eID;
          if(_pfValue != orxNULL)
          {
            *_pfValue = fValue;
          }
          eResult   = orxSTATUS_SUCCESS;

          break;
        }
      }
    }
  }

  /* Failed? */
  if(eResult == orxSTATUS_FAILURE)
  {
    /* Updates result */
    *_peType  = orxINPUT_TYPE_NONE;
    *_peID    = orxENUM_NONE;
    if(_pfValue != orxNULL)
    {
      *_pfValue = orxFLOAT_0;
    }
  }

  /* Done! */
  return eResult;
}

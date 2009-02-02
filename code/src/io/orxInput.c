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
 * @file orxInput.c
 * @date 04/11/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */


#include "orxInclude.h"

#include "io/orxInput.h"
#include "core/orxClock.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxINPUT_KU32_STATIC_FLAG_NONE                0x00000000  /**< No flags */

#define orxINPUT_KU32_STATIC_FLAG_READY               0x00000001  /**< Ready flag */

#define orxINPUT_KU32_STATIC_MASK_ALL                 0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxINPUT_KZ_CONFIG_SECTION                    "Input"     /**< Input set name */
#define orxINPUT_KZ_CONFIG_SET_LIST                   "SetList"   /**< Input set list */
#define orxINPUT_KZ_CONFIG_JOYSTICK_THRESHOLD         "JoystickThreshold" /**< Input joystick threshold */


#define orxINPUT_KU32_SET_BANK_SIZE                   4
#define orxINPUT_KU32_ENTRY_BANK_SIZE                 8


#define orxINPUT_KU32_ENTRY_FLAG_NONE                 0x00000000  /**< No flags */

#define orxINPUT_KU32_ENTRY_FLAG_BOUND                0x10000000  /**< Bound flags */
#define orxINPUT_KU32_ENTRY_FLAG_ACTIVE               0x20000000  /**< Active flags */
#define orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS           0x40000000  /**< New status flags */

#define orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING       0x0000000F  /**< Oldest binding mask */
#define orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING  0x000000F0  /**< Last active binding mask */

#define orxINPUT_KU32_ENTRY_MASK_ALL                  0xFFFFFFFF  /**< All mask */

#define orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING 4           /**< Last active binding shift */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Input binding structure
 */
typedef struct __orxINPUT_BINDING_t
{
  orxINPUT_TYPE   eType;                                        /**< Input type : 4 */
  orxENUM         eID;                                          /**< Input ID : 8 */
  orxFLOAT        fThreshold;                                   /**< Threshold : 12 */
  orxFLOAT        fValue;                                       /**< Value : 16 */

  orxPAD(16)

} orxINPUT_BINDING;

/** Input entry structure
 */
typedef struct __orxINPUT_ENTRY_t
{
  orxSTRING         zName;                                      /**< Entry name : 4 */
  orxU32            u32ID;                                      /**< Name ID (CRC) : 8 */
  orxU32            u32Status;                                  /**< Entry status : 12 */

  orxINPUT_BINDING  astBindingList[orxINPUT_KU32_BINDING_NUMBER]; /**< Entry binding list : 40 */

  orxPAD(44)

} orxINPUT_ENTRY;

/** Input set structure
 */
typedef struct __orxINPUT_SET_t
{
  orxBANK    *pstBank;                                          /**< Bank of entries : 4 */
  orxSTRING   zName;                                            /**< Set name : 8 */
  orxU32      u32ID;                                            /**< Set CRC : 12 */

} orxINPUT_SET;

/** Static structure
 */
typedef struct __orxINPUT_STATIC_t
{
  orxBANK      *pstSetBank;                                     /**< Bank of sets */
  orxINPUT_SET *pstCurrentSet;                                  /**< Current set */
  orxFLOAT      fJoystickAxisThreshold;                         /**< Joystick axis threshold */
  orxU32        u32Flags;                                       /**< Control flags */

} orxINPUT_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
orxSTATIC orxINPUT_STATIC sstInput;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATIC orxINLINE orxFLOAT orxInput_GetBindingValue(orxINPUT_TYPE _eType, orxENUM _eID)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);

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
          fResult = orxMouse_GetWheelDelta();
          fResult = orxMIN(fResult, orxFLOAT_0);
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

    case orxINPUT_TYPE_JOYSTICK_BUTTON:
    {
      /* Updates result */
      fResult = (orxJoystick_IsButtonPressed(0, (orxJOYSTICK_BUTTON)_eID) != orxFALSE) ? orxFLOAT_1 : orxFLOAT_0;

      break;
    }

    case orxINPUT_TYPE_JOYSTICK_AXIS:
    {
      /* Updates result */
      fResult = orxJoystick_GetAxisValue(0, (orxJOYSTICK_AXIS)_eID);

      break;
    }

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input type <%ld> is not recognized!", _eType);

      break;
    }
  }

  /* Done! */
  return fResult;
}

orxSTATIC orxINLINE orxINPUT_SET *orxInput_LoadSet(orxCONST orxSTRING _zSetName)
{
  orxINPUT_SET *pstResult = orxNULL;
  orxSTRING     zPreviousSection;

  /* Stores previous section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Valid? */
  if((_zSetName != orxSTRING_EMPTY)
  && (orxConfig_SelectSection(_zSetName) != orxSTATUS_FAILURE))
  {
    orxINPUT_SET *pstPreviousSet;

    /* Stores current set */
    pstPreviousSet = sstInput.pstCurrentSet;

    /* Selects set */
    if(orxInput_SelectSet(_zSetName) != orxSTATUS_FAILURE)
    {
      orxINPUT_TYPE eType;

      /* Updates result */
      pstResult = sstInput.pstCurrentSet;

      /* For all input types */
      for(eType = 0; eType < orxINPUT_TYPE_NUMBER; eType++)
      {
        orxENUM   eID;
        orxSTRING zBinding = orxNULL;

        /* For all bindings */
        for(eID = 0; zBinding != orxSTRING_EMPTY; eID++)
        {
          /* Gets binding name */
          zBinding = orxInput_GetBindingName(eType, eID);

          /* Valid? */
          if(zBinding != orxSTRING_EMPTY)
          {
            orxSTRING zInput;

            /* Gets bound input */
            zInput = orxConfig_GetString(zBinding);
            
            /* Valid? */
            if(zInput != orxSTRING_EMPTY)
            {
              /* Unbinds it */
              orxInput_Unbind(eType, eID);

              /* Rebinds it */
              orxInput_Bind(zInput, eType, eID);
            }
          }
        }
      }

      /* Restores previous set */
      sstInput.pstCurrentSet = pstPreviousSet;
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }

  /* Done! */
  return pstResult;
}

/* Save filter callback */
orxBOOL orxFASTCALL orxInput_SaveCallback(orxCONST orxSTRING _zSetName, orxCONST orxSTRING _zKeyName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult = orxFALSE;

  /* Is it the input set? */
  if(orxString_Compare(_zSetName, orxINPUT_KZ_CONFIG_SECTION) == 0)
  {
    /* Updates result */
    bResult = orxTRUE;
  }
  else
  {
    orxINPUT_SET *pstSet;

    /* For all sets */
    for(pstSet = orxBank_GetNext(sstInput.pstSetBank, orxNULL);
        pstSet != orxNULL;
        pstSet = orxBank_GetNext(sstInput.pstSetBank, pstSet))
    {
      /* Found? */
      if(orxString_Compare(_zSetName, pstSet->zName) == 0)
      {
        /* Updates result */
        bResult = orxTRUE;
        break;
      }
    }
  }

  /* Done! */
  return bResult;
}

/** Updates inputs
 * @param[in]   _pstClockInfo   Clock info of the clock used upon registration
 * @param[in]   _pstContext     Context sent when registering callback to the clock
 */
orxVOID orxFASTCALL orxInput_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  /* Has current set? */
  if(sstInput.pstCurrentSet != orxNULL)
  {
    orxINPUT_ENTRY *pstEntry;

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        pstEntry != orxNULL;
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      orxU32        i, u32ActiveIndex;
      orxBOOL       bActive = orxFALSE;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Valid? */
        if(pstEntry->astBindingList[i].eType != orxINPUT_TYPE_NONE)
        {
          /* Updates it */
          pstEntry->astBindingList[i].fValue = orxInput_GetBindingValue(pstEntry->astBindingList[i].eType, pstEntry->astBindingList[i].eID);

          /* Active? */
          if(orxMath_Abs(pstEntry->astBindingList[i].fValue) > pstEntry->astBindingList[i].fThreshold)
          {
            /* First one? */
            if(bActive == orxFALSE)
            {
              /* Stores active index value */
              u32ActiveIndex = i;

              /* Updates status */
              bActive = orxTRUE;
            }
          }
        }
      }

      /* Active? */
      if(bActive != orxFALSE)
      {
        /* Was not active? */
        if(!orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE))
        {
          orxINPUT_EVENT_PAYLOAD stPayload;

          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxINPUT_EVENT_PAYLOAD));
          stPayload.zSetName    = sstInput.pstCurrentSet->zName;
          stPayload.zInputName  = pstEntry->zName;
          stPayload.eType       = pstEntry->astBindingList[u32ActiveIndex].eType;
          stPayload.eID         = pstEntry->astBindingList[u32ActiveIndex].eID;
          stPayload.fValue      = pstEntry->astBindingList[u32ActiveIndex].fValue;

          /* Updates status */
          orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS | orxINPUT_KU32_ENTRY_FLAG_ACTIVE | (u32ActiveIndex << orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING), orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING);

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_INPUT, orxINPUT_EVENT_ON, orxNULL, orxNULL, &stPayload);
        }
        else
        {
          /* Updates status */
          orxFLAG_SET(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_NONE, orxINPUT_KU32_ENTRY_FLAG_NEW_STATUS);
        }
      }
      else
      {
        /* Was active? */
        if(orxFLAG_TEST(pstEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_ACTIVE))
        {
          orxU32                  u32LastActiveIndex;
          orxINPUT_EVENT_PAYLOAD  stPayload;

          /* Gets last active index */
          u32LastActiveIndex = (pstEntry->u32Status & orxINPUT_KU32_ENTRY_MASK_LAST_ACTIVE_BINDING) >> orxINPUT_KU32_ENTRY_SHIFT_LAST_ACTIVE_BINDING;

          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxINPUT_EVENT_PAYLOAD));
          stPayload.zSetName    = sstInput.pstCurrentSet->zName;
          stPayload.zInputName  = pstEntry->zName;
          stPayload.eType       = pstEntry->astBindingList[u32LastActiveIndex].eType;
          stPayload.eID         = pstEntry->astBindingList[u32LastActiveIndex].eID;
          stPayload.fValue      = pstEntry->astBindingList[u32LastActiveIndex].fValue;

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

  return;
}

/** Creates an entry
 * @param[in] _zEntryName       Entry name to create
 * @return orxINPUT_ENTRY
 */
orxINPUT_ENTRY *orxFASTCALL orxInput_CreateEntry(orxCONST orxSTRING _zEntryName)
{
  orxINPUT_ENTRY *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstInput.pstCurrentSet != orxNULL);
  orxASSERT(_zEntryName != orxNULL);

  /* Valid? */
  if(_zEntryName != orxSTRING_EMPTY)
  {
    /* Allocates entry */
    pstResult = (orxINPUT_ENTRY *)orxBank_Allocate(sstInput.pstCurrentSet->pstBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxU32 i;

      /* Inits it */
      pstResult->zName      = orxString_Duplicate(_zEntryName);
      pstResult->u32ID      = orxString_ToCRC(_zEntryName);
      pstResult->u32Status  = orxINPUT_KU32_ENTRY_FLAG_NONE;
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        pstResult->astBindingList[i].eType = orxINPUT_TYPE_NONE;
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
orxVOID orxFASTCALL orxInput_DeleteEntry(orxINPUT_SET *_pstSet, orxINPUT_ENTRY *_pstEntry)
{
  /* Checks */
  orxASSERT(_pstSet != orxNULL);
  orxASSERT(_pstEntry != orxNULL);

  /* Deletes its name */
  orxString_Delete(_pstEntry->zName);

  /* Deletes it */
  orxBank_Free(_pstSet->pstBank, _pstEntry);

  return;
}

/** Creates a set
 * @param[in] _zSetName         Name of the set to create
 * @param[in] _u32SetID         ID of the set to create
 */
orxSTATIC orxINLINE orxINPUT_SET *orxInput_CreateSet(orxCONST orxSTRING _zSetName, orxU32 _u32SetID)
{
  orxINPUT_SET *pstResult;

  /* Checks */
  orxASSERT(_zSetName != orxNULL);
  orxASSERT(_zSetName != orxSTRING_EMPTY);

  /* Allocates it */
  pstResult = orxBank_Allocate(sstInput.pstSetBank);

  /* Valid? */
  if(pstResult != orxNULL)
  {
    /* Creates its bank */
    pstResult->pstBank = orxBank_Create(orxINPUT_KU32_ENTRY_BANK_SIZE, sizeof(orxINPUT_ENTRY), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstResult->pstBank != orxNULL)
    {
      /* Duplicates its name */
      pstResult->zName = orxString_Duplicate(_zSetName);

      /* Valid? */
      if(pstResult->zName != orxNULL)
      {
        /* Sets its ID */
        pstResult->u32ID = _u32SetID;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Duplicating set name failed.");

        /* Deletes its bank */
        orxBank_Delete(pstResult->pstBank);

        /* Deletes it */
        orxBank_Free(sstInput.pstSetBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Failed to create input bank.");

      /* Deletes the set */
      orxBank_Free(sstInput.pstSetBank, pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes a set
 * @param[in] _pstSet       Set to delete
 */
orxSTATIC orxINLINE orxVOID orxInput_DeleteSet(orxINPUT_SET *_pstSet)
{
  orxINPUT_ENTRY *pstEntry;

  /* Checks */
  orxASSERT(_pstSet != orxNULL);

  /* While there is still an entry */
  while((pstEntry = orxBank_GetNext(_pstSet->pstBank, orxNULL)) != orxNULL)
  {
    /* Deletes entry */
    orxInput_DeleteEntry(_pstSet, pstEntry);
  }

  /* Is the current selected one? */
  if(sstInput.pstCurrentSet == _pstSet)
  {
    /* Deselects it */
    sstInput.pstCurrentSet = orxNULL;
  }

  /* Removes set */
  orxBank_Free(sstInput.pstSetBank, _pstSet);

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Input module setup
 */
orxVOID orxInput_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_INPUT, orxMODULE_ID_EVENT);
  orxModule_AddOptionalDependency(orxMODULE_ID_INPUT, orxMODULE_ID_KEYBOARD);
  orxModule_AddOptionalDependency(orxMODULE_ID_INPUT, orxMODULE_ID_MOUSE);
  orxModule_AddOptionalDependency(orxMODULE_ID_INPUT, orxMODULE_ID_JOYSTICK);

  return;
}

/** Inits the input module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxInput_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY))
  {
    /* Creates set banks */
    sstInput.pstSetBank = orxBank_Create(orxINPUT_KU32_SET_BANK_SIZE, sizeof(orxINPUT_SET), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstInput.pstSetBank != orxNULL)
    {
      orxCLOCK *pstClock;

      /* Gets core clock */
      pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

      /* Valid? */
      if(pstClock != orxNULL)
      {
        /* Registers to core clock */
        eResult = orxClock_Register(pstClock, orxInput_Update, orxNULL, orxMODULE_ID_INPUT, orxCLOCK_PRIORITY_HIGHER);

        /* Succesful? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Updates flags */
          orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY, orxINPUT_KU32_STATIC_FLAG_NONE);

          /* Loads from input */
          orxInput_Load(orxSTRING_EMPTY);
        }
        else
        {
          /* Deletes clock */
          orxClock_Delete(pstClock);

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
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize input module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the input module
 */
orxVOID orxInput_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY))
  {
    orxINPUT_SET *pstSet;

    /* While there's still a set */
    while((pstSet = orxBank_GetNext(sstInput.pstSetBank, orxNULL)) != orxNULL)
    {
      /* Deletes it */
      orxInput_DeleteSet(pstSet);
    }

    /* Clears sets bank */
    sstInput.pstSetBank = orxNULL;

    /* Updates flags */
    orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_NONE, orxINPUT_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Loads inputs from config file
 * @param[in] _zFileName        File name to load, will use current loaded config if orxSTRING_EMPTY/orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxInput_Load(orxCONST orxSTRING _zFileName)
{
  orxSTRING zPreviousSection;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

  /* Valid name? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY))
  {
    /* Loads it */
    orxConfig_Load(_zFileName);
  }

  /* Gets previous config section */
  zPreviousSection = orxConfig_GetCurrentSection();

  /* Selects input section */
  if((orxConfig_HasSection(orxINPUT_KZ_CONFIG_SECTION) != orxFALSE)
  && (orxConfig_SelectSection(orxINPUT_KZ_CONFIG_SECTION) != orxSTATUS_FAILURE))
  {
    /* Gets joystick threshold */
    sstInput.fJoystickAxisThreshold = orxConfig_GetFloat(orxINPUT_KZ_CONFIG_JOYSTICK_THRESHOLD);

    /* Has list set */
    if(orxConfig_HasValue(orxINPUT_KZ_CONFIG_SET_LIST) != orxFALSE)
    {
      orxU32 i, u32Number;

      /* For all sets */
      for(i = 0, u32Number = orxConfig_GetListCounter(orxINPUT_KZ_CONFIG_SET_LIST); i < u32Number; i++)
      {
        orxINPUT_SET *pstSet;

        /* Loads it */
        if(((pstSet = orxInput_LoadSet(orxConfig_GetListString(orxINPUT_KZ_CONFIG_SET_LIST, i))) != orxNULL)
        && (eResult == orxSTATUS_FAILURE))
        {
          /* Sets it as current set */
          sstInput.pstCurrentSet = pstSet;

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);
  }

  /* Done! */
  return eResult;
}

/** Saves inputs to input
 * @param[in] _zFileName        File name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL  orxInput_Save(orxCONST orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zFileName != orxNULL);

  /* Valid? */
  if(_zFileName != orxSTRING_EMPTY)
  {
    orxSTRING zPreviousSection;

    /* Stores previous section */
    zPreviousSection = orxConfig_GetCurrentSection();

    /* Clears input section */
    orxConfig_ClearSection(orxINPUT_KZ_CONFIG_SECTION);

    /* Selects it */
    if(orxConfig_SelectSection(orxINPUT_KZ_CONFIG_SECTION) != orxSTATUS_FAILURE)
    {
      orxU32        u32Index, u32Counter;
      orxINPUT_SET *pstSet;
      orxSTRING    *azSetNameList;

      /* Gets set counter */
      u32Counter = orxBank_GetCounter(sstInput.pstSetBank);

      /* Allocates set name list */
      azSetNameList = (orxSTRING *)orxMemory_Allocate(u32Counter * sizeof(orxSTRING), orxMEMORY_TYPE_TEMP);

      /* For all sets */
      for(pstSet = orxBank_GetNext(sstInput.pstSetBank, orxNULL), u32Index = 0;
          pstSet != orxNULL;
          pstSet = orxBank_GetNext(sstInput.pstSetBank, pstSet), u32Index++)
      {
        /* Checks */
        orxASSERT(u32Index < u32Counter);

        /* Adds name to list */
        *(azSetNameList + u32Index) = pstSet->zName;
      }

      /* Adds set list to config */
      orxConfig_SetStringList(orxINPUT_KZ_CONFIG_SET_LIST, azSetNameList, u32Counter);        

      /* Frees set name list memory */
      orxMemory_Free(azSetNameList);

      /* Adds joystick threshold */
      orxConfig_SetFloat(orxINPUT_KZ_CONFIG_JOYSTICK_THRESHOLD, sstInput.fJoystickAxisThreshold);

      /* For all sets */
      for(pstSet = orxBank_GetNext(sstInput.pstSetBank, orxNULL);
          pstSet != orxNULL;
          pstSet = orxBank_GetNext(sstInput.pstSetBank, pstSet))
      {
        orxINPUT_ENTRY *pstEntry;

        /* Clears and selects its section */
        orxConfig_ClearSection(pstSet->zName);
        orxConfig_SelectSection(pstSet->zName);

        /* For all its entries */
        for(pstEntry = orxBank_GetNext(pstSet->pstBank, orxNULL);
            pstEntry != orxNULL;
            pstEntry = orxBank_GetNext(pstSet->pstBank, pstEntry))
        {
          orxU32 i;

          /* For all bindings */
          for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
          {
            /* Valid? */
            if(pstEntry->astBindingList[i].eType != orxINPUT_TYPE_NONE)
            {
              /* Adds it to config */
              orxConfig_SetString(orxInput_GetBindingName(pstEntry->astBindingList[i].eType, pstEntry->astBindingList[i].eID), pstEntry->zName);
            }
          }
        }
      }
    }

    /* Restores previous section */
    orxConfig_SelectSection(zPreviousSection);

    /* Saves it */
    eResult = orxConfig_Save(_zFileName, orxFALSE, orxInput_SaveCallback);
  }

  /* Done! */
  return eResult;
}

/** Selects current working set
 * @param[in] _zSetnName        Set name to select
 */
orxSTATUS orxFASTCALL orxInput_SelectSet(orxCONST orxSTRING _zSetName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zSetName != orxNULL);

  /* Valid? */
  if(_zSetName != orxSTRING_EMPTY)
  {
    orxINPUT_SET *pstSet;
    orxU32        u32SetID;

    /* Gets the set ID */
    u32SetID = orxString_ToCRC(_zSetName);

    /* Not already selected? */
    if((sstInput.pstCurrentSet == orxNULL)
    || (sstInput.pstCurrentSet->u32ID != u32SetID))
    {
      /* For all the sets */
      for(pstSet = orxBank_GetNext(sstInput.pstSetBank, orxNULL);
          pstSet != orxNULL;
          pstSet = orxBank_GetNext(sstInput.pstSetBank, pstSet))
      {
        /* Found? */
        if(pstSet->u32ID == u32SetID)
        {
          /* Selects it */
          sstInput.pstCurrentSet = pstSet;

          break;
        }
      }
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
      pstSet = orxInput_CreateSet(_zSetName, u32SetID);

      /* Success? */
      if(pstSet != orxNULL)
      {
        /* Selects it */
        sstInput.pstCurrentSet = pstSet;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Failed to create input set with parameters (%s, %ld).", _zSetName, u32SetID);
      }
    }

    /* Updates result */
    eResult = (sstInput.pstCurrentSet != orxNULL) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets current working set
 * @return Current selected set
 */
orxSTRING orxInput_GetCurrentSet()
{
  orxSTRING zResult = orxSTRING_EMPTY;

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

/** Is input active?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_IsActive(orxCONST orxSTRING _zInputName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_zInputName != orxSTRING_EMPTY))
  {
    orxINPUT_ENTRY *pstEntry;
    orxU32          u32EntryID;

    /* Gets its ID */
    u32EntryID = orxString_ToCRC(_zInputName);

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        pstEntry != orxNULL;
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      /* Found? */
      if(pstEntry->u32ID == u32EntryID)
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

/** Has a new active status since this frame?
 * @param[in] _zInputName       Concerned input name
 * @return orxTRUE if active status is new, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxInput_HasNewStatus(orxCONST orxSTRING _zInputName)
{
  orxBOOL bResult = orxFALSE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_zInputName != orxSTRING_EMPTY))
  {
    orxINPUT_ENTRY *pstEntry;
    orxU32          u32EntryID;

    /* Gets its ID */
    u32EntryID = orxString_ToCRC(_zInputName);

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        pstEntry != orxNULL;
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      /* Found? */
      if(pstEntry->u32ID == u32EntryID)
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
orxFLOAT orxFASTCALL orxInput_GetValue(orxCONST orxSTRING _zInputName)
{
  orxFLOAT fResult = orxFLOAT_0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zInputName != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_zInputName != orxSTRING_EMPTY))
  {
    orxINPUT_ENTRY *pstEntry;
    orxU32          u32EntryID;

    /* Gets its ID */
    u32EntryID = orxString_ToCRC(_zInputName);

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        pstEntry != orxNULL;
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      /* Found? */
      if(pstEntry->u32ID == u32EntryID)
      {
        orxU32 i;

        /* For all bindings */
        for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
        {
          /* Valid & active? */
          if((pstEntry->astBindingList[i].eID != orxINPUT_TYPE_NONE)
          && (orxMath_Abs(pstEntry->astBindingList[i].fValue) > pstEntry->astBindingList[i].fThreshold))
          {
            /* Updates result */
            fResult = pstEntry->astBindingList[i].fValue;
            break;
          }
        }

        break;
      }
    }
  }

  /* Done! */
  return fResult;
}

/** Binds an input to a mouse/joystick button, keyboard key or joystick axis
 * @param[in] _zInputName       Concerned input name
 * @param[in] _eType            Type of peripheral to bind
 * @param[in] _eID              ID of button/key/axis to bind
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Bind(orxCONST orxSTRING _zName, orxINPUT_TYPE _eType, orxENUM _eID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxINPUT_ENTRY *pstEntry, *pstSelectedEntry = orxNULL;
    orxBOOL         bAlreadyBound = orxFALSE;
    orxU32          u32EntryID;

    /* Gets entry ID */
    u32EntryID = orxString_ToCRC(_zName);

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        (pstEntry != orxNULL) && (bAlreadyBound == orxFALSE);
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Is already bound to this? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType))
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input [%s::%s]: can't bind <%s> as it's already bound to input [%s::%s].", sstInput.pstCurrentSet->zName, _zName, orxInput_GetBindingName(_eType, _eID), sstInput.pstCurrentSet->zName, pstEntry->zName);

          /* Updates status */
          bAlreadyBound = orxTRUE;

          break;
        }
      }

      /* Found? */
      if(pstEntry->u32ID == u32EntryID)
      {
        /* Updates selection */
        pstSelectedEntry = pstEntry;
      }
    }

    /* Not already bound? */
    if(bAlreadyBound == orxFALSE)
    {
      /* Entry not found? */
      if(pstSelectedEntry == orxNULL)
      {
        /* Creates a new entry */
        pstSelectedEntry = orxInput_CreateEntry(_zName);
      }

      /* Valid? */
      if(pstSelectedEntry != orxNULL)
      {
        orxU32 u32OldestIndex;

        /* Gets oldest binding index */
        u32OldestIndex = pstSelectedEntry->u32Status & orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING;

        /* Checks */
        orxASSERT(u32OldestIndex < orxINPUT_KU32_BINDING_NUMBER);

        /* Had a previous binding? */
        if(pstSelectedEntry->astBindingList[u32OldestIndex].eType != orxINPUT_TYPE_NONE)
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input [%s::%s]: replacing <%s> with <%s>", sstInput.pstCurrentSet->zName, pstSelectedEntry->zName, orxInput_GetBindingName(pstSelectedEntry->astBindingList[u32OldestIndex].eType, pstSelectedEntry->astBindingList[u32OldestIndex].eID), orxInput_GetBindingName(_eType, _eID));
        }

        /* Updates binding */
        pstSelectedEntry->astBindingList[u32OldestIndex].eType      = _eType;
        pstSelectedEntry->astBindingList[u32OldestIndex].eID        = _eID;
        pstSelectedEntry->astBindingList[u32OldestIndex].fThreshold = (_eType == orxINPUT_TYPE_JOYSTICK_AXIS) ? sstInput.fJoystickAxisThreshold : orxFLOAT_0;
        pstSelectedEntry->astBindingList[u32OldestIndex].fValue     = orxFLOAT_0;

        /* Gets new oldest index */
        u32OldestIndex = (u32OldestIndex + 1) % orxINPUT_KU32_BINDING_NUMBER;

        /* Updates status */
        orxFLAG_SET(pstSelectedEntry->u32Status, orxINPUT_KU32_ENTRY_FLAG_BOUND | u32OldestIndex, orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Can't create an input [%s::%s].", sstInput.pstCurrentSet->zName, _zName);
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Unbinds a mouse/joystick button, keyboard key or joystick axis
 * @param[in] _eType            Type of peripheral to unbind
 * @param[in] _eID              ID of button/key/axis to unbind
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxInput_Unbind(orxINPUT_TYPE _eType, orxENUM _eID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);

  /* Valid? */
  if(sstInput.pstCurrentSet != orxNULL)
  {
    orxINPUT_ENTRY *pstEntry;

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        (eResult == orxSTATUS_FAILURE) && (pstEntry != orxNULL);
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Found? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType))
        {
          orxU32  j;
          orxBOOL bBound = orxFALSE;

          /* Updates binding */
          pstEntry->astBindingList[i].eType = orxINPUT_TYPE_NONE;

          /* For all entries bindings */
          for(j = 0; j < orxINPUT_KU32_BINDING_NUMBER; j++)
          {
            /* Bound? */
            if(pstEntry->astBindingList[j].eType != orxINPUT_TYPE_NONE)
            {
              /* Updates binding status */
              bBound = orxTRUE;
              break;
            }
          }

          /* Still bound? */
          if(bBound != orxFALSE)
          {
            /* Updates status */
            orxFLAG_SET(pstEntry->u32Status, i, orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING);
          }
          else
          {
            /* Updates status */
            orxFLAG_SET(pstEntry->u32Status, i, orxINPUT_KU32_ENTRY_MASK_OLDEST_BINDING | orxINPUT_KU32_ENTRY_FLAG_BOUND);
          }

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;

          break;
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets the input name to which a mouse/joystick button, keyboard key or joystick axis is bound
 * @param[in] _eType            Type of peripheral to test
 * @param[in] _eID              ID of button/key/axis to test
 * @return orxSTRING input name if bound / orxSTRING_EMPY otherwise
 */
orxSTRING orxFASTCALL orxInput_GetBoundInput(orxINPUT_TYPE _eType, orxENUM _eID)
{
  orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_eType < orxINPUT_TYPE_NUMBER);

  /* Valid? */
  if(sstInput.pstCurrentSet != orxNULL)
  {
    orxINPUT_ENTRY *pstEntry;

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        (zResult == orxSTRING_EMPTY) && (pstEntry != orxNULL);
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      orxU32 i;

      /* For all bindings */
      for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
      {
        /* Found? */
        if((pstEntry->astBindingList[i].eID == _eID) && (pstEntry->astBindingList[i].eType == _eType))
        {
          /* Updates result */
          zResult = pstEntry->zName;

          break;
        }
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) at a given index
 * @param[in]   _zName           Concerned input name
 * @param[in]   _u32BindingIndex Index of the desired binding
 * @param[out]  _peType          List of binding types (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out]  _peID            List of binding IDs (button/key/axis)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetBinding(orxCONST orxSTRING _zName, orxU32 _u32BindingIndex, orxINPUT_TYPE *_peType, orxENUM *_peID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_u32BindingIndex < orxINPUT_KU32_BINDING_NUMBER);
  orxASSERT(_peType != orxNULL);
  orxASSERT(_peID != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxINPUT_ENTRY *pstEntry;
    orxU32          u32EntryID;

    /* Gets entry ID */
    u32EntryID = orxString_ToCRC(_zName);

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        pstEntry != orxNULL;
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      /* Found? */
      if(pstEntry->u32ID == u32EntryID)
      {
        /* Updates result */
        *_peType  = pstEntry->astBindingList[_u32BindingIndex].eType;
        *_peID    = pstEntry->astBindingList[_u32BindingIndex].eID;
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
  }

  /* Done! */
  return eResult;
}

/** Gets an input binding (mouse/joystick button, keyboard key or joystick axis) list
 * @param[in] _zName            Concerned input name
 * @param[out] _aeTypeList      List of binding types (if a slot is not bound, its value is orxINPUT_TYPE_NONE)
 * @param[out] _aeIDList        List of binding IDs (button/key/axis)
 * @return orxSTATUS_SUCCESS if input exists, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetBindingList(orxCONST orxSTRING _zName, orxINPUT_TYPE _aeTypeList[orxINPUT_KU32_BINDING_NUMBER], orxENUM _aeIDList[orxINPUT_KU32_BINDING_NUMBER])
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_aeTypeList != orxNULL);
  orxASSERT(_aeIDList != orxNULL);

  /* Valid? */
  if((sstInput.pstCurrentSet != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxINPUT_ENTRY *pstEntry;
    orxU32          u32EntryID;

    /* Gets entry ID */
    u32EntryID = orxString_ToCRC(_zName);

    /* For all entries */
    for(pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, orxNULL);
        pstEntry != orxNULL;
        pstEntry = orxBank_GetNext(sstInput.pstCurrentSet->pstBank, pstEntry))
    {
      /* Found? */
      if(pstEntry->u32ID == u32EntryID)
      {
        orxU32 i;

        /* For all bindings */
        for(i = 0; i < orxINPUT_KU32_BINDING_NUMBER; i++)
        {
          /* Updates result */
          _aeTypeList[i] = pstEntry->astBindingList[i].eType;
          _aeIDList[i] = pstEntry->astBindingList[i].eType;
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
      _aeTypeList[i] = orxINPUT_TYPE_NONE;
      _aeIDList[i] = orxENUM_NONE;
    }
  }

  /* Done! */
  return eResult;
}

/** Gets a binding name
 * @param[in]   _eType          Binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[in]   _eID            Binding ID (ID of button/key/axis to bind)
 * @return orxSTRING (binding's name) if success, orxSTRING_EMPTY otherwise
 */
orxSTRING orxFASTCALL orxInput_GetBindingName(orxINPUT_TYPE _eType, orxENUM _eID)
{
  orxSTRING zResult = orxSTRING_EMPTY;

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

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_INPUT, "Input type <%ld> is not recognized!", _eType);

      break;
    }
  }

  /* Done! */
  return zResult;
}

/** Gets active binding (current pressed key/button/...) so as to allow on-the-fly user rebinding
 * @param[out]  _peType         Active binding type (mouse/joystick button, keyboard key or joystick axis)
 * @param[out]  _peID           Active binding ID (ID of button/key/axis to bind)
 * @return orxSTATUS_SUCCESS if one active binding is found, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxInput_GetActiveBinding(orxINPUT_TYPE *_peType, orxENUM *_peID)
{
  orxINPUT_TYPE eType;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));
  orxASSERT(_peType != orxNULL);
  orxASSERT(_peID != orxNULL);

  /* For all input types */
  for(eType = 0; (eResult == orxSTATUS_FAILURE) && (eType < orxINPUT_TYPE_NUMBER); eType++)
  {
    orxENUM   eID;
    orxSTRING zBinding = orxNULL;

    /* For all bindings */
    for(eID = 0; zBinding != orxSTRING_EMPTY; eID++)
    {
      /* Gets binding name */
      zBinding = orxInput_GetBindingName(eType, eID);

      /* Valid? */
      if(zBinding != orxSTRING_EMPTY)
      {
        orxBOOL bActive;

        /* Joystick axis? */
        if(eType == orxINPUT_TYPE_JOYSTICK_AXIS)
        {
          /* Updates active status */
          bActive = (orxMath_Abs(orxInput_GetBindingValue(eType, eID)) > sstInput.fJoystickAxisThreshold) ? orxTRUE : orxFALSE;
        }
        else
        {
          /* Updates active status */
          bActive = (orxMath_Abs(orxInput_GetBindingValue(eType, eID)) > orxFLOAT_0) ? orxTRUE : orxFALSE;
        }

        /* Active? */
        if(bActive != orxFALSE)
        {
          /* Updates result */
          *_peType  = eType;
          *_peID    = eID;
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
  }

  /* Done! */
  return eResult;
}

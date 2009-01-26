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
#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxINPUT_KU32_STATIC_FLAG_NONE            0x00000000  /**< No flags */

#define orxINPUT_KU32_STATIC_FLAG_READY           0x00000001  /**< Ready flag */

#define orxINPUT_KU32_STATIC_MASK_ALL             0xFFFFFFFF  /**< All mask */


/** Defines
 */
#define orxINPUT_KZ_INPUT_SECTION                "Input"     /**< Input set name */
#define orxINPUT_KZ_INPUT_SET_LIST               "SetList"   /**< Input set list */


#define orxINPUT_KU32_MAX_BINDING_NUMBER          2
#define orxINPUT_KU32_SET_BANK_SIZE               4
#define orxINPUT_KU32_ENTRY_BANK_SIZE             8


#define orxINPUT_KU32_ENTRY_FLAG_NONE             0x00000000  /**< No flags */

#define orxINPUT_KU32_ENTRY_FLAG_ACTIVE           0x00000001  /**< Active flags */
#define orxINPUT_KU32_ENTRY_FLAG_BOUND            0x00000002  /**< Bound flags */

#define orxINPUT_KU32_ENTRY_MASK_ALL              0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Input value structure
 */
typedef struct __orxINPUT_VALUE_t
{
  orxINPUT_TYPE   eType;                                        /**< Input type : 4 */
  orxENUM         eID;                                          /**< Input ID : 8 */
  orxFLOAT        fThreshold;                                   /**< Threshold : 12 */
  orxFLOAT        fCurrentValue;                                /**< Current value : 16 */

  orxPAD(16)

} orxINPUT_VALUE;

/** Input entry structure
 */
typedef struct __orxINPUT_ENTRY_t
{
  orxSTRING         zName;                                      /**< Entry name : 4 */
  orxU32            u32ID;                                      /**< Name ID (CRC) : 8 */
  orxU32            u32Status;                                  /**< Entry status : 12 */

  orxINPUT_VALUE   astValue[orxINPUT_KU32_MAX_BINDING_NUMBER];  /**< Entry value : 40 */

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

/* Save filter callback */
orxBOOL orxFASTCALL orxInput_SaveCallback(orxCONST orxSTRING _zSetName, orxCONST orxSTRING _zKeyName, orxBOOL _bUseEncryption)
{
  orxBOOL bResult = orxFALSE;

  /* Is it the input set? */
  if(orxString_Compare(_zSetName, orxINPUT_KZ_INPUT_SECTION) == 0)
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
  //! TODO
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
      pstResult->zName = orxString_Duplicate(_zEntryName);
      pstResult->u32ID = orxString_ToCRC(_zEntryName);
      for(i = 0; i < orxINPUT_KU32_MAX_BINDING_NUMBER; i++)
      {
        pstResult->astValue[i].eType = orxINPUT_TYPE_NONE;
      }
      pstResult->u32Status = orxINPUT_KU32_ENTRY_FLAG_NONE;
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
          /* Loads from input */
          orxInput_Load();

          /* Updates flags */
          orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY, orxINPUT_KU32_STATIC_FLAG_NONE);
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

    /* While there's still a section */
    while((pstSet = orxBank_GetNext(sstInput.pstSetBank, orxNULL)) != orxNULL)
    {
      /* Deletes it */
      orxInput_DeleteSet(pstSet);
    }

    /* Clears its bank */
    orxBank_Delete(pstSet->pstBank);
    sstInput.pstSetBank = orxNULL;

    /* Updates flags */
    orxFLAG_SET(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_NONE, orxINPUT_KU32_STATIC_MASK_ALL);
  }

  return;
}

/** Loads inputs from input
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxInput_Load()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  //! TODO

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

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

  //! TODO

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstInput.u32Flags, orxINPUT_KU32_STATIC_FLAG_READY));

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

  //! TODO
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

  //! TODO
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

  //! TODO
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

  //! TODO
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

  //! TODO
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

  //! TODO
  return eResult;
}

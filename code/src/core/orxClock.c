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
 * @file orxClock.c
 * @date 04/04/2004
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxClock.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"
#include "utils/orxLinkList.h"
#include "utils/orxHashTable.h"


/** Module flags
 */
#define orxCLOCK_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags */

#define orxCLOCK_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag */

#define orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK   0x10000000  /**< Lock update flag */

#define orxCLOCK_KU32_STATIC_MASK_ALL           0xFFFFFFFF  /**< All mask */


/** orxCLOCK flags
 */
#define orxCLOCK_KU32_FLAG_NONE                 0x00000000  /**< No flags */

#define orxCLOCK_KU32_FLAG_PAUSED               0x10000000  /**< Clock is paused */
#define orxCLOCK_KU32_FLAG_REFERENCED           0x20000000  /**< Referenced flag */

#define orxCLOCK_KU32_MASK_ALL                  0xFFFFFFFF  /**< All mask */


/** Misc
 */
#define orxCLOCK_KZ_CONFIG_SECTION              "Clock"
#define orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY "MainClockFrequency"
#define orxCLOCK_KZ_CONFIG_FREQUENCY            "Frequency"
#define orxCLOCK_KZ_CONFIG_MODIFIER_TYPE        "ModifierType"
#define orxCLOCK_KZ_CONFIG_MODIFIER_VALUE       "ModifierValue"

#define orxCLOCK_KZ_MODIFIER_CAPPED             "capped"
#define orxCLOCK_KZ_MODIFIER_FIXED              "fixed"
#define orxCLOCK_KZ_MODIFIER_MULTIPLY           "multiply"

#define orxCLOCK_KU32_REFERENCE_TABLE_SIZE      8           /**< Reference table size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Clock function storage structure
 */
typedef struct __orxCLOCK_FUNCTION_STORAGE_t
{
  orxLINKLIST_NODE            stNode;           /**< Linklist node : 12 */
  orxCLOCK_FUNCTION           pfnCallback;      /**< Clock function pointer : 16 */
  void                       *pstContext;       /**< Clock function context : 20 */
  orxMODULE_ID                eModuleID;        /**< Clock function module ID : 24 */
  orxCLOCK_PRIORITY           ePriority;        /**< Clock function priority : 28 */

  orxPAD(28)

} orxCLOCK_FUNCTION_STORAGE;

/** Clock structure
 */
struct __orxCLOCK_t
{
  orxLINKLIST_NODE  stNode;                     /**< Clock list node : 12 */
  orxCLOCK_INFO     stClockInfo;                /**< Clock Info Structure : 36 */
  orxFLOAT          fPartialDT;                 /**< Clock partial DT : 40 */
  orxBANK          *pstFunctionBank;            /**< Function bank : 44 */
  orxLINKLIST       stFunctionList;             /**< Function list : 56 */
  orxSTRING         zReference;                 /**< Reference : 60 */
  orxU32            u32Flags;                   /**< Clock flags : 64 */

  orxPAD(64)
};


/** Static structure
 */
typedef struct __orxCLOCK_STATIC_t
{
  orxBANK          *pstClockBank;               /**< Clock banks : 4 */
  orxCLOCK_MOD_TYPE eModType;                   /**< Clock mod type : 8 */
  orxFLOAT          fModValue;                  /**< Clock mod value : 12 */
  orxFLOAT          fTime;                      /**< Current time : 16 */
  orxU32            u32Flags;                   /**< Control flags : 20 */
  orxLINKLIST       stClockList;                /**< Clock list : 32 */
  orxHASHTABLE     *pstReferenceTable;          /**< Table to avoid clock duplication when creating through config file : 36 */

} orxCLOCK_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxCLOCK_STATIC sstClock;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Finds a clock function storage
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @return      orxCLOCK_FUNCTION_STORAGE / orxNULL
 */
static orxINLINE orxCLOCK_FUNCTION_STORAGE *orxClock_FindFunctionStorage(const orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds matching function storage */
  for(pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetFirst(&(_pstClock->stFunctionList));
      pstFunctionStorage != orxNULL;
      pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetNext(&(pstFunctionStorage->stNode)))
  {
    /* Match? */
    if(pstFunctionStorage->pfnCallback == _pfnCallback)
    {
      /* Found */
      break;
    }
  }

  /* Done! */
  return pstFunctionStorage;
}

/** Finds the next clock in list given a tick size and a type
 * @param[in]   _fTickSize                            Desired tick size
 * @param[in]   _eType                                Desired type
 * @param[in]   _pstStartClock                        Clock used as a starting point in the list
 * @return      orxCLOCK / orxNULL
 */
static orxINLINE orxCLOCK *orxClock_FindClock(orxFLOAT _fTickSize, orxCLOCK_TYPE _eType, const orxCLOCK *_pstStartClock)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eType < orxCLOCK_TYPE_NUMBER);

  /* Finds matching clock */
  for(pstClock = (_pstStartClock != orxNULL) ? (orxCLOCK *)orxLinkList_GetNext(&(_pstStartClock->stNode)) : (orxCLOCK *)orxLinkList_GetFirst(&(sstClock.stClockList));
      pstClock != orxNULL;
      pstClock = (orxCLOCK *)orxLinkList_GetNext(&(pstClock->stNode)))
  {
    /* Match? */
    if((pstClock->stClockInfo.eType == _eType)
    && ((_fTickSize < orxFLOAT_0)
     || (pstClock->stClockInfo.fTickSize == _fTickSize)))
    {
      /* Found */
      break;
    }
  }

  /* Done! */
  return pstClock;
}

/** Computes DT according to modifier
 * @param[in]   _fDT                                  Real DT
 * @param[in]   _pstClockInfo                         Concerned clock info
 * @return      Modified DT
 */
static orxINLINE orxFLOAT orxClock_ComputeDT(orxFLOAT _fDT, const orxCLOCK_INFO *_pstClockInfo)
{
  register const orxCLOCK_MOD_TYPE *peModType;
  register const orxFLOAT          *pfModValue;
  register orxFLOAT                 fResult;

  /* Using global one? */
  if(_pstClockInfo == orxNULL)
  {
    peModType   = &(sstClock.eModType);
    pfModValue  = &(sstClock.fModValue);
  }
  /* Using clock one */
  else
  {
    peModType   = &(_pstClockInfo->eModType);
    pfModValue  = &(_pstClockInfo->fModValue);
  }

  /* Depending on modifier type */
  switch(*peModType)
  {
    case orxCLOCK_MOD_TYPE_FIXED:
    {
      /* Fixed DT value */
      fResult = *pfModValue;
      break;
    }

    case orxCLOCK_MOD_TYPE_MULTIPLY:
    {
      /* Multiplied DT value */
      fResult = *pfModValue * _fDT;
      break;
    }

    case orxCLOCK_MOD_TYPE_MAXED:
    {
      /* Updates DT value */
      fResult = orxMIN(*pfModValue, _fDT);
      break;
    }

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Invalid clock modifier type (%ld).", *peModType);

      /* Falls through */
    }

    case orxCLOCK_MOD_TYPE_NONE:
    {
      /* Gets base DT */
      fResult = _fDT;
      break;
    }
  }

  /* Done! */
  return fResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Clock module setup
 */
void orxFASTCALL orxClock_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_SYSTEM);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_CONFIG);

  return;
}

/** Inits clock module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstClock, sizeof(orxCLOCK_STATIC));

    /* Creates clock bank */
    sstClock.pstClockBank = orxBank_Create(orxCLOCK_KU32_CLOCK_BANK_SIZE, sizeof(orxCLOCK), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstClock.pstClockBank != orxNULL)
    {
      /* Creates reference table */
      sstClock.pstReferenceTable = orxHashTable_Create(orxCLOCK_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstClock.pstReferenceTable != orxNULL)
      {
        /* No mod type by default */
        sstClock.eModType = orxCLOCK_MOD_TYPE_NONE;

        /* Gets init time */
        sstClock.fTime  = orxSystem_GetTime();

        /* Inits Flags */
        sstClock.u32Flags = orxCLOCK_KU32_STATIC_FLAG_READY;

        /* Creates default full speed core clock */
        orxConfig_PushSection(orxCLOCK_KZ_CONFIG_SECTION);
        eResult = (orxClock_Create((orxConfig_HasValue(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY) && orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY) > orxFLOAT_0) ? (orxFLOAT_1 / orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY)) : orxFLOAT_0, orxCLOCK_TYPE_CORE) != orxNULL) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
        orxConfig_PopSection();
      }
      else
      {
        /* Deletes bank */
        orxBank_Delete(sstClock.pstClockBank);
        sstClock.pstClockBank = orxNULL;

        /* Clock bank not created */
        eResult = orxSTATUS_FAILURE;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Failed creating clock bank.");

      /* Clock bank not created */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Tried to initialize clock module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from clock module
 */
void orxFASTCALL orxClock_Exit()
{
  /* Initialized? */
  if(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY)
  {
    orxCLOCK *pstClock;

    /* For all clocks */
    while((pstClock = (orxCLOCK *)orxLinkList_GetFirst(&(sstClock.stClockList))) != orxNULL)
    {
      /* Deletes it */
      orxClock_Delete(pstClock);
    }

    /* Deletes clock bank */
    orxBank_Delete(sstClock.pstClockBank);
    sstClock.pstClockBank = orxNULL;

    /* Deletes reference table */
    orxHashTable_Delete(sstClock.pstReferenceTable);

    /* Updates flags */
    sstClock.u32Flags &= ~orxCLOCK_KU32_STATIC_FLAG_READY;
  }

  return;
}

/** Updates the clock system
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Update()
{
  orxFLOAT  fNewTime, fDT;
  orxCLOCK *pstClock;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Not already locked? */
  if(!(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK))
  {
    /* Lock clocks */
    sstClock.u32Flags |= orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK;

    /* Gets new time */
    fNewTime  = orxSystem_GetTime();

    /* Computes natural DT */
    fDT       = fNewTime - sstClock.fTime;

    /* Gets modified DT */
    fDT       = orxClock_ComputeDT(fDT, orxNULL);

    /* Updates time */
    sstClock.fTime = fNewTime;

    /* For all clocks */
    for(pstClock = (orxCLOCK *)orxLinkList_GetFirst(&(sstClock.stClockList));
        pstClock != orxNULL;
        pstClock = (orxCLOCK *)orxLinkList_GetNext(&(pstClock->stNode)))
    {
      orxFLOAT fClockDT;

      /* Is clock not paused? */
      if(orxClock_IsPaused(pstClock) == orxFALSE)
      {
        /* Updates clock real time & partial DT */
        pstClock->fPartialDT += fDT;

        /* New tick happens? */
        if(pstClock->fPartialDT >= pstClock->stClockInfo.fTickSize)
        {
          orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;

          /* Gets clock modified DT */
          fClockDT = orxClock_ComputeDT(pstClock->fPartialDT, &(pstClock->stClockInfo));

          /* Updates clock DT */
          pstClock->stClockInfo.fDT = fClockDT;

          /* Updates clock time */
          pstClock->stClockInfo.fTime += fClockDT;

          /* For all registered callbacks */
          for(pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetFirst(&(pstClock->stFunctionList));
              pstFunctionStorage != orxNULL;
              pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetNext(&(pstFunctionStorage->stNode)))
          {
            /* Calls it */
            pstFunctionStorage->pfnCallback(&(pstClock->stClockInfo), pstFunctionStorage->pstContext);
          }

          /* Updates partial DT */
          pstClock->fPartialDT = orxFLOAT_0;
        }
      }
    }

    /* Unlocks clocks */
    sstClock.u32Flags &= ~orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK;

    /* Waits for next time slice */
    orxSystem_Delay(orxFLOAT_0);
  }

  /* Done! */
  return eResult;
}

/** Creates a clock
 * @param[in]   _fTickSize                            Tick size for the clock (in seconds)
 * @param[in]   _eType                                Type of the clock
 * @return      orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_Create(orxFLOAT _fTickSize, orxCLOCK_TYPE _eType)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_fTickSize >= orxFLOAT_0);

  /* Creates clock */
  pstClock = (orxCLOCK *)orxBank_Allocate(sstClock.pstClockBank);

  /* Valid? */
  if(pstClock != orxNULL)
  {
    /* Creates function bank */
    pstClock->pstFunctionBank = orxBank_Create(orxCLOCK_KU32_FUNCTION_BANK_SIZE, sizeof(orxCLOCK_FUNCTION_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstClock->pstFunctionBank != orxNULL)
    {
      /* Adds it to list */
      orxMemory_Zero(&(pstClock->stNode), sizeof(orxLINKLIST_NODE));
      orxLinkList_AddEnd(&(sstClock.stClockList), &(pstClock->stNode));

      /* Inits clock */
      pstClock->stClockInfo.fTickSize   = _fTickSize;
      pstClock->stClockInfo.eType       = _eType;
      pstClock->stClockInfo.eModType    = orxCLOCK_MOD_TYPE_NONE;
      pstClock->u32Flags                = orxCLOCK_KU32_FLAG_NONE;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't create clock function storage.");

      /* Releases allocated clock */
      orxBank_Free(sstClock.pstClockBank, pstClock);

      /* Not allocated */
      pstClock = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't allocate bank for clock.");
  }

  /* Done! */
  return pstClock;
}

/** Creates a clock from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxCLOCK *pstResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Search for clock */
  pstResult = orxClock_Get(_zConfigID);

  /* Not already created? */
  if(pstResult == orxNULL)
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      orxFLOAT fFrequency;

      /* Gets its frequency */
      fFrequency = orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_FREQUENCY);

      /* Creates clock */
      pstResult = orxClock_Create((fFrequency > orxFLOAT_0) ? orxFLOAT_1 / fFrequency : orxFLOAT_0, orxCLOCK_TYPE_USER);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Has a modifier? */
        if(orxConfig_HasValue(orxCLOCK_KZ_CONFIG_MODIFIER_TYPE) != orxFALSE)
        {
          orxFLOAT fModifierValue;

          /* Gets its value */
          fModifierValue = orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MODIFIER_VALUE);

          /* Valid? */
          if(fModifierValue > orxFLOAT_0)
          {
            orxSTRING         zModifierType;
            orxCLOCK_MOD_TYPE eModifierType;

            /* Gets modifier type */
            zModifierType = orxString_LowerCase(orxConfig_GetString(orxCLOCK_KZ_CONFIG_MODIFIER_TYPE));

            /* Capped? */
            if(orxString_Compare(zModifierType, orxCLOCK_KZ_MODIFIER_CAPPED) == 0)
            {
              /* Updates modifier value */
              fModifierValue = orxFLOAT_1 / fModifierValue;

              /* Updates modifier type */
              eModifierType = orxCLOCK_MOD_TYPE_MAXED;
            }
            /* Fixed? */
            else if(orxString_Compare(zModifierType, orxCLOCK_KZ_MODIFIER_FIXED) == 0)
            {
              /* Updates modifier value */
              fModifierValue = orxFLOAT_1 / fModifierValue;

              /* Updates modifier type */
              eModifierType = orxCLOCK_MOD_TYPE_FIXED;
            }
            /* Multiply? */
            else if(orxString_Compare(zModifierType, orxCLOCK_KZ_MODIFIER_MULTIPLY) == 0)
            {
              /* Updates modifier type */
              eModifierType = orxCLOCK_MOD_TYPE_MULTIPLY;
            }
            /* None */
            else
            {
              /* Updates modifier type */
              eModifierType = orxCLOCK_MOD_TYPE_NONE;
            }

            /* Updates clock */
            orxClock_SetModifier(pstResult, eModifierType, fModifierValue);
          }
        }

        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Protects it */
        orxConfig_ProtectSection(pstResult->zReference, orxTRUE);

        /* Adds it to reference table */
        orxHashTable_Add(sstClock.pstReferenceTable, orxString_ToCRC(pstResult->zReference), pstResult);

        /* Updates status flags */
        orxFLAG_SET(pstResult->u32Flags, orxCLOCK_KU32_FLAG_REFERENCED, orxCLOCK_KU32_FLAG_NONE);
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Cannot find config section named (%s).", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Delete(orxCLOCK *_pstClock)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Not locked? */
  if((sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK) == orxCLOCK_KU32_FLAG_NONE)
  {
    /* Deletes function bank */
    orxBank_Delete(_pstClock->pstFunctionBank);

    /* Removes it from list */
    orxLinkList_Remove(&(_pstClock->stNode));

    /* Is referenced? */
    if(orxFLAG_TEST(_pstClock->u32Flags, orxCLOCK_KU32_FLAG_REFERENCED))
    {
      /* Removes it from reference table */
      orxHashTable_Remove(sstClock.pstReferenceTable, orxString_ToCRC(_pstClock->zReference));
    }

    /* Has reference? */
    if(_pstClock->zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstClock->zReference, orxFALSE);
    }

    /* Frees clock memory */
    orxBank_Free(sstClock.pstClockBank, _pstClock);
  }

  /* Done! */
  return eResult;
}

/** Resyncs a clock (accumulated DT => 0)
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Resync(orxCLOCK *_pstClock)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Not locked? */
  if(!orxFLAG_TEST(sstClock.u32Flags, orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK))
  {
    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_CLOCK, orxCLOCK_EVENT_RESYNC, _pstClock, orxNULL, orxNULL);

    /* Resyncs clock */
    _pstClock->fPartialDT = orxFLOAT_0;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't resync this clock <%P> as it's currenlty locked/in use.", _pstClock);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Resyncs all clocks (accumulated DT => 0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_ResyncAll()
{
  orxCLOCK *pstClock;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* For all clocks */
  for(pstClock = (orxCLOCK *)orxLinkList_GetFirst(&(sstClock.stClockList));
      pstClock != orxNULL;
      pstClock = (orxCLOCK *)orxLinkList_GetNext(&(pstClock->stNode)))
  {
    /* Not already failed? */
    if(eResult != orxSTATUS_FAILURE)
    {    
      /* Resyncs clock */
      eResult = orxClock_Resync(pstClock);
    }
  }

  /* Done! */
  return eResult;
}

/** Restarts a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Restart(orxCLOCK *_pstClock)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Not locked? */
  if(!orxFLAG_TEST(sstClock.u32Flags, orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK))
  {
    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_CLOCK, orxCLOCK_EVENT_RESTART, _pstClock, orxNULL, orxNULL);

    /* Restarts clock */
    _pstClock->stClockInfo.fTime = _pstClock->stClockInfo.fDT = _pstClock->fPartialDT = orxFLOAT_0;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't restart this clock <%P> as it's currenlty locked/in use.", _pstClock);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Pauses a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Pause(orxCLOCK *_pstClock)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_CLOCK, orxCLOCK_EVENT_PAUSE, _pstClock, orxNULL, orxNULL);

  /* Updates clock flags */
  _pstClock->u32Flags |= orxCLOCK_KU32_FLAG_PAUSED;

  // Done!
  return eResult;
}

/** Unpauses a clock
 * @param[in]   _pstClock                             Concerned clock
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Unpause(orxCLOCK *_pstClock)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_CLOCK, orxCLOCK_EVENT_UNPAUSE, _pstClock, orxNULL, orxNULL);

  /* Updates clock flags */
  _pstClock->u32Flags &= ~orxCLOCK_KU32_FLAG_PAUSED;

  // Done!
  return eResult;
}

/** Is a clock paused?
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxTRUE if paused, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxClock_IsPaused(const orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Tests flags */
  return((_pstClock->u32Flags & orxCLOCK_KU32_FLAG_PAUSED) ? orxTRUE : orxFALSE);
}

/** Gets clock info
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK_INFO / orxNULL
 */
const orxCLOCK_INFO *orxFASTCALL  orxClock_GetInfo(const orxCLOCK *_pstClock)
{
  const orxCLOCK_INFO *pstClockInfo = orxNULL;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Gets the clock info structure pointer */
  pstClockInfo = &(_pstClock->stClockInfo);

  /* Done! */
  return pstClockInfo;
}

/** Sets a clock modifier
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _eModType                             Modifier type
 * @param[in]   _fModValue                            Modifier value
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_SetModifier(orxCLOCK *_pstClock, orxCLOCK_MOD_TYPE _eModType, orxFLOAT _fModValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_eModType < orxCLOCK_MOD_TYPE_NUMBER);

  /* Valid modifier value? */
  if(_fModValue >= orxFLOAT_0)
  {
    /* Updates clock modifier */
    _pstClock->stClockInfo.eModType   = _eModType;
    _pstClock->stClockInfo.fModValue  = _fModValue;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "<%g> is an invalid modifier value.", _fModValue);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a clock tick size
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _fTickSize                            Tick size
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_SetTickSize(orxCLOCK *_pstClock, orxFLOAT _fTickSize)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Valid modifier value? */
  if(_fTickSize >= orxFLOAT_0)
  {
    /* Updates clock tick size*/
    _pstClock->stClockInfo.fTickSize = _fTickSize;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "<%g> is an invalid tick size.", _fTickSize);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Registers a callback function to a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Callback to register
 * @param[in]   _pstContext                           Context that will be transmitted to the callback when called
 * @param[in]   _eModuleID                            ID of the module related to this callback
 * @param[in]   _ePriority                            Priority for the function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Register(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, void *_pstContext, orxMODULE_ID _eModuleID, orxCLOCK_PRIORITY _ePriority)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Gets function slot */
  pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxBank_Allocate(_pstClock->pstFunctionBank);

  /* Valid? */
  if(pstFunctionStorage != orxNULL)
  {
    orxCLOCK_FUNCTION_STORAGE *pstRefFunctionStorage;

    /* Finds correct index */
    for(pstRefFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetFirst(&(_pstClock->stFunctionList));
        pstRefFunctionStorage != orxNULL;
        pstRefFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetNext(&(pstRefFunctionStorage->stNode)))
    {
      /* Higher priority */
      if(_ePriority > pstRefFunctionStorage->ePriority)
      {
        /* Stores it */
        orxLinkList_AddBefore(&(pstRefFunctionStorage->stNode), &(pstFunctionStorage->stNode));
        pstFunctionStorage->ePriority = _ePriority;

        break;
      }
    }

    /* No index found? */
    if(pstRefFunctionStorage == orxNULL)
    {
      /* Stores it at the end */
      orxLinkList_AddEnd(&(_pstClock->stFunctionList), &(pstFunctionStorage->stNode));
      pstFunctionStorage->ePriority = _ePriority;
    }

    /* Stores callback */
    pstFunctionStorage->pfnCallback = _pfnCallback;

    /* Stores context */
    pstFunctionStorage->pstContext  = _pstContext;

    /* Stores module id */
    pstFunctionStorage->eModuleID   = _eModuleID;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't allocate bank for function storage.");

    /* Not successful */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Unregisters a callback function from a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _fnCallback                           Callback to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Unregister(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds clock function storage */
  pstFunctionStorage = orxClock_FindFunctionStorage(_pstClock, _pfnCallback);

  /* Found? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Removes it from list */
    orxLinkList_Remove(&(pstFunctionStorage->stNode));

    /* Removes it from bank */
    orxBank_Free(_pstClock->pstFunctionBank, pstFunctionStorage);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't find clock function storage.");

    /* Not found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a callback function context
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @return      Registered context
 */
void *orxFASTCALL orxClock_GetContext(const orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  void *pstContext = orxNULL;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds clock function storage */
  pstFunctionStorage = orxClock_FindFunctionStorage(_pstClock, _pfnCallback);

  /* Found? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Gets context */
    pstContext = pstFunctionStorage->pstContext;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't find clock function storage.");
  }

  /* Done! */
  return pstContext;
}

/** Sets a callback function context
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @param[in]   _pstContext                           Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_SetContext(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, void *_pstContext)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds clock function storage */
  pstFunctionStorage = orxClock_FindFunctionStorage(_pstClock, _pfnCallback);

  /* Found? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Sets context */
    pstFunctionStorage->pstContext = _pstContext;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't find clock function storage.");

    /* Not found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Finds a clock given its tick size and its type
 * @param[in]   _fTickSize                            Tick size of the desired clock (in seconds)
 * @param[in]   _eType                                Type of the desired clock
 * @return      orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_FindFirst(orxFLOAT _fTickSize, orxCLOCK_TYPE _eType)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eType < orxCLOCK_TYPE_NUMBER);

  /* Finds first matching clock */
  pstClock = orxClock_FindClock(_fTickSize, _eType, orxNULL);

  /* Done! */
  return pstClock;
}

/** Finds next clock of same type/tick size
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_FindNext(const orxCLOCK *_pstClock)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Finds next matching clock */
  pstClock = orxClock_FindClock(_pstClock->stClockInfo.fTickSize, _pstClock->stClockInfo.eType, _pstClock);

  /* Done! */
  return pstClock;
}

/** Gets next existing clock in list (can be used to parse all existing clocks)
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_GetNext(const orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Returns next stored clock */
  return((orxCLOCK *)orxLinkList_GetNext(&(_pstClock->stNode)));
}

/** Gets clock given its name
 * @param[in]   _zName          Clock name
 * @return      orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_Get(const orxSTRING _zName)
{
  orxCLOCK *pstResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Valid name? */
  if(_zName != orxSTRING_EMPTY)
  {
    /* Updates result */
    pstResult = (orxCLOCK *)orxHashTable_Get(sstClock.pstReferenceTable, orxString_ToCRC(_zName));
  }
  else
  {
    /* Clears result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

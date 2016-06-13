/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2016 Orx-Project
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
 * @file orxClock.c
 * @date 04/04/2004
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxClock.h"

#include "debug/orxDebug.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"
#include "object/orxStructure.h"
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
#define orxCLOCK_KU32_FLAG_UPDATE_LOCK          0x40000000  /**< Lock update flag */

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

#define orxCLOCK_KU32_BANK_SIZE                 8           /**< Bank size */

#define orxCLOCK_KF_DELAY_ADJUSTMENT            orx2F(-0.001f)


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Clock function storage structure
 */
typedef struct __orxCLOCK_FUNCTION_STORAGE_t
{
  orxLINKLIST_NODE            stNode;           /**< Linklist node : 12 */
  orxCLOCK_FUNCTION           pfnCallback;      /**< Clock function pointer : 16 */
  orxCLOCK_PRIORITY           ePriority;        /**< Clock function priority : 20 */
  void                       *pContext;         /**< Clock function context : 24 */
  orxMODULE_ID                eModuleID;        /**< Clock function module ID : 28 */

} orxCLOCK_FUNCTION_STORAGE;

typedef struct __orxCLOCK_TIMER_STORAGE_t
{
  orxLINKLIST_NODE            stNode;           /**< Linklist node : 12 */
  orxCLOCK_FUNCTION           pfnCallback;      /**< Timer function pointer : 16 */
  orxFLOAT                    fDelay;           /**< Timer delay : 20 */
  void                       *pContext;         /**< Timer function context : 24 */
  orxS32                      s32Repetition;    /**< Timer repetition : 28 */
  orxFLOAT                    fTimeStamp;       /**< Next call time stamp : 32 */

} orxCLOCK_TIMER_STORAGE;

/** Clock structure
 */
struct __orxCLOCK_t
{
  orxSTRUCTURE      stStructure;                /**< Public structure, first structure member : 32 */
  orxCLOCK_INFO     stClockInfo;                /**< Clock Info Structure : 40 */
  orxFLOAT          fPartialDT;                 /**< Clock partial DT : 44 */
  orxBANK          *pstFunctionBank;            /**< Function bank : 48 */
  orxLINKLIST       stFunctionList;             /**< Function list : 60 */
  orxLINKLIST       stTimerList;                /**< Timer list : 72 */
  const orxSTRING   zReference;                 /**< Reference : 76 */
};


/** Static structure
 */
typedef struct __orxCLOCK_STATIC_t
{
  orxBANK          *pstTimerBank;               /**< Timer bank : 4 */
  orxCLOCK_MOD_TYPE eModType;                   /**< Clock mod type : 8 */
  orxDOUBLE         dTime;                      /**< Current time : 16 */
  orxFLOAT          fModValue;                  /**< Clock mod value : 20 */
  orxFLOAT          fMainClockTickSize;         /**< Main clock tick size : 24 */
  orxHASHTABLE     *pstReferenceTable;          /**< Table to avoid clock duplication when creating through config file : 28 */
  orxU32            u32Flags;                   /**< Control flags : 32 */

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
  orxSTRUCTURE_ASSERT(_pstClock);
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
  for(pstClock = (_pstStartClock != orxNULL) ? orxCLOCK(orxStructure_GetNext(_pstStartClock)) : orxCLOCK(orxStructure_GetFirst(orxSTRUCTURE_ID_CLOCK));
      pstClock != orxNULL;
      pstClock = orxCLOCK(orxStructure_GetNext(pstClock)))
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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Invalid clock modifier type (%d).", *peModType);

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

/** Deletes all the clocks
 */
static orxINLINE void orxClock_DeleteAll()
{
  orxCLOCK *pstClock;

  /* Gets first clock */
  pstClock = orxCLOCK(orxStructure_GetFirst(orxSTRUCTURE_ID_CLOCK));

  /* Non empty? */
  while(pstClock != orxNULL)
  {
    /* Deletes clock */
    orxClock_Delete(pstClock);

    /* Gets first clock */
    pstClock = orxCLOCK(orxStructure_GetFirst(orxSTRUCTURE_ID_CLOCK));
  }

  return;
}

/** Command: SetFrequency
 */
void orxFASTCALL orxClock_CommandSetFrequency(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCLOCK *pstClock;

  /* Gets clock */
  pstClock = orxClock_Get(_astArgList[0].zValue);

  /* Not found? */
  if(pstClock == orxNULL)
  {
    /* Creates it */
    pstClock = orxClock_CreateFromConfig(_astArgList[0].zValue);
  }

  /* Valid? */
  if(pstClock != orxNULL)
  {
    orxFLOAT fTickSize;

    /* Gets tick size */
    fTickSize = ((_u32ArgNumber > 1) && (_astArgList[1].fValue > orxFLOAT_0)) ? orxFLOAT_1 / _astArgList[1].fValue : orxFLOAT_0;

    /* Sets it */
    _pstResult->bValue = (orxClock_SetTickSize(pstClock, fTickSize) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Command: SetModifier
 */
void orxFASTCALL orxClock_CommandSetModifier(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxCLOCK *pstClock;

  /* Gets clock */
  pstClock = orxClock_Get(_astArgList[0].zValue);

  /* Not found? */
  if(pstClock == orxNULL)
  {
    /* Creates it */
    pstClock = orxClock_CreateFromConfig(_astArgList[0].zValue);
  }

  /* Valid? */
  if(pstClock != orxNULL)
  {
    orxCLOCK_MOD_TYPE eModifierType = orxCLOCK_MOD_TYPE_NONE;
    orxFLOAT          fModifierValue;

    /* Gets modifier value */
    fModifierValue = _astArgList[2].fValue;

    /* Is modifier value valid? */
    if(fModifierValue > orxFLOAT_0)
    {
      /* Capped? */
      if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_CAPPED) == 0)
      {
        /* Updates modifier value */
        fModifierValue = orxFLOAT_1 / fModifierValue;

        /* Updates modifier type */
        eModifierType = orxCLOCK_MOD_TYPE_MAXED;
      }
      /* Fixed? */
      else if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_FIXED) == 0)
      {
        /* Updates modifier value */
        fModifierValue = orxFLOAT_1 / fModifierValue;

        /* Updates modifier type */
        eModifierType = orxCLOCK_MOD_TYPE_FIXED;
      }
      /* Multiply? */
      else if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_MULTIPLY) == 0)
      {
        /* Updates modifier type */
        eModifierType = orxCLOCK_MOD_TYPE_MULTIPLY;
      }
    }

    /* Updates clock */
    _pstResult->bValue = (orxClock_SetModifier(pstClock, eModifierType, fModifierValue) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Updates result */
    _pstResult->bValue = orxFALSE;
  }

  /* Done! */
  return;
}

/** Registers all the clock commands
 */
static orxINLINE void orxClock_RegisterCommands()
{
  /* Command: SetFrequency */
  orxCOMMAND_REGISTER_CORE_COMMAND(Clock, SetFrequency, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 1, {"Clock", orxCOMMAND_VAR_TYPE_STRING}, {"Frequency = 0.0", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: SetModifier */
  orxCOMMAND_REGISTER_CORE_COMMAND(Clock, SetModifier, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 3, 0, {"Clock", orxCOMMAND_VAR_TYPE_STRING}, {"Type", orxCOMMAND_VAR_TYPE_STRING}, {"Value", orxCOMMAND_VAR_TYPE_FLOAT});
}

/** Unregisters all the clock commands
 */
static orxINLINE void orxClock_UnregisterCommands()
{
  /* Command: SetFrequency */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Clock, SetFrequency);
  /* Command: SetModifier */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Clock, SetModifier);
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
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_SYSTEM);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_COMMAND);
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
    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(CLOCK, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxCLOCK_KU32_BANK_SIZE, orxNULL);

    /* Successful? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Cleans control structure */
      orxMemory_Zero(&sstClock, sizeof(orxCLOCK_STATIC));

      /* Creates timer bank */
      sstClock.pstTimerBank = orxBank_Create(orxCLOCK_KU32_TIMER_BANK_SIZE, sizeof(orxCLOCK_TIMER_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstClock.pstTimerBank != orxNULL)
      {
        /* Creates reference table */
        sstClock.pstReferenceTable = orxHashTable_Create(orxCLOCK_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Valid? */
        if(sstClock.pstReferenceTable != orxNULL)
        {
          orxCLOCK *pstClock;

          /* No mod type by default */
          sstClock.eModType = orxCLOCK_MOD_TYPE_NONE;

          /* Gets init time */
          sstClock.dTime  = orxSystem_GetTime();

          /* Inits Flags */
          sstClock.u32Flags = orxCLOCK_KU32_STATIC_FLAG_READY;

          /* Gets main clock tick size */
          orxConfig_PushSection(orxCLOCK_KZ_CONFIG_SECTION);
          sstClock.fMainClockTickSize = (orxConfig_HasValue(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY) && orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY) > orxFLOAT_0) ? (orxFLOAT_1 / orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY)) : orxFLOAT_0;
          orxConfig_PopSection();

          /* Creates default full speed core clock */
          pstClock = orxClock_Create(sstClock.fMainClockTickSize, orxCLOCK_TYPE_CORE);

          /* Success? */
          if(pstClock != orxNULL)
          {
            /* Sets it as its own owner */
            orxStructure_SetOwner(pstClock, pstClock);

            /* Registers commands */
            orxClock_RegisterCommands();

            /* Updates result */
            eResult = orxSTATUS_SUCCESS;
          }
          else
          {
            /* Updates result */
            eResult = orxSTATUS_FAILURE;
          }
        }
        else
        {
          /* Deletes timer bank */
          orxBank_Delete(sstClock.pstTimerBank);
          sstClock.pstTimerBank = orxNULL;

          /* Updates result */
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
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Failed to register link list structure.");
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
    /* Unregisters commands */
    orxClock_UnregisterCommands();

    /* Deletes all the clocks */
    orxClock_DeleteAll();

    /* Deletes timer bank */
    orxBank_Delete(sstClock.pstTimerBank);
    sstClock.pstTimerBank = orxNULL;

    /* Deletes reference table */
    orxHashTable_Delete(sstClock.pstReferenceTable);

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_CLOCK);

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
  orxDOUBLE dNewTime;
  orxFLOAT  fDT, fDelay;
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
    dNewTime  = orxSystem_GetTime();

    /* Computes natural DT */
    fDT       = (orxFLOAT)(dNewTime - sstClock.dTime);

    /* Gets modified DT */
    fDT       = orxClock_ComputeDT(fDT, orxNULL);

    /* Updates time */
    sstClock.dTime = dNewTime;

    /* Inits delay */
    fDelay = sstClock.fMainClockTickSize;

    /* For all clocks */
    for(pstClock = orxCLOCK(orxStructure_GetFirst(orxSTRUCTURE_ID_CLOCK));
        pstClock != orxNULL;
        pstClock = orxCLOCK(orxStructure_GetNext(pstClock)))
    {
      /* Locks it */
      orxStructure_SetFlags(pstClock, orxCLOCK_KU32_FLAG_UPDATE_LOCK, orxCLOCK_KU32_FLAG_NONE);

      /* Is clock not paused? */
      if(orxClock_IsPaused(pstClock) == orxFALSE)
      {
        orxFLOAT fClockDelay;

        /* Updates clock real time & partial DT */
        pstClock->fPartialDT += fDT;

        /* New tick happens? */
        if(pstClock->fPartialDT >= pstClock->stClockInfo.fTickSize)
        {
          orxFLOAT                    fClockDT;
          orxCLOCK_TIMER_STORAGE     *pstTimerStorage;
          orxCLOCK_FUNCTION_STORAGE  *pstFunctionStorage;

          /* Gets clock modified DT */
          fClockDT = orxClock_ComputeDT(pstClock->fPartialDT, &(pstClock->stClockInfo));

          /* Updates clock DT */
          pstClock->stClockInfo.fDT = fClockDT;

          /* Updates clock time */
          pstClock->stClockInfo.fTime += fClockDT;

          /* For all timers */
          for(pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetFirst(&(pstClock->stTimerList)); pstTimerStorage != orxNULL;)
          {
            /* Should call it? */
            if((pstTimerStorage->fTimeStamp <= pstClock->stClockInfo.fTime) && (pstTimerStorage->s32Repetition != 0))
            {
              /* Calls it */
              pstTimerStorage->pfnCallback(&(pstClock->stClockInfo), pstTimerStorage->pContext);

              /* Updates its time stamp */
              pstTimerStorage->fTimeStamp = pstClock->stClockInfo.fTime + pstTimerStorage->fDelay;

              /* Should update counter */
              if(pstTimerStorage->s32Repetition > 0)
              {
                /* Updates it */
                pstTimerStorage->s32Repetition--;
              }
            }

            /* Should delete it */
            if(pstTimerStorage->s32Repetition == 0)
            {
              orxCLOCK_TIMER_STORAGE *pstDelete;

              /* Gets timer to delete */
              pstDelete = pstTimerStorage;

              /* Gets the next timer */
              pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetNext(&(pstTimerStorage->stNode));

              /* Removes current timer */
              orxLinkList_Remove(&(pstDelete->stNode));

              /* Deletes it */
              orxBank_Free(sstClock.pstTimerBank, pstDelete);
            }
            else
            {
              /* Gets the next timer */
              pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetNext(&(pstTimerStorage->stNode));
            }
          }

          /* For all registered callbacks */
          for(pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetFirst(&(pstClock->stFunctionList));
              pstFunctionStorage != orxNULL;
              pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetNext(&(pstFunctionStorage->stNode)))
          {
            /* Calls it */
            pstFunctionStorage->pfnCallback(&(pstClock->stClockInfo), pstFunctionStorage->pContext);
          }

          /* Updates partial DT */
          pstClock->fPartialDT = orxFLOAT_0;
        }

        /* Gets clock's delay */
        fClockDelay = pstClock->stClockInfo.fTickSize - pstClock->fPartialDT;

        /* Smaller than previous clocks' delay? */
        if(fClockDelay < fDelay)
        {
          /* Stores it */
          fDelay = fClockDelay;
        }
      }

      /* Unlocks it */
      orxStructure_SetFlags(pstClock, orxCLOCK_KU32_FLAG_NONE, orxCLOCK_KU32_FLAG_UPDATE_LOCK);
    }

    /* Unlocks clocks */
    sstClock.u32Flags &= ~orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK;

    /* Gets real remaining delay */
    fDelay = fDelay + orxCLOCK_KF_DELAY_ADJUSTMENT - orx2F(orxSystem_GetTime() - sstClock.dTime);

    /* Should delay? */
    if(fDelay > orxFLOAT_0)
    {
      /* Waits for next time slice */
      orxSystem_Delay(fDelay);
    }
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
  pstClock = orxCLOCK(orxStructure_Create(orxSTRUCTURE_ID_CLOCK));

  /* Valid? */
  if(pstClock != orxNULL)
  {
    /* Creates function bank */
    pstClock->pstFunctionBank = orxBank_Create(orxCLOCK_KU32_FUNCTION_BANK_SIZE, sizeof(orxCLOCK_FUNCTION_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstClock->pstFunctionBank != orxNULL)
    {
      /* Inits clock */
      pstClock->stClockInfo.fTickSize = _fTickSize;
      pstClock->stClockInfo.eType     = _eType;
      pstClock->stClockInfo.eModType  = orxCLOCK_MOD_TYPE_NONE;
      orxStructure_SetFlags(pstClock, orxCLOCK_KU32_FLAG_NONE, orxCLOCK_KU32_MASK_ALL);

      /* Increases counter */
      orxStructure_IncreaseCounter(pstClock);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't create clock function storage.");

      /* Deletes clock */
      orxStructure_Delete(pstClock);

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

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
  }
  else
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
            const orxSTRING   zModifierType;
            orxCLOCK_MOD_TYPE eModifierType;

            /* Gets modifier type */
            zModifierType = orxConfig_GetString(orxCLOCK_KZ_CONFIG_MODIFIER_TYPE);

            /* Capped? */
            if(orxString_ICompare(zModifierType, orxCLOCK_KZ_MODIFIER_CAPPED) == 0)
            {
              /* Updates modifier value */
              fModifierValue = orxFLOAT_1 / fModifierValue;

              /* Updates modifier type */
              eModifierType = orxCLOCK_MOD_TYPE_MAXED;
            }
            /* Fixed? */
            else if(orxString_ICompare(zModifierType, orxCLOCK_KZ_MODIFIER_FIXED) == 0)
            {
              /* Updates modifier value */
              fModifierValue = orxFLOAT_1 / fModifierValue;

              /* Updates modifier type */
              eModifierType = orxCLOCK_MOD_TYPE_FIXED;
            }
            /* Multiply? */
            else if(orxString_ICompare(zModifierType, orxCLOCK_KZ_MODIFIER_MULTIPLY) == 0)
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

        /* Adds it to reference table */
        orxHashTable_Add(sstClock.pstReferenceTable, orxString_ToCRC(pstResult->zReference), pstResult);

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, orxCLOCK_KU32_FLAG_REFERENCED, orxCLOCK_KU32_FLAG_NONE);
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Cannot find config section named (%s).", _zConfigID);

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
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstClock);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstClock) == 0)
  {
    /* Not locked? */
    if(!orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_UPDATE_LOCK))
    {
      orxCLOCK_TIMER_STORAGE *pstTimerStorage;

      /* For all stored timers */
      for(pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetFirst(&(_pstClock->stTimerList));
          pstTimerStorage != orxNULL;
          pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetFirst(&(_pstClock->stTimerList)))
      {
        /* Removes it */
        orxLinkList_Remove(&(pstTimerStorage->stNode));

        /* Deletes it */
        orxBank_Free(sstClock.pstTimerBank, pstTimerStorage);
      }

      /* Deletes function bank */
      orxBank_Delete(_pstClock->pstFunctionBank);

      /* Is referenced? */
      if(orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_REFERENCED))
      {
        /* Removes it from reference table */
        orxHashTable_Remove(sstClock.pstReferenceTable, orxString_ToCRC(_pstClock->zReference));
      }

      /* Deletes clock */
      orxStructure_Delete(_pstClock);
    }
    else
    {
      /* Increases counter */
      orxStructure_IncreaseCounter(_pstClock);

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Can't delete clock <%s> as it's currently locked for processing!", orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_REFERENCED) ? _pstClock->zReference : orxSTRING_EMPTY);

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
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
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Not locked? */
  if(!orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_UPDATE_LOCK))
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
  for(pstClock = orxCLOCK(orxStructure_GetFirst(orxSTRUCTURE_ID_CLOCK));
      pstClock != orxNULL;
      pstClock = orxCLOCK(orxStructure_GetNext(pstClock)))
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
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Not locked? */
  if(!orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_UPDATE_LOCK))
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
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_CLOCK, orxCLOCK_EVENT_PAUSE, _pstClock, orxNULL, orxNULL);

  /* Updates clock flags */
  orxStructure_SetFlags(_pstClock, orxCLOCK_KU32_FLAG_PAUSED, orxCLOCK_KU32_FLAG_NONE);

  /* Done! */
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
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_CLOCK, orxCLOCK_EVENT_UNPAUSE, _pstClock, orxNULL, orxNULL);

  /* Updates clock flags */
  orxStructure_SetFlags(_pstClock, orxCLOCK_KU32_FLAG_NONE, orxCLOCK_KU32_FLAG_PAUSED);

  /* Done! */
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
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Tests flags */
  return(orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_PAUSED) ? orxTRUE : orxFALSE);
}

/** Gets clock info
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK_INFO / orxNULL
 */
const orxCLOCK_INFO *orxFASTCALL  orxClock_GetInfo(const orxCLOCK *_pstClock)
{
  const orxCLOCK_INFO *pstClockInfo;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Gets the clock info structure pointer */
  pstClockInfo = &(_pstClock->stClockInfo);

  /* Done! */
  return pstClockInfo;
}

/** Gets clock from its info
 * @param[in]   _pstClockInfo                         Concerned clock info
 * @return      orxCLOCK / orxNULL
 */
orxCLOCK *orxFASTCALL orxClock_GetFromInfo(const orxCLOCK_INFO *_pstClockInfo)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClockInfo != orxNULL);

  /* Gets clock */
  pstClock = orxSTRUCT_GET_FROM_FIELD(orxCLOCK, stClockInfo, _pstClockInfo);

  /* Done! */
  return pstClock;
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
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT((_eModType < orxCLOCK_MOD_TYPE_NUMBER) || (_eModType == orxCLOCK_MOD_TYPE_NONE));

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
  orxSTRUCTURE_ASSERT(_pstClock);

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
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @param[in]   _eModuleID                            ID of the module related to this callback
 * @param[in]   _ePriority                            Priority for the function
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Register(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, void *_pContext, orxMODULE_ID _eModuleID, orxCLOCK_PRIORITY _ePriority)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
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
    pstFunctionStorage->pContext  = _pContext;

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
  orxSTRUCTURE_ASSERT(_pstClock);
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
  void *pContext = orxNULL;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds clock function storage */
  pstFunctionStorage = orxClock_FindFunctionStorage(_pstClock, _pfnCallback);

  /* Found? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Gets context */
    pContext = pstFunctionStorage->pContext;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Couldn't find clock function storage.");
  }

  /* Done! */
  return pContext;
}

/** Sets a callback function context
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_SetContext(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, void *_pContext)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds clock function storage */
  pstFunctionStorage = orxClock_FindFunctionStorage(_pstClock, _pfnCallback);

  /* Found? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Sets context */
    pstFunctionStorage->pContext = _pContext;
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
  orxSTRUCTURE_ASSERT(_pstClock);

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
  return(orxCLOCK(orxStructure_GetNext(_pstClock)));
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

/** Gets clock config name
 * @param[in]   _pstClock       Concerned clock
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxClock_GetName(const orxCLOCK *_pstClock)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock);

  /* Updates result */
  zResult = (orxStructure_TestFlags(_pstClock, orxCLOCK_KU32_FLAG_REFERENCED)) ? _pstClock->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Adds a timer function to a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned timer callback
 * @param[in]   _fDelay                               Timer's delay between 2 calls, must be strictly positive
 * @param[in]   _s32Repetition                        Number of times this timer should be called before removed, -1 for infinite
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_AddTimer(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, orxS32 _s32Repetition, void *_pContext)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT(_pfnCallback != orxNULL);

  /* Are delay and repetition correct? */
  if((_fDelay > orxFLOAT_0) && (_s32Repetition != 0))
  {
    orxCLOCK_TIMER_STORAGE *pstTimerStorage;

    /* Allocates a timer storage */
    pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxBank_Allocate(sstClock.pstTimerBank);

    /* Valid? */
    if(pstTimerStorage != orxNULL)
    {
      /* Clears it */
      orxMemory_Zero(pstTimerStorage, sizeof(orxCLOCK_TIMER_STORAGE));

      /* Adds it to clock timer list */
      orxLinkList_AddEnd(&(_pstClock->stTimerList), &(pstTimerStorage->stNode));

      /* Inits it */
      pstTimerStorage->pfnCallback    = _pfnCallback;
      pstTimerStorage->pContext       = _pContext;
      pstTimerStorage->fDelay         = _fDelay;
      pstTimerStorage->s32Repetition  = _s32Repetition;

      /* Gets it next call time stamp */
      pstTimerStorage->fTimeStamp = _pstClock->stClockInfo.fTime + _fDelay;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Removes a timer function from a clock
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned timer callback to remove
 * @param[in]   _fDelay                               Delay between 2 calls of the timer to remove, -1.0f for removing all occurrences regardless of their respective delay
 * @param[in]   _pContext                             Context of the timer to remove, orxNULL for removing all occurrences regardless of their context
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_RemoveTimer(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, void *_pContext)
{
  orxCLOCK_TIMER_STORAGE *pstTimerStorage;
  orxSTATUS               eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT(_pfnCallback != orxNULL);

  /* For all stored timers */
  for(pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetFirst(&(_pstClock->stTimerList));
      pstTimerStorage != orxNULL;
      pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetNext(&(pstTimerStorage->stNode)))
  {
    /* Matches criteria? */
    if((pstTimerStorage->pfnCallback == _pfnCallback)
    && ((_fDelay < orxFLOAT_0)
     || (pstTimerStorage->fDelay == _fDelay))
    && ((_pContext == orxNULL)
     || (pstTimerStorage->pContext == _pContext)))
    {
      /* Marks it for deletion */
      pstTimerStorage->s32Repetition = 0;
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a global timer function (ie. using the main core clock)
 * @param[in]   _pfnCallback                          Concerned timer callback
 * @param[in]   _fDelay                               Timer's delay between 2 calls, must be strictly positive
 * @param[in]   _s32Repetition                        Number of times this timer should be called before removed, -1 for infinite
 * @param[in]   _pContext                             Context that will be transmitted to the callback when called
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_AddGlobalTimer(const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, orxS32 _s32Repetition, void *_pContext)
{
  orxCLOCK *pstClock;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfnCallback != orxNULL);

  /* Gets core clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Found? */
  if(pstClock != orxNULL)
  {
    /* Adds timer to it */
    eResult = orxClock_AddTimer(pstClock, _pfnCallback, _fDelay, _s32Repetition, _pContext);
  }

  /* Done! */
  return eResult;
}

/** Removes a global timer function (ie. from the main core clock)
 * @param[in]   _pfnCallback                          Concerned timer callback to remove
 * @param[in]   _fDelay                               Delay between 2 calls of the timer to remove, -1.0f for removing all occurrences regardless of their respective delay
 * @param[in]   _pContext                             Context of the timer to remove, orxNULL for removing all occurrences regardless of their context
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_RemoveGlobalTimer(const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, void *_pContext)
{
  orxCLOCK *pstClock;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pfnCallback != orxNULL);

  /* Gets core clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Found? */
  if(pstClock != orxNULL)
  {
    /* Removes timer from it */
    eResult = orxClock_RemoveTimer(pstClock, _pfnCallback, _fDelay, _pContext);
  }

  /* Done! */
  return eResult;
}

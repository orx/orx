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
#include "display/orxDisplay.h"
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
#define orxCLOCK_KU32_FLAG_DISPLAY              0x80000000  /**< Display sync flag */

#define orxCLOCK_KU32_MASK_ALL                  0xFFFFFFFF  /**< All mask */


/** Misc
 */
#define orxCLOCK_KZ_CONFIG_SECTION              "Clock"
#define orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY "MainClockFrequency"

#define orxCLOCK_KZ_MODIFIER_FIXED              "fixed"
#define orxCLOCK_KZ_MODIFIER_MULTIPLY           "multiply"
#define orxCLOCK_KZ_MODIFIER_MAXED              "maxed"
#define orxCLOCK_KZ_MODIFIER_AVERAGE            "average"

#define orxCLOCK_KZ_DISPLAY                     "display"

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
  void                       *pContext;         /**< Clock function context : 20 */
  orxCLOCK_PRIORITY           ePriority;        /**< Clock function priority : 24 */
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
  orxCLOCK_INFO     stClockInfo;                /**< Clock Info Structure : 60 */
  orxFLOAT          fPartialDT;                 /**< Clock partial DT : 64 */
  orxBANK          *pstFunctionBank;            /**< Function bank : 68 */
  orxLINKLIST       stFunctionList;             /**< Function list : 80 */
  orxLINKLIST       stTimerList;                /**< Timer list : 92 */
  const orxSTRING   zReference;                 /**< Reference : 96 */
  orxU32            u32HistoryIndex;            /**< Average history index : 100 */
  orxFLOAT         *afHistory;                  /**< Average history : 104 */
};


/** Static structure
 */
typedef struct __orxCLOCK_STATIC_t
{
  orxCLOCK         *pstCore;                    /**< Core clock */
  orxBANK          *pstTimerBank;               /**< Timer bank */
  orxDOUBLE         dTime;                      /**< Current time */
  orxHASHTABLE     *pstReferenceTable;          /**< Table to avoid clock duplication when creating through config file */
  orxFLOAT          fDisplayTickSize;           /**< Display tick size */
  orxU32            u32Flags;                   /**< Control flags */

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

/** Computes DT according to modifier
 * @param[in]   _fDT                                  Real DT
 * @param[in]   _pstClock                             Concerned clock
 * @return      Modified DT
 */
static orxINLINE orxFLOAT orxClock_ComputeDT(orxFLOAT _fDT, orxCLOCK *_pstClock)
{
  orxFLOAT fResult = _fDT;

  /* Fixed modifier? */
  if(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_FIXED] != orxFLOAT_0)
  {
    /* Should match tick size? */
    if(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_FIXED] < orxFLOAT_0)
    {
      /* Has tick size? */
      if(_pstClock->stClockInfo.fTickSize > orxFLOAT_0)
      {
        /* Updates result */
        fResult = _pstClock->stClockInfo.fTickSize;
      }
    }
    else
    {
      /* Updates result */
      fResult = _pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_FIXED];
    }
  }

  /* Multiplied modifier? */
  if(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_MULTIPLY] != orxFLOAT_0)
  {
    /* Updates result */
    fResult *= _pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_MULTIPLY];
  }

  /* Maxed modifier? */
  if(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_MAXED] != orxFLOAT_0)
  {
    /* Updates result */
    fResult = orxMIN(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_MAXED], fResult);
  }

  /* Average modifier? */
  if(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_AVERAGE] != orxFLOAT_0)
  {
    orxU32 i, iCount, u32HistorySize;

    /* Gets history size */
    u32HistorySize = orxF2U(_pstClock->stClockInfo.afModifierList[orxCLOCK_MODIFIER_AVERAGE]);

    /* For all history */
    for(i = 0, iCount = orxMIN(_pstClock->u32HistoryIndex, u32HistorySize);
        i < iCount;
        i++)
    {
      /* Sums values */
      fResult += _pstClock->afHistory[i];
    }

    /* Updates result */
    fResult /= orxU2F(1 + iCount);

    /* Updates history */
    _pstClock->afHistory[_pstClock->u32HistoryIndex++ % u32HistorySize] = fResult;
  }

  /* Done! */
  return fResult;
}

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxClock_EventHandler(const orxEVENT *_pstEvent)
{
  orxDISPLAY_EVENT_PAYLOAD *pstPayload;
  orxSTATUS                 eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(_pstEvent->eType == orxEVENT_TYPE_DISPLAY);
  orxASSERT(_pstEvent->eID == orxDISPLAY_EVENT_SET_VIDEO_MODE);

  /* Gets payload */
  pstPayload = (orxDISPLAY_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  /* Stores display tick size */
  sstClock.fDisplayTickSize = (pstPayload->stVideoMode.u32RefreshRate != 0) ? orxFLOAT_1 / orxU2F(pstPayload->stVideoMode.u32RefreshRate) : orxFLOAT_0;

  /* Done! */
  return eResult;
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

  /* Done! */
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
    orxFLOAT fTickSize = orxFLOAT_0;

    /* No valid tick size? */
    if((_u32ArgNumber == 1) || (orxString_ToFloat(_astArgList[1].zValue, &fTickSize, orxNULL) == orxSTATUS_FAILURE))
    {
      /* Defaults to display */
      fTickSize = -orxFLOAT_1;
    }
    /* No frequency? */
    else if(fTickSize != orxFLOAT_0)
    {
      /* Gets tick size */
      fTickSize = orxFLOAT_1 / fTickSize;
    }

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
    orxCLOCK_MODIFIER eModifier = orxCLOCK_MODIFIER_NONE;
    orxFLOAT          fModifierValue;

    /* Gets modifier value */
    fModifierValue = _astArgList[2].fValue;

    /* Fixed? */
    if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_FIXED) == 0)
    {
      /* Updates modifier type */
      eModifier = orxCLOCK_MODIFIER_FIXED;
    }
    /* Multiply? */
    else if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_MULTIPLY) == 0)
    {
      /* Updates modifier type */
      eModifier = orxCLOCK_MODIFIER_MULTIPLY;
    }
    /* Maxed? */
    else if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_MAXED) == 0)
    {
      /* Updates modifier type */
      eModifier = orxCLOCK_MODIFIER_MAXED;
    }
    /* Average? */
    else if(orxString_ICompare(_astArgList[1].zValue, orxCLOCK_KZ_MODIFIER_AVERAGE) == 0)
    {
      /* Updates modifier type */
      eModifier = orxCLOCK_MODIFIER_AVERAGE;
    }

    /* Updates clock */
    _pstResult->bValue = ((eModifier != orxCLOCK_MODIFIER_NONE) && (orxClock_SetModifier(pstClock, eModifier, fModifierValue) != orxSTATUS_FAILURE)) ? orxTRUE : orxFALSE;
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
  orxCOMMAND_REGISTER_CORE_COMMAND(Clock, SetFrequency, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 1, 1, {"Clock", orxCOMMAND_VAR_TYPE_STRING}, {"Frequency = display", orxCOMMAND_VAR_TYPE_STRING});
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

  /* Done! */
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

      /* Adds event handler */
      eResult = orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxClock_EventHandler);

      /* Valid? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Filters relevant event IDs */
        orxEvent_SetHandlerIDFlags(orxClock_EventHandler, orxEVENT_TYPE_DISPLAY, orxNULL, orxEVENT_GET_FLAG(orxDISPLAY_EVENT_SET_VIDEO_MODE), orxEVENT_KU32_MASK_ID_ALL);

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
            orxFLOAT fTickSize;

            /* Gets init time */
            sstClock.dTime = orxSystem_GetTime();

            /* Inits Flags */
            sstClock.u32Flags = orxCLOCK_KU32_STATIC_FLAG_READY;

            /* Gets main clock tick size */
            orxConfig_PushSection(orxCLOCK_KZ_CONFIG_SECTION);
            fTickSize = (orxConfig_HasValue(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY) && orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY) > orxFLOAT_0) ? (orxFLOAT_1 / orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_MAIN_CLOCK_FREQUENCY)) : orxFLOAT_0;
            orxConfig_PopSection();

            /* Pushes core clock section */
            orxConfig_PushSection(orxCLOCK_KZ_CORE);
            orxConfig_SetParent(orxCLOCK_KZ_CORE, orxCLOCK_KZ_CONFIG_SECTION);

            /* No frequency? */
            if(orxConfig_HasValue(orxCLOCK_KZ_CONFIG_FREQUENCY) == orxFALSE)
            {
              /* Should set one? */
              if(fTickSize != orxFLOAT_0)
              {
                /* Updates it */
                orxConfig_SetFloat(orxCLOCK_KZ_CONFIG_FREQUENCY, (fTickSize == orxFLOAT_0) ? orxFLOAT_0 : orxFLOAT_1 / fTickSize);
              }
            }

            /* Pops config section */
            orxConfig_PopSection();

            /* Creates core clock */
            sstClock.pstCore = orxClock_CreateFromConfig(orxCLOCK_KZ_CORE);

            /* Success? */
            if(sstClock.pstCore != orxNULL)
            {
              /* Sets it as its own owner */
              orxStructure_SetOwner(sstClock.pstCore, sstClock.pstCore);

              /* Registers commands */
              orxClock_RegisterCommands();

              /* Updates result */
              eResult = orxSTATUS_SUCCESS;
            }
            else
            {
              /* Deletes reference table */
              orxHashTable_Delete(sstClock.pstReferenceTable);

              /* Deletes timer bank */
              orxBank_Delete(sstClock.pstTimerBank);

              /* Updates result */
              eResult = orxSTATUS_FAILURE;
            }
          }
          else
          {
            /* Deletes timer bank */
            orxBank_Delete(sstClock.pstTimerBank);

            /* Updates result */
            eResult = orxSTATUS_FAILURE;
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Failed to create clock bank.");

          /* Clock bank not created */
          eResult = orxSTATUS_FAILURE;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Failed to add event handler.");
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
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_DISPLAY, orxClock_EventHandler);

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

  /* Done! */
  return;
}

/** Updates the clock system
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_Update()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Not already locked? */
  if(!(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK))
  {
    orxDOUBLE dNewTime;
    orxCLOCK *pstClock;
    orxFLOAT  fDT, fDelay;

    /* Lock clocks */
    sstClock.u32Flags |= orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK;

    /* Gets new time */
    dNewTime  = orxSystem_GetTime();

    /* Computes natural DT */
    fDT       = (orxFLOAT)(dNewTime - sstClock.dTime);

    /* Updates time */
    sstClock.dTime = dNewTime;

    /* Inits delay */
    fDelay = sstClock.pstCore->stClockInfo.fTickSize;

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

        /* Should sync with display? */
        if(orxStructure_TestFlags(pstClock, orxCLOCK_KU32_FLAG_DISPLAY))
        {
          /* Updates its tick size */
          pstClock->stClockInfo.fTickSize = sstClock.fDisplayTickSize;
        }

        /* Updates clock real time & partial DT */
        pstClock->fPartialDT += fDT;

        /* New tick happens? */
        if(pstClock->fPartialDT >= pstClock->stClockInfo.fTickSize)
        {
          orxFLOAT                    fClockDT;
          orxCLOCK_TIMER_STORAGE     *pstTimerStorage, *pstNextTimerStorage;
          orxCLOCK_FUNCTION_STORAGE  *pstFunctionStorage, *pstNextFunctionStorage;

          /* Gets clock modified DT */
          fClockDT = orxClock_ComputeDT(pstClock->fPartialDT, pstClock);

          /* Updates clock DT */
          pstClock->stClockInfo.fDT = fClockDT;

          /* Updates clock time */
          pstClock->stClockInfo.fTime += fClockDT;

          /* For all timers */
          for(pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetFirst(&(pstClock->stTimerList));
              pstTimerStorage != orxNULL;
              pstTimerStorage = pstNextTimerStorage)
          {
            /* Gets the next timer */
            pstNextTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetNext(&(pstTimerStorage->stNode));

            /* Should call it? */
            if((pstTimerStorage->fTimeStamp <= pstClock->stClockInfo.fTime) && (pstTimerStorage->s32Repetition != 0))
            {
              /* Calls it */
              pstTimerStorage->pfnCallback(&(pstClock->stClockInfo), pstTimerStorage->pContext);

              /* Updates its time stamp */
              pstTimerStorage->fTimeStamp = pstClock->stClockInfo.fTime + pstTimerStorage->fDelay;

              /* Should update count */
              if(pstTimerStorage->s32Repetition > 0)
              {
                /* Updates it */
                pstTimerStorage->s32Repetition--;
              }
            }

            /* Should delete it? */
            if(pstTimerStorage->s32Repetition == 0)
            {
              /* Removes current timer */
              orxLinkList_Remove(&(pstTimerStorage->stNode));

              /* Deletes it */
              orxBank_Free(sstClock.pstTimerBank, pstTimerStorage);
            }
          }

          /* For all registered callbacks */
          for(pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetFirst(&(pstClock->stFunctionList));
              pstFunctionStorage != orxNULL;
              pstFunctionStorage = pstNextFunctionStorage)
          {
            /* Gets next function storage */
            pstNextFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxLinkList_GetNext(&(pstFunctionStorage->stNode));

            /* Not marked for deletion? */
            if(pstFunctionStorage->pfnCallback != orxNULL)
            {
              /* Calls callback */
              pstFunctionStorage->pfnCallback(&(pstClock->stClockInfo), pstFunctionStorage->pContext);
            }

            /* Should delete it? */
            if(pstFunctionStorage->pfnCallback == orxNULL)
            {
              /* Removes it from list */
              orxLinkList_Remove(&(pstFunctionStorage->stNode));

              /* Removes it from bank */
              orxBank_Free(pstClock->pstFunctionBank, pstFunctionStorage);
            }
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
orxCLOCK *orxFASTCALL orxClock_Create(orxFLOAT _fTickSize)
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
      orxStructure_SetFlags(pstClock, orxCLOCK_KU32_FLAG_NONE, orxCLOCK_KU32_MASK_ALL);

      /* Increases count */
      orxStructure_IncreaseCount(pstClock);
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
    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      orxFLOAT  fFrequency;
      orxU32    u32Flags = orxCLOCK_KU32_FLAG_NONE;

      /* Should match display refresh rate? */
      if((orxConfig_HasValue(orxCLOCK_KZ_CONFIG_FREQUENCY) == orxFALSE) || (orxString_ICompare(orxConfig_GetString(orxCLOCK_KZ_CONFIG_FREQUENCY), orxCLOCK_KZ_DISPLAY) == 0))
      {
        /* Clears its frequency */
        fFrequency = orxFLOAT_0;

        /* Updates status flags */
        u32Flags |= orxCLOCK_KU32_FLAG_DISPLAY;
      }
      else
      {
        /* Gets its frequency */
        fFrequency = orxConfig_GetFloat(orxCLOCK_KZ_CONFIG_FREQUENCY);
      }

      /* Creates clock */
      pstResult = orxClock_Create((fFrequency > orxFLOAT_0) ? orxFLOAT_1 / fFrequency : orxFLOAT_0);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        orxCLOCK *pstClock;

        /* Has a modifier list? */
        if(orxConfig_HasValue(orxCLOCK_KZ_CONFIG_MODIFIER_LIST) != orxFALSE)
        {
          orxS32 i, s32Count;

          /* For all defined modifiers */
          for(i = 0, s32Count = orxConfig_GetListCount(orxCLOCK_KZ_CONFIG_MODIFIER_LIST); i < s32Count; i++)
          {
            const orxSTRING   zModifier;
            orxCLOCK_MODIFIER eModifier = orxCLOCK_MODIFIER_NONE;

            /* Gets its value */
            zModifier = orxConfig_GetListString(orxCLOCK_KZ_CONFIG_MODIFIER_LIST, i);

            /* Fixed? */
            if(orxString_SearchString(zModifier, orxCLOCK_KZ_MODIFIER_FIXED) == zModifier)
            {
              /* Updates modifier */
              zModifier  += orxString_GetLength(orxCLOCK_KZ_MODIFIER_FIXED);
              eModifier   = orxCLOCK_MODIFIER_FIXED;
            }
            /* Multiply? */
            else if(orxString_SearchString(zModifier, orxCLOCK_KZ_MODIFIER_MULTIPLY) == zModifier)
            {
              /* Updates modifier */
              zModifier  += orxString_GetLength(orxCLOCK_KZ_MODIFIER_MULTIPLY);
              eModifier   = orxCLOCK_MODIFIER_MULTIPLY;
            }
            /* Maxed? */
            else if(orxString_SearchString(zModifier, orxCLOCK_KZ_MODIFIER_MAXED) == zModifier)
            {
              /* Updates modifier */
              zModifier  += orxString_GetLength(orxCLOCK_KZ_MODIFIER_MAXED);
              eModifier   = orxCLOCK_MODIFIER_MAXED;
            }
            /* Average? */
            else if(orxString_SearchString(zModifier, orxCLOCK_KZ_MODIFIER_AVERAGE) == zModifier)
            {
              /* Updates modifier */
              zModifier  += orxString_GetLength(orxCLOCK_KZ_MODIFIER_AVERAGE);
              eModifier   = orxCLOCK_MODIFIER_AVERAGE;
            }

            /* Valid? */
            if(eModifier != orxCLOCK_MODIFIER_NONE)
            {
              orxFLOAT fValue = orxFLOAT_0;

              /* Gets its value */
              orxString_ToFloat(zModifier, &fValue, orxNULL);

              /* Valid? */
              if(fValue != orxFLOAT_0)
              {
                /* Sets it */
                orxClock_SetModifier(pstResult, eModifier, fValue);
              }
            }
          }
        }
        else
        {
          /* Defaults to fixed/-1 modifier */
          orxClock_SetModifier(pstResult, orxCLOCK_MODIFIER_FIXED, -orxFLOAT_1);
        }

        /* Has core clock? */
        if((pstClock = orxClock_Get(orxCLOCK_KZ_CORE)) != orxNULL)
        {
          /* Updates clock's DT */
          pstResult->stClockInfo.fDT = orxClock_ComputeDT(pstClock->stClockInfo.fDT, pstResult);
        }

        /* Stores its reference key */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Adds it to reference table */
        orxHashTable_Add(sstClock.pstReferenceTable, orxString_Hash(pstResult->zReference), pstResult);

        /* Updates status flags */
        orxStructure_SetFlags(pstResult, u32Flags | orxCLOCK_KU32_FLAG_REFERENCED, orxCLOCK_KU32_FLAG_NONE);
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

  /* Decreases count */
  orxStructure_DecreaseCount(_pstClock);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstClock) == 0)
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
        orxHashTable_Remove(sstClock.pstReferenceTable, orxString_Hash(_pstClock->zReference));
      }

      /* Deletes clock */
      orxStructure_Delete(_pstClock);
    }
    else
    {
      /* Increases count */
      orxStructure_IncreaseCount(_pstClock);

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

/** Sets a clock's modifier
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _eModifier                            Concerned modifier
 * @param[in]   _fValue                               Modifier value, orxFLOAT_0 to deactivate the modifier
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_SetModifier(orxCLOCK *_pstClock, orxCLOCK_MODIFIER _eModifier, orxFLOAT _fValue)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT((_eModifier >= 0) && (_eModifier < orxCLOCK_MODIFIER_NUMBER));

  /* Valid value? */
  if((_fValue >= orxFLOAT_0) || (_eModifier == orxCLOCK_MODIFIER_FIXED))
  {
    /* Updates modifier */
    _pstClock->stClockInfo.afModifierList[_eModifier] = _fValue;

    /* Average? */
    if(_eModifier == orxCLOCK_MODIFIER_AVERAGE)
    {
      /* Should remove? */
      if(_fValue == orxFLOAT_0)
      {
        /* Removes it */
        orxMemory_Free(_pstClock->afHistory);
        _pstClock->afHistory        = orxNULL;
        _pstClock->u32HistoryIndex  = 0;
      }
      else
      {
        /* Allocates history */
        _pstClock->afHistory        = (orxFLOAT *)orxMemory_Reallocate(_pstClock->afHistory, orxF2U(_fValue) * sizeof(orxFLOAT), orxMEMORY_TYPE_MAIN);
        _pstClock->u32HistoryIndex  = 0;

        /* Checks */
        orxASSERT(_pstClock->afHistory != orxNULL);
      }
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "[%s]: Can't set modifier [%u], invalid value <%g>.", orxClock_GetName(_pstClock), _eModifier, _fValue);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a clock's modifier
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _eModifier                            Concerned modifier
 * @return      Modifier value / orxFLOAT_0 if deactivated
 */
orxFLOAT orxFASTCALL orxClock_GetModifier(orxCLOCK *_pstClock, orxCLOCK_MODIFIER _eModifier)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);
  orxASSERT((_eModifier < orxCLOCK_MODIFIER_NUMBER) || (_eModifier == orxCLOCK_MODIFIER_NONE));

  /* Updates result */
  fResult = _pstClock->stClockInfo.afModifierList[_eModifier];

  /* Done! */
  return fResult;
}

/** Sets a clock tick size
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _fTickSize                            Tick size, -1 for 'display'
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_SetTickSize(orxCLOCK *_pstClock, orxFLOAT _fTickSize)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);

  /* Updates clock tick size */
  _pstClock->stClockInfo.fTickSize = orxMAX(_fTickSize, orxFLOAT_0);

  /* Sync with display? */
  if(_fTickSize < orxFLOAT_0)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstClock, orxCLOCK_KU32_FLAG_DISPLAY, orxCLOCK_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstClock, orxCLOCK_KU32_FLAG_NONE, orxCLOCK_KU32_FLAG_DISPLAY);
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

    /* Clears it */
    orxMemory_Zero(pstFunctionStorage, sizeof(orxCLOCK_FUNCTION_STORAGE));

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
    /* Marks it for deletion */
    pstFunctionStorage->pfnCallback = orxNULL;
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
    pstResult = (orxCLOCK *)orxHashTable_Get(sstClock.pstReferenceTable, orxString_Hash(_zName));
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
 * @param[in]   _pfnCallback                          Concerned timer callback to remove, orxNULL to remove all occurrences regardless of their callback
 * @param[in]   _fDelay                               Delay between 2 calls of the timer to remove, -1.0f to remove all occurrences regardless of their respective delay
 * @param[in]   _pContext                             Context of the timer to remove, orxNULL to remove all occurrences regardless of their context
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_RemoveTimer(orxCLOCK *_pstClock, const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, void *_pContext)
{
  orxCLOCK_TIMER_STORAGE *pstTimerStorage;
  orxSTATUS               eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstClock);

  /* For all stored timers */
  for(pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetFirst(&(_pstClock->stTimerList));
      pstTimerStorage != orxNULL;
      pstTimerStorage = (orxCLOCK_TIMER_STORAGE *)orxLinkList_GetNext(&(pstTimerStorage->stNode)))
  {
    /* Matches criteria? */
    if(((_pfnCallback == orxNULL)
     || (pstTimerStorage->pfnCallback == _pfnCallback))
    && ((_fDelay < orxFLOAT_0)
     || (pstTimerStorage->fDelay == _fDelay))
    && ((_pContext == orxNULL)
     || (pstTimerStorage->pContext == _pContext)))
    {
      /* Marks it for deletion */
      pstTimerStorage->s32Repetition = 0;

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
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
  pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

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
 * @param[in]   _pfnCallback                          Concerned timer callback to remove, orxNULL to remove all occurrences regardless of their callback
 * @param[in]   _fDelay                               Delay between 2 calls of the timer to remove, -1.0f to remove all occurrences regardless of their respective delay
 * @param[in]   _pContext                             Context of the timer to remove, orxNULL to remove all occurrences regardless of their context
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxClock_RemoveGlobalTimer(const orxCLOCK_FUNCTION _pfnCallback, orxFLOAT _fDelay, void *_pContext)
{
  orxCLOCK *pstClock;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Gets core clock */
  pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

  /* Found? */
  if(pstClock != orxNULL)
  {
    /* Removes timer from it */
    eResult = orxClock_RemoveTimer(pstClock, _pfnCallback, _fDelay, _pContext);
  }

  /* Done! */
  return eResult;
}

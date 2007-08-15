/**
 * \file orxClock.c
 */

/***************************************************************************
 begin                : 04/02/2004
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "core/orxClock.h"

#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"


/*
 * Platform independant defines
 */

#define orxCLOCK_KU32_FLAG_NONE             0x00000000

#define orxCLOCK_KU32_FLAG_READY            0x00000001
#define orxCLOCK_KU32_FLAG_UPDATE_LOCK      0x10000000


/** Clock ID flags */
#define orxCLOCK_KU32_CLOCK_FLAG_NONE       0x00000000  /**< No flags */
#define orxCLOCK_KU32_CLOCK_FLAG_PAUSED     0x10000000  /**< Clock is paused */


/*
 * Internal clock function storage structure.
 */
typedef struct __orxCLOCK_FUNCTION_STORAGE_t
{
  /* Clock function pointer : 4 */
  orxCLOCK_FUNCTION pfnCallback;

  /* Clock function context : 8 */
  orxVOID          *pstContext;

  /* Clock function module ID : 12 */
  orxMODULE_ID      eModuleID;

  /* Padding */
  orxPAD(12)

} orxCLOCK_FUNCTION_STORAGE;

/*
 * Internal clock structure.
 */
struct __orxCLOCK_t
{
  /* Clock Info Structure : 32 */
  orxCLOCK_INFO stClockInfo;

  /* Callback bank : 36 */
  orxBANK *pstFunctionBank;

  /* Clock flags : 40 */
  orxU32 u32Flags;

  /* Padding */
  orxPAD(40)
};

/*
 * Static structure
 */
typedef struct __orxCLOCK_STATIC_t
{
  /* Clock banks */
  orxBANK          *pstClockBank;

  /* Clock mod type */
  orxCLOCK_MOD_TYPE eModType;

  /* Clock mod value */
  orxFLOAT          fModValue;

  /* Current time */
  orxU32            u32Time;

  /* Control flags */
  orxU32            u32Flags;

} orxCLOCK_STATIC;


/*
 * Static data
 */
orxSTATIC orxCLOCK_STATIC sstClock;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxClock_FindFunctionStorage
 Finds a clock function storage.

 returns: orxCLOCK_FUNCTION_STORAGE * / orxNULL
 ***************************************************************************/
orxSTATIC orxINLINE orxCLOCK_FUNCTION_STORAGE *orxClock_FindFunctionStorage(orxCONST orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds matching function storage */
  for(pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxBank_GetNext(_pstClock->pstFunctionBank, orxNULL);
      pstFunctionStorage != orxNULL;
      pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxBank_GetNext(_pstClock->pstFunctionBank, pstFunctionStorage))
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

/***************************************************************************
 orxClock_FindClock
 Finds a clock matching given criterias from a given start clock.

 returns: orxCLOCK * / orxNULL
 ***************************************************************************/
orxSTATIC orxINLINE orxCLOCK *orxClock_FindClock(orxU32 _u32TickSize, orxCLOCK_TYPE _eType, orxCONST orxCLOCK *_pstStartClock)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_eType < orxCLOCK_TYPE_NUMBER);

  /* Finds matching clock */
  for(pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, _pstStartClock);
      pstClock != orxNULL;
      pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, pstClock))
  {
    /* Match? */
    if((pstClock->stClockInfo.eType == _eType)
    && (pstClock->stClockInfo.u32TickSize == _u32TickSize))
    {
      /* Found */
      break;
    }
  }

  /* Done! */
  return pstClock;
}

/***************************************************************************
 orxClock_ComputeDT
 Computes DT according to Mod.

 returns: Computed DT.
 ***************************************************************************/
orxSTATIC orxINLINE orxU32 orxClock_ComputeDT(orxU32 _u32DT, orxCLOCK_INFO *_pstClockInfo)
{
  orxREGISTER orxCLOCK_MOD_TYPE  *peModType;
  orxREGISTER orxFLOAT           *pfModValue;
  orxREGISTER orxU32              u32NewDT = _u32DT;

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
      u32NewDT = orxF2U(*pfModValue);
      break;
    }

    case orxCLOCK_MOD_TYPE_MULTIPLY:
    {
      /* Multiplied DT value */
      u32NewDT = orxF2U(*pfModValue * orxU2F(_u32DT));
      break;
    }

    case orxCLOCK_MOD_TYPE_NONE:
    {
      break;
    }

    default:
    {
      /* !!! MSG !!! */
      break;
    }
  }

  /* Done! */  
  return u32NewDT;
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxClock_Setup
 Clock module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxClock_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_TIME);

  return;
}

/***************************************************************************
 orxClock_Init
 Inits the clock module.

 returns: orxSTATUS_SUCCESS/orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxClock_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;
  
  /* Not already Initialized? */
  if(!(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Set(&sstClock, 0, sizeof(orxCLOCK_STATIC));

    /* Creates clock bank */
    sstClock.pstClockBank = orxBank_Create(orxCLOCK_KU32_CLOCK_BANK_SIZE, sizeof(orxCLOCK), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstClock.pstClockBank != orxNULL)
    {
      /* No mod type by default */
      sstClock.eModType = orxCLOCK_MOD_TYPE_NONE;

      /* Gets init time */
      sstClock.u32Time  = orxTime_GetTime();

      /* Inits Flags */
      sstClock.u32Flags = orxCLOCK_KU32_FLAG_READY;

      /* Success */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Clock bank not created */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxClock_Exit
 Exits from clock module.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxClock_Exit()
{
  /* Initialized? */
  if(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY)
  {
    orxCLOCK *pstClock;

    /* For all clocks */
    for(pstClock = orxBank_GetNext(sstClock.pstClockBank, orxNULL);
        pstClock != orxNULL;
        pstClock = orxBank_GetNext(sstClock.pstClockBank, pstClock))
    {
      /* Deletes it */
      orxClock_Delete(pstClock);
    }

    /* Deletes clock bank */
    orxBank_Delete(sstClock.pstClockBank);
    sstClock.pstClockBank = orxNULL;

    /* Updates flags */
    sstClock.u32Flags &= ~orxCLOCK_KU32_FLAG_READY;
  }

  return;
}

/***************************************************************************
 orxorxClock_Update
 Udpates all clocks.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxClock_Update()
{
  orxU32    u32NewTime, u32DT;
  orxCLOCK *pstClock;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);

  /* Not already locked? */
  if(!(sstClock.u32Flags & orxCLOCK_KU32_FLAG_UPDATE_LOCK))
  {
    /* Lock clocks */
    sstClock.u32Flags |= orxCLOCK_KU32_FLAG_UPDATE_LOCK;

    /* Gets new time */
    u32NewTime  = orxTime_GetTime();

    /* Computes natural DT */
    u32DT        = u32NewTime - sstClock.u32Time;

    /* Gets modified DT */
    u32DT        = orxClock_ComputeDT(u32DT, orxNULL);

    /* For all clocks */
    for(pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, orxNULL);
        pstClock != orxNULL;
        pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, pstClock))
    {
      orxU32 u32Time, u32TickCounterBackup, u32ClockDT;
  
      /* Is clock not paused? */
      if(orxClock_IsPaused(pstClock) == orxFALSE)
      {
        /* Gets clock modified DT */
        u32ClockDT            = orxClock_ComputeDT(u32DT, &(pstClock->stClockInfo));
    
        /* Backups clock tick counter */
        u32TickCounterBackup  = pstClock->stClockInfo.u32TickCounter;
    
        /* Gets cumulated time */
        u32Time               = u32ClockDT + pstClock->stClockInfo.u32TickValue;
    
        /* Updates new ticks if needed */
        while(u32Time >= pstClock->stClockInfo.u32TickSize)
        {
          /* Updates tick counter */
          pstClock->stClockInfo.u32TickCounter++;
    
          /* Updates remaining cumulated time */
          u32Time -= pstClock->stClockInfo.u32TickSize;
        }
    
        /* Updates tick value */
        pstClock->stClockInfo.u32TickValue  = u32Time;
    
        /* Computes global time */
        pstClock->stClockInfo.u32Time      += u32ClockDT;
    
        /* New tick happened? */
        if(pstClock->stClockInfo.u32TickCounter != u32TickCounterBackup)
        {
          orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
    
          /* Updates clock DT for these calls */
          pstClock->stClockInfo.u32StableDT = (pstClock->stClockInfo.u32TickCounter - u32TickCounterBackup) * pstClock->stClockInfo.u32TickSize;
    
          /* For all registered callbacks */
          for(pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxBank_GetNext(pstClock->pstFunctionBank, orxNULL);
              pstFunctionStorage != orxNULL;
              pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxBank_GetNext(pstClock->pstFunctionBank, pstFunctionStorage))
          {
            /* Calls it */
            pstFunctionStorage->pfnCallback(&(pstClock->stClockInfo), pstFunctionStorage->pstContext);
          }
        }
      }
    }
  
    /* Updates time */
    sstClock.u32Time = u32NewTime;
    
    /* Unlocks clocks */
    sstClock.u32Flags &= ~orxCLOCK_KU32_FLAG_UPDATE_LOCK;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxClock_Create
 Creates a clock.

 returns: orxCLOCK * on success / NULL otherwise
 ***************************************************************************/
orxCLOCK *orxFASTCALL orxClock_Create(orxU32 _u32TickSize, orxCLOCK_TYPE _eType)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);

  /* Creates clock */
  pstClock = (orxCLOCK *)orxBank_Allocate(sstClock.pstClockBank);

  /* Valid? */
  if(pstClock != orxNULL)
  {
    /* Creates function bank */
    pstClock->pstFunctionBank = orxBank_Create(orxCLOCK_KU32_FUNCTION_BANK_SIZE, sizeof(orxCLOCK_FUNCTION_STORAGE), orxBANK_KU32_FLAGS_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstClock->pstFunctionBank != orxNULL)
    {
      /* Inits clock */
      pstClock->stClockInfo.u32TickSize = _u32TickSize;
      pstClock->stClockInfo.eType       = _eType;
      pstClock->stClockInfo.eModType    = orxCLOCK_MOD_TYPE_NONE;
      pstClock->u32Flags                = orxCLOCK_KU32_CLOCK_FLAG_NONE;
    }
    else
    {
      /* !!! MSG !!! */

      /* Releases allocated clock */
      orxBank_Free(sstClock.pstClockBank, pstClock);
      
      /* Not allocated */
      pstClock = orxNULL;
    }
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstClock;
}

/***************************************************************************
 orxClock_Delete
 Removes a clock.

 returns: orxVOID
 ***************************************************************************/
orxVOID orxFASTCALL orxClock_Delete(orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Not locked? */
  if((sstClock.u32Flags & orxCLOCK_KU32_FLAG_UPDATE_LOCK) == orxCLOCK_KU32_CLOCK_FLAG_NONE)
  {
	  /* Deletes function bank */
	  orxBank_Delete(_pstClock->pstFunctionBank);

	  /* Frees clock memory */
	  orxBank_Free(sstClock.pstClockBank, _pstClock);
  }

  return;
}

/** Resync a clock (accumulated DT => 0) */
orxVOID orxClock_Resync()
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);

  /* Not locked? */
  if((sstClock.u32Flags & orxCLOCK_KU32_FLAG_UPDATE_LOCK) == orxCLOCK_KU32_CLOCK_FLAG_NONE)
  {
	  /* Resync with current time */
  	sstClock.u32Time = orxTime_GetTime();
  }

  return;
}

/***************************************************************************
 orxClock_Pause
 Pauses a clock.

 returns: nothing
 ***************************************************************************/
orxVOID orxFASTCALL orxClock_Pause(orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Updates clock flags */
  _pstClock->u32Flags |= orxCLOCK_KU32_CLOCK_FLAG_PAUSED;

  return;
}

/***************************************************************************
 orxClock_Unpause
 Unpauses a clock.

 returns: nothing
 ***************************************************************************/
orxVOID orxFASTCALL orxClock_Unpause(orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Updates clock flags */
  _pstClock->u32Flags &= ~orxCLOCK_KU32_CLOCK_FLAG_PAUSED;

  return;
}

/***************************************************************************
 orxClock_IsPaused
 Is a clock paused?

 returns: orxTRUE / orxFALSE
 ***************************************************************************/
orxBOOL orxFASTCALL orxClock_IsPaused(orxCONST orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Tests flags */
  return((_pstClock->u32Flags & orxCLOCK_KU32_CLOCK_FLAG_PAUSED) ? orxTRUE : orxFALSE);
}

/***************************************************************************
 orxClock_GetInfo
 Gets informations about a clock.

 returns: Pointer on the clock info structure
 ***************************************************************************/
orxCONST orxCLOCK_INFO *orxFASTCALL  orxClock_GetInfo(orxCONST orxCLOCK *_pstClock)
{
  orxCONST orxCLOCK_INFO *pstClockInfo = orxNULL;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Gets the clock info structure pointer */
  pstClockInfo = &(_pstClock->stClockInfo);

  /* Done! */
  return pstClockInfo;
}

/***************************************************************************
 orxClock_Register
 Registers a callback function to a clock.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxClock_Register(orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback, orxVOID *_pstContext, orxMODULE_ID _eModuleID)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Gets function slot */
  pstFunctionStorage = (orxCLOCK_FUNCTION_STORAGE *)orxBank_Allocate(_pstClock->pstFunctionBank);

  /* Valid? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Stores callback */
    pstFunctionStorage->pfnCallback = _pfnCallback;

    /* Stores context */
    pstFunctionStorage->pstContext  = _pstContext;
    
    /* Stores module id */
    pstFunctionStorage->eModuleID   = _eModuleID;
  }
  else
  {
    /* !!! MSG !!! */
    
    /* Not successful */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxClock_Unregister
 Unregisters a callback function from a clock.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxClock_Unregister(orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_pfnCallback != orxNULL);

  /* Finds clock function storage */
  pstFunctionStorage = orxClock_FindFunctionStorage(_pstClock, _pfnCallback);

  /* Found? */
  if(pstFunctionStorage != orxNULL)
  {
    /* Removes storage from bank */
    orxBank_Free(_pstClock->pstFunctionBank, pstFunctionStorage);
  }
  else
  {
    /* !!! MSG !!! */

    /* Not found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxClock_GetContext
 Gets a callback function context.

 returns: orxVOID *
 ***************************************************************************/
orxVOID *orxFASTCALL orxClock_GetContext(orxCONST orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxVOID *pstContext = orxNULL;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
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
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstContext;
}

/***************************************************************************
 orxClock_SetContext
 Sets a callback function context.

 returns: orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 ***************************************************************************/
orxSTATUS orxFASTCALL orxClock_SetContext(orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback, orxVOID *_pstContext)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
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
    /* !!! MSG !!! */
    
    /* Not found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 orxClock_FindFirst
 Finds a clock according to its tick size and its type.

 returns: orxCLOCK * / orxNULL
 ***************************************************************************/
orxCLOCK *orxFASTCALL orxClock_FindFirst(orxU32 _u32TickSize, orxCLOCK_TYPE _eType)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_eType < orxCLOCK_TYPE_NUMBER);

  /* Finds first matching clock */
  pstClock = orxClock_FindClock(_u32TickSize, _eType, orxNULL);

  /* Done! */
  return pstClock;
}

/***************************************************************************
 orxClock_FindNext
 Finds next clock of same type/tick size.

 returns: orxCLOCK * / orxNULL
 ***************************************************************************/
orxCLOCK *orxFASTCALL orxClock_FindNext(orxCONST orxCLOCK *_pstClock)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Finds next matching clock */
  pstClock = orxClock_FindClock(_pstClock->stClockInfo.u32TickSize, _pstClock->stClockInfo.eType, _pstClock);
  
  /* Done! */
  return pstClock;
}

/***************************************************************************
 orxClock_GetNext
 Gets next existing clock (can be used to parse all existing clocks).

 returns: orxCLOCK * / orxNULL
 ***************************************************************************/
orxCLOCK *orxFASTCALL orxClock_GetNext(orxCONST orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_FLAG_READY);

  /* Returns next stored clock */
  return((orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, _pstClock));
}

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
 * @todo
 * - Add internal/external dependency system
 * - Optimize dependencies storage
 * - Add freezing behaviour
 */


#include "core/orxClock.h"

#include "debug/orxDebug.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "math/orxMath.h"
#include "utils/orxLinkList.h"


/** Module flags
 */
#define orxCLOCK_KU32_STATIC_FLAG_NONE          0x00000000  /**< No flags */

#define orxCLOCK_KU32_STATIC_FLAG_READY         0x00000001  /**< Ready flag */

#define orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK   0x10000000  /**< Lock update flag */

#define orxCLOCK_KU32_STATIC_MASK_ALL           0xFFFFFFFF  /**< All mask */


/** orxCLOCK flags
 */
#define orxCLOCK_KU32_CLOCK_FLAG_NONE           0x00000000  /**< No flags */

#define orxCLOCK_KU32_CLOCK_FLAG_PAUSED         0x10000000  /**< Clock is paused */

#define orxCLOCK_KU32_CLOCK_MASK_ALL            0xFFFFFFFF  /**< All mask */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Clock function storage structure
 */
typedef struct __orxCLOCK_FUNCTION_STORAGE_t
{
  orxLINKLIST_NODE            stNode;           /**< Linklist node : 12 */
  orxCLOCK_FUNCTION           pfnCallback;      /**< Clock function pointer : 16 */
  orxVOID                    *pstContext;       /**< Clock function context : 20 */
  orxMODULE_ID                eModuleID;        /**< Clock function module ID : 24 */
  orxCLOCK_FUNCTION_PRIORITY  ePriority;        /**< Clock function priority : 28 */

  orxPAD(28)

} orxCLOCK_FUNCTION_STORAGE;

/** Clock structure
 */
struct __orxCLOCK_t
{
  orxCLOCK_INFO stClockInfo;                    /**< Clock Info Structure : 24 */
  orxFLOAT      fRealTime;                      /**< Clock real time : 28 */
  orxFLOAT      fLastTick;                      /**< Clock last tick : 32 */
  orxLINKLIST   stFunctionList;                 /**< Function list : 44 */
  orxBANK      *pstFunctionBank;                /**< Function bank : 48 */
  orxU32        u32Flags;                       /**< Clock flags : 52 */

  orxPAD(52)
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

} orxCLOCK_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxCLOCK_STATIC sstClock;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Finds a clock function storage
 * @param[in]   _pstClock                             Concerned clock
 * @param[in]   _pfnCallback                          Concerned callback
 * @return      orxCLOCK_FUNCTION_STORAGE / orxNULL
 */
orxSTATIC orxINLINE orxCLOCK_FUNCTION_STORAGE *orxClock_FindFunctionStorage(orxCONST orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback)
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
orxSTATIC orxINLINE orxCLOCK *orxClock_FindClock(orxFLOAT _fTickSize, orxCLOCK_TYPE _eType, orxCONST orxCLOCK *_pstStartClock)
{
  orxCLOCK *pstClock;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_eType < orxCLOCK_TYPE_NUMBER);

  /* Finds matching clock */
  for(pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, _pstStartClock);
      pstClock != orxNULL;
      pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, pstClock))
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
orxSTATIC orxINLINE orxFLOAT orxClock_ComputeDT(orxFLOAT _fDT, orxCLOCK_INFO *_pstClockInfo)
{
  orxREGISTER orxCLOCK_MOD_TYPE  *peModType;
  orxREGISTER orxFLOAT           *pfModValue;
  orxREGISTER orxFLOAT            fNewDT = _fDT;

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
      fNewDT = *pfModValue;
      break;
    }

    case orxCLOCK_MOD_TYPE_MULTIPLY:
    {
      /* Multiplied DT value */
      fNewDT = *pfModValue * _fDT;
      break;
    }

    case orxCLOCK_MOD_TYPE_MAXED:
    {
      /* Updates DT value */
      fNewDT = orxMIN(*pfModValue, _fDT);
      break;
    }

    case orxCLOCK_MOD_TYPE_NONE:
    {
      break;
    }

    default:
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_CLOCK, "Invalid clock modifier type (%i).", *peModType );
      break;
    }
  }

  /* Done! */
  return fNewDT;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Clock module setup
 */
orxVOID orxClock_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_SYSTEM);
  orxModule_AddDependency(orxMODULE_ID_CLOCK, orxMODULE_ID_LINKLIST);

  return;
}

/** Inits clock module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxClock_Init()
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
      /* No mod type by default */
      sstClock.eModType = orxCLOCK_MOD_TYPE_NONE;

      /* Gets init time */
      sstClock.fTime  = orxSystem_GetTime();

      /* Inits Flags */
      sstClock.u32Flags = orxCLOCK_KU32_STATIC_FLAG_READY;

      /* Success */
      eResult = orxSTATUS_SUCCESS;
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
orxVOID orxClock_Exit()
{
  /* Initialized? */
  if(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY)
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
    sstClock.u32Flags &= ~orxCLOCK_KU32_STATIC_FLAG_READY;
  }

  return;
}

/** Updates the clock system
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxClock_Update()
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
    for(pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, orxNULL);
        pstClock != orxNULL;
        pstClock = (orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, pstClock))
    {
      orxFLOAT fClockDT, fDiff;

      /* Is clock not paused? */
      if(orxClock_IsPaused(pstClock) == orxFALSE)
      {
        /* Updates clock real time */
        pstClock->fRealTime += fDT;

        /* Gets time diff since last tick */
        fDiff = pstClock->fRealTime - pstClock->fLastTick;

        /* New tick happens? */
        if(fDiff >= pstClock->stClockInfo.fTickSize)
        {
          orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;

          /* Gets clock modified DT */
          fClockDT = orxClock_ComputeDT(fDiff, &(pstClock->stClockInfo));

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

          /* Updates last tick time stamp */
          pstClock->fLastTick = pstClock->fRealTime;
        }
      }
    }

    /* Unlocks clocks */
    sstClock.u32Flags &= ~orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK;

    /* Wait for next time slice */
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
      /* Inits clock */
      pstClock->stClockInfo.fTickSize   = _fTickSize;
      pstClock->stClockInfo.eType       = _eType;
      pstClock->stClockInfo.eModType    = orxCLOCK_MOD_TYPE_NONE;
      pstClock->u32Flags                = orxCLOCK_KU32_CLOCK_FLAG_NONE;
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

/** Deletes a clock
 * @param[in]   _pstClock                             Concerned clock
 */
orxVOID orxFASTCALL orxClock_Delete(orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Not locked? */
  if((sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK) == orxCLOCK_KU32_CLOCK_FLAG_NONE)
  {
	  /* Deletes function bank */
	  orxBank_Delete(_pstClock->pstFunctionBank);

	  /* Frees clock memory */
	  orxBank_Free(sstClock.pstClockBank, _pstClock);
  }

  return;
}

/** Resyncs a clock (accumulated DT => 0)
 */
orxVOID orxClock_Resync()
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Not locked? */
  if((sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_UPDATE_LOCK) == orxCLOCK_KU32_CLOCK_FLAG_NONE)
  {
    orxCLOCK *pstClock;

    /* For all clocks */
    for(pstClock = orxBank_GetNext(sstClock.pstClockBank, orxNULL);
        pstClock != orxNULL;
        pstClock = orxBank_GetNext(sstClock.pstClockBank, pstClock))
    {
      /* Resyncs clock time & real time */
      pstClock->fLastTick = pstClock->fRealTime = sstClock.fTime;
    }
  }

  return;
}

/** Pauses a clock
 * @param[in]   _pstClock                             Concerned clock
 */
orxVOID orxFASTCALL orxClock_Pause(orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Updates clock flags */
  _pstClock->u32Flags |= orxCLOCK_KU32_CLOCK_FLAG_PAUSED;

  return;
}

/** Unpauses a clock
 * @param[in]   _pstClock                             Concerned clock
 */
orxVOID orxFASTCALL orxClock_Unpause(orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Updates clock flags */
  _pstClock->u32Flags &= ~orxCLOCK_KU32_CLOCK_FLAG_PAUSED;

  return;
}

/** Is a clock paused?
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxTRUE if paused, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxClock_IsPaused(orxCONST orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);

  /* Tests flags */
  return((_pstClock->u32Flags & orxCLOCK_KU32_CLOCK_FLAG_PAUSED) ? orxTRUE : orxFALSE);
}

/** Gets clock info
 * @param[in]   _pstClock                             Concerned clock
 * @return      orxCLOCK_INFO / orxNULL
 */
orxCONST orxCLOCK_INFO *orxFASTCALL  orxClock_GetInfo(orxCONST orxCLOCK *_pstClock)
{
  orxCONST orxCLOCK_INFO *pstClockInfo = orxNULL;

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
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstClock != orxNULL);
  orxASSERT(_eModType < orxCLOCK_MOD_TYPE_NUMBER);
  orxASSERT(_fModValue >= orxFLOAT_0);

  /* Updates clock modifier */
  _pstClock->stClockInfo.eModType   = _eModType;
  _pstClock->stClockInfo.fModValue  = _fModValue;

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
orxSTATUS orxFASTCALL orxClock_Register(orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback, orxVOID *_pstContext, orxMODULE_ID _eModuleID, orxCLOCK_FUNCTION_PRIORITY _ePriority)
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
orxSTATUS orxFASTCALL orxClock_Unregister(orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback)
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
orxVOID *orxFASTCALL orxClock_GetContext(orxCONST orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback)
{
  orxCLOCK_FUNCTION_STORAGE *pstFunctionStorage;
  orxVOID *pstContext = orxNULL;

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
orxSTATUS orxFASTCALL orxClock_SetContext(orxCLOCK *_pstClock, orxCONST orxCLOCK_FUNCTION _pfnCallback, orxVOID *_pstContext)
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
orxCLOCK *orxFASTCALL orxClock_FindNext(orxCONST orxCLOCK *_pstClock)
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
orxCLOCK *orxFASTCALL orxClock_GetNext(orxCONST orxCLOCK *_pstClock)
{
  /* Checks */
  orxASSERT(sstClock.u32Flags & orxCLOCK_KU32_STATIC_FLAG_READY);

  /* Returns next stored clock */
  return((orxCLOCK *)orxBank_GetNext(sstClock.pstClockBank, _pstClock));
}

/**
 * @file orxFXPointer.c
 */

/***************************************************************************
 orxFXPointer.c
 FX module
 begin                : 30/06/2008
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/


#include "render/orxFXPointer.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxClock.h"
#include "object/orxStructure.h"
#include "object/orxObject.h"


/** Module flags
 */
#define orxFXPOINTER_KU32_STATIC_FLAG_NONE      0x00000000

#define orxFXPOINTER_KU32_STATIC_FLAG_READY     0x00000001

#define orxFXPOINTER_KU32_STATIC_MASK_ALL       0xFFFFFFFF


/** Flags
 */
#define orxFXPOINTER_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxFXPOINTER_KU32_FLAG_ENABLED          0x10000000  /**< Enabled flag */

#define orxFXPOINTER_KU32_MASK_ALL              0xFFFFFFFF  /**< All mask */


/** Holder flags
 */
#define orxFXPOINTER_HOLDER_KU32_FLAG_NONE      0x00000000  /**< No flags */

#define orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL  0x10000000  /**< Internal flag */

#define orxFXPOINTER_HOLDER_KU32_MASK_ALL       0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFXPOINTER_KU32_FX_NUMBER             4


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** FX holder structure
 */
typedef struct __orxFXPOINTER_HOLDER_t
{
  orxFX    *pstFX;                                          /**< FX reference : 4 */
  orxFLOAT  fStartTime;                                     /**< Start time : 8 */
  orxU32    u32Flags;                                       /**< Flags : 12 */

} orxFXPOINTER_HOLDER;

/** FX structure
 */
struct __orxFXPOINTER_t
{
  orxSTRUCTURE        stStructure;                          /**< Public structure, first structure member : 16 */
  orxFXPOINTER_HOLDER astFXList[orxFXPOINTER_KU32_FX_NUMBER];/**< FX list : 64 */
  orxFLOAT            fTime;                                /**< Time stamp : 68 */

  /* Padding */
  orxPAD(68)
};

/** Static structure
 */
typedef struct __orxFXPOINTER_STATIC_t
{
  orxU32 u32Flags;                                            /**< Control flags */

} orxFXPOINTER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxFXPOINTER_STATIC sstFXPointer;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Deletes all the FXs
 */
orxSTATIC orxINLINE orxVOID orxFXPointer_DeleteAll()
{
  orxFXPOINTER *pstFXPointer;

  /* Gets first FX */
  pstFXPointer = orxFXPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_FXPOINTER));

  /* Non empty? */
  while(pstFXPointer != orxNULL)
  {
    /* Deletes it */
    orxFXPointer_Delete(pstFXPointer);

    /* Gets first FX */
    pstFXPointer = orxFXPOINTER(orxStructure_GetFirst(orxSTRUCTURE_ID_FX));
  }

  return;
}

/** Updates the FXPointer (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATIC orxSTATUS orxFASTCALL orxFXPointer_Update(orxSTRUCTURE *_pstStructure, orxCONST orxSTRUCTURE *_pstCaller, orxCONST orxCLOCK_INFO *_pstClockInfo)
{
  orxFXPOINTER *pstFXPointer;
  orxOBJECT    *pstObject;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);

  /* Gets FXPointer */
  pstFXPointer = orxFXPOINTER(_pstStructure);

  /* Gets calling object */
  pstObject = orxOBJECT(_pstCaller);

  /* Is enabled? */
  if(orxFXPointer_IsEnabled(pstFXPointer) != orxFALSE)
  {
    orxFLOAT  fLastTime;
    orxU32    i;

    /* Backups last time */
    fLastTime = pstFXPointer->fTime;

    /* Computes its new time cursor */
    pstFXPointer->fTime += _pstClockInfo->fDT;

    /* For all FXs */
    for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
    {
      orxFX *pstFX;

      /* Gets FX */
      pstFX = pstFXPointer->astFXList[i].pstFX;

      /* Valid? */
      if(pstFX != orxNULL)
      {
        orxFLOAT fFXLocalStartTime, fFXLocalEndTime;

        /* Gets FX local times */
        fFXLocalStartTime = fLastTime - pstFXPointer->astFXList[i].fStartTime;
        fFXLocalEndTime   = pstFXPointer->fTime - pstFXPointer->astFXList[i].fStartTime;

        /* Applies FX from last time to now */
        if(orxFX_Apply(pstFX, pstObject, fFXLocalStartTime, fFXLocalEndTime) == orxSTATUS_FAILURE)
        {
          /* Is a looping FX? */
          if(orxFX_IsLooping(pstFX) != orxFALSE)
          {
            /* Updates its start time */
            pstFXPointer->astFXList[i].fStartTime = pstFXPointer->fTime;
          }
          else
          {
            /* Decreases its reference counter */
            orxStructure_DecreaseCounter(pstFX);

            /* Removes its reference */
            pstFXPointer->astFXList[i].pstFX = orxNULL;

            /* Is internal? */
            if(orxFLAG_TEST(pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
            {
              /* Deletes it */
              orxFX_Delete(pstFX);
            }
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** FXPointer module setup
 */
orxVOID orxFXPointer_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_FXPOINTER, orxMODULE_ID_FX);

  return;
}

/** Inits the FXPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFXPointer_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFXPointer, sizeof(orxFXPOINTER_STATIC));

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(FXPOINTER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxFXPointer_Update);

    /* Initialized? */
    if(eResult == orxSTATUS_SUCCESS)
    {
      /* Inits Flags */
      sstFXPointer.u32Flags = orxFXPOINTER_KU32_STATIC_FLAG_READY;
    }
    else
    {
      /* !!! MSG !!! */
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

/** Exits from the FXPointer module
 */
orxVOID orxFXPointer_Exit()
{
  /* Initialized? */
  if(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY)
  {
    /* Deletes FX list */
    orxFXPointer_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FXPOINTER);

    /* Updates flags */
    sstFXPointer.u32Flags &= ~orxFXPOINTER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/** Creates an empty FXPointer
 * @return      Created orxFXPOINTER / orxNULL
 */
orxFXPOINTER *orxFXPointer_Create()
{
  orxFXPOINTER *pstResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);

  /* Creates FX */
  pstResult = orxFXPOINTER(orxStructure_Create(orxSTRUCTURE_ID_FXPOINTER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxFXPOINTER_KU32_FLAG_ENABLED, orxFXPOINTER_KU32_MASK_ALL);
  }
  else
  {
    /* !!! MSG !!! */
  }

  /* Done! */
  return pstResult;
}

/** Deletes an FXPointer
 * @param[in] _pstFXPointer     Concerned FXPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_Delete(orxFXPOINTER *_pstFXPointer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstFXPointer) == 0)
  {
    orxU32 i;

    /* For all FXs */
    for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
    {
      /* Valid? */
      if(_pstFXPointer->astFXList[i].pstFX != orxNULL)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(_pstFXPointer->astFXList[i].pstFX);

        /* Is internal? */
        if(orxFLAG_TEST(_pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxFX_Delete(_pstFXPointer->astFXList[i].pstFX);
        }
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstFXPointer);
  }
  else
  {
    /* !!! MSG !!! */

    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Enables/disables an FXPointer
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _bEnable      enable / disable
 */
orxVOID orxFASTCALL orxFXPointer_Enable(orxFXPOINTER *_pstFXPointer, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstFXPointer, orxFXPOINTER_KU32_FLAG_ENABLED, orxFXPOINTER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstFXPointer, orxFXPOINTER_KU32_FLAG_NONE, orxFXPOINTER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is FXPointer enabled?
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFXPointer_IsEnabled(orxCONST orxFXPOINTER *_pstFXPointer)
{
  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);

  /* Done! */
  return(orxStructure_TestFlags(_pstFXPointer, orxFXPOINTER_KU32_FLAG_ENABLED));
}

/** Adds an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* Adds FX */
  eResult = orxFXPointer_AddDelayedFX(_pstFXPointer, _pstFX, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds a delayed FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddDelayedFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX, orxFLOAT _fDelay)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstFX);
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxFXPOINTER_KU32_FX_NUMBER) && (_pstFXPointer->astFXList[u32Index].pstFX != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxFXPOINTER_KU32_FX_NUMBER)
  {
    /* Increases its reference counter */
    orxStructure_IncreaseCounter(_pstFX);

    /* Adds it to holder */
    _pstFXPointer->astFXList[u32Index].pstFX = _pstFX;

    /* Inits its start time */
    _pstFXPointer->astFXList[u32Index].fStartTime = _pstFXPointer->fTime + _fDelay;

    /* Updates its flags */
    orxFLAG_SET(_pstFXPointer->astFXList[u32Index].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_NONE, orxFXPOINTER_HOLDER_KU32_MASK_ALL);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes an FX
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _pstFX        FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_RemoveFX(orxFXPOINTER *_pstFXPointer, orxFX *_pstFX)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxSTRUCTURE_ASSERT(_pstFX);

  /* For all slots */
  for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
  {
    orxFX *pstFX;

    /* Gets FX */
    pstFX = _pstFXPointer->astFXList[i].pstFX;

    /* Valid? */
    if(pstFX != orxNULL)
    {
      /* Found? */
      if(pstFX == _pstFX)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstFX);

        /* Removes its reference */
        _pstFXPointer->astFXList[i].pstFX = orxNULL;

        /* Is internal? */
        if(orxFLAG_TEST(_pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxFX_Delete(pstFX);
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

/** Adds an FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddFXFromConfig(orxFXPOINTER *_pstFXPointer, orxCONST orxSTRING _zFXConfigID)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (*_zFXConfigID != *orxSTRING_EMPTY));

  /* Adds FX */
  eResult = orxFXPointer_AddDelayedFXFromConfig(_pstFXPointer, _zFXConfigID, orxFLOAT_0);

  /* Done! */
  return eResult;
}

/** Adds a delayed FX using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to add
 * @param[in]   _fDelay       Delay time
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_AddDelayedFXFromConfig(orxFXPOINTER *_pstFXPointer, orxCONST orxSTRING _zFXConfigID, orxFLOAT _fDelay)
{
  orxU32    u32Index;
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (*_zFXConfigID != *orxSTRING_EMPTY));
  orxASSERT(_fDelay >= orxFLOAT_0);

  /* Finds an empty slot */
  for(u32Index = 0; (u32Index < orxFXPOINTER_KU32_FX_NUMBER) && (_pstFXPointer->astFXList[u32Index].pstFX != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxFXPOINTER_KU32_FX_NUMBER)
  {
    orxFX *pstFX;

    /* Creates FX */
    pstFX = orxFX_CreateFromConfig(_zFXConfigID);

    /* Valid? */
    if(pstFX != orxNULL)
    {
      /* Increases its reference counter */
      orxStructure_IncreaseCounter(pstFX);

      /* Adds it to holder */
      _pstFXPointer->astFXList[u32Index].pstFX = pstFX;

      /* Inits its start time */
      _pstFXPointer->astFXList[u32Index].fStartTime = _pstFXPointer->fTime + _fDelay;

      /* Updates its flags */
      orxFLAG_SET(_pstFXPointer->astFXList[u32Index].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL, orxFXPOINTER_HOLDER_KU32_MASK_ALL);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* !!! MSG !!! */

      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }
  else
  {
    /* !!! MSG !!! */

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes an FX using using its config ID
 * @param[in]   _pstFXPointer Concerned FXPointer
 * @param[in]   _zFXConfigID  Config ID of the FX to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFXPointer_RemoveFXFromConfig(orxFXPOINTER *_pstFXPointer, orxCONST orxSTRING _zFXConfigID)
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstFXPointer.u32Flags & orxFXPOINTER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFXPointer);
  orxASSERT((_zFXConfigID != orxNULL) && (*_zFXConfigID != *orxSTRING_EMPTY));

  /* For all slots */
  for(i = 0; i < orxFXPOINTER_KU32_FX_NUMBER; i++)
  {
    orxFX *pstFX;

    /* Gets FX */
    pstFX = _pstFXPointer->astFXList[i].pstFX;

    /* Valid? */
    if(pstFX != orxNULL)
    {
      /* Found? */
      if(orxFX_IsName(pstFX, _zFXConfigID) != orxFALSE)
      {
        /* Decreases its reference counter */
        orxStructure_DecreaseCounter(pstFX);

        /* Removes its reference */
        _pstFXPointer->astFXList[i].pstFX = orxNULL;

        /* Is internal? */
        if(orxFLAG_TEST(_pstFXPointer->astFXList[i].u32Flags, orxFXPOINTER_HOLDER_KU32_FLAG_INTERNAL))
        {
          /* Deletes it */
          orxFX_Delete(pstFX);
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

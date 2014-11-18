/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2014 Orx-Project
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
 * @file orxFrame.c
 * @date 02/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxFrame.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxStructure.h"
#include "math/orxMath.h"


/** Module flags
 */
#define orxFRAME_KU32_STATIC_FLAG_NONE      0x00000000  /**< No flags */

#define orxFRAME_KU32_STATIC_FLAG_READY     0x00000001  /**< Ready flag */
#define orxFRAME_KU32_STATIC_FLAG_DATA_2D   0x00000010  /**< 2D flag */

#define orxFRAME_KU32_STATIC_MASK_DEFAULT   0x00000010  /**< Default flag */

#define orxFRAME_KU32_STATIC_MASK_ALL       0xFFFFFFFF  /**< All mask */


/** orxFRAME flags
 */
#define orxFRAME_KU32_FLAG_NONE             0x00000000  /**< No flags */

#define orxFRAME_KU32_FLAG_DATA_2D          0x10000000  /**< 2D ID flag */

#define orxFRAME_KU32_MASK_ALL              0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFRAME_KU32_BANK_SIZE             512         /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal 2D Frame Data structure
 */
typedef struct __orxFRAME_DATA_2D_t
{
  orxVECTOR vGlobalPos;                     /**< Global 2D coordinates : 12 */
  orxFLOAT  fGlobalRotation;                   /**< Global 2D rotation angle : 16 */
  orxVECTOR vLocalPos;                      /**< Local 2D coordinates : 28 */
  orxFLOAT  fLocalRotation;                    /**< Local 2D rotation angle : 32 */
  orxFLOAT  fGlobalScaleX;                  /**< Global 2D isometric X scale : 36 */
  orxFLOAT  fGlobalScaleY;                  /**< Global 2D isometric Y scale : 40 */
  orxFLOAT  fLocalScaleX;                   /**< Local 2D isometric X scale : 44 */
  orxFLOAT  fLocalScaleY;                   /**< Local 2D isometric Y scale : 48 */

} orxFRAME_DATA_2D;


/** Frame structure
 */
struct __orxFRAME_t
{
  orxSTRUCTURE      stStructure;            /**< Public structure, first structure member : 32 */
  orxFRAME_DATA_2D  stData;                 /**< Frame data : 64 */
};

/** Static structure
 */
typedef struct __orxFRAME_STATIC_t
{
  orxU32    u32Flags;                       /**< Control flags : 4 */
  orxFRAME *pstRoot;                        /**< Frame root : 8 */
  orxBANK  *pst2DDataBank;                  /**< 2D Data bank : 12 */

} orxFRAME_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFRAME_STATIC sstFrame;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Sets frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pvPos          Position to set
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxTRUE if changed, orxFALSE otherwise
 */
static orxINLINE orxBOOL _orxFrame_SetPosition(orxFRAME *_pstFrame, const orxVECTOR *_pvPos, orxFRAME_SPACE _eSpace)
{
  orxBOOL bResult = orxFALSE;

  /* Global? */
  if(_eSpace == orxFRAME_SPACE_GLOBAL)
  {
    /* Different? */
    if((_pvPos->fX != _pstFrame->stData.vGlobalPos.fX)
    || (_pvPos->fY != _pstFrame->stData.vGlobalPos.fY)
    || (_pvPos->fZ != _pstFrame->stData.vGlobalPos.fZ))
    {
      /* Stores it */
      orxVector_Copy(&(_pstFrame->stData.vGlobalPos), _pvPos);

      /* Updates result */
      bResult = orxTRUE;
    }
  }
  else
  {
    /* Different? */
    if((_pvPos->fX != _pstFrame->stData.vLocalPos.fX)
    || (_pvPos->fY != _pstFrame->stData.vLocalPos.fY)
    || (_pvPos->fZ != _pstFrame->stData.vLocalPos.fZ))
    {
      /* Stores it */
      orxVector_Copy(&(_pstFrame->stData.vLocalPos), _pvPos);

      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Sets frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _fRotation         Rotation angle to set (radians)
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxTRUE if changed, orxFALSE otherwise
 */
static orxINLINE orxBOOL _orxFrame_SetRotation(orxFRAME *_pstFrame, orxFLOAT _fRotation, orxFRAME_SPACE _eSpace)
{
  orxBOOL bResult = orxFALSE;

  /* Global? */
  if(_eSpace == orxFRAME_SPACE_GLOBAL)
  {
    /* Different? */
    if(_fRotation != _pstFrame->stData.fGlobalRotation)
    {
      /* Stores it */
      _pstFrame->stData.fGlobalRotation = _fRotation;

      /* Updates result */
      bResult = orxTRUE;
    }
  }
  else
  {
    /* Different? */
    if(_fRotation != _pstFrame->stData.fLocalRotation)
    {
      /* Stores it */
      _pstFrame->stData.fLocalRotation = _fRotation;

      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Sets frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pvScale        Scale to set
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxTRUE if changed, orxFALSE otherwise
 */
static orxINLINE orxBOOL _orxFrame_SetScale(orxFRAME *_pstFrame, const orxVECTOR *_pvScale, orxFRAME_SPACE _eSpace)
{
  orxBOOL bResult = orxFALSE;

  /* Global? */
  if(_eSpace == orxFRAME_SPACE_GLOBAL)
  {
    /* Different? */
    if((_pvScale->fX != _pstFrame->stData.fGlobalScaleX)
    || (_pvScale->fY != _pstFrame->stData.fGlobalScaleY))
    {
      /* Stores it */
      _pstFrame->stData.fGlobalScaleX = _pvScale->fX;
      _pstFrame->stData.fGlobalScaleY = _pvScale->fY;

      /* Updates result */
      bResult = orxTRUE;
    }
  }
  else
  {
    /* Different? */
    if((_pvScale->fX != _pstFrame->stData.fLocalScaleX)
    || (_pvScale->fY != _pstFrame->stData.fLocalScaleY))
    {
      /* Stores it */
      _pstFrame->stData.fLocalScaleX = _pvScale->fX;
      _pstFrame->stData.fLocalScaleY = _pvScale->fY;

      /* Updates result */
      bResult = orxTRUE;
    }
  }

  /* Done! */
  return bResult;
}

/** Gets frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxVECTOR / orxNULL
 */
static orxINLINE const orxVECTOR *_orxFrame_GetPosition(const orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace)
{
  const orxVECTOR *pvResult = orxNULL;

  /* Updates result */
  if(_eSpace == orxFRAME_SPACE_GLOBAL)
  {
    pvResult = &(_pstFrame->stData.vGlobalPos);
  }
  else
  {
    pvResult = &(_pstFrame->stData.vLocalPos);
  }

  /* Done */
  return pvResult;
}

/** Gets frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return orxFLOAT (radians) / orxNULL
 */
static orxINLINE orxFLOAT _orxFrame_GetRotation(const orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace)
{
  orxFLOAT fRotation = orxFLOAT_0;

  /* Updates result */
  if(_eSpace == orxFRAME_SPACE_GLOBAL)
  {
    fRotation = _pstFrame->stData.fGlobalRotation;
  }
  else
  {
    fRotation = _pstFrame->stData.fLocalRotation;
  }

  /* Done */
  return fRotation;
}

/** Gets frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvScale        Scale
 * @return      orxVECTOR / orxNULL
 */
static orxINLINE orxVECTOR *_orxFrame_GetScale(const orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult = _pvScale;

  /* Updates result */
  if(_eSpace == orxFRAME_SPACE_GLOBAL)
  {
    _pvScale->fX = _pstFrame->stData.fGlobalScaleX;
    _pvScale->fY = _pstFrame->stData.fGlobalScaleY;
  }
  else
  {
    _pvScale->fX = _pstFrame->stData.fLocalScaleX;
    _pvScale->fY = _pstFrame->stData.fLocalScaleY;
  }

  /* No z scale */
  _pvScale->fZ = orxFLOAT_1;

  /* Done */
  return pvResult;
}

static orxINLINE orxVECTOR *orxFrame_FromLocalToGlobalPosition(const orxFRAME *_pstFrame, orxVECTOR *_pvPos)
{
  orxVECTOR *pvResult;

  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    const orxVECTOR  *pvGlobalPos;
    orxVECTOR         vGlobalScale, vPos;
    orxFLOAT          fGlobalRotation;

    /* Gets global data */
    _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL, &vGlobalScale);
    fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);
    pvGlobalPos     = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_GLOBAL);

    /* Transforms input position with scale */
    orxVector_Mul(&vPos, _pvPos, &vGlobalScale);

    /* Has rotation? */
    if(fGlobalRotation != orxFLOAT_0)
    {
      /* Transforms input position with rotation */
      orxVector_2DRotate(&vPos, &vPos, fGlobalRotation);
    }

    /* Transforms input position with translation (position) */
    orxVector_Add(_pvPos, &vPos, pvGlobalPos);

    /* Updates result */
    pvResult = _pvPos;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

static orxINLINE orxVECTOR *orxFrame_FromGlobalToLocalPosition(const orxFRAME *_pstFrame, orxVECTOR *_pvPos)
{
  orxVECTOR *pvResult;

  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    const orxVECTOR  *pvGlobalPos;
    orxVECTOR         vGlobalScale, vPos;
    orxFLOAT          fGlobalRotation;

    /* Gets parent's global data */
    pvGlobalPos     = _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_GLOBAL);
    fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);
    _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL, &vGlobalScale);

    /* Transforms input position with translation (position) */
    orxVector_Sub(&vPos, _pvPos, pvGlobalPos);

    /* Has rotation? */
    if(fGlobalRotation != orxFLOAT_0)
    {
      /* Transforms input position with rotation */
      orxVector_2DRotate(&vPos, &vPos, -fGlobalRotation);
    }

    /* Valid scale? */
    if((vGlobalScale.fX != orxFLOAT_0) && (vGlobalScale.fY != orxFLOAT_0))
    {
      /* Transforms input position with scale */
      orxVector_Div(&vPos, &vPos, &vGlobalScale);
    }

    /* Updates result */
    orxVector_Copy(_pvPos, &vPos);
    pvResult = _pvPos;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

static orxINLINE orxFLOAT orxFrame_FromLocalToGlobalRotation(const orxFRAME *_pstFrame, orxFLOAT _fRotation)
{
  orxFLOAT fResult;

  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    orxFLOAT fGlobalRotation;

    /* Gets global angle */
    fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);

    /* Transforms input rotation */
    fResult = _fRotation + fGlobalRotation;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

static orxINLINE orxFLOAT orxFrame_FromGlobalToLocalRotation(const orxFRAME *_pstFrame, orxFLOAT _fRotation)
{
  orxFLOAT fResult;

  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    orxFLOAT fGlobalRotation;

    /* Gets global angle */
    fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);

    /* Transforms input rotation */
    fResult = _fRotation - fGlobalRotation;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");

    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

static orxINLINE orxVECTOR *orxFrame_FromLocalToGlobalScale(const orxFRAME *_pstFrame, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    orxVECTOR vGlobalScale;

    /* Gets global scale */
    _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL, &vGlobalScale);

    /* Transforms input scale */
    orxVector_Mul(_pvScale, _pvScale, &vGlobalScale);

    /* Updates result */
    pvResult = _pvScale;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

static orxINLINE orxVECTOR *orxFrame_FromGlobalToLocalScale(const orxFRAME *_pstFrame, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    orxVECTOR vGlobalScale;

    /* Gets global scale */
    _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_GLOBAL, &vGlobalScale);

    /* Transforms input scale */
    if((vGlobalScale.fX != orxFLOAT_0) && (vGlobalScale.fY != orxFLOAT_0))
    {
      orxVector_Div(_pvScale, _pvScale, &vGlobalScale);
    }

    /* Updates result */
    pvResult = _pvScale;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Updates frame global data using parent's global and frame local ones
 * @param[out]  _pstFrame       Frame to update
 * @param[in]   _pstParent Parent frame to use for update
 */
static orxINLINE void orxFrame_UpdateData(orxFRAME *_pstFrame, const orxFRAME *_pstParent)
{
  /* 2D data? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    orxVECTOR vPos, vScale;
    orxFLOAT  fRotation;

    /* Gets frame's local data */
    _orxFrame_GetScale(_pstFrame, orxFRAME_SPACE_LOCAL, &vScale);
    fRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_LOCAL);
    orxVector_Copy(&vPos, _orxFrame_GetPosition(_pstFrame, orxFRAME_SPACE_LOCAL));

    /* Is not root? */
    if(_pstParent != sstFrame.pstRoot)
    {
      /* Transforms them */
      orxFrame_FromLocalToGlobalScale(_pstParent, &vScale);
      fRotation = orxFrame_FromLocalToGlobalRotation(_pstParent, fRotation);
      orxFrame_FromLocalToGlobalPosition(_pstParent, &vPos);
    }

    /* Stores them */
    _orxFrame_SetRotation(_pstFrame, fRotation, orxFRAME_SPACE_GLOBAL);
    _orxFrame_SetScale(_pstFrame, &vScale, orxFRAME_SPACE_GLOBAL);
    _orxFrame_SetPosition(_pstFrame, &vPos, orxFRAME_SPACE_GLOBAL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
  }

  return;
}

/** Processes a frame
 * @param[in]   _pstFrame       Concerned frame
 */
static void orxFASTCALL orxFrame_Process(orxFRAME *_pstFrame, const orxFRAME *_pstParent)
{
  orxFRAME *pstChild;

  /* Updates frame's data */
  orxFrame_UpdateData(_pstFrame, _pstParent);

  /* For all children */
  for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
      pstChild != orxNULL;
      pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
  {
    /* Updates it */
    orxFrame_Process(pstChild, _pstFrame);
  }

  /* Done! */
  return;
}

/** Deletes all frames
 */
static orxINLINE void orxFrame_DeleteAll()
{
  register orxFRAME *pstFrame;

  /* Gets first frame */
  pstFrame = orxFRAME(orxStructure_GetChild(sstFrame.pstRoot));

  /* Untill only root remains */
  while(pstFrame != orxNULL)
  {
    /* Deletes firt child cell */
    orxFrame_Delete(pstFrame);

    /* Gets root new child */
    pstFrame = (orxFRAME *)orxStructure_GetChild(sstFrame.pstRoot);
  }

  /* Removes root */
  orxFrame_Delete(sstFrame.pstRoot);
  sstFrame.pstRoot = orxNULL;

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Animation module setup
 */
void orxFASTCALL orxFrame_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_PROFILER);

  return;
}

/** Inits the Frame module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFrame_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstFrame, sizeof(orxFRAME_STATIC));

    /* Inits flags */
    sstFrame.u32Flags = orxFRAME_KU32_STATIC_MASK_DEFAULT|orxFRAME_KU32_STATIC_FLAG_READY;

    /* Registers structure type */
    eResult = orxSTRUCTURE_REGISTER(FRAME, orxSTRUCTURE_STORAGE_TYPE_TREE, orxMEMORY_TYPE_MAIN, orxFRAME_KU32_BANK_SIZE, orxNULL);

    /* Successful? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Inits frame tree */
      sstFrame.pstRoot = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

      /* Not created? */
      if(sstFrame.pstRoot == orxNULL)
      {
        /* Unregister structure type */
        orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

        /* Cleans flags */
        sstFrame.u32Flags = orxFRAME_KU32_STATIC_FLAG_NONE;

        /* Can't process */
        eResult = orxSTATUS_FAILURE;
      }
      else
      {
        /* Continue */
        eResult = orxSTATUS_SUCCESS;
      }
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize frame mdoule when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the Frame module
 */
void orxFASTCALL orxFrame_Exit()
{
  /* Initialized? */
  if(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY)
  {
    /* Deletes frame tree */
    orxFrame_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

    /* Updates flags */
    sstFrame.u32Flags &= ~orxFRAME_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit frame module when it wasn't initialized.");
  }

  return;
}

/** Creates a frame
 * @param[in]   _u32Flags     flags for created animation
 * @return      Created orxFRAME / orxNULL
 */
orxFRAME *orxFASTCALL orxFrame_Create(orxU32 _u32Flags)
{
  orxFRAME *pstFrame;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxASSERT((_u32Flags & orxFRAME_KU32_MASK_USER_ALL) == _u32Flags);

  /* Creates frame */
  pstFrame = orxFRAME(orxStructure_Create(orxSTRUCTURE_ID_FRAME));

  /* Valid? */
  if(pstFrame != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstFrame, _u32Flags & orxFRAME_KU32_MASK_USER_ALL, orxFRAME_KU32_MASK_ALL);

    /* Inits members */
    if(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_DATA_2D)
    {
      /* Updates flags */
      orxStructure_SetFlags(pstFrame, orxFRAME_KU32_FLAG_DATA_2D, orxFRAME_KU32_FLAG_NONE);

      /* Inits values */
      pstFrame->stData.fGlobalScaleX  = orxFLOAT_1;
      pstFrame->stData.fGlobalScaleY  = orxFLOAT_1;
      pstFrame->stData.fLocalScaleX   = orxFLOAT_1;
      pstFrame->stData.fLocalScaleY   = orxFLOAT_1;

      /* Has already a root? */
      if(sstFrame.pstRoot != orxNULL)
      {
        /* Sets frame to root */
        orxFrame_SetParent(pstFrame, sstFrame.pstRoot);
      }

      /* Increases counter */
      orxStructure_IncreaseCounter(pstFrame);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d data yet.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create structure for frame.");
  }

  /* Done! */
  return pstFrame;
}

/** Deletes a frame
 * @param[in]   _pstFrame       Frame to delete
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFrame_Delete(orxFRAME *_pstFrame)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstFrame);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstFrame) == 0)
  {
    /* Deletes structure */
    orxStructure_Delete(_pstFrame);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets frame parent
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pstParent      Parent frame to set
 */
void orxFASTCALL orxFrame_SetParent(orxFRAME *_pstFrame, orxFRAME *_pstParent)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Has no parent? */
  if(_pstParent == orxNULL)
  {
    /* Is current parent not root? */
    if(orxStructure_GetParent(_pstFrame) != orxSTRUCTURE(sstFrame.pstRoot))
    {
      /* Root is parent */
      orxStructure_SetParent(_pstFrame, sstFrame.pstRoot);

      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* Processes frame */
      orxFrame_Process(_pstFrame, sstFrame.pstRoot);

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
  }
  else
  {
    /* New parent? */
    if(orxStructure_GetParent(_pstFrame) != orxSTRUCTURE(_pstParent))
    {
      /* Sets parent */
      orxStructure_SetParent(_pstFrame, _pstParent);

      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* Processes frame */
      orxFrame_Process(_pstFrame, _pstParent);

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
  }

  /* Done! */
  return;
}

/** Gets frame parent
 * @param[in]   _pstFrame       Concerned frame
 * @return orxFRAME / orxNULL
 */
orxFRAME *orxFASTCALL orxFrame_GetParent(const orxFRAME *_pstFrame)
{
  orxFRAME *pstResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Gets parent */
  pstResult = orxFRAME(orxStructure_GetParent(_pstFrame));

  /* Is root? */
  if(pstResult == sstFrame.pstRoot)
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets frame first child
 * @param[in]   _pstFrame       Concerned frame
 * @return orxFRAME / orxNULL
 */
orxFRAME *orxFASTCALL orxFrame_GetChild(const orxFRAME *_pstFrame)
{
  orxFRAME *pstResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Gets child */
  pstResult = orxFRAME(orxStructure_GetChild(_pstFrame));

  /* Done! */
  return pstResult;
}

/** Gets frame next sibling
 * @param[in]   _pstFrame       Concerned frame
 * @return orxFRAME / orxNULL
 */
orxFRAME *orxFASTCALL orxFrame_GetSibling(const orxFRAME *_pstFrame)
{
  orxFRAME *pstResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Gets sibling */
  pstResult = orxFRAME(orxStructure_GetSibling(_pstFrame));

  /* Done! */
  return pstResult;
}

/** Is a root child?
 * @param[in]   _pstFrame       Concerned frame
 * @return orxTRUE if its parent is root, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxFrame_IsRootChild(const orxFRAME *_pstFrame)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Updates result*/
  bResult = orxFRAME(orxStructure_GetParent(_pstFrame)) == sstFrame.pstRoot;

  /* Done! */
  return bResult;
}

/** Sets frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _pvPos          Position to set
 */
void orxFASTCALL orxFrame_SetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, const orxVECTOR *_pvPos)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvPos != orxNULL);

  /* Local? */
  if(_eSpace == orxFRAME_SPACE_LOCAL)
  {
    /* Updates coord values */
    if(_orxFrame_SetPosition(_pstFrame, _pvPos, orxFRAME_SPACE_LOCAL) != orxFALSE)
    {
      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* Processes frame */
      orxFrame_Process(_pstFrame, orxFRAME(orxStructure_GetParent(_pstFrame)));

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
  }
  else
  {
    /* 2D data? */
    if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
    {
      orxFRAME *pstParent, *pstChild;
      orxVECTOR vPos;

      /* Gets parent frame */
      pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

      /* Stores global position */
      _orxFrame_SetPosition(_pstFrame, _pvPos, orxFRAME_SPACE_GLOBAL);

      /* Computes & stores local position */
      orxVector_Copy(&vPos, _pvPos);
      _orxFrame_SetPosition(_pstFrame, orxFrame_FromGlobalToLocalPosition(pstParent, &vPos), orxFRAME_SPACE_LOCAL);

      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* For all children */
      for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
          pstChild != orxNULL;
          pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
      {
        /* Processes it */
        orxFrame_Process(pstChild, _pstFrame);
      }

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
    }
  }

  return;
}

/** Sets frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _fRotation      Rotation angle to set (radians)
 */
void orxFASTCALL orxFrame_SetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxFLOAT _fRotation)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);

  /* Local or root child? */
  if((_eSpace == orxFRAME_SPACE_LOCAL) || (orxFrame_IsRootChild(_pstFrame) != orxFALSE))
  {
    /* Updates rotation value */
    if(_orxFrame_SetRotation(_pstFrame, _fRotation, orxFRAME_SPACE_LOCAL) != orxFALSE)
    {
      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* Processes frame */
      orxFrame_Process(_pstFrame, orxFRAME(orxStructure_GetParent(_pstFrame)));

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
  }
  else
  {
    /* 2D data? */
    if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
    {
      orxFRAME *pstParent, *pstChild;

      /* Gets parent frame */
      pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

      /* Stores global rotation */
      _orxFrame_SetRotation(_pstFrame, _fRotation, orxFRAME_SPACE_GLOBAL);

      /* Computes & stores local rotation */
      _orxFrame_SetRotation(_pstFrame, orxFrame_FromGlobalToLocalRotation(pstParent, _fRotation), orxFRAME_SPACE_LOCAL);

      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* For all children */
      for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
          pstChild != orxNULL;
          pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
      {
        /* Processes it */
        orxFrame_Process(pstChild, _pstFrame);
      }

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
    }
  }

  return;
}

/** Sets frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[in]   _pvScale        Scale to set
 */
void orxFASTCALL orxFrame_SetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, const orxVECTOR *_pvScale)
{
  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvScale != orxNULL);

  /* Local or root child? */
  if((_eSpace == orxFRAME_SPACE_LOCAL) || (orxFrame_IsRootChild(_pstFrame) != orxFALSE))
  {
    /* Updates scale value */
    if(_orxFrame_SetScale(_pstFrame, _pvScale, orxFRAME_SPACE_LOCAL) != orxFALSE)
    {
      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* Processes frame */
      orxFrame_Process(_pstFrame, orxFRAME(orxStructure_GetParent(_pstFrame)));

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
  }
  else
  {
    /* 2D data? */
    if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
    {
      orxFRAME *pstParent, *pstChild;
      orxVECTOR vScale;

      /* Gets parent frame */
      pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

      /* Stores global scale */
      _orxFrame_SetScale(_pstFrame, _pvScale, orxFRAME_SPACE_GLOBAL);

      /* Computes & stores local scale */
      orxVector_Copy(&vScale, _pvScale);
      _orxFrame_SetScale(_pstFrame, orxFrame_FromGlobalToLocalScale(pstParent, &vScale), orxFRAME_SPACE_LOCAL);

      /* Profiles */
      orxPROFILER_PUSH_MARKER("orxFrame_Process");

      /* For all children */
      for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
          pstChild != orxNULL;
          pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
      {
        /* Processes it */
        orxFrame_Process(pstChild, _pstFrame);
      }

      /* Profiles */
      orxPROFILER_POP_MARKER();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Frame does not support non-2d yet.");
    }
  }

  return;
}

/** Gets frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvPos          Position of the given frame
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFrame_GetPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvPos)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_pvPos != orxNULL);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);

  /* Is a 2D Frame? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    /* Updates result */
    orxVector_Copy(_pvPos, _orxFrame_GetPosition(_pstFrame, _eSpace));
    pvResult = _pvPos;
  }
  else
  {
    /* Resets coord structure */
    orxVector_SetAll(_pvPos, orxFLOAT_0);
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets frame rotation
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @return Rotation of the given frame (radians)
 */
orxFLOAT orxFASTCALL orxFrame_GetRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace)
{
  orxFLOAT fRotation;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);

  /* Is Frame 2D? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    /* Updates result */
    fRotation = _orxFrame_GetRotation(_pstFrame, _eSpace);
  }
  else
  {
    /* Updates result */
    fRotation = orxFLOAT_0;
  }

  /* Done! */
  return fRotation;
}

/** Gets frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Coordinate space system to use
 * @param[out]  _pvScale        Scale
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFrame_GetScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);
  orxASSERT(_pvScale != orxNULL);

  /* Is Frame 2D? */
  if(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D) != orxFALSE)
  {
    /* Updates result */
    pvResult = _orxFrame_GetScale(_pstFrame, _eSpace, _pvScale);
  }
  else
  {
    /* Clears scale */
    orxVector_Copy(_pvScale, &orxVECTOR_0);

    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Transforms a position given its input space (local -> global or global -> local)
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Input coordinate space system to use
 * @param[out]  _pvPos          Concerned position
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFrame_TransformPosition(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvPos)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);
  orxASSERT(_pvPos != orxNULL);

  /* Updates result */
  pvResult = (_eSpace == orxFRAME_SPACE_LOCAL) ? orxFrame_FromLocalToGlobalPosition(_pstFrame, _pvPos) : orxFrame_FromGlobalToLocalPosition(_pstFrame, _pvPos);

  /* Done! */
  return pvResult;
}

/** Transforms a rotation given its input space (local -> global or global -> local)
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Input coordinate space system to use
 * @param[out]  _fRotation      Concerned rotation
 * @return Transformed rotation (radians)
 */
orxFLOAT orxFASTCALL orxFrame_TransformRotation(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxFLOAT _fRotation)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);

  /* Updates result */
  fResult = (_eSpace == orxFRAME_SPACE_LOCAL) ? orxFrame_FromLocalToGlobalRotation(_pstFrame, _fRotation) : orxFrame_FromGlobalToLocalRotation(_pstFrame, _fRotation);

  /* Done! */
  return fResult;
}

/** Transforms a scale given its input space (local -> global or global -> local)
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _eSpace         Input coordinate space system to use
 * @param[out]  _pvScale        Concerned scale
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxFrame_TransformScale(orxFRAME *_pstFrame, orxFRAME_SPACE _eSpace, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstFrame);
  orxASSERT(_eSpace < orxFRAME_SPACE_NUMBER);
  orxASSERT(_pvScale != orxNULL);

  /* Updates result */
  pvResult = (_eSpace == orxFRAME_SPACE_LOCAL) ? orxFrame_FromLocalToGlobalScale(_pstFrame, _pvScale) : orxFrame_FromGlobalToLocalScale(_pstFrame, _pvScale);

  /* Done! */
  return pvResult;
}

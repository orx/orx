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
#define orxFRAME_KU32_STATIC_FLAG_NONE                        0x00000000  /**< No flags */

#define orxFRAME_KU32_STATIC_FLAG_READY                       0x00000001  /**< Ready flag */
#define orxFRAME_KU32_STATIC_FLAG_DATA_2D                     0x00000010  /**< 2D flag */

#define orxFRAME_KU32_STATIC_MASK_DEFAULT                     0x00000010  /**< Default flag */

#define orxFRAME_KU32_STATIC_MASK_ALL                         0xFFFFFFFF  /**< All mask */


/** orxFRAME flags
 */
#define orxFRAME_KU32_FLAG_NONE                               0x00000000  /**< No flags */

#define orxFRAME_KU32_FLAG_DATA_2D                            0x10000000  /**< 2D ID flag */

#define orxFRAME_KU32_MASK_ALL                                0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxFRAME_KU32_BANK_SIZE                               2048        /**< Bank size */
#define orxFRAME_KU32_RESULT_BUFFER_SIZE                      128

#define orxFRAME_KZ_NONE                                      "none"
#define orxFRAME_KZ_ROTATION                                  "rotation"
#define orxFRAME_KZ_SCALE                                     "scale"
#define orxFRAME_KZ_POSITION                                  "position"
#define orxFRAME_KZ_POSITION_ROTATION                         "position.rotation"
#define orxFRAME_KZ_POSITION_SCALE                            "position.scale"
#define orxFRAME_KZ_POSITION_POSITION                         "position.position"
#define orxFRAME_KZ_ALL                                       "all"

#define orxFRAME_KC_SEPARATOR                                 '.'
#define orxFRAME_KU32_SCALE_LENGTH                            5
#define orxFRAME_KU32_POSITION_LENGTH                         8


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Internal 2D Frame Data structure
 */
typedef struct __orxFRAME_DATA_2D_t
{
  orxVECTOR vGlobalPos;                                       /**< Global 2D coordinates : 12 */
  orxFLOAT  fGlobalRotation;                                  /**< Global 2D rotation angle : 16 */
  orxVECTOR vLocalPos;                                        /**< Local 2D coordinates : 28 */
  orxFLOAT  fLocalRotation;                                   /**< Local 2D rotation angle : 32 */
  orxFLOAT  fGlobalScaleX;                                    /**< Global 2D isometric X scale : 36 */
  orxFLOAT  fGlobalScaleY;                                    /**< Global 2D isometric Y scale : 40 */
  orxFLOAT  fLocalScaleX;                                     /**< Local 2D isometric X scale : 44 */
  orxFLOAT  fLocalScaleY;                                     /**< Local 2D isometric Y scale : 48 */

} orxFRAME_DATA_2D;


/** Frame structure
 */
struct __orxFRAME_t
{
  orxSTRUCTURE      stStructure;                              /**< Public structure, first structure member : 32 */
  orxFRAME_DATA_2D  stData;                                   /**< Frame data : 64 */
};

/** Static structure
 */
typedef struct __orxFRAME_STATIC_t
{
  orxU32    u32Flags;                                         /**< Control flags */
  orxFRAME *pstRoot;                                          /**< Frame root */
  orxBANK  *pst2DDataBank;                                    /**< 2D Data bank */
  orxCHAR   acResultBuffer[orxFRAME_KU32_RESULT_BUFFER_SIZE]; /**< Result buffer */

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

/** Gets corrected global frame scale
 * @param[in]   _pstFrame       Concerned frame
 * @param[out]  _pvScale        Scale
 * @param[int]  _u32Flags       Flags
 * @return      orxVECTOR / orxNULL
 */
static orxINLINE orxVECTOR *orxFrame_GetCorrectedGlobalScale(const orxFRAME *_pstFrame, orxVECTOR *_pvScale, orxU32 _u32Flags)
{
  /* Depending on flags */
  switch(_u32Flags)
  {
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_X:
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_X|orxFRAME_KU32_FLAG_IGNORE_SCALE_Z:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X|orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z:
    {
      _pvScale->fX = orxFLOAT_1;
      _pvScale->fY = _pstFrame->stData.fGlobalScaleY;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_Y:
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_Y|orxFRAME_KU32_FLAG_IGNORE_SCALE_Z:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y|orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z:
    {
      _pvScale->fX = _pstFrame->stData.fGlobalScaleX;
      _pvScale->fY = orxFLOAT_1;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_X|orxFRAME_KU32_FLAG_IGNORE_SCALE_Y:
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_X|orxFRAME_KU32_FLAG_IGNORE_SCALE_Y|orxFRAME_KU32_FLAG_IGNORE_SCALE_Z:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X|orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X|orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y|orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z:
    {
      _pvScale->fX = orxFLOAT_1;
      _pvScale->fY = orxFLOAT_1;
      break;
    }
    default:
    case orxFRAME_KU32_FLAG_IGNORE_SCALE_Z:
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z:
    {
      _pvScale->fX = _pstFrame->stData.fGlobalScaleX;
      _pvScale->fY = _pstFrame->stData.fGlobalScaleY;
      break;
    }
  }
  _pvScale->fZ = orxFLOAT_1;

  /* Done! */
  return _pvScale;
}

/** Gets corrected global frame position
 * @param[in]   _pstFrame       Concerned frame
 * @param[out]  _pvPos          Position
 * @param[int]  _u32Flags       Flags
 * @return      orxVECTOR / orxNULL
 */
static orxINLINE orxVECTOR *orxFrame_GetCorrectedGlobalPosition(const orxFRAME *_pstFrame, orxVECTOR *_pvPos, orxU32 _u32Flags)
{
  /* Depending on flags */
  switch(_u32Flags)
  {
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X:
    {
      _pvPos->fX = orxFLOAT_0;
      _pvPos->fY = _pstFrame->stData.vGlobalPos.fY;
      _pvPos->fZ = _pstFrame->stData.vGlobalPos.fZ;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y:
    {
      _pvPos->fX = _pstFrame->stData.vGlobalPos.fX;
      _pvPos->fY = orxFLOAT_0;
      _pvPos->fZ = _pstFrame->stData.vGlobalPos.fZ;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z:
    {
      _pvPos->fX = _pstFrame->stData.vGlobalPos.fX;
      _pvPos->fY = _pstFrame->stData.vGlobalPos.fY;
      _pvPos->fZ = orxFLOAT_0;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X|orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y:
    {
      _pvPos->fX = orxFLOAT_0;
      _pvPos->fY = orxFLOAT_0;
      _pvPos->fZ = _pstFrame->stData.vGlobalPos.fZ;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X|orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z:
    {
      _pvPos->fX = orxFLOAT_0;
      _pvPos->fY = _pstFrame->stData.vGlobalPos.fY;
      _pvPos->fZ = orxFLOAT_0;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y|orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z:
    {
      _pvPos->fX = _pstFrame->stData.vGlobalPos.fX;
      _pvPos->fY = orxFLOAT_0;
      _pvPos->fZ = orxFLOAT_0;
      break;
    }
    case orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X|orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y|orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z:
    {
      _pvPos->fX = orxFLOAT_0;
      _pvPos->fY = orxFLOAT_0;
      _pvPos->fZ = orxFLOAT_0;
      break;
    }
    default:
    {
      _pvPos->fX = _pstFrame->stData.vGlobalPos.fX;
      _pvPos->fY = _pstFrame->stData.vGlobalPos.fY;
      _pvPos->fZ = _pstFrame->stData.vGlobalPos.fZ;
      break;
    }
  }

  /* Done! */
  return _pvPos;
}

static orxINLINE orxVECTOR *orxFrame_FromLocalToGlobalPosition(const orxFRAME *_pstFrame, orxVECTOR *_pvPos, orxU32 _u32Flags)
{
  orxVECTOR   vGlobalScale, vGlobalPos;
  orxVECTOR  *pvResult;

  /* Transforms input position with scale */
  orxFrame_GetCorrectedGlobalScale(_pstFrame, &vGlobalScale, _u32Flags & orxFRAME_KU32_MASK_IGNORE_POSITION_SCALE);
  orxVector_Mul(_pvPos, _pvPos, &vGlobalScale);

  /* Has rotation? */
  if(!orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_ROTATION))
  {
    orxFLOAT fGlobalRotation;

    fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);
    if(fGlobalRotation != orxFLOAT_0)
    {
      /* Transforms input position with rotation */
      orxVector_2DRotate(_pvPos, _pvPos, fGlobalRotation);
    }
  }

  /* Transforms input position with translation (position) */
  orxFrame_GetCorrectedGlobalPosition(_pstFrame, &vGlobalPos, _u32Flags & orxFRAME_KU32_MASK_IGNORE_POSITION_POSITION);
  orxVector_Add(_pvPos, _pvPos, &vGlobalPos);

  /* Updates result */
  pvResult = _pvPos;

  /* Done! */
  return pvResult;
}

static orxINLINE orxVECTOR *orxFrame_FromGlobalToLocalPosition(const orxFRAME *_pstFrame, const orxVECTOR *_pvInPos, orxVECTOR *_pvOutPos, orxU32 _u32Flags)
{
  orxVECTOR   vGlobalScale, vGlobalPos;
  orxVECTOR  *pvResult;

  /* Transforms input position with translation (position) */
  orxFrame_GetCorrectedGlobalPosition(_pstFrame, &vGlobalPos, _u32Flags & orxFRAME_KU32_MASK_IGNORE_POSITION_POSITION);
  orxVector_Sub(_pvOutPos, _pvInPos, &vGlobalPos);

  /* Transforms input position with rotation */
  if(!orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_ROTATION))
  {
    orxFLOAT fGlobalRotation;

    fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);
    if(fGlobalRotation != orxFLOAT_0)
    {
      orxVector_2DRotate(_pvOutPos, _pvOutPos, -fGlobalRotation);
    }
  }

  /* Transforms input position with scale */
  orxFrame_GetCorrectedGlobalScale(_pstFrame, &vGlobalScale, _u32Flags & orxFRAME_KU32_MASK_IGNORE_POSITION_SCALE);
  if((vGlobalScale.fX != orxFLOAT_0) && (vGlobalScale.fY != orxFLOAT_0))
  {
    orxVector_Div(_pvOutPos, _pvOutPos, &vGlobalScale);
  }

  /* Updates result */
  pvResult = _pvOutPos;

  /* Done! */
  return pvResult;
}

static orxINLINE orxFLOAT orxFrame_FromLocalToGlobalRotation(const orxFRAME *_pstFrame, orxFLOAT _fRotation)
{
  orxFLOAT fGlobalRotation;
  orxFLOAT fResult;

  /* Gets global angle */
  fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);

  /* Transforms input rotation */
  fResult = _fRotation + fGlobalRotation;

  /* Done! */
  return fResult;
}

static orxINLINE orxFLOAT orxFrame_FromGlobalToLocalRotation(const orxFRAME *_pstFrame, orxFLOAT _fRotation)
{
  orxFLOAT fGlobalRotation;
  orxFLOAT fResult;

  /* Gets global angle */
  fGlobalRotation = _orxFrame_GetRotation(_pstFrame, orxFRAME_SPACE_GLOBAL);

  /* Transforms input rotation */
  fResult = _fRotation - fGlobalRotation;

  /* Done! */
  return fResult;
}

static orxINLINE orxVECTOR *orxFrame_FromLocalToGlobalScale(const orxFRAME *_pstFrame, orxVECTOR *_pvScale, orxU32 _u32Flags)
{
  orxVECTOR   vGlobalScale;
  orxVECTOR  *pvResult;

  /* Gets corrected global scale */
  orxFrame_GetCorrectedGlobalScale(_pstFrame, &vGlobalScale, _u32Flags);

  /* Transforms input scale */
  orxVector_Mul(_pvScale, _pvScale, &vGlobalScale);

  /* Updates result */
  pvResult = _pvScale;

  /* Done! */
  return pvResult;
}

static orxINLINE orxVECTOR *orxFrame_FromGlobalToLocalScale(const orxFRAME *_pstFrame, const orxVECTOR *_pvInScale, orxVECTOR *_pvOutScale, orxU32 _u32Flags)
{
  orxVECTOR   vGlobalScale;
  orxVECTOR  *pvResult;

  /* Gets corrected global scale */
  orxFrame_GetCorrectedGlobalScale(_pstFrame, &vGlobalScale, _u32Flags);

  /* Transforms input scale */
  if((vGlobalScale.fX != orxFLOAT_0) && (vGlobalScale.fY != orxFLOAT_0))
  {
    orxVector_Div(_pvOutScale, _pvInScale, &vGlobalScale);
  }
  else
  {
    orxVector_Copy(_pvOutScale, _pvInScale);
  }

  /* Updates result */
  pvResult = _pvOutScale;

  /* Done! */
  return pvResult;
}

/** Updates frame global data using parent's global and frame local ones
 * @param[out]  _pstFrame       Frame to update
 * @param[in]   _pstParent Parent frame to use for update
 */
static orxINLINE void orxFrame_UpdateData(orxFRAME *_pstFrame, const orxFRAME *_pstParent)
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
    orxFrame_FromLocalToGlobalScale(_pstParent, &vScale, orxStructure_GetFlags(_pstFrame, orxFRAME_KU32_MASK_IGNORE_SCALE));
    if(!orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_IGNORE_ROTATION))
    {
      fRotation = orxFrame_FromLocalToGlobalRotation(_pstParent, fRotation);
    }
    orxFrame_FromLocalToGlobalPosition(_pstParent, &vPos, orxStructure_GetFlags(_pstFrame, orxFRAME_KU32_MASK_IGNORE_POSITION));
  }

  /* Stores them */
  _orxFrame_SetRotation(_pstFrame, fRotation, orxFRAME_SPACE_GLOBAL);
  _orxFrame_SetScale(_pstFrame, &vScale, orxFRAME_SPACE_GLOBAL);
  _orxFrame_SetPosition(_pstFrame, &vPos, orxFRAME_SPACE_GLOBAL);

  /* Done! */
  return;
}

/** Internally processes a frame
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pstParent      Parent frame
 */
static void orxFASTCALL orxFrame_ProcessInternal(orxFRAME *_pstFrame, const orxFRAME *_pstParent)
{
  orxFRAME *pstChild;

  /* Updates frame's data */
  orxFrame_UpdateData(_pstFrame, _pstParent);

  /* For all children */
  for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
      pstChild != orxNULL;
      pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
  {
    /* Processes it */
    orxFrame_ProcessInternal(pstChild, _pstFrame);
  }

  /* Done! */
  return;
}

/** Processes a frame
 * @param[in]   _pstFrame       Concerned frame
 * @param[in]   _pstParent      Parent frame
 */
static orxINLINE void orxFrame_Process(orxFRAME *_pstFrame, const orxFRAME *_pstParent)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxFrame_Process");

  /* Processes frame */
  orxFrame_ProcessInternal(_pstFrame, _pstParent);

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Deletes all frames
 */
static orxINLINE void orxFrame_DeleteAll()
{
  orxFRAME *pstFrame;

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

  /* Done! */
  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Get ignore flag values
 * @param[in]   _zFlags         Literal ignore flags
 * @return Ignore flags
 */
orxU32 orxFASTCALL orxFrame_GetIgnoreFlagValues(const orxSTRING _zFlags)
{
  orxCHAR   acBuffer[128];
  orxSTRING zFlags;
  orxU32    u32Result = orxFRAME_KU32_FLAG_IGNORE_NONE;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFlags != orxNULL);

  /* Gets lower case version */
  zFlags = orxString_LowerCase(orxString_NCopy(acBuffer, _zFlags, sizeof(acBuffer) - 1));
  acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;

  /* Not none? */
  if(orxString_SearchString(zFlags, orxFRAME_KZ_NONE) == orxNULL)
  {
    /* All? */
    if(orxString_SearchString(zFlags, orxFRAME_KZ_ALL) != orxNULL)
    {
      /* Updates result */
      u32Result = orxFRAME_KU32_MASK_IGNORE_ALL;
    }
    else
    {
      const orxSTRING zFoundFlag;
      const orxSTRING zStart;

      /* Rotation? */
      if(((zFoundFlag = orxString_SearchString(zFlags, orxFRAME_KZ_ROTATION)) != orxNULL)
      && ((zFoundFlag == acBuffer)
       || (*(zFoundFlag - 1) != orxFRAME_KC_SEPARATOR)
       || (orxString_SearchString(zFoundFlag + 1, orxFRAME_KZ_ROTATION) != orxNULL)))
      {
        /* Updates result */
        u32Result |= orxFRAME_KU32_FLAG_IGNORE_ROTATION;
      }
      /* Scale? */
      if(((zFoundFlag = orxString_SearchString(zFlags, orxFRAME_KZ_SCALE)) != orxNULL)
      && ((zFoundFlag == acBuffer)
       || (*(zFoundFlag - 1) != orxFRAME_KC_SEPARATOR)
       || ((zFoundFlag = orxString_SearchString(zFoundFlag + 1, orxFRAME_KZ_SCALE)) != orxNULL)))
      {
        /* Checks for swizzle */
        zFoundFlag += orxFRAME_KU32_SCALE_LENGTH;
        if(*zFoundFlag == orxFRAME_KC_SEPARATOR)
        {
          orxCHAR c;
          for(c = *(++zFoundFlag); (c >= 'x') && (c <= 'z'); c = *(++zFoundFlag))
          {
            switch(c)
            {
              case 'x': u32Result |= orxFRAME_KU32_FLAG_IGNORE_SCALE_X; break;
              case 'y': u32Result |= orxFRAME_KU32_FLAG_IGNORE_SCALE_Y; break;
              case 'z': u32Result |= orxFRAME_KU32_FLAG_IGNORE_SCALE_Z; break;
              default:  break;
            }
          }
        }
        else
        {
          /* Updates result */
          u32Result |= orxFRAME_KU32_MASK_IGNORE_SCALE;
        }
      }
      /* Position? */
      for(zStart = zFlags, zFoundFlag = orxString_SearchString(zStart, orxFRAME_KZ_POSITION);
          zFoundFlag != orxNULL;
          zFoundFlag = orxString_SearchString(zStart, orxFRAME_KZ_POSITION))
      {
        if(*(zFoundFlag + orxFRAME_KU32_POSITION_LENGTH) != orxFRAME_KC_SEPARATOR)
        {
          /* Updates result */
          u32Result |= orxFRAME_KU32_MASK_IGNORE_POSITION;
          break;
        }
        else
        {
          zStart = orxMIN(zFoundFlag + orxFRAME_KU32_POSITION_LENGTH + 2, acBuffer + sizeof(acBuffer) - 1);
        }
      }
      /* Not ignoring entire position? */
      if(!orxFLAG_TEST(u32Result, orxFRAME_KU32_MASK_IGNORE_POSITION))
      {
        /* Position.Rotation? */
        if(orxString_SearchString(zFlags, orxFRAME_KZ_POSITION_ROTATION) != orxNULL)
        {
          /* Updates result */
          u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_ROTATION;
        }
        /* Position.Scale? */
        if((zFoundFlag = orxString_SearchString(zFlags, orxFRAME_KZ_POSITION_SCALE)) != orxNULL)
        {
          /* Checks for swizzle */
          zFoundFlag += orxFRAME_KU32_POSITION_LENGTH + orxFRAME_KU32_SCALE_LENGTH + 1;
          if(*zFoundFlag == orxFRAME_KC_SEPARATOR)
          {
            orxCHAR c;
            for(c = *(++zFoundFlag); (c >= 'x') && (c <= 'z'); c = *(++zFoundFlag))
            {
              switch(c)
              {
                case 'x': u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X; break;
                case 'y': u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y; break;
                case 'z': u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z; break;
                default:  break;
              }
            }
          }
          else
          {
            /* Updates result */
            u32Result |= orxFRAME_KU32_MASK_IGNORE_POSITION_SCALE;
          }
        }
        /* Position.Position? */
        if((zFoundFlag = orxString_SearchString(zFlags, orxFRAME_KZ_POSITION_POSITION)) != orxNULL)
        {
          /* Checks for swizzle */
          zFoundFlag += orxFRAME_KU32_POSITION_LENGTH + orxFRAME_KU32_POSITION_LENGTH + 1;
          if(*zFoundFlag == orxFRAME_KC_SEPARATOR)
          {
            orxCHAR c;
            for(c = *(++zFoundFlag); (c >= 'x') && (c <= 'z'); c = *(++zFoundFlag))
            {
              switch(c)
              {
                case 'x': u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X; break;
                case 'y': u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y; break;
                case 'z': u32Result |= orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z; break;
                default:  break;
              }
            }
          }
          else
          {
            /* Updates result */
            u32Result |= orxFRAME_KU32_MASK_IGNORE_POSITION_POSITION;
          }
        }
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Get ignore flag names (beware: result won't persist from one call to the other)
 * @param[in]   _u32Flags       Literal ignore flags
 * @return Ignore flags names
 */
const orxSTRING orxFASTCALL orxFrame_GetIgnoreFlagNames(orxU32 _u32Flags)
{
  orxCHAR        *pc = sstFrame.acResultBuffer;
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_READY);

  /* Clears result buffer */
  orxMemory_Zero(&(sstFrame.acResultBuffer), sizeof(sstFrame.acResultBuffer));

#define orxFRAME_PRINT(STRING) pc += orxString_NPrint(pc, (orxU32)(sizeof(sstFrame.acResultBuffer) - 1 - (pc - sstFrame.acResultBuffer)), STRING)

  /* Depending on flags */
  switch(orxFLAG_GET(_u32Flags, orxFRAME_KU32_MASK_IGNORE_ALL))
  {
    case orxFRAME_KU32_FLAG_IGNORE_NONE:
    {
      /* Updates result */
      zResult = orxFRAME_KZ_NONE;
      break;
    }

    case orxFRAME_KU32_MASK_IGNORE_ALL:
    {
      /* Updates result */
      zResult = orxFRAME_KZ_ALL;
      break;
    }

    default:
    {
      /* Full position? */
      if(orxFLAG_TEST_ALL(_u32Flags, orxFRAME_KU32_MASK_IGNORE_POSITION))
      {
        orxFRAME_PRINT(" " orxFRAME_KZ_POSITION);
      }
      /* Partial position? */
      else if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_MASK_IGNORE_POSITION))
      {
        /* Position.position? */
        if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_MASK_IGNORE_POSITION_POSITION))
        {
          orxFRAME_PRINT(" " orxFRAME_KZ_POSITION "." orxFRAME_KZ_POSITION);

          /* Partial position.position? */
          if(!orxFLAG_TEST_ALL(_u32Flags, orxFRAME_KU32_MASK_IGNORE_POSITION_POSITION))
          {
            orxFRAME_PRINT(".");
            if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_X))
            {
              orxFRAME_PRINT("x");
            }
            if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Y))
            {
              orxFRAME_PRINT("y");
            }
            if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_POSITION_Z))
            {
              orxFRAME_PRINT("z");
            }
          }
        }

        /* Position.rotation? */
        if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_ROTATION))
        {
          orxFRAME_PRINT(" " orxFRAME_KZ_POSITION "." orxFRAME_KZ_ROTATION);
        }

        /* Position.scale? */
        if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_MASK_IGNORE_POSITION_SCALE))
        {
          orxFRAME_PRINT(" " orxFRAME_KZ_POSITION "." orxFRAME_KZ_SCALE);

          /* Partial position.scale? */
          if(!orxFLAG_TEST_ALL(_u32Flags, orxFRAME_KU32_MASK_IGNORE_POSITION_SCALE))
          {
            orxFRAME_PRINT(".");
            if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_X))
            {
              orxFRAME_PRINT("x");
            }
            if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Y))
            {
              orxFRAME_PRINT("y");
            }
            if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_POSITION_SCALE_Z))
            {
              orxFRAME_PRINT("z");
            }
          }
        }
      }

      /* Rotation? */
      if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_ROTATION))
      {
        orxFRAME_PRINT(" " orxFRAME_KZ_ROTATION);
      }

      /* Scale? */
      if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_MASK_IGNORE_SCALE))
      {
        orxFRAME_PRINT(" " orxFRAME_KZ_SCALE);

        /* Partial scale? */
        if(!orxFLAG_TEST_ALL(_u32Flags, orxFRAME_KU32_MASK_IGNORE_SCALE))
        {
          orxFRAME_PRINT(".");
          if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_SCALE_X))
          {
            orxFRAME_PRINT("x");
          }
          if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_SCALE_Y))
          {
            orxFRAME_PRINT("y");
          }
          if(orxFLAG_TEST(_u32Flags, orxFRAME_KU32_FLAG_IGNORE_SCALE_Z))
          {
            orxFRAME_PRINT("z");
          }
        }
      }

      /* Updates result */
      zResult = sstFrame.acResultBuffer + 1;

      break;
    }
  }

#undef orxFRAME_PRINT

  /* Done! */
  return zResult;
}

/** Animation module setup
 */
void orxFASTCALL orxFrame_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_FRAME, orxMODULE_ID_PROFILER);

  /* Done! */
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

      /* Success? */
      if(sstFrame.pstRoot != orxNULL)
      {
        /* Sets it as its own owner */
        orxStructure_SetOwner(sstFrame.pstRoot, sstFrame.pstRoot);

        /* Continue */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Unregister structure type */
        orxStructure_Unregister(orxSTRUCTURE_ID_FRAME);

        /* Cleans flags */
        sstFrame.u32Flags = orxFRAME_KU32_STATIC_FLAG_NONE;

        /* Can't process */
        eResult = orxSTATUS_FAILURE;
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

  /* Done! */
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
    orxStructure_SetFlags(pstFrame, (_u32Flags & orxFRAME_KU32_MASK_USER_ALL) | orxFRAME_KU32_FLAG_DATA_2D, orxFRAME_KU32_MASK_ALL);

    /* Checks */
    orxASSERT(sstFrame.u32Flags & orxFRAME_KU32_STATIC_FLAG_DATA_2D);

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

    /* Increases count */
    orxStructure_IncreaseCount(pstFrame);
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

  /* Decreases count */
  orxStructure_DecreaseCount(_pstFrame);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstFrame) == 0)
  {
    orxFRAME *pstParent;

    /* Gets parent frame */
    pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

    /* Valid? */
    if(pstParent != orxNULL)
    {
      orxFRAME *pstChild;

      /* Resets global values to parent's */
      _pstFrame->stData.vGlobalPos.fX   = pstParent->stData.vGlobalPos.fX;
      _pstFrame->stData.vGlobalPos.fY   = pstParent->stData.vGlobalPos.fY;
      _pstFrame->stData.vGlobalPos.fZ   = pstParent->stData.vGlobalPos.fZ;
      _pstFrame->stData.fGlobalRotation = pstParent->stData.fGlobalRotation;
      _pstFrame->stData.fGlobalScaleX   = pstParent->stData.fGlobalScaleX;
      _pstFrame->stData.fGlobalScaleY   = pstParent->stData.fGlobalScaleY;

      /* For all children */
      for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
          pstChild != orxNULL;
          pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
      {
        /* Processes it */
        orxFrame_Process(pstChild, _pstFrame);
      }
    }

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
      /* Set root as parent */
      orxStructure_SetParent(_pstFrame, sstFrame.pstRoot);

      /* Processes frame */
      orxFrame_Process(_pstFrame, sstFrame.pstRoot);
    }
  }
  else
  {
    /* New parent? */
    if(orxStructure_GetParent(_pstFrame) != orxSTRUCTURE(_pstParent))
    {
      /* Sets parent */
      orxStructure_SetParent(_pstFrame, _pstParent);

      /* Processes frame */
      orxFrame_Process(_pstFrame, _pstParent);
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
      /* Processes frame */
      orxFrame_Process(_pstFrame, orxFRAME(orxStructure_GetParent(_pstFrame)));
    }
  }
  else
  {
    orxVECTOR vPos;
    orxFRAME *pstParent, *pstChild;

    /* Checks */
    orxASSERT(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D));

    /* Gets parent frame */
    pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

    /* Stores global position */
    _orxFrame_SetPosition(_pstFrame, _pvPos, orxFRAME_SPACE_GLOBAL);

    /* Computes & stores local position */
    _orxFrame_SetPosition(_pstFrame, orxFrame_FromGlobalToLocalPosition(pstParent, _pvPos, &vPos, orxStructure_GetFlags(_pstFrame, orxFRAME_KU32_MASK_IGNORE_POSITION)), orxFRAME_SPACE_LOCAL);

    /* For all children */
    for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
        pstChild != orxNULL;
        pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
    {
      /* Processes it */
      orxFrame_Process(pstChild, _pstFrame);
    }
  }

  /* Done! */
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
      /* Processes frame */
      orxFrame_Process(_pstFrame, orxFRAME(orxStructure_GetParent(_pstFrame)));
    }
  }
  else
  {
    orxFRAME *pstParent, *pstChild;

    /* Checks */
    orxASSERT(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D));

    /* Gets parent frame */
    pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

    /* Stores global rotation */
    _orxFrame_SetRotation(_pstFrame, _fRotation, orxFRAME_SPACE_GLOBAL);

    /* Computes & stores local rotation */
    _orxFrame_SetRotation(_pstFrame, orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_IGNORE_ROTATION) ? _fRotation : orxFrame_FromGlobalToLocalRotation(pstParent, _fRotation), orxFRAME_SPACE_LOCAL);

    /* For all children */
    for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
        pstChild != orxNULL;
        pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
    {
      /* Processes it */
      orxFrame_Process(pstChild, _pstFrame);
    }
  }

  /* Done! */
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
      /* Processes frame */
      orxFrame_Process(_pstFrame, orxFRAME(orxStructure_GetParent(_pstFrame)));
    }
  }
  else
  {
    orxVECTOR vScale;
    orxFRAME *pstParent, *pstChild;

    /* Checks */
    orxASSERT(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D));

    /* Gets parent frame */
    pstParent = orxFRAME(orxStructure_GetParent(_pstFrame));

    /* Stores global scale */
    _orxFrame_SetScale(_pstFrame, _pvScale, orxFRAME_SPACE_GLOBAL);

    /* Computes & stores local scale */
    _orxFrame_SetScale(_pstFrame, orxFrame_FromGlobalToLocalScale(pstParent, _pvScale, &vScale, orxStructure_GetFlags(_pstFrame, orxFRAME_KU32_MASK_IGNORE_SCALE)), orxFRAME_SPACE_LOCAL);

    /* For all children */
    for(pstChild = orxFRAME(orxStructure_GetChild(_pstFrame));
        pstChild != orxNULL;
        pstChild = orxFRAME(orxStructure_GetSibling(pstChild)))
    {
      /* Processes it */
      orxFrame_Process(pstChild, _pstFrame);
    }
  }

  /* Done! */
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
  orxASSERT(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D));

  /* Updates result */
  orxVector_Copy(_pvPos, _orxFrame_GetPosition(_pstFrame, _eSpace));
  pvResult = _pvPos;

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
  orxASSERT(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D));

  /* Updates result */
  fRotation = _orxFrame_GetRotation(_pstFrame, _eSpace);

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
  orxASSERT(orxStructure_TestFlags(_pstFrame, orxFRAME_KU32_FLAG_DATA_2D));

  /* Updates result */
  pvResult = _orxFrame_GetScale(_pstFrame, _eSpace, _pvScale);

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
  pvResult = (_eSpace == orxFRAME_SPACE_LOCAL) ? orxFrame_FromLocalToGlobalPosition(_pstFrame, _pvPos, orxFRAME_KU32_FLAG_IGNORE_NONE) : orxFrame_FromGlobalToLocalPosition(_pstFrame, _pvPos, _pvPos, orxFRAME_KU32_FLAG_IGNORE_NONE);

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
  pvResult = (_eSpace == orxFRAME_SPACE_LOCAL) ? orxFrame_FromLocalToGlobalScale(_pstFrame, _pvScale, orxFRAME_KU32_FLAG_IGNORE_NONE) : orxFrame_FromGlobalToLocalScale(_pstFrame, _pvScale, _pvScale, orxFRAME_KU32_FLAG_IGNORE_NONE);

  /* Done! */
  return pvResult;
}

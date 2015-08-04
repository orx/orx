/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxSpawner.c
 * @date 06/09/2008
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxSpawner.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "memory/orxMemory.h"
#include "object/orxObject.h"
#include "render/orxCamera.h"


/** Module flags
 */
#define orxSPAWNER_KU32_STATIC_FLAG_NONE          0x00000000

#define orxSPAWNER_KU32_STATIC_FLAG_READY         0x00000001

#define orxSPAWNER_KU32_STATIC_MASK_ALL           0xFFFFFFFF


/** Flags
 */
#define orxSPAWNER_KU32_FLAG_ENABLED              0x10000000  /**< Enabled flag */
#define orxSPAWNER_KU32_FLAG_TOTAL_LIMIT          0x20000000  /**< Total limit flag */
#define orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT         0x40000000  /**< Active limit flag */
#define orxSPAWNER_KU32_FLAG_WAVE_MODE            0x80000000  /**< Wave mode flag */
#define orxSPAWNER_KU32_FLAG_OBJECT_SPEED         0x01000000  /**< Speed flag */

#define orxSPAWNER_KU32_FLAG_RANDOM_OBJECT_SPEED  0x00100000  /**< Random object speed flag */
#define orxSPAWNER_KU32_FLAG_RANDOM_WAVE_SIZE     0x00200000  /**< Random wave size flag */
#define orxSPAWNER_KU32_FLAG_RANDOM_WAVE_DELAY    0x00400000  /**< Random wave delay flag */

#define orxSPAWNER_KU32_MASK_RANDOM_ALL           0x00700000  /**< Random all mask */

#define orxSPAWNER_KU32_MASK_ALL                  0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxSPAWNER_KZ_CONFIG_OBJECT               "Object"
#define orxSPAWNER_KZ_CONFIG_POSITION             "Position"
#define orxSPAWNER_KZ_CONFIG_ROTATION             "Rotation"
#define orxSPAWNER_KZ_CONFIG_SCALE                "Scale"
#define orxSPAWNER_KZ_CONFIG_TOTAL_OBJECT         "TotalObject"
#define orxSPAWNER_KZ_CONFIG_ACTIVE_OBJECT        "ActiveObject"
#define orxSPAWNER_KZ_CONFIG_WAVE_SIZE            "WaveSize"
#define orxSPAWNER_KZ_CONFIG_WAVE_DELAY           "WaveDelay"
#define orxSPAWNER_KZ_CONFIG_AUTO_RESET           "AutoReset"
#define orxSPAWNER_KZ_CONFIG_USE_ALPHA            "UseAlpha"
#define orxSPAWNER_KZ_CONFIG_USE_COLOR            "UseColor"
#define orxSPAWNER_KZ_CONFIG_USE_ROTATION         "UseRotation"
#define orxSPAWNER_KZ_CONFIG_USE_SCALE            "UseScale"
#define orxSPAWNER_KZ_CONFIG_OBJECT_SPEED         "ObjectSpeed"
#define orxSPAWNER_KZ_CONFIG_USE_RELATIVE_SPEED   "UseRelativeSpeed"
#define orxSPAWNER_KZ_CONFIG_USE_SELF_AS_PARENT   "UseSelfAsParent"
#define orxSPAWNER_KZ_CONFIG_CLEAN_ON_DELETE      "CleanOnDelete"
#define orxSPAWNER_KZ_CONFIG_INTERPOLATE          "Interpolate"

#define orxSPAWNER_KU32_BANK_SIZE                 128         /**< Bank size */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Spawner structure
 */
struct __orxSPAWNER_t
{
  orxSTRUCTURE        stStructure;                /**< Public structure, first structure member : 32 */
  orxVECTOR           vSpeed;                     /**< Speed : 44 */
  const orxSTRING     zReference;                 /**< Spawner reference : 48 */
  orxU32              u32TotalObjectLimit;        /**< Limit of objects that can be spawned, 0 for unlimited stock : 52 */
  orxU32              u32ActiveObjectLimit;       /**< Limit of active objects at the same time, 0 for unlimited : 56 */
  orxU32              u32TotalObjectCounter;      /**< Total spawned objects counter : 60 */
  orxU32              u32ActiveObjectCounter;     /**< Active objects counter : 64 */
  orxFRAME           *pstFrame;                   /**< Frame : 68 */
  orxFLOAT            fWaveTimer;                 /**< Wave timer : 72 */
  orxFLOAT            fWaveDelay;                 /**< Active objects counter : 76 */
  orxU32              u32WaveSize;                /**< Number of objects spawned in a wave : 80 */
  orxVECTOR           vLastPosition;              /**< Last position: 92 */
  orxVECTOR           vLastScale;                 /**< Last scale: 104 */
  orxFLOAT            fLastRotation;              /**< Last rotation: 108 */
};

/** Static structure
 */
typedef struct __orxSPAWNER_STATIC_t
{
  orxU32              u32DefaultGroupID;          /**< Default group ID */
  orxU32              u32Flags;                   /**< Control flags */

} orxSPAWNER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSPAWNER_STATIC sstSpawner;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static orxSTATUS orxFASTCALL orxSpawner_ProcessConfigData(orxSPAWNER *_pstSpawner, orxBOOL _bFirstCall)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Has reference? */
  if((_pstSpawner->zReference != orxNULL)
  && (*(_pstSpawner->zReference) != orxCHAR_NULL))
  {
    orxVECTOR vValue;
    orxU32    u32Value;

    /* Pushes its config section */
    orxConfig_PushSection(_pstSpawner->zReference);

    /* Not first call? */
    if(_bFirstCall == orxFALSE)
    {
      /* Has spawned objects? */
      if(_pstSpawner->u32TotalObjectCounter != 0)
      {
        orxOBJECT *pstObject;

        /* For all objects */
        for(pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
            pstObject != orxNULL;
            pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
        {
          /* Is spawner the owner */
          if(orxSPAWNER(orxObject_GetOwner(pstObject)) == _pstSpawner)
          {
            /* Removes it */
            orxObject_SetOwner(pstObject, orxNULL);
          }
        }

        /* Clears its counters */
        _pstSpawner->u32TotalObjectCounter  =
        _pstSpawner->u32ActiveObjectCounter = 0;
      }
    }

    /* Resets its flags */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ENABLED, orxSPAWNER_KU32_MASK_ALL);

    /* Gets total limit */
    u32Value = orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_TOTAL_OBJECT);

    /* Stores it */
    _pstSpawner->u32TotalObjectLimit = u32Value;

    /* Has limit? */
    if(u32Value != 0)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Gets active limit */
    u32Value = orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_ACTIVE_OBJECT);

    /* Stores it */
    _pstSpawner->u32ActiveObjectLimit = u32Value;

    /* Has limit? */
    if(u32Value != 0)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Sets wave size */
    orxSpawner_SetWaveSize(_pstSpawner, orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_WAVE_SIZE));

    /* Has list/random value? */
    if((orxConfig_GetListCounter(orxSPAWNER_KZ_CONFIG_WAVE_SIZE) > 1) || (orxConfig_IsRandomValue(orxSPAWNER_KZ_CONFIG_WAVE_SIZE) != orxFALSE))
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_RANDOM_WAVE_SIZE, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Sets wave delay */
    orxSpawner_SetWaveDelay(_pstSpawner, orxConfig_GetFloat(orxSPAWNER_KZ_CONFIG_WAVE_DELAY));

    /* Has list/random value? */
    if((orxConfig_GetListCounter(orxSPAWNER_KZ_CONFIG_WAVE_DELAY) > 1) || (orxConfig_IsRandomValue(orxSPAWNER_KZ_CONFIG_WAVE_DELAY) != orxFALSE))
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_RANDOM_WAVE_DELAY, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Has a position? */
    if(orxConfig_GetVector(orxSPAWNER_KZ_CONFIG_POSITION, &vValue) != orxNULL)
    {
      /* Updates object position */
      orxSpawner_SetPosition(_pstSpawner, &vValue);
    }

    /* Updates object rotation */
    orxSpawner_SetRotation(_pstSpawner, orxMATH_KF_DEG_TO_RAD * orxConfig_GetFloat(orxSPAWNER_KZ_CONFIG_ROTATION));

    /* Has scale? */
    if(orxConfig_HasValue(orxSPAWNER_KZ_CONFIG_SCALE) != orxFALSE)
    {
      /* Is config scale not a vector? */
      if(orxConfig_GetVector(orxSPAWNER_KZ_CONFIG_SCALE, &vValue) == orxNULL)
      {
        orxFLOAT fScale;

        /* Gets config uniformed scale */
        fScale = orxConfig_GetFloat(orxSPAWNER_KZ_CONFIG_SCALE);

        /* Updates vector */
        orxVector_SetAll(&vValue, fScale);
      }

      /* Updates object scale */
      orxSpawner_SetScale(_pstSpawner, &vValue);
    }

    /* Should use self as parent? */
    if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_SELF_AS_PARENT) != orxFALSE)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Should clean on delete? */
    if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_CLEAN_ON_DELETE) != orxFALSE)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_CLEAN_ON_DELETE, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Has speed? */
    if(orxConfig_GetVector(orxSPAWNER_KZ_CONFIG_OBJECT_SPEED, &(_pstSpawner->vSpeed)) != orxNULL)
    {
      /* Use relative speed? */
      if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_RELATIVE_SPEED) != orxFALSE)
      {
        /* Updates status */
        orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_OBJECT_SPEED | orxSPAWNER_KU32_FLAG_USE_RELATIVE_SPEED, orxSPAWNER_KU32_FLAG_NONE);
      }
      else
      {
        /* Updates status */
        orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_OBJECT_SPEED, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Has list/random value? */
      if((orxConfig_GetListCounter(orxSPAWNER_KZ_CONFIG_OBJECT_SPEED) > 1) || (orxConfig_IsRandomValue(orxSPAWNER_KZ_CONFIG_OBJECT_SPEED) != orxFALSE))
      {
        /* Updates status */
        orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_RANDOM_OBJECT_SPEED, orxSPAWNER_KU32_FLAG_NONE);
      }
    }

    /* Auto reset? */
    if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_AUTO_RESET) != orxFALSE)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_AUTO_RESET, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Use alpha? */
    if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_ALPHA) != orxFALSE)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_ALPHA, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Use color? */
    if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_COLOR) != orxFALSE)
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_COLOR, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Use rotation? */
    if(((orxConfig_HasValue(orxSPAWNER_KZ_CONFIG_USE_ROTATION) == orxFALSE)
     && (!orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT)))
    || (orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_ROTATION) != orxFALSE))
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_ROTATION, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Use scale? */
    if(((orxConfig_HasValue(orxSPAWNER_KZ_CONFIG_USE_SCALE) == orxFALSE)
     && (!orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT)))
    || (orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_SCALE) != orxFALSE))
    {
      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SCALE, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Should interpolate? */
    if((orxConfig_HasValue(orxSPAWNER_KZ_CONFIG_INTERPOLATE) == orxFALSE) || (orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_INTERPOLATE) != orxFALSE))
    {
      /* Stores last values */
      orxSpawner_GetWorldPosition(_pstSpawner, &(_pstSpawner->vLastPosition));
      orxSpawner_GetWorldScale(_pstSpawner, &(_pstSpawner->vLastScale));
      _pstSpawner->fLastRotation  = orxSpawner_GetWorldRotation(_pstSpawner);

      /* Updates status */
      orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_INTERPOLATE, orxSPAWNER_KU32_FLAG_NONE);
    }

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

/** Creates an empty spawner
 * @param[in]   _bInternal                    Internal call
 * @return      Created orxSPAWNER / orxNULL
 */
orxSPAWNER *orxFASTCALL orxSpawner_CreateInternal(orxBOOL _bInternal)
{
  orxSPAWNER *pstResult;

  /* Creates spawner */
  pstResult = orxSPAWNER(orxStructure_Create(orxSTRUCTURE_ID_SPAWNER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Creates frame */
    pstResult->pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

    /* Valid? */
    if(pstResult->pstFrame != orxNULL)
    {
      /* Increases its frame counter */
      orxStructure_IncreaseCounter(pstResult->pstFrame);

      /* Inits flags */
      orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_ENABLED, orxSPAWNER_KU32_MASK_ALL);

      /* Updates its owner */
      orxStructure_SetOwner(pstResult->pstFrame, pstResult);

      /* Increases counter */
      orxStructure_IncreaseCounter(pstResult);

      /* Not creating it internally? */
      if(_bInternal == orxFALSE)
      {
        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_CREATE, pstResult, orxNULL, orxNULL);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create frame for spawner.");

      /* Deletes spawner */
      orxStructure_Delete(pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create spawner structure.");
  }

  return pstResult;
}

/** Spawns objects
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32Number      Number of objects to spawn
 * @return      Number of spawned objects
 */
orxU32 orxFASTCALL orxSpawner_SpawnInternal(orxSPAWNER *_pstSpawner, orxU32 _u32Number, const orxVECTOR *_pvPosition, const orxVECTOR *_pvScale, orxFLOAT _fRotation, orxFLOAT _fDT)
{
  orxU32 u32Result = 0;

  /* Enabled? */
  if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ENABLED))
  {
    orxU32 u32SpawnNumber, i;

    /* Has a total limit? */
    if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT))
    {
      orxU32 u32AvailableNumber;

      /* Gets number of total available objects left */
      u32AvailableNumber = _pstSpawner->u32TotalObjectLimit - _pstSpawner->u32TotalObjectCounter;

      /* Gets total spawnable number */
      u32SpawnNumber = orxMIN(_u32Number, u32AvailableNumber);
    }
    else
    {
      /* Gets full requested number */
      u32SpawnNumber = _u32Number;
    }

    /* Has an active limit? */
    if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT))
    {
      orxU32 u32AvailableNumber;

      /* Gets number of available active objects left */
      u32AvailableNumber = _pstSpawner->u32ActiveObjectLimit - _pstSpawner->u32ActiveObjectCounter;

      /* Gets active spawnable number */
      u32SpawnNumber = orxMIN(u32SpawnNumber, u32AvailableNumber);
    }

    /* Should spawn? */
    if(u32SpawnNumber > 0)
    {
      orxCLOCK_INFO stClockInfo;

      /* Inits clock info for object simulation */
      orxMemory_Zero(&stClockInfo, sizeof(orxCLOCK_INFO));
      stClockInfo.fDT = _fDT;

      /* Pushes section */
      orxConfig_PushSection(_pstSpawner->zReference);

      /* For all objects to spawn */
      for(i = 0; i < u32SpawnNumber; i++)
      {
        orxOBJECT *pstObject;

        /* Creates object */
        pstObject = orxObject_CreateFromConfig(orxConfig_GetString(orxSPAWNER_KZ_CONFIG_OBJECT));

        /* Valid? */
        if(pstObject != orxNULL)
        {
          orxVECTOR vPosition, vScale;

          /* Updates active object counter */
          _pstSpawner->u32ActiveObjectCounter++;

          /* Updates total object counter */
          _pstSpawner->u32TotalObjectCounter++;

          /* Sets spawner as owner */
          orxObject_SetOwner(pstObject, _pstSpawner);

          /* Should use self as parent? */
          if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT))
          {
            /* Updates spawned object's parent */
            orxObject_SetParent(pstObject, _pstSpawner);
          }

          /* Should update rotation? */
          if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_ROTATION))
          {
            /* Updates object rotation */
            orxObject_SetRotation(pstObject, orxObject_GetRotation(pstObject) + _fRotation);
          }

          /* Should update scale? */
          if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SCALE))
          {
            /* Updates object scale */
            orxObject_SetScale(pstObject, orxVector_Mul(&vScale, orxObject_GetScale(pstObject, &vScale), _pvScale));
          }

          /* Not using self as parent? */
          if(!orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT))
          {
            /* Updates object position */
            orxObject_SetPosition(pstObject, orxVector_Add(&vPosition, orxVector_2DRotate(&vPosition, orxVector_Mul(&vPosition, orxObject_GetPosition(pstObject, &vPosition), _pvScale), _fRotation), _pvPosition));
          }

          /* Should apply speed? */
          if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_OBJECT_SPEED))
          {
            /* Use random speed? */
            if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_RANDOM_OBJECT_SPEED))
            {
              /* Updates its value */
              orxConfig_GetVector(orxSPAWNER_KZ_CONFIG_OBJECT_SPEED, &(_pstSpawner->vSpeed));
            }

            /* Use relative speed? */
            if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_RELATIVE_SPEED))
            {
              orxVECTOR vSpeed;

              /* Applies relative speed */
              orxVector_Mul(&vSpeed, orxVector_2DRotate(&vSpeed, &(_pstSpawner->vSpeed), _fRotation), _pvScale);
              orxObject_SetSpeed(pstObject, &vSpeed);
            }
            else
            {
              /* Applies speed */
              orxObject_SetSpeed(pstObject, &(_pstSpawner->vSpeed));
            }
          }

          /* Updates result */
          u32Result++;

          /* Should simulate object? */
          if(stClockInfo.fDT > orxFLOAT_0)
          {
            /* Simulates object update */
            orxObject_Update(pstObject, &stClockInfo);
          }

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_SPAWN, _pstSpawner, pstObject, orxNULL);
        }
      }

      /* Should update wave size? */
      if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_RANDOM_WAVE_SIZE))
      {
        /* Stores it */
        _pstSpawner->u32WaveSize = orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_WAVE_SIZE);
      }

      /* Should update wave delay? */
      if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_RANDOM_WAVE_DELAY))
      {
        /* Stores it */
        _pstSpawner->fWaveDelay = orxConfig_GetFloat(orxSPAWNER_KZ_CONFIG_WAVE_DELAY);
      }

      /* Pops previous section */
      orxConfig_PopSection();

      /* Has a total limit? */
      if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT))
      {
        /* No available object left? */
        if(_pstSpawner->u32TotalObjectLimit - _pstSpawner->u32TotalObjectCounter == 0)
        {
          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_EMPTY, _pstSpawner, orxNULL, orxNULL);

          /* Auto reset? */
          if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_AUTO_RESET))
          {
            /* Resets spawner */
            orxSpawner_Reset(_pstSpawner);
          }
          /* Disables */
          else
          {
            /* Updates status */
            orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_ENABLED);
          }
        }
      }
    }
  }

  /* Done! */
  return u32Result;
}

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxSpawner_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Depending on type */
  switch(_pstEvent->eType)
  {
    case orxEVENT_TYPE_OBJECT:
    {
      /* Depending on event ID */
      switch(_pstEvent->eID)
      {
        /* Delete event */
        case orxOBJECT_EVENT_DELETE:
        {
          orxOBJECT  *pstObject;
          orxSPAWNER *pstSpawner;

          /* Gets corresponding object */
          pstObject = orxOBJECT(_pstEvent->hSender);

          /* Gets owner */
          pstSpawner = orxSPAWNER(orxObject_GetOwner(pstObject));

          /* Is a spawner linked to it? */
          if(pstSpawner != orxNULL)
          {
            /* Checks */
            orxASSERT(pstSpawner->u32ActiveObjectCounter > 0);

            /* Decreases its active objects counter */
            pstSpawner->u32ActiveObjectCounter--;

            break;
          }

          break;
        }

        default:
        {
          break;
        }
      }

      break;
    }

    case orxEVENT_TYPE_RESOURCE:
    {
      /* Add or update? */
      if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
      {
        orxRESOURCE_EVENT_PAYLOAD *pstPayload;

        /* Gets payload */
        pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        /* Is config group? */
        if(pstPayload->u32GroupID == orxString_ToCRC(orxCONFIG_KZ_RESOURCE_GROUP))
        {
          orxSPAWNER *pstSpawner;

          /* For all spawners */
          for(pstSpawner = orxSPAWNER(orxStructure_GetFirst(orxSTRUCTURE_ID_SPAWNER));
              pstSpawner != orxNULL;
              pstSpawner = orxSPAWNER(orxStructure_GetNext(pstSpawner)))
          {
            /* Has reference? */
            if((pstSpawner->zReference != orxNULL) && (pstSpawner->zReference != orxSTRING_EMPTY))
            {
              /* Match origin? */
              if(orxConfig_GetOriginID(pstSpawner->zReference) == pstPayload->u32NameID)
              {
                /* Re-processes its config data */
                orxSpawner_ProcessConfigData(pstSpawner, orxFALSE);
              }
            }
          }
        }
      }

      break;
    }

    case orxEVENT_TYPE_SPAWNER:
    {
      /* Is a spawn event? */
      if(_pstEvent->eID == orxSPAWNER_EVENT_SPAWN)
      {
        orxSPAWNER *pstSpawner;
        orxOBJECT  *pstOwner;

        /* Gets spawner */
        pstSpawner = orxSPAWNER(_pstEvent->hSender);

        /* Gets owner */
        pstOwner = orxOBJECT(orxStructure_GetOwner(pstSpawner));

        /* Valid? */
        if(pstOwner != orxNULL)
        {
          orxOBJECT *pstObject;

          /* Gets spawned object */
          pstObject = orxOBJECT(_pstEvent->hRecipient);

          /* Doesn't have a group? */
          if(orxObject_GetGroupID(pstObject) == sstSpawner.u32DefaultGroupID)
          {
            /* Transfers group ID */
            orxObject_SetGroupID(pstObject, orxObject_GetGroupID(pstOwner));
          }

          /* Should apply color or alpha? */
          if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_USE_ALPHA | orxSPAWNER_KU32_FLAG_USE_COLOR))
          {
            orxCOLOR stColor, stTemp;

            /* Inits color */
            orxColor_Set(&stColor, &orxVECTOR_WHITE, orxFLOAT_1);

            /* Should apply color? */
            if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_USE_COLOR))
            {
              /* Has color? */
              if(orxObject_HasColor(pstOwner) != orxFALSE)
              {
                /* Gets it */
                orxObject_GetColor(pstOwner, &stColor);
              }
            }
            else
            {
              /* Has color? */
              if(orxObject_HasColor(pstObject) != orxFALSE)
              {
                /* Uses object's one */
                orxObject_GetColor(pstObject, &stColor);
              }
            }

            /* Should apply alpha? */
            if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_USE_ALPHA))
            {
              /* Has color? */
              if(orxObject_HasColor(pstOwner) != orxFALSE)
              {
                /* Stores it */
                stColor.fAlpha = orxObject_GetColor(pstOwner, &stTemp)->fAlpha;
              }
            }
            else
            {
              /* Has color? */
              if(orxObject_HasColor(pstObject) != orxFALSE)
              {
                /* Uses object's alpha */
                stColor.fAlpha = orxObject_GetColor(pstObject, &stTemp)->fAlpha;
              }
            }

            /* Applies new value */
            orxObject_SetColor(pstObject, &stColor);
          }
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the spawners
 */
static orxINLINE void orxSpawner_DeleteAll()
{
  orxSPAWNER *pstSpawner;

  /* Gets first spawner */
  pstSpawner = orxSPAWNER(orxStructure_GetFirst(orxSTRUCTURE_ID_SPAWNER));

  /* Non empty? */
  while(pstSpawner != orxNULL)
  {
    /* Deletes spawner */
    orxSpawner_Delete(pstSpawner);

    /* Gets first spawner */
    pstSpawner = orxSPAWNER(orxStructure_GetFirst(orxSTRUCTURE_ID_SPAWNER));
  }

  return;
}

/** Updates the spawner (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxSpawner_Update(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo)
{
  orxSPAWNER *pstSpawner;
  orxOBJECT  *pstObject;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSpawner_Update");

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);

  /* Gets spawner */
  pstSpawner = orxSPAWNER(_pstStructure);

  /* Gets calling object */
  pstObject = orxOBJECT(_pstCaller);

  /* Is enabled? */
  if(orxSpawner_IsEnabled(pstSpawner) != orxFALSE)
  {
    /* Is in wave mode? */
    if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_WAVE_MODE))
    {
      /* Updates timer */
      pstSpawner->fWaveTimer -= _pstClockInfo->fDT;

      /* Should spawn a new wave? */
      if(pstSpawner->fWaveTimer <= orxFLOAT_0)
      {
        /* Checks */
        orxASSERT(orxOBJECT(orxStructure_GetOwner(pstSpawner)) == pstObject);

        /* Sends wave start event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_WAVE_START, pstSpawner, orxNULL, orxNULL);

        /* Adds event handler */
        orxEvent_AddHandler(orxEVENT_TYPE_SPAWNER, orxSpawner_EventHandler);

        /* Is in interpolate mode with a valid wave delay? */
        if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_INTERPOLATE)
        && (pstSpawner->fWaveDelay > orxFLOAT_0))
        {
          orxVECTOR vSpawnerPosition, vSpawnerScale, vPosition, vScale;
          orxFLOAT  fInvDT, fSpawnerRotation, fRotation, fDT, fCoef, fDelta;

          /* Gets current spawner frame values */
          orxSpawner_GetWorldPosition(pstSpawner, &vSpawnerPosition);
          orxSpawner_GetWorldScale(pstSpawner, &vSpawnerScale);
          fSpawnerRotation = orxSpawner_GetWorldRotation(pstSpawner);

          /* For all the waves that need to be spawned */
          for(fInvDT = orxFLOAT_1 / _pstClockInfo->fDT, fCoef = orxFLOAT_1 + (pstSpawner->fWaveTimer * fInvDT), fDelta = pstSpawner->fWaveDelay * fInvDT, fDT = -pstSpawner->fWaveTimer;
              fCoef <= orxFLOAT_1;
              fCoef += fDelta, fDT -= pstSpawner->fWaveDelay)
          {
            /* Gets interpolated frame values */
            orxVector_Lerp(&vPosition, &(pstSpawner->vLastPosition), &vSpawnerPosition, fCoef);
            orxVector_Lerp(&vScale, &(pstSpawner->vLastScale), &vSpawnerScale, fCoef);
            fRotation = orxLERP(pstSpawner->fLastRotation, fSpawnerRotation, fCoef);

            /* Spawns objects */
            orxSpawner_SpawnInternal(pstSpawner, pstSpawner->u32WaveSize, &vPosition, &vScale, fRotation, fDT);
          }

          /* Updates timer */
          pstSpawner->fWaveTimer = (fCoef - orxFLOAT_1) * _pstClockInfo->fDT;

          /* Updates latest frame values with current ones */
          orxVector_Copy(&(pstSpawner->vLastPosition), &vSpawnerPosition);
          orxVector_Copy(&(pstSpawner->vLastScale), &vSpawnerScale);
          pstSpawner->fLastRotation = fSpawnerRotation;
        }
        else
        {
          /* Is in interpolate mode? */
          if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_INTERPOLATE))
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Spawner <%s>: Ignoring interpolate mode as its WaveDelay isn't strictly positive.", orxSpawner_GetName(pstSpawner));
          }

          /* Spawns the wave */
          orxSpawner_Spawn(pstSpawner, pstSpawner->u32WaveSize);

          /* Updates wave timer */
          pstSpawner->fWaveTimer = pstSpawner->fWaveDelay;
        }

        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_SPAWNER, orxSpawner_EventHandler);

        /* Sends wave stop event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_WAVE_STOP, pstSpawner, orxNULL, orxNULL);
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Spawner module setup
 */
void orxFASTCALL orxSpawner_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SPAWNER, orxMODULE_ID_FRAME);

  return;
}

/** Inits the spawner module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstSpawner, sizeof(orxSPAWNER_STATIC));

    /* Adds event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxSpawner_EventHandler);

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(SPAWNER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSPAWNER_KU32_BANK_SIZE, &orxSpawner_Update);

      /* Initialized? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Stores default group ID */
        sstSpawner.u32DefaultGroupID = orxString_GetID(orxOBJECT_KZ_DEFAULT_GROUP);

        /* Inits Flags */
        orxFLAG_SET(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY, orxSPAWNER_KU32_STATIC_MASK_ALL);

        /* Adds resource event handler */
        orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxSpawner_EventHandler);
      }
      else
      {
        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxSpawner_EventHandler);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to register link list structure.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to add spawner event handler.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize spawner module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the spawner module
 */
void orxFASTCALL orxSpawner_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY))
  {
    /* Deletes spawner list */
    orxSpawner_DeleteAll();

    /* Removes event handlers */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxSpawner_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_OBJECT, orxSpawner_EventHandler);

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SPAWNER);

    /* Updates flags */
    orxFLAG_SET(sstSpawner.u32Flags, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_STATIC_FLAG_READY);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit spawner module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty spawner
 * @return      Created orxSPAWNER / orxNULL
 */
orxSPAWNER *orxFASTCALL orxSpawner_Create()
{
  orxSPAWNER *pstResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);

  /* Creates spawner */
  pstResult = orxSpawner_CreateInternal(orxFALSE);

  return pstResult;
}

/** Creates a spawner from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxSPAWNER / orxNULL
 */
orxSPAWNER *orxFASTCALL orxSpawner_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxSPAWNER *pstResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates spawner */
    pstResult = orxSpawner_CreateInternal(orxTRUE);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Stores its reference */
      pstResult->zReference = orxString_Store(orxConfig_GetCurrentSection());

      /* Processes its config data */
      if(orxSpawner_ProcessConfigData(pstResult, orxTRUE) != orxSTATUS_FAILURE)
      {
        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_CREATE, pstResult, orxNULL, orxNULL);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't process config data for spawner <%s>.", _zConfigID);

        /* Deletes spawner */
        orxSpawner_Delete(pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't find config section named (%s).", _zConfigID);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Deletes a spawner
 * @param[in] _pstSpawner        Concerned spawner
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_Delete(orxSPAWNER *_pstSpawner)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstSpawner);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstSpawner) == 0)
  {
    orxOBJECT *pstObject;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_DELETE, _pstSpawner, orxNULL, orxNULL);

    /* Should clean? */
    if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_CLEAN_ON_DELETE))
    {
      /* For all objects */
      for(pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
          pstObject != orxNULL;
          pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
      {
        /* Is spawner the owner */
        if(orxSPAWNER(orxObject_GetOwner(pstObject)) == _pstSpawner)
        {
          /* Removes it */
          orxObject_SetOwner(pstObject, orxNULL);

          /* Updates its lifetime */
          orxObject_SetLifeTime(pstObject, orxFLOAT_0);
        }
      }
    }
    else
    {
      /* For all objects */
      for(pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
          pstObject != orxNULL;
          pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
      {
        /* Is spawner the owner */
        if(orxSPAWNER(orxObject_GetOwner(pstObject)) == _pstSpawner)
        {
          /* Removes it */
          orxObject_SetOwner(pstObject, orxNULL);
        }
      }
    }

    /* Removes frame's owner */
    orxStructure_SetOwner(_pstSpawner->pstFrame, orxNULL);

    /* Decreases frame's ref counter */
    orxStructure_DecreaseCounter(_pstSpawner->pstFrame);

    /* Deletes its frame */
    orxFrame_Delete(_pstSpawner->pstFrame);

    /* Deletes structure */
    orxStructure_Delete(_pstSpawner);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Enables/disables a spawner
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _bEnable      Enable / disable
 */
void orxFASTCALL    orxSpawner_Enable(orxSPAWNER *_pstSpawner, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Is in interpolate mode and wasn't enabled? */
    if(orxStructure_GetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_INTERPOLATE | orxSPAWNER_KU32_FLAG_ENABLED) == orxSPAWNER_KU32_FLAG_INTERPOLATE)
    {
      /* Stores last values */
      orxSpawner_GetWorldPosition(_pstSpawner, &(_pstSpawner->vLastPosition));
      orxSpawner_GetWorldScale(_pstSpawner, &(_pstSpawner->vLastScale));
      _pstSpawner->fLastRotation  = orxSpawner_GetWorldRotation(_pstSpawner);
    }

    /* Updates status flags */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ENABLED, orxSPAWNER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is spawner enabled?
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxSpawner_IsEnabled(const orxSPAWNER *_pstSpawner)
{
  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Done! */
  return(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ENABLED));
}

/** Resets (and re-enables) a spawner
 * @param[in]   _pstSpawner     Concerned spawner
 */
void orxFASTCALL orxSpawner_Reset(orxSPAWNER *_pstSpawner)
{
  orxOBJECT *pstObject;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Updates status */
  orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_ENABLED);

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_RESET, _pstSpawner, orxNULL, orxNULL);

  /* Resets counters */
  _pstSpawner->u32ActiveObjectCounter = 0;
  _pstSpawner->u32TotalObjectCounter  = 0;
  _pstSpawner->fWaveTimer             = orxFLOAT_0;

  /* For all objects */
  for(pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT));
      pstObject != orxNULL;
      pstObject = orxOBJECT(orxStructure_GetNext(pstObject)))
  {
    /* Is spawner the owner */
    if(orxSPAWNER(orxObject_GetOwner(pstObject)) == _pstSpawner)
    {
      /* Removes it */
      orxObject_SetOwner(pstObject, orxNULL);
    }
  }

  return;
}

/** Sets spawner total object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32TotalObjectLimit Total object limit, 0 for unlimited
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetTotalObjectLimit(orxSPAWNER *_pstSpawner, orxU32 _u32TotalObjectLimit)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Has limit? */
  if(_u32TotalObjectLimit > 0)
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT, orxSPAWNER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT);
  }

  /* Sets it */
  _pstSpawner->u32TotalObjectLimit = _u32TotalObjectLimit;

  /* Done! */
  return eResult;
}

/** Sets spawner active object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32ActiveObjectLimit Active object limit, 0 for unlimited
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetActiveObjectLimit(orxSPAWNER *_pstSpawner, orxU32 _u32ActiveObjectLimit)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Has limit? */
  if(_u32ActiveObjectLimit > 0)
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT, orxSPAWNER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT);
  }

  /* Sets it */
  _pstSpawner->u32ActiveObjectLimit = _u32ActiveObjectLimit;

  /* Done! */
  return eResult;
}

/** Gets spawner total object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Total object limit, 0 for unlimited
 */
orxU32 orxFASTCALL orxSpawner_GetTotalObjectLimit(const orxSPAWNER *_pstSpawner)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Has limit */
  if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT))
  {
    /* Updates result */
    u32Result = _pstSpawner->u32TotalObjectLimit;
  }

  /* Done! */
  return u32Result;
}

/** Gets spawner active object limit
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Active object limit, 0 for unlimited
 */
orxU32 orxFASTCALL orxSpawner_GetActiveObjectLimit(const orxSPAWNER *_pstSpawner)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Has limit */
  if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT))
  {
    /* Updates result */
    u32Result = _pstSpawner->u32ActiveObjectLimit;
  }

  /* Done! */
  return u32Result;
}

/** Gets spawner total object counter
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Total object counter
 */
orxU32 orxFASTCALL orxSpawner_GetTotalObjectCounter(const orxSPAWNER *_pstSpawner)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Updates result */
  u32Result = _pstSpawner->u32TotalObjectCounter;

  /* Done! */
  return u32Result;
}

/** Gets spawner active object counter
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Active object counter
 */
orxU32 orxFASTCALL orxSpawner_GetActiveObjectCounter(const orxSPAWNER *_pstSpawner)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Updates result */
  u32Result = _pstSpawner->u32ActiveObjectCounter;

  /* Done! */
  return u32Result;
}

/** Sets spawner wave size
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32WaveSize    Number of objects to spawn in a wave / 0 for deactivating wave mode
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetWaveSize(orxSPAWNER *_pstSpawner, orxU32 _u32WaveSize)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Stores wave size */
  _pstSpawner->u32WaveSize = _u32WaveSize;

  /* Active? */
  if((_pstSpawner->u32WaveSize > 0) && (_pstSpawner->fWaveDelay >= orxFLOAT_0))
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_WAVE_MODE, orxSPAWNER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_WAVE_MODE);
  }

  /* Clears random status */
  orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_RANDOM_WAVE_SIZE);

  /* Done! */
  return eResult;
}

/** Sets spawner wave delay
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _fWaveDelay     Delay between two waves / -1 for deactivating wave mode
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetWaveDelay(orxSPAWNER *_pstSpawner, orxFLOAT _fWaveDelay)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Stores wave delay & timer */
  _pstSpawner->fWaveTimer = _pstSpawner->fWaveDelay = (_fWaveDelay >= orxFLOAT_0) ? _fWaveDelay : orx2F(-1.0f);

  /* Active? */
  if((_pstSpawner->fWaveDelay >= orxFLOAT_0) && (_pstSpawner->u32WaveSize > 0))
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_WAVE_MODE, orxSPAWNER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_WAVE_MODE);
  }

  /* Clears random status */
  orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_RANDOM_WAVE_DELAY);

  /* Done! */
  return eResult;
}

/** Gets spawner wave size
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Number of objects spawned in a wave / 0 if not in wave mode
 */
orxU32 orxFASTCALL orxSpawner_GetWaveSize(const orxSPAWNER *_pstSpawner)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Updates result */
  u32Result = orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_WAVE_MODE) ? _pstSpawner->u32WaveSize : 0;

  /* Done! */
  return u32Result;
}

/** Gets spawner wave delay
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Delay between two waves / -1 if not in wave mode
 */
orxFLOAT orxFASTCALL orxSpawner_GetWaveDelay(const orxSPAWNER *_pstSpawner)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Updates result */
  fResult = orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_WAVE_MODE) ? _pstSpawner->fWaveDelay : orx2F(-1.0f);

  /* Done! */
  return fResult;
}

/** Gets spawner next wave delay
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Delay before next wave is spawned / -1 if not in wave mode
 */
orxFLOAT orxFASTCALL orxSpawner_GetNextWaveDelay(const orxSPAWNER *_pstSpawner)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Updates result */
  fResult = orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_WAVE_MODE) ? _pstSpawner->fWaveTimer : orx2F(-1.0f);

  /* Done! */
  return fResult;
}

/** Sets spawner object speed
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvObjectSpeed  Speed to apply to every spawned object / orxNULL to not apply any speed
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetObjectSpeed(orxSPAWNER *_pstSpawner, const orxVECTOR *_pvObjectSpeed)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Should apply speed? */
  if(_pvObjectSpeed != orxNULL)
  {
    /* Stores object speed */
    orxVector_Copy(&(_pstSpawner->vSpeed), _pvObjectSpeed);

    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_OBJECT_SPEED, orxSPAWNER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_OBJECT_SPEED);
  }

  /* Clears random status */
  orxStructure_SetFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_NONE, orxSPAWNER_KU32_FLAG_RANDOM_OBJECT_SPEED);

  /* Done! */
  return eResult;
}

/** Gets spawner object speed
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      Speed applied to every spawned object / orxNULL if none is applied
 */
orxVECTOR *orxFASTCALL orxSpawner_GetObjectSpeed(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvObjectSpeed)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvObjectSpeed != orxNULL);

  /* Does use object speed? */
  if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_OBJECT_SPEED))
  {
    /* Stores object speed */
    orxVector_Copy(_pvObjectSpeed, &(_pstSpawner->vSpeed));

    /* Updates result */
    pvResult = _pvObjectSpeed;
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Spawns objects
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _u32Number      Number of objects to spawn
 * @return      Number of spawned objects
 */
orxU32 orxFASTCALL orxSpawner_Spawn(orxSPAWNER *_pstSpawner, orxU32 _u32Number)
{
  orxVECTOR vPosition, vScale;
  orxFLOAT  fRotation;
  orxU32    u32Result = 0;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_u32Number > 0);

  /* Gets spawner's frame values */
  orxSpawner_GetWorldPosition(_pstSpawner, &vPosition);
  orxSpawner_GetWorldScale(_pstSpawner, &vScale);
  fRotation = orxSpawner_GetWorldRotation(_pstSpawner);

  /* Spawns objects */
  u32Result = orxSpawner_SpawnInternal(_pstSpawner, _u32Number, &vPosition, &vScale, fRotation, orxFLOAT_0);

  /* Done! */
  return u32Result;
}

/** Gets spawner frame
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFRAME
 */
orxFRAME *orxFASTCALL orxSpawner_GetFrame(const orxSPAWNER *_pstSpawner)
{
  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Gets spawner frame */
  return(_pstSpawner->pstFrame);
}

/** Sets spawner position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pvPosition     Spawner position
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetPosition(orxSPAWNER *_pstSpawner, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvPosition != orxNULL);

  /* Sets spawner position */
  orxFrame_SetPosition(_pstSpawner->pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition);

  /* Done! */
  return eResult;
}

/** Sets spawner rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _fRotation      Spawner rotation (radians)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetRotation(orxSPAWNER *_pstSpawner, orxFLOAT _fRotation)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Sets Spawner rotation */
  orxFrame_SetRotation(_pstSpawner->pstFrame, orxFRAME_SPACE_LOCAL, _fRotation);

  /* Done! */
  return eResult;
}

/** Sets Spawner scale
 * @param[in]   _pstSpawner     Concerned Spawner
 * @param[in]   _pvScale        Spawner scale vector
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetScale(orxSPAWNER *_pstSpawner, const orxVECTOR *_pvScale)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvScale != orxNULL);

  /* Sets frame scale */
  orxFrame_SetScale(_pstSpawner->pstFrame, orxFRAME_SPACE_LOCAL, _pvScale);

  /* Done! */
  return eResult;
}

/** Get spawner position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvPosition     Spawner position
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxSpawner_GetPosition(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets spawner position */
  pvResult = orxFrame_GetPosition(_pstSpawner->pstFrame, orxFRAME_SPACE_LOCAL, _pvPosition);

  /* Done! */
  return pvResult;
}

/** Get spawner world position
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvPosition     Spawner world position
 * @return      orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxSpawner_GetWorldPosition(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvPosition != orxNULL);

  /* Gets spawner position */
  pvResult = orxFrame_GetPosition(_pstSpawner->pstFrame, orxFRAME_SPACE_GLOBAL, _pvPosition);

  /* Done! */
  return pvResult;
}

/** Get spawner rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFLOAT (radians)
 */
orxFLOAT orxFASTCALL orxSpawner_GetRotation(const orxSPAWNER *_pstSpawner)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Gets spawner rotation */
  fResult = orxFrame_GetRotation(_pstSpawner->pstFrame, orxFRAME_SPACE_LOCAL);

  /* Done! */
  return fResult;
}

/** Get spawner world rotation
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxFLOAT (radians)
 */
orxFLOAT orxFASTCALL orxSpawner_GetWorldRotation(const orxSPAWNER *_pstSpawner)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Gets spawner rotation */
  fResult = orxFrame_GetRotation(_pstSpawner->pstFrame, orxFRAME_SPACE_GLOBAL);

  /* Done! */
  return fResult;
}

/** Gets spawner scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvScale        Spawner scale vector
 * @return      Scale vector
 */
orxVECTOR *orxFASTCALL orxSpawner_GetScale(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvScale != orxNULL);

  /* Gets spawner scale */
  pvResult = orxFrame_GetScale(_pstSpawner->pstFrame, orxFRAME_SPACE_LOCAL, _pvScale);

  /* Done! */
  return pvResult;
}

/** Gets spawner world scale
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[out]  _pvScale        Spawner world scale
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxVECTOR *orxFASTCALL orxSpawner_GetWorldScale(const orxSPAWNER *_pstSpawner, orxVECTOR *_pvScale)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_pvScale != orxNULL);

  /* Gets spawner scale */
  pvResult = orxFrame_GetScale(_pstSpawner->pstFrame, orxFRAME_SPACE_GLOBAL, _pvScale);

  /* Done! */
  return pvResult;
}

/** Sets spawner parent
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pParent        Parent structure to set (object, spawner, camera or frame) / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetParent(orxSPAWNER *_pstSpawner, void *_pParent)
{
  orxFRAME   *pstFrame;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT((_pParent == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pParent) < orxSTRUCTURE_ID_NUMBER));

  /* Gets frame */
  pstFrame = _pstSpawner->pstFrame;

  /* No parent? */
  if(_pParent == orxNULL)
  {
    /* Removes parent */
    orxFrame_SetParent(pstFrame, orxNULL);
  }
  else
  {
    /* Depending on parent ID */
    switch(orxStructure_GetID(_pParent))
    {
      case orxSTRUCTURE_ID_CAMERA:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxCamera_GetFrame(orxCAMERA(_pParent)));

        break;
      }

      case orxSTRUCTURE_ID_FRAME:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxFRAME(_pParent));

        break;
      }

      case orxSTRUCTURE_ID_OBJECT:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxOBJECT_GET_STRUCTURE(orxOBJECT(_pParent), FRAME));

        break;
      }

      case orxSTRUCTURE_ID_SPAWNER:
      {
        /* Updates its parent */
        orxFrame_SetParent(pstFrame, orxSPAWNER(_pParent)->pstFrame);

        break;
      }

      default:
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Invalid parent structure ID.");

        /* Updates result */
        eResult = orxSTATUS_FAILURE;

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets spawner parent
 * @param[in]   _pstSpawner Concerned spawner
 * @return      Parent (object, spawner, camera or frame) / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxSpawner_GetParent(const orxSPAWNER *_pstSpawner)
{
  orxFRAME     *pstFrame, *pstParentFrame;
  orxSTRUCTURE *pstResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Gets frame */
  pstFrame = _pstSpawner->pstFrame;

  /* Checks */
  orxSTRUCTURE_ASSERT(pstFrame);

  /* Gets frame's parent */
  pstParentFrame = orxFrame_GetParent(pstFrame);

  /* Valid? */
  if(pstParentFrame != orxNULL)
  {
    /* Gets its owner */
    pstResult = orxStructure_GetOwner(pstParentFrame);

    /* No owner? */
    if(pstResult == orxNULL)
    {
      /* Updates result with frame itself */
      pstResult = (orxSTRUCTURE *)pstParentFrame;
    }
  }
  else
  {
    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}

/** Gets spawner name
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxSpawner_GetName(const orxSPAWNER *_pstSpawner)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Has reference? */
  if(_pstSpawner->zReference != orxNULL)
  {
    /* Updates result */
    zResult = _pstSpawner->zReference;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

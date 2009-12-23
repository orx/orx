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
 * @file orxSpawner.c
 * @date 06/09/2008
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxSpawner.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
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


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Spawner structure
 */
struct __orxSPAWNER_t
{
  orxSTRUCTURE        stStructure;                /**< Public structure, first structure member : 16 */
  orxVECTOR           vSpeed;                     /**< Speed : 28 */
  orxSTRING           zReference;                 /**< Spawner reference : 32 */
  orxU16              u16TotalObjectLimit;        /**< Limit of objects that can be spawned, 0 for unlimited stock : 34 */
  orxU16              u16ActiveObjectLimit;       /**< Limit of active objects at the same time, 0 for unlimited : 36 */
  orxU16              u16TotalObjectCounter;      /**< Total spawned objects counter : 38 */
  orxU16              u16ActiveObjectCounter;     /**< Active objects counter : 40 */
  orxFRAME           *pstFrame;                   /**< Frame : 44 */
  orxSTRUCTURE       *pstOwner;                   /**< Owner: 48 */
  orxFLOAT            fWaveTimeStamp;             /**< Wave time stamp : 52 */
  orxFLOAT            fWaveDelay;                 /**< Active objects counter : 56 */
  orxU32              u32WaveSize;                /**< Number of objects spawned in a wave : 60 */
};

/** Static structure
 */
typedef struct __orxSPAWNER_STATIC_t
{
  orxU32 u32Flags;                                /**< Control flags */

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

/** Event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxSpawner_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((_pstEvent->eType == orxEVENT_TYPE_OBJECT) || (_pstEvent->eType == orxEVENT_TYPE_SPAWNER));

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
            orxASSERT(pstSpawner->u16ActiveObjectCounter > 0);

            /* Decreases its active objects counter */
            pstSpawner->u16ActiveObjectCounter--;

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

    case orxEVENT_TYPE_SPAWNER:
    {
      /* Is a spawn event? */
      if(_pstEvent->eID == orxSPAWNER_EVENT_SPAWN)
      {
        orxSPAWNER *pstSpawner;

        /* Gets spawner */
        pstSpawner = orxSPAWNER(_pstEvent->hSender);

        /* Should apply color or alpha? */
        if(orxStructure_TestFlags(pstSpawner, orxSPAWNER_KU32_FLAG_USE_ALPHA | orxSPAWNER_KU32_FLAG_USE_COLOR))
        {
          orxOBJECT *pstOwner;

          /* Gets owner */
          pstOwner = orxOBJECT(pstSpawner->pstOwner);

          /* Valid? */
          if(pstOwner != orxNULL)
          {
            orxCOLOR    stColor, stTemp;
            orxOBJECT  *pstObject;

            /* Inits color */
            orxColor_Set(&stColor, &orxVECTOR_WHITE, orxFLOAT_1);

            /* Gets spawned object */
            pstObject = orxOBJECT(_pstEvent->hRecipient);

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
          else
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Can't update color/alpha for object <%s> spawned by <%s> as spawner has no valid owner.", orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)), orxSpawner_GetName(pstSpawner));
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
      /* Should spawn a new wave? */
      if(_pstClockInfo->fTime >= pstSpawner->fWaveTimeStamp)
      {
        /* Checks */
        orxASSERT(orxOBJECT(pstSpawner->pstOwner) == pstObject);

        /* Sends wave start event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_WAVE_START, pstSpawner, orxNULL, orxNULL);

        /* Adds event handler */
        orxEvent_AddHandler(orxEVENT_TYPE_SPAWNER, orxSpawner_EventHandler);

        /* Spawn the wave */
        orxSpawner_Spawn(pstSpawner, pstSpawner->u32WaveSize);

        /* Removes event handler */
        orxEvent_RemoveHandler(orxEVENT_TYPE_SPAWNER, orxSpawner_EventHandler);

        /* Updates wave time stamp */
        pstSpawner->fWaveTimeStamp = _pstClockInfo->fTime + pstSpawner->fWaveDelay;

        /* Sends wave stop event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_WAVE_STOP, pstSpawner, orxNULL, orxNULL);
      }
    }
  }

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

    /* Registers event handler */
    eResult = orxEvent_AddHandler(orxEVENT_TYPE_OBJECT, orxSpawner_EventHandler);

    /* Valid? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(SPAWNER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSpawner_Update);

      /* Initialized? */
      if(eResult != orxSTATUS_FAILURE)
      {
        /* Inits Flags */
        orxFLAG_SET(sstSpawner.u32Flags, orxSPAWNER_KU32_STATIC_FLAG_READY, orxSPAWNER_KU32_STATIC_MASK_ALL);
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

    /* Removes event handler */
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
    pstResult = orxSpawner_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      orxVECTOR vValue;
      orxU32    u32Value;

      /* Stores its reference */
      pstResult->zReference = orxConfig_GetCurrentSection();

      /* Protects it */
      orxConfig_ProtectSection(pstResult->zReference, orxTRUE);

      /* Gets total limit */
      u32Value = orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_TOTAL_OBJECT);

      /* Checks */
      orxASSERT(u32Value <= 0xFFFF);

      /* Has limit? */
      if(u32Value > 0)
      {
        /* Sets it */
        pstResult->u16TotalObjectLimit = (orxU16)u32Value;

        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Gets active limit */
      u32Value = orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_ACTIVE_OBJECT);

      /* Checks */
      orxASSERT(u32Value <= 0xFFFF);

      /* Has limit? */
      if(u32Value > 0)
      {
        /* Sets it */
        pstResult->u16ActiveObjectLimit = (orxU16)u32Value;

        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_ACTIVE_LIMIT, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Sets wave size */
      orxSpawner_SetWaveSize(pstResult, orxConfig_GetU32(orxSPAWNER_KZ_CONFIG_WAVE_SIZE));

      /* Sets wave delay */
      orxSpawner_SetWaveDelay(pstResult, orxConfig_GetFloat(orxSPAWNER_KZ_CONFIG_WAVE_DELAY));

      /* Has a position? */
      if(orxConfig_GetVector(orxSPAWNER_KZ_CONFIG_POSITION, &vValue) != orxNULL)
      {
        /* Updates object position */
        orxSpawner_SetPosition(pstResult, &vValue);
      }

      /* Updates object rotation */
      orxSpawner_SetRotation(pstResult, orxMATH_KF_DEG_TO_RAD * orxConfig_GetFloat(orxSPAWNER_KZ_CONFIG_ROTATION));

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
        orxSpawner_SetScale(pstResult, &vValue);
      }

      /* Should use self as parent? */
      if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_SELF_AS_PARENT) != orxFALSE)
      {
        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Has speed? */
      if(orxConfig_GetVector(orxSPAWNER_KZ_CONFIG_OBJECT_SPEED, &(pstResult->vSpeed)) != orxNULL)
      {
        /* Use relative speed? */
        if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_RELATIVE_SPEED) != orxFALSE)
        {
          /* Updates status */
          orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_OBJECT_SPEED | orxSPAWNER_KU32_FLAG_USE_RELATIVE_SPEED, orxSPAWNER_KU32_FLAG_NONE);
        }
        else
        {
          /* Updates status */
          orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_OBJECT_SPEED, orxSPAWNER_KU32_FLAG_NONE);
        }
      }

      /* Auto reset? */
      if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_AUTO_RESET) != orxFALSE)
      {
        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_AUTO_RESET, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Use alpha? */
      if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_ALPHA) != orxFALSE)
      {
        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_USE_ALPHA, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Use color? */
      if(orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_COLOR) != orxFALSE)
      {
        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_USE_COLOR, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Use rotation? */
      if((orxConfig_HasValue(orxSPAWNER_KZ_CONFIG_USE_ROTATION) == orxFALSE) || (orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_ROTATION) != orxFALSE))
      {
        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_USE_ROTATION, orxSPAWNER_KU32_FLAG_NONE);
      }

      /* Use scale? */
      if((orxConfig_HasValue(orxSPAWNER_KZ_CONFIG_USE_SCALE) == orxFALSE) || (orxConfig_GetBool(orxSPAWNER_KZ_CONFIG_USE_SCALE) != orxFALSE))
      {
        /* Updates status */
        orxStructure_SetFlags(pstResult, orxSPAWNER_KU32_FLAG_USE_SCALE, orxSPAWNER_KU32_FLAG_NONE);
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

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstSpawner) == 0)
  {
    orxOBJECT *pstObject;

    /* Sends event */
    orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_DELETE, _pstSpawner, orxNULL, orxNULL);

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

    /* Decreases frame's ref counter */
    orxStructure_DecreaseCounter(_pstSpawner->pstFrame);

    /* Deletes its frame */
    orxFrame_Delete(_pstSpawner->pstFrame);

    /* Has reference? */
    if(_pstSpawner->zReference != orxNULL)
    {
      /* Unprotects it */
      orxConfig_ProtectSection(_pstSpawner->zReference, orxFALSE);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstSpawner);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to delete spawner when it was still referenced.");

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
  _pstSpawner->u16ActiveObjectCounter = 0;
  _pstSpawner->u16TotalObjectCounter  = 0;
  _pstSpawner->fWaveTimeStamp         = orxFLOAT_0;

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
  orxASSERT(_u32TotalObjectLimit <= 0xFFFF);

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
  _pstSpawner->u16TotalObjectLimit = (orxU16)_u32TotalObjectLimit;

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
  orxASSERT(_u32ActiveObjectLimit <= 0xFFFF);

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
  _pstSpawner->u16ActiveObjectLimit = (orxU16)_u32ActiveObjectLimit;

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
    u32Result = (orxU32)_pstSpawner->u16TotalObjectLimit;
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
    u32Result = (orxU32)_pstSpawner->u16ActiveObjectLimit;
  }

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
  orxASSERT(_u32WaveSize <= 0xFFFF);

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

  /* Stores wave delay */
  _pstSpawner->fWaveDelay = (_fWaveDelay >= orxFLOAT_0) ? _fWaveDelay : orx2F(-1.0f);

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
  u32Result = _pstSpawner->u32WaveSize;

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
  fResult = _pstSpawner->fWaveDelay;

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
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT(_u32Number > 0);

  /* Enabled? */
  if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_ENABLED))
  {
    orxU32 u32SpawnNumber, i;

    /* Has a total limit? */
    if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT))
    {
      orxU32 u32AvailableNumber;

      /* Gets number of total available objects left */
      u32AvailableNumber = (orxU32)_pstSpawner->u16TotalObjectLimit - (orxU32)_pstSpawner->u16TotalObjectCounter;

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
      u32AvailableNumber = (orxU32)_pstSpawner->u16ActiveObjectLimit - (orxU32)_pstSpawner->u16ActiveObjectCounter;

      /* Gets active spawnable number */
      u32SpawnNumber = orxMIN(u32SpawnNumber, u32AvailableNumber);
    }

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
        orxVECTOR vPosition, vSpawnerPosition, vScale, vSpawnerScale;
        orxFLOAT  fSpawnerRotation;

        /* Updates active object counter */
        _pstSpawner->u16ActiveObjectCounter++;

        /* Updates total object counter */
        _pstSpawner->u16TotalObjectCounter++;

        /* Sets spawner as owner */
        orxObject_SetOwner(pstObject, _pstSpawner);

        /* Should use self as parent? */
        if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT))
        {
          /* Updates spawned object's parent */
          orxObject_SetParent(pstObject, _pstSpawner);
        }

        /* Gets spawner rotation */
        fSpawnerRotation = orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_ROTATION) ? orxSpawner_GetWorldRotation(_pstSpawner) : orxSpawner_GetRotation(_pstSpawner) ;

        /* Updates object rotation */
        orxObject_SetRotation(pstObject, orxObject_GetRotation(pstObject) + fSpawnerRotation);

        /* Gets spawner scale */
        if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SCALE))
        {
          orxSpawner_GetWorldScale(_pstSpawner, &vSpawnerScale);
        }
        else
        {
          orxSpawner_GetScale(_pstSpawner, &vSpawnerScale);
        }

        /* Updates object scale */
        orxObject_SetScale(pstObject, orxVector_Mul(&vScale, orxObject_GetScale(pstObject, &vScale), &vSpawnerScale));

        /* Not using self as parent or has a body? */
        if(!orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_SELF_AS_PARENT) || (_orxObject_GetStructure(pstObject, orxSTRUCTURE_ID_BODY) != orxNULL))
        {
          /* Updates object position */
          orxObject_SetPosition(pstObject, orxVector_Add(&vPosition, orxVector_2DRotate(&vPosition, orxVector_Mul(&vPosition, orxObject_GetPosition(pstObject, &vPosition), &vSpawnerScale), fSpawnerRotation), orxSpawner_GetWorldPosition(_pstSpawner, &vSpawnerPosition)));
        }

        /* Should apply speed? */
        if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_OBJECT_SPEED))
        {
          /* Use relative speed? */
          if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_USE_RELATIVE_SPEED))
          {
            orxVECTOR vSpeed, vSpawnerScale;

            /* Applies relative speed */
            orxVector_Mul(&vSpeed, orxVector_2DRotate(&vSpeed, &(_pstSpawner->vSpeed), fSpawnerRotation), orxSpawner_GetWorldScale(_pstSpawner, &vSpawnerScale));
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

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SPAWNER, orxSPAWNER_EVENT_SPAWN, _pstSpawner, pstObject, orxNULL);
      }
    }

    /* Pops previous section */
    orxConfig_PopSection();

    /* Has a total limit? */
    if(orxStructure_TestFlags(_pstSpawner, orxSPAWNER_KU32_FLAG_TOTAL_LIMIT))
    {
      /* No available object left? */
      if((orxU32)_pstSpawner->u16TotalObjectLimit - (orxU32)_pstSpawner->u16TotalObjectCounter == 0)
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

/** Sets an spawner parent
 * @param[in]   _pstSpawner     Concerned spawner
 * @param[in]   _pParent        Parent structure to set (spawner, spawner, camera or frame) / orxNULL
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSpawner_SetParent(orxSPAWNER *_pstSpawner, void *_pParent)
{
  orxFRAME   *pstFrame;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT((_pParent == orxNULL) || (((orxSTRUCTURE *)(_pParent))->eID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE) < orxSTRUCTURE_ID_NUMBER);

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

/** Sets owner for a spawner
 * @param[in]   _pstSpawner   Concerned spawner
 * @param[in]   _pOwner       Owner to set / orxNULL
 */
void orxFASTCALL orxSpawner_SetOwner(orxSPAWNER *_pstSpawner, void *_pOwner)
{
  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);
  orxASSERT((_pOwner == orxNULL) || (((orxSTRUCTURE *)(_pOwner))->eID ^ orxSTRUCTURE_MAGIC_TAG_ACTIVE) < orxSTRUCTURE_ID_NUMBER);

  /* Sets new owner */
  _pstSpawner->pstOwner = orxSTRUCTURE(_pOwner);

  return;
}

/** Gets spawner's owner
 * @param[in]   _pstSpawner   Concerned object
 * @return      Owner / orxNULL
 */
orxSTRUCTURE *orxFASTCALL orxSpawner_GetOwner(const orxSPAWNER *_pstSpawner)
{
  orxSTRUCTURE *pResult;

  /* Checks */
  orxASSERT(sstSpawner.u32Flags & orxSPAWNER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSpawner);

  /* Gets owner */
  pResult = _pstSpawner->pstOwner;

  /* Done! */
  return pResult;
}

/** Gets spawner name
 * @param[in]   _pstSpawner     Concerned spawner
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxSpawner_GetName(const orxSPAWNER *_pstSpawner)
{
  orxSTRING zResult;

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

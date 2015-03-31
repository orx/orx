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
 * @file orxTimeLine.c
 * @date 22/04/2012
 * @author iarwain@orx-project.org
 *
 */


#include "object/orxTimeLine.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "core/orxConfig.h"
#include "core/orxClock.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "object/orxObject.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

  #pragma warning(disable : 4200)

  #include "malloc.h"

#endif /* __orxMSVC__ */


/** Module flags
 */
#define orxTIMELINE_KU32_STATIC_FLAG_NONE             0x00000000

#define orxTIMELINE_KU32_STATIC_FLAG_READY            0x00000001

#define orxTIMELINE_KU32_STATIC_MASK_ALL              0xFFFFFFFF


/** Flags
 */
#define orxTIMELINE_KU32_FLAG_NONE                    0x00000000  /**< No flags */

#define orxTIMELINE_KU32_FLAG_ENABLED                 0x10000000  /**< Enabled flag */
#define orxTIMELINE_KU32_FLAG_DIRTY                   0x20000000  /**< Dirty flag */

#define orxTIMELINE_KU32_MASK_ALL                     0xFFFFFFFF  /**< All mask */

/** Holder flags
 */
#define orxTIMELINE_HOLDER_KU32_FLAG_NONE             0x00000000  /**< No flag */

#define orxTIMELINE_HOLDER_KU32_FLAG_PLAYED           0x10000000  /**< Played flag */
#define orxTIMELINE_HOLDER_KU32_FLAG_LOOP             0x20000000  /**< Loop flag */
#define orxTIMELINE_HOLDER_KU32_FLAG_UPDATED          0x40000000  /**< Updated flag */

#define orxTIMELINE_HOLDER_KU32_MASK_ALL              0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxTIMELINE_KU32_TRACK_TABLE_SIZE             256
#define orxTIMELINE_KU32_TRACK_BANK_SIZE              128

#define orxTIMELINE_KU32_BANK_SIZE                    256         /**< Bank size */

#define orxTIMELINE_KU32_TRACK_NUMBER                 8

#define orxTIMELINE_KZ_CONFIG_LOOP                    "Loop"
#define orxTIMELINE_KZ_CONFIG_IMMEDIATE               "Immediate"
#define orxTIMELINE_KZ_CONFIG_KEEP_IN_CACHE           "KeepInCache"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** TimeLine track event
 */
typedef struct __orxTIMELINE_TRACK_EVENT_t
{
  const orxSTRING           zEventText;               /**< Event text : 4 */
  orxFLOAT                  fTimeStamp;               /**< Event timestamp : 8 */

} orxTIMELINE_TRACK_EVENT;

/** TimeLine track
 */
typedef struct __orxTIMELINE_TRACK_t
{
  const orxSTRING           zReference;               /**< Track reference : 4 */
  orxU32                    u32ID;                    /**< ID : 8 */
  orxU32                    u32RefCounter;            /**< Reference counter : 12 */
  orxU32                    u32EventCounter;          /**< Event counter : 16 */
  orxTIMELINE_TRACK_EVENT   astEventList[0];          /**< Track event list */

} orxTIMELINE_TRACK;

/** TimeLine track holder
 */
typedef struct __orxTIMELINE_TRACK_HOLDER_t
{
  orxTIMELINE_TRACK        *pstTrack;                 /**< Track : 4 / 8 */
  orxFLOAT                  fStartTime;               /**< Start time : 8 / 12 */
  orxU32                    u32NextEventIndex;        /**< Next event index : 12 / 16 */
  orxU32                    u32Flags;                 /**< Flags : 16 / 20 */

} orxTIMELINE_TRACK_HOLDER;

/** TimeLine structure
 */
struct __orxTIMELINE_t
{
  orxSTRUCTURE              stStructure;              /**< Public structure, first structure member : 32 */
  orxFLOAT                  fTime;                    /**< Time : 36 */
  orxTIMELINE_TRACK_HOLDER  astTrackList[orxTIMELINE_KU32_TRACK_NUMBER]; /**< TimeLine track list : 164 / 196 */
};

/** Static structure
 */
typedef struct __orxTIMELINE_STATIC_t
{
  orxHASHTABLE             *pstTrackTable;            /**< Track hash table */
  orxU32                    u32Flags;                 /**< Control flags */

} orxTIMELINE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxTIMELINE_STATIC sstTimeLine;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Adds a track
 */
static orxINLINE orxTIMELINE_TRACK *orxTimeLine_CreateTrack(const orxSTRING _zTrackID)
{
  orxTIMELINE_TRACK *pstResult = orxNULL;

  /* Pushes section */
  if((orxConfig_HasSection(_zTrackID) != orxFALSE)
  && (orxConfig_PushSection(_zTrackID) != orxSTATUS_FAILURE))
  {
    orxU32 u32KeyCounter;

    /* Gets number of keys */
    u32KeyCounter = orxConfig_GetKeyCounter();

    /* Valid? */
    if(u32KeyCounter > 0)
    {
      orxU32 u32EventCounter = 0, i;

#ifdef __orxMSVC__

      orxFLOAT *afTimeList = (orxFLOAT *)alloca(u32KeyCounter * sizeof(orxFLOAT));

#else /* __orxMSVC__ */

      orxFLOAT afTimeList[u32KeyCounter];

#endif /* __orxMSVC__ */

      /* For all time entries */
      for(i = 0; i < u32KeyCounter; i++)
      {
        /* Inits it */
        afTimeList[i] = orxFLOAT_MAX;
      }

      /* For all config keys */
      for(i = 0; i < u32KeyCounter; i++)
      {
        const orxSTRING zKey;
        orxFLOAT        fTime;

        /* Gets it */
        zKey = orxConfig_GetKey(i);

        /* Is a valid time stamp? */
        if((orxString_ToFloat(zKey, &fTime, orxNULL) != orxSTATUS_FAILURE)
        && (fTime >= orxFLOAT_0))
        {
          /* Stores it */
          afTimeList[i] = fTime;

          /* Updates event counter */
          u32EventCounter += orxConfig_GetListCounter(zKey);
        }
        else
        {
          /* Not keep in cache, immediate nor loop? */
          if((orxString_Compare(orxTIMELINE_KZ_CONFIG_KEEP_IN_CACHE, zKey) != 0)
          && (orxString_Compare(orxTIMELINE_KZ_CONFIG_IMMEDIATE, zKey) != 0)
          && (orxString_Compare(orxTIMELINE_KZ_CONFIG_LOOP, zKey) != 0))
          {
            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "TimeLine track [%s]: ignoring invalid key (%s).", _zTrackID, zKey);
          }
        }
      }

      /* Allocates track */
      pstResult = (orxTIMELINE_TRACK *)orxMemory_Allocate(sizeof(orxTIMELINE_TRACK) + (u32EventCounter * sizeof(orxTIMELINE_TRACK_EVENT)), orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Stores its ID */
        pstResult->u32ID = orxString_GetID(orxConfig_GetCurrentSection());

        /* Adds it to reference table */
        if(orxHashTable_Set(sstTimeLine.pstTrackTable, pstResult->u32ID, pstResult) != orxSTATUS_FAILURE)
        {
          orxU32 u32EventIndex;

          /* For all events */
          for(u32EventIndex = 0; u32EventIndex < u32EventCounter;)
          {
            const orxSTRING zKey;
            orxFLOAT        fTime;
            orxU32          u32KeyIndex, u32ListCounter;

            /* Finds time to add next */
            for(fTime = orxFLOAT_MAX, u32KeyIndex = orxU32_UNDEFINED, i = 0; i < u32KeyCounter; i++)
            {
              /* Is sooner? */
              if(afTimeList[i] < fTime)
              {
                /* Stores it */
                fTime       = afTimeList[i];
                u32KeyIndex = i;
              }
            }

            /* Checks */
            orxASSERT(u32KeyIndex != orxU32_UNDEFINED);

            /* Gets corresponding key */
            zKey = orxConfig_GetKey(u32KeyIndex);

            /* For all events */
            for(i = 0, u32ListCounter = orxConfig_GetListCounter(zKey);
                i < u32ListCounter;
                i++, u32EventIndex++)
            {
              /* Checks */
              orxASSERT(u32EventIndex < u32EventCounter);

              /* Stores event */
              pstResult->astEventList[u32EventIndex].fTimeStamp = fTime;
              pstResult->astEventList[u32EventIndex].zEventText = orxString_Store(orxConfig_GetListString(zKey, i));
            }

            /* Clears time entry */
            afTimeList[u32KeyIndex] = orxFLOAT_MAX;
          }

          /* Stores its reference */
          pstResult->zReference = orxString_GetFromID(pstResult->u32ID);

          /* Updates track counters */
          pstResult->u32RefCounter    = 1;
          pstResult->u32EventCounter  = u32EventCounter;

          /* Should keep in cache? */
          if(orxConfig_GetBool(orxTIMELINE_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference counter to keep it in cache table */
            pstResult->u32RefCounter++;
          }
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to add track to hashtable.");

          /* Deletes it */
          orxMemory_Free(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create TimeLine track [%s]: memory allocation failure.", _zTrackID);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create TimeLine track [%s]: config section is empty.", _zTrackID);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Couldn't create TimeLine track [%s]: config section not found.", _zTrackID);
  }

  /* Done! */
  return pstResult;
}

/** Removes a track
 */
static orxINLINE void orxTimeLine_DeleteTrack(orxTIMELINE_TRACK *_pstTrack)
{
  /* Decreases counter */
  _pstTrack->u32RefCounter--;

  /* Not referenced? */
  if(_pstTrack->u32RefCounter == 0)
  {
    /* Has an ID? */
    if((_pstTrack->zReference != orxNULL)
    && (_pstTrack->zReference != orxSTRING_EMPTY))
    {
      /* Removes it from the table */
      orxHashTable_Remove(sstTimeLine.pstTrackTable, _pstTrack->u32ID);
    }

    /* Deletes it */
    orxMemory_Free(_pstTrack);
  }

  /* Done! */
  return;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxTimeLine_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Add or update? */
  if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->u32GroupID == orxString_ToCRC(orxCONFIG_KZ_RESOURCE_GROUP))
    {
      orxHANDLE           hIterator;
      orxU64              u64Key;
      orxTIMELINE_TRACK  *pstTrack;

      /* For all tracks */
      for(hIterator = orxHashTable_GetNext(sstTimeLine.pstTrackTable, orxHANDLE_UNDEFINED, &u64Key, (void **)&pstTrack);
          hIterator != orxHANDLE_UNDEFINED;
          hIterator = orxHashTable_GetNext(sstTimeLine.pstTrackTable, hIterator, &u64Key, (void **)&pstTrack))
      {
        /* Match origin? */
        if(orxConfig_GetOriginID(pstTrack->zReference) == pstPayload->u32NameID)
        {
          orxTIMELINE        *pstTimeLine;
          orxTIMELINE_TRACK  *pstNewTrack;
          orxU32              u32Counter, u32ID, u32Flags = orxTIMELINE_HOLDER_KU32_FLAG_NONE;
          const orxSTRING     zReference;

          /* Backups counter, ID & reference */
          u32Counter  = pstTrack->u32RefCounter;
          u32ID       = pstTrack->u32ID;
          zReference  = pstTrack->zReference;

          /* Deletes it (but keeps it reference in the hashtable to prevent infinite loop upon table changes) */
          orxMemory_Free(pstTrack);

          /* Creates new track */
          pstNewTrack = orxTimeLine_CreateTrack(zReference);

          /* Success? */
          if(pstNewTrack != orxNULL)
          {
            /* Restores its counter */
            pstNewTrack->u32RefCounter = u32Counter;

            /* Pushes its config section */
            orxConfig_PushSection(zReference);

            /* Should loop? */
            if(orxConfig_GetBool(orxTIMELINE_KZ_CONFIG_LOOP) != orxFALSE)
            {
              /* Updates flags */
              u32Flags = orxTIMELINE_HOLDER_KU32_FLAG_LOOP;
            }

            /* Pops config section */
            orxConfig_PopSection();
          }
          else
          {
            /* Removes old reference from the table */
            orxHashTable_Remove(sstTimeLine.pstTrackTable, u32ID);
          }

          /* For all timelines */
          for(pstTimeLine = orxTIMELINE(orxStructure_GetFirst(orxSTRUCTURE_ID_TIMELINE));
              pstTimeLine != orxNULL;
              pstTimeLine = orxTIMELINE(orxStructure_GetNext(pstTimeLine)))
          {
            orxU32 u32Index;

            /* For all its track */
            for(u32Index = 0; u32Index < orxTIMELINE_KU32_TRACK_NUMBER; u32Index++)
            {
              /* Matches? */
              if(pstTimeLine->astTrackList[u32Index].pstTrack == pstTrack)
              {
                /* Updates its data */
                pstTimeLine->astTrackList[u32Index].pstTrack = pstNewTrack;

                /* Updates its flags */
                orxFLAG_SET(pstTimeLine->astTrackList[u32Index].u32Flags, u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_LOOP);
              }
            }
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the TimeLines
 */
static orxINLINE void orxTimeLine_DeleteAll()
{
  orxTIMELINE *pstTimeLine;

  /* Gets first TimeLine */
  pstTimeLine = orxTIMELINE(orxStructure_GetFirst(orxSTRUCTURE_ID_TIMELINE));

  /* Non empty? */
  while(pstTimeLine != orxNULL)
  {
    /* Deletes it */
    orxTimeLine_Delete(pstTimeLine);

    /* Gets first TimeLine */
    pstTimeLine = orxTIMELINE(orxStructure_GetFirst(orxSTRUCTURE_ID_TIMELINE));
  }

  /* Done! */
  return;
}

/** Updates the TimeLine (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxTimeLine_Update(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo)
{
  orxTIMELINE  *pstTimeLine;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxTimeLine_Update");

  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);

  /* Gets TimeLine */
  pstTimeLine = orxTIMELINE(_pstStructure);

  /* Is enabled? */
  if(orxTimeLine_IsEnabled(pstTimeLine) != orxFALSE)
  {
    orxU32 i;

    /* Cleans its flags */
    orxStructure_SetFlags(pstTimeLine, orxTIMELINE_KU32_FLAG_NONE, orxTIMELINE_KU32_FLAG_DIRTY);

    /* Has clock info? */
    if(_pstClockInfo != orxNULL)
    {
      /* Computes its new time cursor */
      pstTimeLine->fTime += _pstClockInfo->fDT;
    }

    /* For all tracks */
    for(i = 0; i < orxTIMELINE_KU32_TRACK_NUMBER; i++)
    {
      orxTIMELINE_TRACK *pstTrack;

      /* Is timeline dirty? */
      if(orxStructure_TestFlags(pstTimeLine, orxTIMELINE_KU32_FLAG_DIRTY))
      {
        orxU32 j;

        /* For all previous tracks */
        for(j = 0; j < i; j++)
        {
          /* Is defined? */
          if(pstTimeLine->astTrackList[j].pstTrack != orxNULL)
          {
            /* Hasn't been updated? */
            if(!orxFLAG_TEST(pstTimeLine->astTrackList[j].u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_UPDATED))
            {
              /* Selects it */
              i = j;

              break;
            }
          }
        }
      }

      /* Gets track */
      pstTrack = pstTimeLine->astTrackList[i].pstTrack;

      /* Valid and not already updated? */
      if((pstTrack != orxNULL) && (!orxFLAG_TEST(pstTimeLine->astTrackList[i].u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_UPDATED)))
      {
        orxFLOAT fTrackLocalTime;

        /* Gets track local time */
        fTrackLocalTime = pstTimeLine->fTime - pstTimeLine->astTrackList[i].fStartTime;

        /* Has time come? */
        if(fTrackLocalTime >= orxFLOAT_0)
        {
          orxTIMELINE_EVENT_PAYLOAD stPayload;
          orxU32                    u32EventIndex;

          /* Is the first time? */
          if(!orxFLAG_TEST(pstTimeLine->astTrackList[i].u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_PLAYED))
          {
            /* Inits event payload */
            orxMemory_Zero(&stPayload, sizeof(orxTIMELINE_EVENT_PAYLOAD));
            stPayload.pstTimeLine = pstTimeLine;
            stPayload.zTrackName  = pstTrack->zReference;

            /* Sends event */
            orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRACK_START, _pstCaller, _pstCaller, &stPayload);
          }

          /* Updates its status */
          orxFLAG_SET(pstTimeLine->astTrackList[i].u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_PLAYED | orxTIMELINE_HOLDER_KU32_FLAG_UPDATED, orxTIMELINE_HOLDER_KU32_FLAG_NONE);

          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxTIMELINE_EVENT_PAYLOAD));
          stPayload.pstTimeLine = pstTimeLine;
          stPayload.zTrackName  = pstTrack->zReference;

          /* For all recently past events */
          for(u32EventIndex = pstTimeLine->astTrackList[i].u32NextEventIndex;
              (u32EventIndex < pstTrack->u32EventCounter) && (fTrackLocalTime >= pstTrack->astEventList[u32EventIndex].fTimeStamp);
              u32EventIndex++)
          {
            /* Updates payload */
            stPayload.zEvent      = pstTrack->astEventList[u32EventIndex].zEventText;
            stPayload.fTimeStamp  = pstTrack->astEventList[u32EventIndex].fTimeStamp;

            /* Sends event */
            orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRIGGER, _pstCaller, _pstCaller, &stPayload);
          }

          /* Is over? */
          if(u32EventIndex >= pstTrack->u32EventCounter)
          {
            orxTIMELINE_TRACK *pstTrack;

            /* Gets track */
            pstTrack = pstTimeLine->astTrackList[i].pstTrack;

            /* Inits event payload */
            orxMemory_Zero(&stPayload, sizeof(orxTIMELINE_EVENT_PAYLOAD));
            stPayload.pstTimeLine = pstTimeLine;
            stPayload.zTrackName  = pstTrack->zReference;

            /* Is a looping track? */
            if(orxFLAG_TEST(pstTimeLine->astTrackList[i].u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_LOOP))
            {
              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_LOOP, _pstCaller, _pstCaller, &stPayload);

              /* Resets track */
              pstTimeLine->astTrackList[i].u32NextEventIndex  = 0;
              pstTimeLine->astTrackList[i].fStartTime         = pstTimeLine->fTime;
            }
            else
            {
              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRACK_STOP, _pstCaller, _pstCaller, &stPayload);

              /* Removes its reference */
              pstTimeLine->astTrackList[i].pstTrack = orxNULL;

              /* Sends event */
              orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRACK_REMOVE, _pstCaller, _pstCaller, &stPayload);

              /* Deletes it */
              orxTimeLine_DeleteTrack(pstTrack);
            }
          }
          else
          {
            /* Updates next event index */
            pstTimeLine->astTrackList[i].u32NextEventIndex = u32EventIndex;
          }
        }
      }
    }

    /* For all tracks */
    for(i = 0; i < orxTIMELINE_KU32_TRACK_NUMBER; i++)
    {
      /* Clears its update flag */
      orxFLAG_SET(pstTimeLine->astTrackList[i].u32Flags, orxTIMELINE_HOLDER_KU32_FLAG_NONE, orxTIMELINE_HOLDER_KU32_FLAG_UPDATED);
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

/** TimeLine module setup
 */
void orxFASTCALL orxTimeLine_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_TIMELINE, orxMODULE_ID_EVENT);

  return;
}

/** Inits the TimeLine module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTimeLine_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstTimeLine, sizeof(orxTIMELINE_STATIC));

    /* Creates track table */
    sstTimeLine.pstTrackTable = orxHashTable_Create(orxTIMELINE_KU32_TRACK_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstTimeLine.pstTrackTable != orxNULL)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(TIMELINE, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxTIMELINE_KU32_BANK_SIZE, &orxTimeLine_Update);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create TimeLine track table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to initialize the TimeLine module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    orxFLAG_SET(sstTimeLine.u32Flags, orxTIMELINE_KU32_STATIC_FLAG_READY, orxTIMELINE_KU32_STATIC_FLAG_NONE);

    /* Adds event handler */
    orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxTimeLine_EventHandler);
  }
  else
  {
    /* Deletes track table if needed */
    if(sstTimeLine.pstTrackTable != orxNULL)
    {
      orxHashTable_Delete(sstTimeLine.pstTrackTable);
    }
  }

  /* Done! */
  return eResult;
}

/** Exits from the TimeLine module
 */
void orxFASTCALL orxTimeLine_Exit()
{
  /* Initialized? */
  if(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY)
  {
    orxTIMELINE_TRACK *pstTrack;

    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxTimeLine_EventHandler);

    /* Deletes TimeLine list */
    orxTimeLine_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_TIMELINE);

    /* For all remaining tracks */
    while(orxHashTable_GetNext(sstTimeLine.pstTrackTable, orxNULL, orxNULL, (void **)&pstTrack) != orxHANDLE_UNDEFINED)
    {
      /* Deletes it */
      orxTimeLine_DeleteTrack(pstTrack);
    }

    /* Deletes track table */
    orxHashTable_Delete(sstTimeLine.pstTrackTable);

    /* Updates flags */
    sstTimeLine.u32Flags &= ~orxTIMELINE_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Tried to exit from the TimeLine module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Creates an empty TimeLine
 * @return      Created orxTIMELINE / orxNULL
 */
orxTIMELINE *orxFASTCALL orxTimeLine_Create()
{
  orxTIMELINE *pstResult;

  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);

  /* Creates TimeLine */
  pstResult = orxTIMELINE(orxStructure_Create(orxSTRUCTURE_ID_TIMELINE));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Inits flags */
    orxStructure_SetFlags(pstResult, orxTIMELINE_KU32_FLAG_ENABLED, orxTIMELINE_KU32_MASK_ALL);

    /* Increases counter */
    orxStructure_IncreaseCounter(pstResult);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "Failed to create TimeLine structure.");
  }

  /* Done! */
  return pstResult;
}

/** Deletes a TimeLine
 * @param[in] _pstTimeLine            Concerned TimeLine
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTimeLine_Delete(orxTIMELINE *_pstTimeLine)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTimeLine);

  /* Decreases counter */
  orxStructure_DecreaseCounter(_pstTimeLine);

  /* Not referenced? */
  if(orxStructure_GetRefCounter(_pstTimeLine) == 0)
  {
    orxTIMELINE_EVENT_PAYLOAD stPayload;
    orxSTRUCTURE             *pstOwner;
    orxU32                    i;

    /* Gets owner */
    pstOwner = orxStructure_GetOwner(_pstTimeLine);

    /* Inits event payload */
    orxMemory_Zero(&stPayload, sizeof(orxTIMELINE_EVENT_PAYLOAD));
    stPayload.pstTimeLine = _pstTimeLine;

    /* For all tracks */
    for(i = 0; i < orxTIMELINE_KU32_TRACK_NUMBER; i++)
    {
      /* Valid? */
      if(_pstTimeLine->astTrackList[i].pstTrack != orxNULL)
      {
        orxTIMELINE_TRACK *pstTrack;

        /* Gets track */
        pstTrack = _pstTimeLine->astTrackList[i].pstTrack;

        /* Removes its reference */
        _pstTimeLine->astTrackList[i].pstTrack = orxNULL;

        /* Updates payload */
        stPayload.zTrackName = pstTrack->zReference;

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRACK_REMOVE, pstOwner, pstOwner, &stPayload);

        /* Deletes it */
        orxTimeLine_DeleteTrack(pstTrack);
      }
    }

    /* Deletes structure */
    orxStructure_Delete(_pstTimeLine);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Enables/disables a TimeLine
 * @param[in]   _pstTimeLine        Concerned TimeLine
 * @param[in]   _bEnable      enable / disable
 */
void orxFASTCALL orxTimeLine_Enable(orxTIMELINE *_pstTimeLine, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTimeLine);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstTimeLine, orxTIMELINE_KU32_FLAG_ENABLED, orxTIMELINE_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstTimeLine, orxTIMELINE_KU32_FLAG_NONE, orxTIMELINE_KU32_FLAG_ENABLED);
  }

  /* Done! */
  return;
}

/** Is TimeLine enabled?
 * @param[in]   _pstTimeLine        Concerned TimeLine
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxTimeLine_IsEnabled(const orxTIMELINE *_pstTimeLine)
{
  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstTimeLine);

  /* Done! */
  return(orxStructure_TestFlags(_pstTimeLine, orxTIMELINE_KU32_FLAG_ENABLED));
}

/** Adds a track to a TimeLine from config
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @param[in]   _zTrackID             Config ID
 * return       orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTimeLine_AddTrackFromConfig(orxTIMELINE *_pstTimeLine, const orxSTRING _zTrackID)
{
  orxU32    u32Index;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstTimeLine);
  orxASSERT((_zTrackID != orxNULL) && (_zTrackID != orxSTRING_EMPTY));

  /* Checks */
  orxSTRUCTURE_ASSERT(_pstTimeLine);

  /* Finds an empty track */
  for(u32Index = 0; (u32Index < orxTIMELINE_KU32_TRACK_NUMBER) && (_pstTimeLine->astTrackList[u32Index].pstTrack != orxNULL); u32Index++);

  /* Found? */
  if(u32Index < orxTIMELINE_KU32_TRACK_NUMBER)
  {
    orxTIMELINE_TRACK  *pstTrack;
    orxU32              u32ID;

    /* Gets track ID */
    u32ID = orxString_ToCRC(_zTrackID);

    /* Search for reference */
    pstTrack = (orxTIMELINE_TRACK *)orxHashTable_Get(sstTimeLine.pstTrackTable, u32ID);

    /* Found? */
    if(pstTrack != orxNULL)
    {
      /* Increases counter */
      pstTrack->u32RefCounter++;
    }
    else
    {
      /* Creates track */
      pstTrack = orxTimeLine_CreateTrack(_zTrackID);
    }

    /* Valid? */
    if(pstTrack != orxNULL)
    {
      orxTIMELINE_EVENT_PAYLOAD stPayload;
      orxBOOL                   bImmediate;
      orxSTRUCTURE             *pstOwner;
      orxU32                    u32Flags = orxTIMELINE_HOLDER_KU32_FLAG_NONE;

      /* Gets owner */
      pstOwner = orxStructure_GetOwner(_pstTimeLine);

      /* Pushes its config section */
      orxConfig_PushSection(pstTrack->zReference);

      /* Should loop? */
      if(orxConfig_GetBool(orxTIMELINE_KZ_CONFIG_LOOP) != orxFALSE)
      {
        /* Updates flags */
        u32Flags = orxTIMELINE_HOLDER_KU32_FLAG_LOOP;
      }

      /* Gets immediate status */
      bImmediate = orxConfig_GetBool(orxTIMELINE_KZ_CONFIG_IMMEDIATE);

      /* Pops config section */
      orxConfig_PopSection();

      /* Updates track holder */
      _pstTimeLine->astTrackList[u32Index].pstTrack           = pstTrack;
      _pstTimeLine->astTrackList[u32Index].fStartTime         = _pstTimeLine->fTime;
      _pstTimeLine->astTrackList[u32Index].u32NextEventIndex  = 0;
      _pstTimeLine->astTrackList[u32Index].u32Flags           = u32Flags;

      /* Inits event payload */
      orxMemory_Zero(&stPayload, sizeof(orxTIMELINE_EVENT_PAYLOAD));
      stPayload.pstTimeLine = _pstTimeLine;
      stPayload.zTrackName  = pstTrack->zReference;

      /* Sends event */
      orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRACK_ADD, pstOwner, pstOwner, &stPayload);

      /* Updates timeline flags */
      orxStructure_SetFlags(_pstTimeLine, orxTIMELINE_KU32_FLAG_DIRTY, orxTIMELINE_KU32_FLAG_NONE);

      /* Is immediate? */
      if(bImmediate != orxFALSE)
      {
        /* Updates it */
        orxTimeLine_Update(orxSTRUCTURE(_pstTimeLine), pstOwner, orxNULL);
      }

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_OBJECT, "No room for a new track in TimeLine, can't add track <%s>.", _zTrackID);
  }

  /* Done! */
  return eResult;
}

/** Removes a track using its config ID
 * @param[in]   _pstTimeLine          Concerned TimeLine
 * @param[in]   _zTrackID             Config ID of the track to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxTimeLine_RemoveTrackFromConfig(orxTIMELINE *_pstTimeLine, const orxSTRING _zTrackID)
{
  orxU32        u32Index, u32TrackID;
  orxSTRUCTURE *pstOwner;
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);
  orxASSERT((_zTrackID != orxNULL) && (_zTrackID != orxSTRING_EMPTY));

  /* Gets owner */
  pstOwner = orxStructure_GetOwner(_pstTimeLine);

  /* Gets track ID */
  u32TrackID = orxString_ToCRC(_zTrackID);

  /* For all tracks */
  for(u32Index = 0; u32Index < orxTIMELINE_KU32_TRACK_NUMBER; u32Index++)
  {
    /* Is defined? */
    if(_pstTimeLine->astTrackList[u32Index].pstTrack != orxNULL)
    {
      /* Do IDs match? */
      if(_pstTimeLine->astTrackList[u32Index].pstTrack->u32ID == u32TrackID)
      {
        orxTIMELINE_EVENT_PAYLOAD stPayload;
        orxTIMELINE_TRACK        *pstTrack;

        /* Gets track */
        pstTrack = _pstTimeLine->astTrackList[u32Index].pstTrack;

        /* Removes its reference */
        _pstTimeLine->astTrackList[u32Index].pstTrack = orxNULL;

        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxTIMELINE_EVENT_PAYLOAD));
        stPayload.pstTimeLine = _pstTimeLine;
        stPayload.zTrackName  = pstTrack->zReference;

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_TIMELINE, orxTIMELINE_EVENT_TRACK_REMOVE, pstOwner, pstOwner, &stPayload);

        /* Deletes it */
        orxTimeLine_DeleteTrack(pstTrack);

        break;
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets a track duration using its config ID
 * @param[in]   _zTrackID             Config ID of the concerned track
 * @return      Duration if found, -orxFLOAT_1 otherwise
 */
orxFLOAT orxFASTCALL orxTimeLine_GetTrackDuration(const orxSTRING _zTrackID)
{
  orxTIMELINE_TRACK  *pstTrack;
  orxU32              u32TrackID;
  orxFLOAT            fResult;

  /* Checks */
  orxASSERT(sstTimeLine.u32Flags & orxTIMELINE_KU32_STATIC_FLAG_READY);
  orxASSERT((_zTrackID != orxNULL) && (_zTrackID != orxSTRING_EMPTY));

  /* Gets track CRC */
  u32TrackID = orxString_ToCRC(_zTrackID);

  /* Gets it */
  if((pstTrack = (orxTIMELINE_TRACK *)orxHashTable_Get(sstTimeLine.pstTrackTable, u32TrackID)) != orxNULL)
  {
    /* Updates result */
    fResult = pstTrack->astEventList[pstTrack->u32EventCounter - 1].fTimeStamp;
  }
  else
  {
    /* Updates result */
    fResult = -orxFLOAT_1;
  }

  /* Done! */
  return fResult;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4200)

#endif /* __orxMSVC__ */

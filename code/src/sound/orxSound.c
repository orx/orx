/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @file orxSound.c
 * @date 13/07/2008
 * @author iarwain@orx-project.org
 *
 */


#include "sound/orxSound.h"

#include "debug/orxDebug.h"
#include "debug/orxProfiler.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxClock.h"
#include "core/orxEvent.h"
#include "core/orxLocale.h"
#include "core/orxResource.h"
#include "memory/orxMemory.h"
#include "memory/orxBank.h"
#include "object/orxObject.h"
#include "object/orxStructure.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"
#include "utils/orxTree.h"


/** Module flags
 */
#define orxSOUND_KU32_STATIC_FLAG_NONE                  0x00000000  /**< No flags */

#define orxSOUND_KU32_STATIC_FLAG_READY                 0x00000001  /**< Ready flag */

#define orxSOUND_KU32_STATIC_MASK_ALL                   0xFFFFFFFF  /**< All mask */


/** Flags
 */
#define orxSOUND_KU32_FLAG_NONE                         0x00000000  /**< No flags */

#define orxSOUND_KU32_FLAG_HAS_SAMPLE                   0x00000001  /**< Has referenced sample flag */
#define orxSOUND_KU32_FLAG_HAS_STREAM                   0x00000002  /**< Has referenced stream flag */

#define orxSOUND_KU32_FLAG_BACKUP_PLAY                  0x10000000  /**< Backup play flag */
#define orxSOUND_KU32_FLAG_BACKUP_PAUSE                 0x20000000  /**< Backup pause flag */

#define orxSOUND_KU32_MASK_BACKUP_ALL                   0x30000000  /**< Backup all mask */

#define orxSOUND_KU32_MASK_ALL                          0xFFFFFFFF  /**< All mask */

/** Sample flags
 */
#define orxSOUND_SAMPLE_KU32_FLAG_NONE                  0x00000000  /**< No flag */

#define orxSOUND_SAMPLE_KU32_FLAG_INTERNAL              0x10000000  /**< Internal flag */
#define orxSOUND_SAMPLE_KU32_FLAG_CACHED                0x20000000  /**< Cached flag */

#define orxSOUND_SAMPLE_KU32_MASK_ALL                   0xFFFFFFFF  /**< All mask */


/** Misc defines
 */
#define orxSOUND_KU32_SAMPLE_BANK_SIZE                  32

#define orxSOUND_KU32_BANK_SIZE                         64          /**< Bank size */

#define orxSOUND_KU32_STREAM_DEFAULT_CHANNEL_NUMBER     1
#define orxSOUND_KU32_STREAM_DEFAULT_SAMPLE_RATE        48000

#define orxSOUND_KU32_BUS_BANK_SIZE                     64
#define orxSOUND_KU32_BUS_TABLE_SIZE                    64

#define orxSOUND_KZ_CONFIG_SOUND                        "Sound"
#define orxSOUND_KZ_CONFIG_MUSIC                        "Music"
#define orxSOUND_KZ_CONFIG_LOOP                         "Loop"
#define orxSOUND_KZ_CONFIG_PITCH                        "Pitch"
#define orxSOUND_KZ_CONFIG_VOLUME                       "Volume"
#define orxSOUND_KZ_CONFIG_DISTANCE_LIST                "DistanceList"
#define orxSOUND_KZ_CONFIG_GAIN_LIST                    "GainList"
#define orxSOUND_KZ_CONFIG_ROLL_OFF                     "RollOff"
#define orxSOUND_KZ_CONFIG_KEEP_IN_CACHE                "KeepInCache"
#define orxSOUND_KZ_CONFIG_PANNING                      "Panning"
#define orxSOUND_KZ_CONFIG_MIX                          "Mix"
#define orxSOUND_KZ_CONFIG_BUS                          "Bus"
#define orxSOUND_KZ_CONFIG_FILTER_LIST                  "FilterList"
#define orxSOUND_KZ_CONFIG_TYPE                         "Type"
#define orxSOUND_KZ_CONFIG_USE_CUSTOM_PARAM             "UseCustomParam"
#define orxSOUND_KZ_CONFIG_FREQUENCY                    "Frequency"
#define orxSOUND_KZ_CONFIG_ORDER                        "Order"
#define orxSOUND_KZ_CONFIG_Q                            "Q"
#define orxSOUND_KZ_CONFIG_GAIN                         "Gain"
#define orxSOUND_KZ_CONFIG_DELAY                        "Delay"
#define orxSOUND_KZ_CONFIG_DECAY                        "Decay"
#define orxSOUND_KZ_CONFIG_A0                           "A0"
#define orxSOUND_KZ_CONFIG_A1                           "A1"
#define orxSOUND_KZ_CONFIG_A2                           "A2"
#define orxSOUND_KZ_CONFIG_B0                           "B0"
#define orxSOUND_KZ_CONFIG_B1                           "B1"
#define orxSOUND_KZ_CONFIG_B2                           "B2"

#define orxSOUND_KZ_EMPTY_STREAM                        "empty"
#define orxSOUND_KZ_TYPE_BIQUAD                         "biquad"
#define orxSOUND_KZ_TYPE_LOW_PASS                       "lowpass"
#define orxSOUND_KZ_TYPE_HIGH_PASS                      "highpass"
#define orxSOUND_KZ_TYPE_BAND_PASS                      "bandpass"
#define orxSOUND_KZ_TYPE_LOW_SHELF                      "lowshelf"
#define orxSOUND_KZ_TYPE_HIGH_SHELF                     "highshelf"
#define orxSOUND_KZ_TYPE_NOTCH                          "notch"
#define orxSOUND_KZ_TYPE_PEAKING                        "peaking"
#define orxSOUND_KZ_TYPE_DELAY                          "delay"

#define orxSOUND_KC_LOCALE_MARKER                       '$'


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Sound bus structure
 */
typedef struct __orxSOUND_BUS_t
{
  orxTREE_NODE            stNode;                       /**< Tree node : 8/12 */
  orxLINKLIST             stList;                       /**< Sound list : 20/32 */
  orxSTRINGID             stID;                         /**< ID : 24/36 */
  orxHANDLE               hData;                        /**< Bus data : 28/44 */
  orxFLOAT                fGlobalVolume;                /**< Global volume : 32/48 */
  orxFLOAT                fGlobalPitch;                 /**< Global pitch : 36/52 */
  orxFLOAT                fLocalVolume;                 /**< Local volume : 40/56 */
  orxFLOAT                fLocalPitch;                  /**< Local pitch : 44/60 */

} orxSOUND_BUS;

/** Sound sample structure
 */
struct __orxSOUND_SAMPLE_t
{
  orxSOUNDSYSTEM_SAMPLE  *pstData;                      /**< Sound data : 4 */
  orxSTRINGID             stID;                         /**< Sample ID : 8 */
  orxU32                  u32Count;                     /**< Reference count : 12 */
  orxU32                  u32Flags;                     /**< Flags : 16 */

};

/** Sound structure
 */
struct __orxSOUND_t
{
  orxSTRUCTURE          stStructure;                    /**< Public structure, first structure member : 32 */
  orxLINKLIST_NODE      stBusNode;                      /**< Bus node : 44/56 */
  orxSOUNDSYSTEM_SOUND *pstData;                        /**< Sound data : 48/64 */
  union
  {
    orxSOUND_SAMPLE    *pstSample;                      /**< Sound sample : 52/72 */
    orxSTRINGID         stStreamID;                     /**< Stream ID : 56/72 */
  };
  orxSTRINGID           stBusID;                        /**< Sound bus ID: 64/80 */
  const orxSTRING       zReference;                     /**< Sound reference : 68/88 */
  orxSOUND_STATUS       eStatus;                        /**< Sound status : 72/92 */
  orxFLOAT              fVolume;                        /**< Sound volume : 76/96 */
  orxFLOAT              fPitch;                         /**< Sound pitch : 80/100 */
  orxFLOAT              fPitchModifier;                 /**< Sound pitch modifier : 84/104 */
};

/** Static structure
 */
typedef struct __orxSOUND_STATIC_t
{
  orxBANK      *pstSampleBank;                          /**< Sample bank */
  orxHASHTABLE *pstSampleTable;                         /**< Sample hash table */
  orxBANK      *pstBusBank;                             /**< Bus bank */
  orxHASHTABLE *pstBusTable;                            /**< Bus table */
  orxTREE       stBusTree;                              /**< Bus tree */
  orxSOUND_BUS *pstCachedBus;                           /**< Bus cache */
  orxSTRINGID   stMasterBusID;                          /**< Master bus ID */
  orxU32        u32Flags;                               /**< Control flags */

} orxSOUND_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** static data
 */
static orxSOUND_STATIC sstSound;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Loads a sound sample
 * @return orxSOUND_SAMPLE / orxNULL
 */
static orxINLINE orxSOUND_SAMPLE *orxSound_LoadSample(const orxSTRING _zFileName, orxBOOL _bKeepInCache)
{
  orxSOUND_SAMPLE  *pstResult;
  orxSTRINGID       stID;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* Gets its ID */
  stID = orxString_GetID(_zFileName);

  /* Looks for sample */
  pstResult = (orxSOUND_SAMPLE *)orxHashTable_Get(sstSound.pstSampleTable, stID);

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases its reference count */
    pstResult->u32Count++;
  }
  else
  {
    /* Allocates a sample */
    pstResult = (orxSOUND_SAMPLE *)orxBank_Allocate(sstSound.pstSampleBank);

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Loads its data */
      pstResult->pstData = orxSoundSystem_LoadSample(_zFileName);

      /* Adds it to sample table */
      if((pstResult->pstData != orxNULL)
      && (orxHashTable_Add(sstSound.pstSampleTable, stID, pstResult) != orxSTATUS_FAILURE))
      {
        /* Should keep in cache? */
        if(_bKeepInCache != orxFALSE)
        {
          /* Inits its reference count */
          pstResult->u32Count = 1;

          /* Stores its flags */
          orxFLAG_SET(pstResult->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_INTERNAL | orxSOUND_SAMPLE_KU32_FLAG_CACHED, orxSOUND_SAMPLE_KU32_MASK_ALL);
        }
        else
        {
          /* Inits its reference count */
          pstResult->u32Count = 0;

          /* Stores its flags */
          orxFLAG_SET(pstResult->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_INTERNAL, orxSOUND_SAMPLE_KU32_MASK_ALL);
        }

        /* Stores its ID */
        pstResult->stID = stID;
      }
      else
      {
        /* Deletes it */
        orxBank_Free(sstSound.pstSampleBank, pstResult);

        /* Updates result */
        pstResult = orxNULL;

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to add sound to hashtable.");
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Unloads a sound sample
 */
static orxINLINE void orxSound_UnloadSample(orxSOUND_SAMPLE *_pstSample)
{
  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Not referenced anymore? */
  if(_pstSample->u32Count == 0)
  {
    /* Is internal? */
    if(orxFLAG_TEST(_pstSample->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_INTERNAL))
    {
      /* Has data? */
      if(_pstSample->pstData != orxNULL)
      {
        /* Unloads its data */
        orxSoundSystem_DeleteSample(_pstSample->pstData);
      }
    }

    /* Removes it from sample table */
    orxHashTable_Remove(sstSound.pstSampleTable, _pstSample->stID);

    /* Deletes it */
    orxBank_Free(sstSound.pstSampleBank, _pstSample);
  }
  else
  {
    /* Updates its reference count */
    _pstSample->u32Count--;
  }

  /* Done! */
  return;
}

/** Unloads all the sound samples
 */
static orxINLINE void orxSound_UnloadAllSample()
{
  orxSOUND_SAMPLE *pstSample;

  /* Gets first sample */
  pstSample = (orxSOUND_SAMPLE *)orxBank_GetNext(sstSound.pstSampleBank, orxNULL);

  /* Non empty? */
  while(pstSample != orxNULL)
  {
    /* Forces its deletion */
    orxFLAG_SET(pstSample->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_INTERNAL, orxSOUND_SAMPLE_KU32_FLAG_NONE);

    /* Deletes it */
    orxSound_UnloadSample(pstSample);

    /* Gets first sample */
    pstSample = (orxSOUND_SAMPLE *)orxBank_GetNext(sstSound.pstSampleBank, orxNULL);
  }

  /* Done! */
  return;
}

/** Processes config data
 */
static orxSTATUS orxFASTCALL orxSound_ProcessConfigData(orxSOUND *_pstSound, orxBOOL _bOnlySettings)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Has reference? */
  if((_pstSound->zReference != orxNULL)
  && (*(_pstSound->zReference) != orxCHAR_NULL))
  {
    /* Pushes its config section */
    orxConfig_PushSection(_pstSound->zReference);

    /* Don't process only settings? */
    if(_bOnlySettings == orxFALSE)
    {
      const orxSTRING zName;

      /* Has data? */
      if(_pstSound->pstData != orxNULL)
      {
        /* Deletes it */
        orxSoundSystem_Delete(_pstSound->pstData);
        _pstSound->pstData = orxNULL;
      }

      /* Has a referenced sample? */
      if(orxStructure_TestFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_SAMPLE))
      {
        /* Unloads it */
        orxSound_UnloadSample(_pstSound->pstSample);
        _pstSound->pstSample = orxNULL;
      }
      else
      {
        _pstSound->stStreamID = 0;
      }

      /* Inits pitch modifier */
      _pstSound->fPitchModifier = orxFLOAT_1;

      /* Updates flags */
      orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_NONE, orxSOUND_KU32_FLAG_HAS_SAMPLE | orxSOUND_KU32_FLAG_HAS_STREAM);

      /* Is a sound? */
      if(((zName = orxConfig_GetString(orxSOUND_KZ_CONFIG_SOUND)) != orxSTRING_EMPTY)
      && (*zName != orxCHAR_NULL))
      {
        /* Profiles */
        orxPROFILER_PUSH_MARKER("orxSound_CreateFromConfig (Sound)");

        /* Begins with locale marker? */
        if(*zName == orxSOUND_KC_LOCALE_MARKER)
        {
          /* Gets its locale value */
          zName = (*(zName + 1) == orxSOUND_KC_LOCALE_MARKER) ? zName + 1 : orxLocale_GetString(zName + 1, orxSOUND_KZ_LOCALE_GROUP);
        }

        /* Loads its corresponding sample */
        _pstSound->pstSample = orxSound_LoadSample(zName, orxConfig_GetBool(orxSOUND_KZ_CONFIG_KEEP_IN_CACHE));

        /* Valid? */
        if(_pstSound->pstSample != orxNULL)
        {
          /* Creates sound data based on it */
          _pstSound->pstData = orxSoundSystem_CreateFromSample(_pstSound, _pstSound->pstSample->pstData);

          /* Valid? */
          if(_pstSound->pstData != orxNULL)
          {
            /* Updates its status */
            orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_SAMPLE, orxSOUND_KU32_FLAG_NONE);
          }
          else
          {
            /* Unloads its sample */
            orxSound_UnloadSample(_pstSound->pstSample);

            /* Removes its reference */
            _pstSound->pstSample = orxNULL;

            /* Updates its status */
            orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_NONE, orxSOUND_KU32_FLAG_HAS_SAMPLE);
          }
        }

        /* Profiles */
        orxPROFILER_POP_MARKER();
      }
      /* Is a music? */
      else if(((zName = orxConfig_GetString(orxSOUND_KZ_CONFIG_MUSIC)) != orxSTRING_EMPTY)
           && (*zName != orxCHAR_NULL))
      {
        /* Profiles */
        orxPROFILER_PUSH_MARKER("orxSound_CreateFromConfig (Music)");

        /* Begins with locale marker? */
        if(*zName == orxSOUND_KC_LOCALE_MARKER)
        {
          /* Gets its locale value */
          zName = (*(zName + 1) == orxSOUND_KC_LOCALE_MARKER) ? zName + 1 : orxLocale_GetString(zName + 1, orxSOUND_KZ_LOCALE_GROUP);
        }

        /* Is empty stream ? */
        if(orxString_NICompare(zName, orxSOUND_KZ_EMPTY_STREAM, orxString_GetLength(orxSOUND_KZ_EMPTY_STREAM)) == 0)
        {
          const orxSTRING zStream;
          orxU32          u32Value, u32SampleRate, u32ChannelNumber;

          /* Gets default sample rate and channel number */
          u32SampleRate     = orxSOUND_KU32_STREAM_DEFAULT_SAMPLE_RATE;
          u32ChannelNumber  = orxSOUND_KU32_STREAM_DEFAULT_CHANNEL_NUMBER;

          /* Gets stream specifications */
          zStream = zName + orxString_GetLength(orxSOUND_KZ_EMPTY_STREAM);

          /* For all specifications */
          while(orxString_ToU32(orxString_SkipWhiteSpaces(zStream), &u32Value, &zStream) != orxSTATUS_FAILURE)
          {
            /* Channel? */
            if(u32Value < 100)
            {
              u32ChannelNumber = u32Value;
            }
            /* Sample rate */
            else
            {
              u32SampleRate = u32Value;
            }
          }

          /* Creates empty stream */
          _pstSound->pstData    = orxSoundSystem_CreateStream(_pstSound, u32ChannelNumber, u32SampleRate);
          _pstSound->stStreamID = orxSTRINGID_UNDEFINED;
        }
        else
        {
          /* Loads it */
          _pstSound->pstData    = orxSoundSystem_LoadStream(_pstSound, zName);
          _pstSound->stStreamID = orxString_Hash(zName);
        }

        /* Valid? */
        if(_pstSound->pstData != orxNULL)
        {
          /* Updates its status */
          orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_STREAM, orxSOUND_KU32_FLAG_NONE);
        }
        else
        {
          /* Updates its status */
          orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_NONE, orxSOUND_KU32_FLAG_HAS_STREAM);
        }

        /* Profiles */
        orxPROFILER_POP_MARKER();
      }
    }

    /* Valid content? */
    if(_pstSound->pstData != orxNULL)
    {
      /* Has filter list? */
      if(orxConfig_HasValue(orxSOUND_KZ_CONFIG_FILTER_LIST) != orxFALSE)
      {
        orxS32 i, iCount;

        /* For all filters */
        for(i = 0, iCount = orxConfig_GetListCount(orxSOUND_KZ_CONFIG_FILTER_LIST); i < iCount; i++)
        {
          /* Adds it */
          orxSound_AddFilterFromConfig(_pstSound, orxConfig_GetListString(orxSOUND_KZ_CONFIG_FILTER_LIST, i));
        }
      }

      /* Has bus? */
      if(orxConfig_HasValue(orxSOUND_KZ_CONFIG_BUS) != orxFALSE)
      {
        const orxSTRING zBus;

        /* Gets it */
        zBus = orxConfig_GetString(orxSOUND_KZ_CONFIG_BUS);

        /* Valid? */
        if(*zBus != orxCHAR_NULL)
        {
          /* Sets it */
          orxSound_SetBusID(_pstSound, orxString_GetID(zBus));
        }
        else
        {
          /* Sets master bus */
          orxSound_SetBusID(_pstSound, sstSound.stMasterBusID);
        }
      }
      else
      {
        /* Sets master bus */
        orxSound_SetBusID(_pstSound, sstSound.stMasterBusID);
      }

      /* Updates looping status */
      orxSoundSystem_Loop(_pstSound->pstData, orxConfig_GetBool(orxSOUND_KZ_CONFIG_LOOP));

      /* Has volume? */
      if(orxConfig_HasValue(orxSOUND_KZ_CONFIG_VOLUME) != orxFALSE)
      {
        /* Updates volume */
        orxSound_SetVolume(_pstSound, orxConfig_GetFloat(orxSOUND_KZ_CONFIG_VOLUME));
      }
      else
      {
        /* Updates volume */
        orxSound_SetVolume(_pstSound, orxFLOAT_1);
      }

      /* Has pitch? */
      if(orxConfig_HasValue(orxSOUND_KZ_CONFIG_PITCH) != orxFALSE)
      {
        /* Updates pitch (updating internal shadowing for time-stretching purpose) */
        orxSound_SetPitch(_pstSound, orxConfig_GetFloat(orxSOUND_KZ_CONFIG_PITCH));
      }
      else
      {
        /* Updates pitch (updating internal shadowing for time-stretching purpose) */
        orxSound_SetPitch(_pstSound, orxFLOAT_1);
      }

      /* Has distance list? */
      if(orxConfig_HasValue(orxSOUND_KZ_CONFIG_DISTANCE_LIST) != orxFALSE)
      {
        orxFLOAT afDistanceList[2], afGainList[2];
        orxFLOAT fRollOff;

        /* Gets distances */
        afDistanceList[0] = orxConfig_GetListFloat(orxSOUND_KZ_CONFIG_DISTANCE_LIST, 0);
        if(orxConfig_GetListCount(orxSOUND_KZ_CONFIG_DISTANCE_LIST) > 1)
        {
          afDistanceList[1] = orxConfig_GetListFloat(orxSOUND_KZ_CONFIG_DISTANCE_LIST, 1);
          if(afDistanceList[1] < afDistanceList[0])
          {
            orxFLOAT fTemp;
            fTemp             = afDistanceList[0];
            afDistanceList[0] = afDistanceList[1];
            afDistanceList[1] = fTemp;
          }
        }
        else
        {
          afDistanceList[1] = orxMATH_KF_MAX;
        }

        /* Has gain list? */
        if(orxConfig_HasValue(orxSOUND_KZ_CONFIG_GAIN_LIST) != orxFALSE)
        {
          /* Gets gains */
          afGainList[0] = orxConfig_GetListFloat(orxSOUND_KZ_CONFIG_GAIN_LIST, 0);
          if(orxConfig_GetListCount(orxSOUND_KZ_CONFIG_GAIN_LIST) > 1)
          {
            afGainList[1] = orxConfig_GetListFloat(orxSOUND_KZ_CONFIG_GAIN_LIST, 1);
            if(afGainList[1] < afGainList[0])
            {
              orxFLOAT fTemp;
              fTemp         = afGainList[0];
              afGainList[0] = afGainList[1];
              afGainList[1] = fTemp;
            }
          }
          else
          {
            afGainList[1] = orxFLOAT_1;
          }
        }
        else
        {
          /* Clears gains */
          afGainList[0] = orxFLOAT_0;
          afGainList[1] = orxFLOAT_1;
        }

        /* Gets roll off */
        fRollOff = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_ROLL_OFF) != orxFALSE) ? orxConfig_GetFloat(orxSOUND_KZ_CONFIG_ROLL_OFF) : orxFLOAT_1;

        /* Updates spatialization */
        orxSound_SetSpatialization(_pstSound, afDistanceList[0], afDistanceList[1], afGainList[0], afGainList[1], fRollOff);
      }
      else
      {
        /* Deactivates spatialization */
        orxSound_SetSpatialization(_pstSound, -orxFLOAT_1, -orxFLOAT_1, orxFLOAT_0, orxFLOAT_1, orxFLOAT_1);
      }

      /* Updates panning */
      orxSound_SetPanning(_pstSound, orxConfig_GetFloat(orxSOUND_KZ_CONFIG_PANNING), orxConfig_GetBool(orxSOUND_KZ_CONFIG_MIX));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Done! */
  return eResult;
}

static orxSTATUS orxFASTCALL orxSound_ProcessFilterConfigData(orxSOUND_FILTER_DATA *_pstData)
{
  const orxSTRING zType;
  orxSTATUS       eResult = orxSTATUS_FAILURE;

  /* Clears data */
  orxMemory_Zero(_pstData, sizeof(orxSOUND_FILTER_DATA));
  _pstData->eType = orxSOUND_FILTER_TYPE_NONE;

  /* Gets its type */
  zType = orxConfig_GetString(orxSOUND_KZ_CONFIG_TYPE);

  /* Biquad? */
  if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_BIQUAD) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_BIQUAD;

    /* Updates filter data */
    _pstData->stBiquad.fA0 = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_A0);
    _pstData->stBiquad.fA1 = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_A1);
    _pstData->stBiquad.fA2 = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_A2);
    _pstData->stBiquad.fB0 = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_B0);
    _pstData->stBiquad.fB1 = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_B1);
    _pstData->stBiquad.fB2 = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_B2);
  }
  /* Low pass? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_LOW_PASS) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_LOW_PASS;

    /* Updates filter data */
    _pstData->stLowPass.fFrequency = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stLowPass.u32Order   = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_ORDER) != orxFALSE) ? orxConfig_GetU32(orxSOUND_KZ_CONFIG_ORDER) : 2;
  }
  /* High pass? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_HIGH_PASS) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_HIGH_PASS;

    /* Updates filter data */
    _pstData->stHighPass.fFrequency  = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stHighPass.u32Order    = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_ORDER) != orxFALSE) ? orxConfig_GetU32(orxSOUND_KZ_CONFIG_ORDER) : 2;
  }
  /* Band pass? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_BAND_PASS) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_BAND_PASS;

    /* Updates filter data */
    _pstData->stBandPass.fFrequency  = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stBandPass.u32Order    = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_ORDER) != orxFALSE) ? orxConfig_GetU32(orxSOUND_KZ_CONFIG_ORDER) : 2;
  }
  /* Low shelf? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_LOW_SHELF) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_LOW_SHELF;

    /* Updates filter data */
    _pstData->stLowShelf.fFrequency  = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stLowShelf.fQ          = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_Q) != orxFALSE) ? orxConfig_GetFloat(orxSOUND_KZ_CONFIG_Q) : orxFLOAT_1 / orxMATH_KF_SQRT_2;
    _pstData->stLowShelf.fGain       = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_GAIN);
  }
  /* High shelf? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_HIGH_SHELF) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_HIGH_SHELF;

    /* Updates filter data */
    _pstData->stHighShelf.fFrequency = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stHighShelf.fQ         = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_Q) != orxFALSE) ? orxConfig_GetFloat(orxSOUND_KZ_CONFIG_Q) : orxFLOAT_1 / orxMATH_KF_SQRT_2;
    _pstData->stHighShelf.fGain      = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_GAIN);
  }
  /* Notch? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_NOTCH) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_NOTCH;

    /* Updates filter data */
    _pstData->stNotch.fFrequency = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stNotch.fQ         = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_Q) != orxFALSE) ? orxConfig_GetFloat(orxSOUND_KZ_CONFIG_Q) : orxFLOAT_1 / orxMATH_KF_SQRT_2;
  }
  /* Peaking? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_PEAKING) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_PEAKING;

    /* Updates filter data */
    _pstData->stPeaking.fFrequency = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_FREQUENCY);
    _pstData->stPeaking.fQ         = (orxConfig_HasValue(orxSOUND_KZ_CONFIG_Q) != orxFALSE) ? orxConfig_GetFloat(orxSOUND_KZ_CONFIG_Q) : orxFLOAT_1 / orxMATH_KF_SQRT_2;
    _pstData->stPeaking.fGain      = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_GAIN);
  }
  /* Delay? */
  else if(orxString_ICompare(zType, orxSOUND_KZ_TYPE_DELAY) == 0)
  {
    /* Updates filter type */
    _pstData->eType = orxSOUND_FILTER_TYPE_DELAY;

    /* Updates filter data */
    _pstData->stDelay.fDelay = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_DELAY);
    _pstData->stDelay.fDecay = orxConfig_GetFloat(orxSOUND_KZ_CONFIG_DECAY);
  }

  /* Valid? */
  if(_pstData->eType != orxSOUND_FILTER_TYPE_NONE)
  {
    /* Stores its name ID */
    _pstData->stNameID = orxString_Hash(orxConfig_GetCurrentSection());

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxSound_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Resource event? */
  if(_pstEvent->eType == orxEVENT_TYPE_RESOURCE)
  {
    /* Add or update? */
    if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
    {
      orxRESOURCE_EVENT_PAYLOAD *pstPayload;

      /* Gets payload */
      pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Is config group? */
      if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
      {
        orxSOUND *pstSound;

        /* For all sounds */
        for(pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
            pstSound != orxNULL;
            pstSound = orxSOUND(orxStructure_GetNext(pstSound)))
        {
          /* Has reference? */
          if((pstSound->zReference != orxNULL) && (pstSound->zReference != orxSTRING_EMPTY))
          {
            /* Matches? */
            if(orxConfig_GetOriginID(pstSound->zReference) == pstPayload->stNameID)
            {
              orxSOUND_STATUS eStatus;
              orxFLOAT        fTime;

              /* Gets current status */
              eStatus = orxSound_GetStatus(pstSound);

              /* Gets current time */
              fTime = orxSound_GetTime(pstSound);

              /* Stops sound */
              orxSound_Stop(pstSound);

              /* Re-processes its config data */
              orxSound_ProcessConfigData(pstSound, orxFALSE);

              /* Has stream? */
              if(orxStructure_TestFlags(pstSound, orxSOUND_KU32_FLAG_HAS_STREAM))
              {
                /* Restores time */
                orxSound_SetTime(pstSound, fTime);
              }

              /* Depending on previous status */
              switch(eStatus)
              {
                case orxSOUND_STATUS_PLAY:
                {
                  /* Updates sound */
                  orxSound_Play(pstSound);

                  break;
                }

                case orxSOUND_STATUS_PAUSE:
                {
                  /* Updates sound */
                  orxSound_Play(pstSound);
                  orxSound_Pause(pstSound);

                  break;
                }

                case orxSOUND_STATUS_STOP:
                default:
                {
                  /* Updates sound */
                  orxSound_Stop(pstSound);

                  break;
                }
              }
            }
          }
        }
      }
      /* Is sound group? */
      else if(pstPayload->stGroupID == orxString_Hash(orxSOUND_KZ_RESOURCE_GROUP))
      {
        orxHANDLE         hIterator;
        orxSOUND_SAMPLE  *pstSample;

        /* Looks for matching sample */
        for(hIterator = orxHashTable_GetNext(sstSound.pstSampleTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstSample);
            (hIterator != orxHANDLE_UNDEFINED) && (pstSample->stID != pstPayload->stNameID);
            hIterator = orxHashTable_GetNext(sstSound.pstSampleTable, hIterator, orxNULL, (void **)&pstSample));

        /* Found? */
        if(hIterator != orxHANDLE_UNDEFINED)
        {
          orxSOUND *pstSound;
          orxBOOL   bLoaded;

          /* For all sounds */
          for(pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
              pstSound != orxNULL;
              pstSound = orxSOUND(orxStructure_GetNext(pstSound)))
          {
            /* Use concerned sample? */
            if(pstSound->pstSample == pstSample)
            {
              orxU32 u32BackupFlags;

              /* Depending on its status */
              switch(orxSound_GetStatus(pstSound))
              {
                case orxSOUND_STATUS_PLAY:
                {
                  /* Updates flags */
                  u32BackupFlags = orxSOUND_KU32_FLAG_BACKUP_PLAY;

                  break;
                }

                case orxSOUND_STATUS_PAUSE:
                {
                  /* Updates flags */
                  u32BackupFlags = orxSOUND_KU32_FLAG_BACKUP_PAUSE;

                  break;
                }

                default:
                case orxSOUND_STATUS_STOP:
                {
                  /* Updates flags */
                  u32BackupFlags = orxSOUND_KU32_FLAG_NONE;
                }
              }

              /* Stops it */
              orxSound_Stop(pstSound);

              /* Deletes its data */
              orxSoundSystem_Delete(pstSound->pstData);
              pstSound->pstData = orxNULL;

              /* Updates flags */
              orxStructure_SetFlags(pstSound, u32BackupFlags, orxSOUND_KU32_MASK_BACKUP_ALL);
            }
          }

          /* Updates sample */
          orxSoundSystem_DeleteSample(pstSample->pstData);
          pstSample->pstData = orxSoundSystem_LoadSample(orxString_GetFromID(pstSample->stID));

          /* Updates load status */
          bLoaded = (pstSample->pstData != orxNULL) ? orxTRUE : orxFALSE;

          /* For all sounds */
          for(pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
              pstSound != orxNULL;
              pstSound = orxSOUND(orxStructure_GetNext(pstSound)))
          {
            /* Use concerned sample? */
            if(pstSound->pstSample == pstSample)
            {
              /* Was sample loaded? */
              if(bLoaded != orxFALSE)
              {
                /* Recreates sound data based on sample */
                pstSound->pstData = orxSoundSystem_CreateFromSample(pstSound, pstSound->pstSample->pstData);
              }
              else
              {
                /* Clears data */
                pstSound->pstData = orxNULL;
              }

              /* Success? */
              if(pstSound->pstData != orxNULL)
              {
                /* Re-processes its config data */
                orxSound_ProcessConfigData(pstSound, orxTRUE);

                /* Depending on previous status */
                switch(orxStructure_GetFlags(pstSound, orxSOUND_KU32_MASK_BACKUP_ALL))
                {
                  case orxSOUND_KU32_FLAG_BACKUP_PLAY:
                  {
                    /* Plays sound */
                    orxSound_Play(pstSound);

                    break;
                  }

                  case orxSOUND_KU32_FLAG_BACKUP_PAUSE:
                  {
                    /* Pauses sound */
                    orxSound_Play(pstSound);
                    orxSound_Pause(pstSound);

                    break;
                  }

                  default:
                  {
                    break;
                  }
                }
              }
              else
              {
                /* Removes its reference */
                pstSound->pstSample = orxNULL;

                /* Updates its status */
                orxStructure_SetFlags(pstSound, orxSOUND_KU32_FLAG_NONE, orxSOUND_KU32_FLAG_HAS_SAMPLE);
              }

              /* Clears backup flags */
              orxStructure_SetFlags(pstSound, orxSOUND_KU32_FLAG_NONE, orxSOUND_KU32_MASK_BACKUP_ALL);
            }
          }

          /* Failed loading? */
          if(bLoaded == orxFALSE)
          {
            /* Resets its reference count */
            pstSample->u32Count = 0;

            /* Unloads it */
            orxSound_UnloadSample(pstSample);
          }
        }
        else
        {
          orxSOUND *pstSound;

          /* For all sounds */
          for(pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
              pstSound != orxNULL;
              pstSound = orxSOUND(orxStructure_GetNext(pstSound)))
          {
            /* Has stream? */
            if(orxStructure_TestFlags(pstSound, orxSOUND_KU32_FLAG_HAS_STREAM))
            {
              /* Matches? */
              if(pstPayload->stNameID == pstSound->stStreamID)
              {
                orxSOUND_STATUS eStatus;
                orxFLOAT        fTime;

                /* Gets current status */
                eStatus = orxSound_GetStatus(pstSound);

                /* Gets current time */
                fTime = orxSound_GetTime(pstSound);

                /* Stops sound */
                orxSound_Stop(pstSound);

                /* Re-processes its config data */
                orxSound_ProcessConfigData(pstSound, orxFALSE);

                /* Has stream? */
                if(orxStructure_TestFlags(pstSound, orxSOUND_KU32_FLAG_HAS_STREAM))
                {
                  /* Restores time */
                  orxSound_SetTime(pstSound, fTime);
                }

                /* Depending on previous status */
                switch(eStatus)
                {
                  case orxSOUND_STATUS_PLAY:
                  {
                    /* Updates sound */
                    orxSound_Play(pstSound);

                    break;
                  }

                  case orxSOUND_STATUS_PAUSE:
                  {
                    /* Updates sound */
                    orxSound_Play(pstSound);
                    orxSound_Pause(pstSound);

                    break;
                  }

                  case orxSOUND_STATUS_STOP:
                  default:
                  {
                    /* Updates sound */
                    orxSound_Stop(pstSound);

                    break;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  /* Locale event? */
  else if(_pstEvent->eType == orxEVENT_TYPE_LOCALE)
  {
    /* Select language event? */
    if(_pstEvent->eID == orxLOCALE_EVENT_SELECT_LANGUAGE)
    {
      orxLOCALE_EVENT_PAYLOAD *pstPayload;

      /* Gets its payload */
      pstPayload = (orxLOCALE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Sound group? */
      if((pstPayload->zGroup == orxNULL) || (orxString_Compare(pstPayload->zGroup, orxSOUND_KZ_LOCALE_GROUP) == 0))
      {
        orxSOUND *pstSound;

        /* For all sounds */
        for(pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
            pstSound != orxNULL;
            pstSound = orxSOUND(orxStructure_GetNext(pstSound)))
        {
          /* Has a reference? */
          if(pstSound->zReference != orxNULL)
          {
            const orxSTRING azProperties[] = {orxSOUND_KZ_CONFIG_SOUND, orxSOUND_KZ_CONFIG_MUSIC};
            orxBOOL         bUseLocale = orxFALSE;
            orxU32          i;

            /* Pushes its section */
            orxConfig_PushSection(pstSound->zReference);

            /* For all properties */
            for(i = 0; i < orxARRAY_GET_ITEM_COUNT(azProperties); i++)
            {
              const orxSTRING zName;

              /* Gets its value */
              zName = orxConfig_GetString(azProperties[i]);

              /* Uses locale? */
              if((*zName == orxSOUND_KC_LOCALE_MARKER) && (*(zName + 1) != orxSOUND_KC_LOCALE_MARKER))
              {
                /* Updates status */
                bUseLocale = orxTRUE;
                break;
              }
            }

            /* Uses locale? */
            if(bUseLocale != orxFALSE)
            {
              orxVECTOR       vPosition;
              orxSTRINGID     stBusID;
              orxFLOAT        fVolume, fPitch,
                              fMinDistance = -orxFLOAT_1, fMaxDistance = -orxFLOAT_1,
                              fMinGain = -orxFLOAT_1, fMaxGain = -orxFLOAT_1,
                              fRollOff = orxFLOAT_1,
                              fPanning = orxFLOAT_0;
              orxBOOL         bMix = orxFALSE;
              orxSOUND_STATUS eStatus;

              /* Gets current status */
              eStatus = orxSound_GetStatus(pstSound);

              /* Backups current state */
              fVolume       = orxSound_GetVolume(pstSound);
              fPitch        = orxSound_GetPitch(pstSound);
              stBusID       = orxSound_GetBusID(pstSound);
              orxSound_GetSpatialization(pstSound, &fMinDistance, &fMaxDistance, &fMinGain, &fMaxGain, &fRollOff);
              orxSound_GetPanning(pstSound, &fPanning, &bMix);
              orxSound_GetPosition(pstSound, &vPosition);

              /* Stops sound */
              orxSound_Stop(pstSound);

              /* Removes all filters */
              orxSound_RemoveAllFilters(pstSound);

              /* Re-processes its config data */
              orxSound_ProcessConfigData(pstSound, orxFALSE);

              /* Restores state */
              orxSound_SetVolume(pstSound, fVolume);
              orxSound_SetPitch(pstSound, fPitch);
              orxSound_SetBusID(pstSound, stBusID);
              orxSound_SetSpatialization(pstSound, fMinDistance, fMaxDistance, fMinGain, fMaxGain, fRollOff);
              orxSound_SetPanning(pstSound, fPanning, bMix);
              orxSound_SetPosition(pstSound, &vPosition);

              /* Depending on previous status */
              switch(eStatus)
              {
                case orxSOUND_STATUS_PLAY:
                {
                  /* Updates sound */
                  orxSound_Play(pstSound);

                  break;
                }

                case orxSOUND_STATUS_PAUSE:
                {
                  /* Updates sound */
                  orxSound_Play(pstSound);
                  orxSound_Pause(pstSound);

                  break;
                }

                case orxSOUND_STATUS_STOP:
                default:
                {
                  /* Updates sound */
                  orxSound_Stop(pstSound);

                  break;
                }
              }
            }

            /* Pops config section */
            orxConfig_PopSection();
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the sounds
 */
static orxINLINE void orxSound_DeleteAll()
{
  orxSOUND *pstSound;

  /* Gets first sound */
  pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));

  /* Non empty? */
  while(pstSound != orxNULL)
  {
    /* Deletes it */
    orxSound_Delete(pstSound);

    /* Gets first sound */
    pstSound = orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND));
  }

  /* Done! */
  return;
}

/** Deletes all the buses
 */
static orxINLINE void orxSound_DeleteAllBuses()
{
  orxSOUND_BUS *pstBus;

  /* For all buses */
  for(pstBus = (orxSOUND_BUS *)orxBank_GetNext(sstSound.pstBusBank, orxNULL);
      pstBus != orxNULL;
      pstBus = (orxSOUND_BUS *)orxBank_GetNext(sstSound.pstBusBank, pstBus))
  {
    /* Has data */
    if(pstBus->hData != orxHANDLE_UNDEFINED)
    {
      /* Deletes it */
      orxSoundSystem_DeleteBus(pstBus->hData);
    }
  }

  /* Done! */
  return;
}

/** Updates the SoundPointer (Callback for generic structure update calling)
 * @param[in]   _pstStructure                 Generic Structure or the concerned Body
 * @param[in]   _pstCaller                    Structure of the caller
 * @param[in]   _pstClockInfo                 Clock info used for time updates
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
static orxSTATUS orxFASTCALL orxSound_Update(orxSTRUCTURE *_pstStructure, const orxSTRUCTURE *_pstCaller, const orxCLOCK_INFO *_pstClockInfo)
{
  orxVECTOR       vPosition;
  orxSOUND       *pstSound;
  orxOBJECT      *pstObject;
  orxSOUND_STATUS eNewStatus;
  orxSTATUS       eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxSound_Update");

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstStructure);
  orxSTRUCTURE_ASSERT(_pstCaller);
  orxASSERT(orxOBJECT(_pstCaller) != orxNULL);

  /* Gets calling object */
  pstObject = orxOBJECT(_pstCaller);

  /* Gets sound */
  pstSound = orxSOUND(_pstStructure);

  /* Updates its position */
  orxSoundSystem_SetPosition(pstSound->pstData, orxObject_GetWorldPosition(pstObject, &vPosition));

  /* Has clock? */
  if(_pstClockInfo != orxNULL)
  {
    orxFLOAT fPitchModifier;

    /* Gets pitch modifier */
    fPitchModifier = (_pstClockInfo->afModifierList[orxCLOCK_MODIFIER_MULTIPLY] != orxFLOAT_0) ? _pstClockInfo->afModifierList[orxCLOCK_MODIFIER_MULTIPLY] : orxFLOAT_1;

    /* Should update? */
    if(fPitchModifier != pstSound->fPitchModifier)
    {
      /* Stores it */
      pstSound->fPitchModifier = fPitchModifier;

      /* Updates sound */
      orxSound_SetPitch(pstSound, pstSound->fPitch);
    }
  }

  /* Gets new status */
  eNewStatus = orxSound_GetStatus(pstSound);

  /* Changed? */
  if(eNewStatus != pstSound->eStatus)
  {
    orxSOUND_EVENT_PAYLOAD  stPayload;
    orxSOUND_STATUS         ePreviousStatus;

    /* Stores new status */
    ePreviousStatus   = pstSound->eStatus;
    pstSound->eStatus = eNewStatus;

    /* Depending on status */
    switch(eNewStatus)
    {
      case orxSOUND_STATUS_PLAY:
      {
        /* Wasn't paused? */
        if(ePreviousStatus != orxSOUND_STATUS_PAUSE)
        {
          /* Inits event payload */
          orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
          stPayload.pstSound = pstSound;

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_START, pstObject, pstObject, &stPayload);
        }

        break;
      }
      case orxSOUND_STATUS_STOP:
      {
        /* Inits event payload */
        orxMemory_Zero(&stPayload, sizeof(orxSOUND_EVENT_PAYLOAD));
        stPayload.pstSound = pstSound;

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SOUND, orxSOUND_EVENT_STOP, pstObject, pstObject, &stPayload);

        break;
      }
      default:
      {
        break;
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

static orxSOUND_BUS *orxFASTCALL orxSound_GetBus(orxSTRINGID _stBusID, orxBOOL _bCreate)
{
  orxSOUND_BUS *pstResult;

  /* Is cached bus? */
  if((sstSound.pstCachedBus != orxNULL)
  && (sstSound.pstCachedBus->stID == _stBusID))
  {
    /* Updates result */
    pstResult = sstSound.pstCachedBus;
  }
  else
  {
    orxSOUND_BUS **ppstBucket;

    /* Gets bus bucket */
    ppstBucket = (orxSOUND_BUS **)orxHashTable_Retrieve(sstSound.pstBusTable, _stBusID);

    /* Checks */
    orxASSERT(ppstBucket != orxNULL);

    /* Updates result */
    pstResult = *ppstBucket;

    /* Not found and should create? */
    if((*ppstBucket == orxNULL) && (_bCreate != orxFALSE))
    {
      orxTREE_NODE *pstRoot;

      /* Allocates it */
      pstResult = (orxSOUND_BUS *)orxBank_Allocate(sstSound.pstBusBank);

      /* Checks */
      orxASSERT(pstResult != orxNULL);

      /* Inits it */
      orxMemory_Zero(pstResult, sizeof(orxSOUND_BUS));
      pstResult->stID           = _stBusID;
      pstResult->fGlobalVolume  =
      pstResult->fGlobalPitch   =
      pstResult->fLocalVolume   =
      pstResult->fLocalPitch    = orxFLOAT_1;
      pstResult->hData          = orxSoundSystem_CreateBus(_stBusID);

      /* Gets root */
      pstRoot = orxTree_GetRoot(&(sstSound.stBusTree));

      /* Master? */
      if(pstRoot == orxNULL)
      {
        /* Adds it as root */
        orxTree_AddRoot(&(sstSound.stBusTree), &(pstResult->stNode));
      }
      else
      {
        /* Adds it to root */
        orxTree_AddChild(pstRoot, &(pstResult->stNode));

        /* Updates its parent */
        orxSoundSystem_SetBusParent(pstResult->hData, orxSTRUCT_GET_FROM_FIELD(orxSOUND_BUS, stNode, pstRoot)->hData);
      }

      /* Stores it */
      *ppstBucket = pstResult;
    }

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Updates cached bus */
      sstSound.pstCachedBus = pstResult;
    }
  }

  /* Done! */
  return pstResult;
}

static orxSTATUS orxFASTCALL orxSound_UpdateBus(orxSOUND_BUS *pstBus)
{
  orxTREE_NODE *pstNode, *pstParentNode;
  orxFLOAT      fNewVolume, fNewPitch;
  orxBOOL       bHasNewVolume, bHasNewPitch;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Gets bus & parent nodes */
  pstNode       = &(pstBus->stNode);
  pstParentNode = orxTree_GetParent(pstNode);

  /* Found? */
  if(pstParentNode != orxNULL)
  {
    orxSOUND_BUS *pstParentBus;

    /* Gets parent bus */
    pstParentBus = orxSTRUCT_GET_FROM_FIELD(orxSOUND_BUS, stNode, pstParentNode);

    /* Computes new volume & pitch */
    fNewVolume  = pstParentBus->fGlobalVolume * pstBus->fLocalVolume;
    fNewPitch   = pstParentBus->fGlobalPitch * pstBus->fLocalPitch;
  }
  else
  {
    /* Computes new volume & pitch */
    fNewVolume  = pstBus->fLocalVolume;
    fNewPitch   = pstBus->fLocalPitch;
  }

  /* Updates status */
  bHasNewVolume = (fNewVolume != pstBus->fGlobalVolume) ? orxTRUE : orxFALSE;
  bHasNewPitch  = (fNewPitch != pstBus->fGlobalPitch) ? orxTRUE : orxFALSE;

  /* Needs update? */
  if((bHasNewVolume != orxFALSE)
  || (bHasNewPitch != orxFALSE))
  {
    orxTREE_NODE     *pstChildNode;
    orxLINKLIST_NODE *pstSoundNode;

    /* Stores new values */
    pstBus->fGlobalVolume = fNewVolume;
    pstBus->fGlobalPitch  = fNewPitch;

    /* For all its sound */
    for(pstSoundNode = orxLinkList_GetFirst(&(pstBus->stList));
        pstSoundNode != orxNULL;
        pstSoundNode = orxLinkList_GetNext(pstSoundNode))
    {
      orxSOUND *pstSound;

      /* Gets it */
      pstSound = orxSTRUCT_GET_FROM_FIELD(orxSOUND, stBusNode, pstSoundNode);

      /* Has data? */
      if(pstSound->pstData != orxNULL)
      {
        /* New volume? */
        if(bHasNewVolume != orxFALSE)
        {
          /* Updates its volume */
          orxSoundSystem_SetVolume(pstSound->pstData, fNewVolume * pstSound->fVolume);
        }

        /* New pitch? */
        if(bHasNewPitch != orxFALSE)
        {
          /* Updates its pitch */
          orxSoundSystem_SetPitch(pstSound->pstData, fNewPitch * pstSound->fPitchModifier * pstSound->fPitch);
        }
      }
    }

    /* Updates all children */
    for(pstChildNode = orxTree_GetChild(pstNode);
        pstChildNode != orxNULL;
        pstChildNode = orxTree_GetSibling(pstChildNode))
    {
      /* Updates it */
      orxSound_UpdateBus(orxSTRUCT_GET_FROM_FIELD(orxSOUND_BUS, stNode, pstChildNode));
    }
  }

  /* Done! */
  return eResult;
}

/** Command: SetBusParent
 */
void orxFASTCALL orxSound_CommandSetBusParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID, stParentID;

  /* Gets bus ID */
  stBusID = orxString_GetID(_astArgList[0].zValue);

  /* Gets parent ID */
  stParentID = ((_u32ArgNumber > 1) && (*_astArgList[1].zValue != orxCHAR_NULL)) ? orxString_GetID(_astArgList[1].zValue) : orxSound_GetMasterBusID();

  /* Sets bus parent */
  orxSound_SetBusParent(stBusID, stParentID);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: GetBusParent
 */
void orxFASTCALL orxSound_CommandGetBusParent(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID, stParentID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Gets parent ID */
  stParentID = orxSound_GetBusParent(stBusID);

  /* Updates result */
  _pstResult->zValue = (stParentID != orxSTRINGID_UNDEFINED) ? orxString_GetFromID(stParentID) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: GetBusChild
 */
void orxFASTCALL orxSound_CommandGetBusChild(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID, stChildID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Gets child ID */
  stChildID = orxSound_GetBusChild(stBusID);

  /* Updates result */
  _pstResult->zValue = (stChildID != orxSTRINGID_UNDEFINED) ? orxString_GetFromID(stChildID) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: GetBusSibling
 */
void orxFASTCALL orxSound_CommandGetBusSibling(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID, stSiblingID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Gets sibling ID */
  stSiblingID = orxSound_GetBusSibling(stBusID);

  /* Updates result */
  _pstResult->zValue = (stSiblingID != orxSTRINGID_UNDEFINED) ? orxString_GetFromID(stSiblingID) : orxSTRING_EMPTY;

  /* Done! */
  return;
}

/** Command: SetBusVolume
 */
void orxFASTCALL orxSound_CommandSetBusVolume(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_GetID(_astArgList[0].zValue);

  /* Sets its volume */
  orxSound_SetBusVolume(stBusID, _astArgList[1].fValue);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: SetBusPitch
 */
void orxFASTCALL orxSound_CommandSetBusPitch(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_GetID(_astArgList[0].zValue);

  /* Sets its pitch */
  orxSound_SetBusPitch(stBusID, _astArgList[1].fValue);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: GetBusVolume
 */
void orxFASTCALL orxSound_CommandGetBusVolume(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->fValue = orxSound_GetBusVolume(stBusID);

  /* Done! */
  return;
}

/** Command: GetBusPitch
 */
void orxFASTCALL orxSound_CommandGetBusPitch(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Updates result */
  _pstResult->fValue = orxSound_GetBusPitch(stBusID);

  /* Done! */
  return;
}

/** Command: AddBusFilter
 */
void orxFASTCALL orxSound_CommandAddBusFilter(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Adds filter */
  orxSound_AddBusFilterFromConfig(stBusID, _astArgList[1].zValue);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: RemoveLastBusFilter
 */
void orxFASTCALL orxSound_CommandRemoveLastBusFilter(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Removes its last filter */
  orxSound_RemoveLastBusFilter(stBusID);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Command: RemoveAllBusFilters
 */
void orxFASTCALL orxSound_CommandRemoveAllBusFilters(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  orxSTRINGID stBusID;

  /* Gets bus ID */
  stBusID = orxString_Hash(_astArgList[0].zValue);

  /* Removes all its filter */
  orxSound_RemoveAllBusFilters(stBusID);

  /* Updates result */
  _pstResult->zValue = _astArgList[0].zValue;

  /* Done! */
  return;
}

/** Registers all the sound commands
 */
static orxINLINE void orxSound_RegisterCommands()
{
  /* Command: SetBusParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, SetBusParent, "Bus", orxCOMMAND_VAR_TYPE_STRING, 1, 1, {"Bus", orxCOMMAND_VAR_TYPE_STRING}, {"Parent = <master>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetBusParent */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, GetBusParent, "Bus", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetBusChild */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, GetBusChild, "Bus", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetBusSibling */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, GetBusSibling, "Bus", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: SetBusVolume */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, SetBusVolume, "Bus", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING}, {"Volume", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: SetBusPitch */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, SetBusPitch, "Bus", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING}, {"Pitch", orxCOMMAND_VAR_TYPE_FLOAT});
  /* Command: GetBusVolume */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, GetBusVolume, "Volume", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: GetBusPitch */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, GetBusPitch, "Pitch", orxCOMMAND_VAR_TYPE_FLOAT, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: AddBusFilter */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, AddBusFilter, "Bus", orxCOMMAND_VAR_TYPE_STRING, 2, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING}, {"Filter", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: RemoveLastBusFilter */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, RemoveLastBusFilter, "Bus", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: RemoveAllBusFilters */
  orxCOMMAND_REGISTER_CORE_COMMAND(Sound, RemoveAllBusFilters, "Bus", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Bus", orxCOMMAND_VAR_TYPE_STRING});
}

/** Unregisters all the sound commands
 */
static orxINLINE void orxSound_UnregisterCommands()
{
  /* Command: SetBusParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, SetBusParent);
  /* Command: GetBusParent */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, GetBusParent);
  /* Command: GetBusChild */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, GetBusChild);
  /* Command: GetBusSibling */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, GetBusSibling);

  /* Command: SetBusVolume */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, SetBusVolume);
  /* Command: SetBusPitch */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, SetBusPitch);
  /* Command: GetBusVolume */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, GetBusVolume);
  /* Command: GetBusPitch */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, GetBusPitch);

  /* Command: AddBusFilter */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, AddBusFilter);
  /* Command: RemoveLastBusFilter */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, RemoveLastBusFilter);
  /* Command: RemoveAllBusFilters */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Sound, RemoveAllBusFilters);
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Sound module setup
 */
void orxFASTCALL orxSound_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_SOUNDSYSTEM);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_RESOURCE);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_SOUND, orxMODULE_ID_COMMAND);
  orxModule_AddOptionalDependency(orxMODULE_ID_SOUND, orxMODULE_ID_LOCALE);

  /* Done! */
  return;
}

/** Inits the sound module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!orxFLAG_TEST(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstSound, sizeof(orxSOUND_STATIC));

    /* Creates sample table */
    sstSound.pstSampleTable = orxHashTable_Create(orxSOUND_KU32_SAMPLE_BANK_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstSound.pstSampleTable != orxNULL)
    {
      /* Creates sample bank */
      sstSound.pstSampleBank = orxBank_Create(orxSOUND_KU32_SAMPLE_BANK_SIZE, sizeof(orxSOUND_SAMPLE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Valid? */
      if(sstSound.pstSampleBank != orxNULL)
      {
        /* Creates bus bank */
        sstSound.pstBusBank = orxBank_Create(orxSOUND_KU32_BUS_BANK_SIZE, sizeof(orxSOUND_BUS), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

        /* Success? */
        if(sstSound.pstBusBank != orxNULL)
        {
          /* Creates bus table */
          sstSound.pstBusTable = orxHashTable_Create(orxSOUND_KU32_BUS_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

          /* Success? */
          if(sstSound.pstBusTable != orxNULL)
          {
            /* Registers structure type */
            eResult = orxSTRUCTURE_REGISTER(SOUND, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSOUND_KU32_BANK_SIZE, &orxSound_Update);

            /* Success? */
            if(eResult != orxSTATUS_FAILURE)
            {
              /* Stores master bus ID */
              sstSound.stMasterBusID = orxString_GetID(orxSOUND_KZ_MASTER_BUS);

              /* Creates master bus */
              orxSound_GetBus(sstSound.stMasterBusID, orxTRUE);

              /* Adds event handlers */
              orxEvent_AddHandler(orxEVENT_TYPE_LOCALE, orxSound_EventHandler);
              orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxSound_EventHandler);
              orxEvent_SetHandlerIDFlags(orxSound_EventHandler, orxEVENT_TYPE_LOCALE, orxNULL, orxEVENT_GET_FLAG(orxLOCALE_EVENT_SELECT_LANGUAGE), orxEVENT_KU32_MASK_ID_ALL);
              orxEvent_SetHandlerIDFlags(orxSound_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);

              /* Inits Flags */
              orxFLAG_SET(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_READY, orxSOUND_KU32_STATIC_FLAG_NONE);

              /* Registers commands */
              orxSound_RegisterCommands();
            }
            else
            {
              /* Deletes sample table */
              orxHashTable_Delete(sstSound.pstSampleTable);

              /* Deletes sample bank */
              orxBank_Delete(sstSound.pstSampleBank);

              /* Deletes bus bank */
              orxBank_Delete(sstSound.pstBusBank);

              /* Deletes bus table */
              orxHashTable_Delete(sstSound.pstBusTable);

              /* Logs message */
              orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to register sound structure.");
            }
          }
          else
          {
            /* Deletes sample table */
            orxHashTable_Delete(sstSound.pstSampleTable);

            /* Deletes sample bank */
            orxBank_Delete(sstSound.pstSampleBank);

            /* Deletes bus bank */
            orxBank_Delete(sstSound.pstBusBank);

            /* Logs message */
            orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to create bus table.");
          }
        }
        else
        {
          /* Deletes sample table */
          orxHashTable_Delete(sstSound.pstSampleTable);

          /* Deletes sample bank */
          orxBank_Delete(sstSound.pstSampleBank);

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to create bus bank.");
        }
      }
      else
      {
        /* Deletes sample table */
        orxHashTable_Delete(sstSound.pstSampleTable);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to create sample bank.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to create reference table.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Tried to initialize sound module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the sound module
 */
void orxFASTCALL orxSound_Exit()
{
  /* Initialized? */
  if(orxFLAG_TEST(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_READY))
  {
    /* Unregisters commands */
    orxSound_UnregisterCommands();

    /* Removes event handlers */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxSound_EventHandler);
    orxEvent_RemoveHandler(orxEVENT_TYPE_LOCALE, orxSound_EventHandler);

    /* Deletes all sounds */
    orxSound_DeleteAll();

    /* Deletes all sound samples */
    orxSound_UnloadAllSample();

    /* Deletes sample table */
    orxHashTable_Delete(sstSound.pstSampleTable);

    /* Deletes sample bank */
    orxBank_Delete(sstSound.pstSampleBank);

    /* Deletes all buses */
    orxSound_DeleteAllBuses();

    /* Deletes bus table */
    orxHashTable_Delete(sstSound.pstBusTable);

    /* Deletes bus bank */
    orxBank_Delete(sstSound.pstBusBank);

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SOUND);

    /* Updates flags */
    orxFLAG_SET(sstSound.u32Flags, orxSOUND_KU32_STATIC_FLAG_NONE, orxSOUND_KU32_STATIC_MASK_ALL);
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Tried to exit from sound module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Creates an empty sound
 * @return      Created orxSOUND / orxNULL
 */
orxSOUND *orxFASTCALL orxSound_Create()
{
  orxSOUND *pstResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* Creates sound */
  pstResult = orxSOUND(orxStructure_Create(orxSTRUCTURE_ID_SOUND));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Increases count */
    orxStructure_IncreaseCount(pstResult);

    /* Clears its status */
    pstResult->eStatus = orxSOUND_STATUS_NONE;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Failed to create structure for sound.");
  }

  /* Done! */
  return pstResult;
}

/** Creates a sound with an empty stream (ie. you'll need to provide actual sound data for each packet sent to the sound card using the event system)
 * @param[in] _u32ChannelNumber Number of channels of the stream
 * @param[in] _u32SampleRate    Sampling rate of the stream (ie. number of frames per second)
 * @param[in] _zName            Name to associate with this sound
 * @return orxSOUND / orxNULL
 */
orxSOUND *orxFASTCALL orxSound_CreateWithEmptyStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zName)
{
  orxSOUND *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Valid name? */
  if(_zName != orxSTRING_EMPTY)
  {
    /* Creates sound */
    pstResult = orxSound_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Creates empty stream */
      pstResult->pstData = orxSoundSystem_CreateStream(pstResult, _u32ChannelNumber, _u32SampleRate);

      /* Valid? */
      if(pstResult->pstData != orxNULL)
      {
        /* Sets master bus ID */
        orxSound_SetBusID(pstResult, sstSound.stMasterBusID);

        /* Stores its reference */
        pstResult->zReference = orxString_Store(_zName);

        /* Updates its status */
        orxStructure_SetFlags(pstResult, orxSOUND_KU32_FLAG_HAS_STREAM, orxSOUND_KU32_FLAG_NONE);
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound <%s>: invalid stream parameters [%u channels, %u sample rate].", _zName, _u32ChannelNumber, _u32SampleRate);

        /* Deletes it */
        orxSound_Delete(pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Creates a sound from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSOUND / orxNULL
 */
orxSOUND *orxFASTCALL orxSound_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxSOUND *pstResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Pushes section */
  if((orxConfig_HasSection(_zConfigID) != orxFALSE)
  && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
  {
    /* Creates sound */
    pstResult = orxSound_Create();

    /* Valid? */
    if(pstResult != orxNULL)
    {
      /* Stores its reference */
      pstResult->zReference = orxConfig_GetCurrentSection();

      /* Processes its config data */
      if(orxSound_ProcessConfigData(pstResult, orxFALSE) == orxSTATUS_FAILURE)
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound <%s>: invalid content.", _zConfigID);

        /* Deletes it */
        orxSound_Delete(pstResult);

        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sound <%s>: can't allocate memory.", _zConfigID);
    }

    /* Pops previous section */
    orxConfig_PopSection();
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Couldn't find sound section (%s) in config.", _zConfigID);

    /* Updates result */
    pstResult = orxNULL;
  }

  /* Done! */
  return pstResult;
}
/** Deletes a sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_Delete(orxSOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstSound);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstSound) == 0)
  {
    /* Stops it */
    orxSound_Stop(_pstSound);

    /* Removes all its filters */
    orxSound_RemoveAllFilters(_pstSound);

    /* Has data? */
    if(_pstSound->pstData != orxNULL)
    {
      /* Deletes it */
      orxSoundSystem_Delete(_pstSound->pstData);
    }

    /* Has a referenced sample? */
    if(orxStructure_TestFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_SAMPLE))
    {
      /* Unloads it */
      orxSound_UnloadSample(_pstSound->pstSample);
    }

    /* Removes sound from its current bus */
    if(orxLinkList_GetList(&(_pstSound->stBusNode)) != orxNULL)
    {
      orxLinkList_Remove(&(_pstSound->stBusNode));
    }

    /* Deletes structure */
    orxStructure_Delete(_pstSound);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Clears cache (if any sound sample is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_ClearCache()
{
  orxSOUND_SAMPLE *pstSample, *pstNextSample;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* For all samples */
  for(pstSample = (orxSOUND_SAMPLE*)orxBank_GetNext(sstSound.pstSampleBank, orxNULL);
      pstSample != orxNULL;
      pstSample = pstNextSample)
  {
    /* Gets next sample */
    pstNextSample = (orxSOUND_SAMPLE*)orxBank_GetNext(sstSound.pstSampleBank, pstSample);

    /* Is cached? */
    if(orxFLAG_TEST(pstSample->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_CACHED))
    {
      /* Updates its flags */
      orxFLAG_SET(pstSample->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_NONE, orxSOUND_SAMPLE_KU32_FLAG_CACHED);

      /* Unloads its extra reference */
      orxSound_UnloadSample(pstSample);
    }
  }

  /* Done! */
  return eResult;
}

/** Creates a sample
 * @param[in] _u32ChannelNumber Number of channels of the sample
 * @param[in] _u32FrameNumber   Number of frame of the sample (number of "samples" = number of frames * number of channels)
 * @param[in] _u32SampleRate    Sampling rate of the sample (ie. number of frames per second)
 * @param[in] _zName            Name to associate with the sample
 * @return orxSOUND_SAMPLE / orxNULL
 */
orxSOUND_SAMPLE *orxFASTCALL orxSound_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate, const orxSTRING _zName)
{
  orxSOUND_SAMPLE *pstResult = orxNULL;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* Valid name? */
  if(_zName != orxSTRING_EMPTY)
  {
    orxSTRINGID stID;

    /* Gets its ID */
    stID = orxString_Hash(_zName);

    /* Not already present? */
    if(orxHashTable_Get(sstSound.pstSampleTable, stID) == orxNULL)
    {
      orxSOUNDSYSTEM_SAMPLE *pstSample;

      /* Creates sample */
      pstSample = orxSoundSystem_CreateSample(_u32ChannelNumber, _u32FrameNumber, _u32SampleRate);

      /* Success? */
      if(pstSample != orxNULL)
      {
        orxSOUND_SAMPLE *pstSoundSample;

        /* Creates sound sample */
        pstSoundSample = (orxSOUND_SAMPLE *)orxBank_Allocate(sstSound.pstSampleBank);

        /* Success? */
        if(pstSoundSample != orxNULL)
        {
          /* Inits it */
          pstSoundSample->pstData     = pstSample;
          pstSoundSample->u32Count    = 0;
          pstSoundSample->stID        = stID;
          orxFLAG_SET(pstSoundSample->u32Flags, orxSOUND_SAMPLE_KU32_FLAG_NONE, orxSOUND_SAMPLE_KU32_MASK_ALL);

          /* Stores it */
          orxHashTable_Add(sstSound.pstSampleTable, stID, pstSoundSample);

          /* Updates result */
          pstResult = (orxSOUND_SAMPLE *)pstSoundSample;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sample [%s]: couldn't allocate internal structure.", _zName);

          /* Deletes sample */
          orxSoundSystem_DeleteSample(pstSample);
        }
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't create sample [%s]: a sample with the same name already exists.", _zName);
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes a sample
 * @param[in] _pstSample        Concerned sample
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_DeleteSample(orxSOUND_SAMPLE *_pstSample)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);

  /* Not referenced anymore? */
  if(_pstSample->u32Count == 0)
  {
    /* Deletes its data */
    orxSoundSystem_DeleteSample(_pstSample->pstData);

    /* Removes it from sample table */
    orxHashTable_Remove(sstSound.pstSampleTable, _pstSample->stID);

    /* Deletes it */
    orxBank_Free(sstSound.pstSampleBank, _pstSample);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't delete sample [%s]: it is still referenced by <%u> sound(s).", orxString_GetFromID(_pstSample->stID), _pstSample->u32Count);
  }

  /* Done! */
  return eResult;
}

/** Gets sample info
 * @param[in]   _pstSample                    Concerned sample
 * @param[in]   _pu32ChannelNumber            Number of channels of the sample
 * @param[in]   _pu32FrameNumber              Number of frame of the sample (number of "samples" = number of frames * number of channels)
 * @param[in]   _pu32SampleRate               Sampling rate of the sample (ie. number of frames per second)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_GetSampleInfo(const orxSOUND_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_pu32ChannelNumber != orxNULL);
  orxASSERT(_pu32FrameNumber != orxNULL);
  orxASSERT(_pu32SampleRate != orxNULL);

  /* Gets sample's info */
  eResult = orxSoundSystem_GetSampleInfo(_pstSample->pstData, _pu32ChannelNumber, _pu32FrameNumber, _pu32SampleRate);

  /* Done! */
  return eResult;
}

/** Sets sample data
 * @param[in]   _pstSample                    Concerned sample
 * @param[in]   _afData                       Data to set (samples are expected to be signed/normalized)
 * @param[in]   _u32SampleNumber              Number of samples in the data array
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetSampleData(orxSOUND_SAMPLE *_pstSample, const orxFLOAT *_afData, orxU32 _u32SampleNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstSample != orxNULL);
  orxASSERT(_afData != orxNULL);

  /* Sets sample's data */
  eResult = orxSoundSystem_SetSampleData(_pstSample->pstData, _afData, _u32SampleNumber);

  /* Done! */
  return eResult;
}

/** Links a sample
 * @param[in]   _pstSound     Concerned sound
 * @param[in]   _zSampleName  Name of the sample to link (must already be loaded/created)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_LinkSample(orxSOUND *_pstSound, const orxSTRING _zSampleName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_zSampleName != orxNULL);

  /* Unlink previous sample if needed */
  orxSound_UnlinkSample(_pstSound);

  /* Has no sample now? */
  if(orxStructure_TestFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_SAMPLE | orxSOUND_KU32_FLAG_HAS_STREAM) == orxFALSE)
  {
    orxSOUND_SAMPLE *pstSoundSample;

    /* Loads corresponding sample */
    pstSoundSample = orxSound_LoadSample(_zSampleName, orxFALSE);

    /* Found? */
    if(pstSoundSample != orxNULL)
    {
      /* Stores it */
      _pstSound->pstSample = pstSoundSample;

      /* Creates sound data based on it */
      _pstSound->pstData = orxSoundSystem_CreateFromSample(_pstSound, pstSoundSample->pstData);

      /* Valid? */
      if(_pstSound->pstData != orxNULL)
      {
        /* Stores its reference */
        _pstSound->zReference = orxString_Store(_zSampleName);

        /* Doesn't have a bus yet? */
        if(_pstSound->stBusID == 0)
        {
          /* Sets master bus ID */
          orxSound_SetBusID(_pstSound, sstSound.stMasterBusID);
        }

        /* Updates its status */
        orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_SAMPLE, orxSOUND_KU32_FLAG_NONE);

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Unloads sound sample */
        orxSound_UnloadSample(pstSoundSample);

        /* Removes its reference */
        _pstSound->pstSample = orxNULL;
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't link sample [%s] to sound: sample not found.", _zSampleName);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Can't link sample [%s]: sound is already linked to another sample or a stream.", _zSampleName);
  }

  /* Done! */
  return eResult;
}

/** Unlinks (and deletes if not used anymore) a sample
 * @param[in]   _pstSound     Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_UnlinkSample(orxSOUND *_pstSound)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sample? */
  if(orxStructure_TestFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_SAMPLE) != orxFALSE)
  {
    /* Stops it */
    orxSound_Stop(_pstSound);

    /* Removes reference */
    _pstSound->zReference = orxNULL;

    /* Has data? */
    if(_pstSound->pstData != orxNULL)
    {
      /* Deletes it */
      orxSoundSystem_Delete(_pstSound->pstData);
      _pstSound->pstData = orxNULL;
    }

    /* Unloads sound sample */
    orxSound_UnloadSample(_pstSound->pstSample);
    _pstSound->pstSample = orxNULL;

    /* Updates its status */
    orxStructure_SetFlags(_pstSound, orxSOUND_KU32_FLAG_NONE, orxSOUND_KU32_FLAG_HAS_SAMPLE);
  }
  else
  {
    /* No sample found */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Is a stream (ie. music)?
 * @param[in] _pstSound       Concerned Sound
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxSound_IsStream(orxSOUND *_pstSound)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Updates result */
  bResult = orxStructure_TestFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_STREAM);

  /* Done! */
  return bResult;
}

/** Plays sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_Play(orxSOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Plays it */
    eResult = orxSoundSystem_Play(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Pauses sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_Pause(orxSOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Pauses it */
    eResult = orxSoundSystem_Pause(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Stops sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_Stop(orxSOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Stops it */
    eResult = orxSoundSystem_Stop(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a filter to a sound (cascading)
 * @param[in]   _pstSound         Concerned sound
 * @param[in]   _pstFilterData    Concerned filter data
 * @param[in]   _bUseCustomParam  Filter uses custom parameters
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_AddFilter(orxSOUND *_pstSound, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_pstFilterData != orxNULL);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Adds filter to it */
    eResult = orxSoundSystem_AddFilter(_pstSound->pstData, _pstFilterData, _bUseCustomParam);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes last added filter from a sound
 * @param[in]   _pstSound     Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_RemoveLastFilter(orxSOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Removes last filter from it */
    eResult = orxSoundSystem_RemoveLastFilter(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes all filters from a sound
 * @param[in]   _pstSound     Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_RemoveAllFilters(orxSOUND *_pstSound)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Removes all filters from it */
    eResult = orxSoundSystem_RemoveAllFilters(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a filter to a sound (cascading) from config
 * @param[in]   _pstSound         Concerned sound
 * @param[in]   _zFilterConfigID  Config ID of the filter to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_AddFilterFromConfig(orxSOUND *_pstSound, const orxSTRING _zFilterConfigID)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT((_zFilterConfigID != orxNULL) && (_zFilterConfigID != orxSTRING_EMPTY));

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zFilterConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zFilterConfigID) != orxSTATUS_FAILURE))
    {
      orxSOUND_FILTER_DATA stData;

      /* Processes filter config data */
      if(orxSound_ProcessFilterConfigData(&stData) != orxSTATUS_FAILURE)
      {
        /* Adds filter */
        eResult = orxSoundSystem_AddFilter(_pstSound->pstData, &stData, orxConfig_GetBool(orxSOUND_KZ_CONFIG_USE_CUSTOM_PARAM));
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Couldn't find filter section (%s) in config.", _zFilterConfigID);
    }
  }

  /* Done! */
  return eResult;
}

/** Starts recording
 * @param[in] _zName             Name for the recorded sound/file
 * @param[in] _bWriteToFile      Should write to file?
 * @param[in] _u32SampleRate     Sample rate, 0 for default rate (48000Hz)
 * @param[in] _u32ChannelNumber  Channel number, 0 for default mono channel
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_StartRecording(const orxCHAR *_zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* Starts recording */
  eResult = orxSoundSystem_StartRecording(_zName, _bWriteToFile, _u32SampleRate, _u32ChannelNumber);

  /* Done! */
  return eResult;
}

/** Stops recording
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_StopRecording()
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* Stops recording */
  eResult = orxSoundSystem_StopRecording();

  /* Done! */
  return eResult;
}

/** Is recording possible on the current system?
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxSound_HasRecordingSupport()
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* Updates result */
  bResult = orxSoundSystem_HasRecordingSupport();

  /* Done! */
  return bResult;
}

/** Sets sound volume
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fVolume        Desired volume (0.0 - 1.0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetVolume(orxSOUND *_pstSound, orxFLOAT _fVolume)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    orxSOUND_BUS *pstBus;

    /* Gets bus */
    pstBus = orxSound_GetBus(_pstSound->stBusID, orxFALSE);

    /* Checks */
    orxASSERT(pstBus != orxNULL);

    /* Sets its internal volume */
    eResult = orxSoundSystem_SetVolume(_pstSound->pstData, pstBus->fGlobalVolume * _fVolume);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Stores it */
      _pstSound->fVolume = _fVolume;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fPitch         Desired pitch (< 1.0 => lower pitch, = 1.0 => original pitch, > 1.0 => higher pitch). 0.0 is ignored.
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetPitch(orxSOUND *_pstSound, orxFLOAT _fPitch)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    orxSOUND_BUS *pstBus;

    /* Gets bus */
    pstBus = orxSound_GetBus(_pstSound->stBusID, orxFALSE);

    /* Checks */
    orxASSERT(pstBus != orxNULL);

    /* Sets its pitch */
    eResult = orxSoundSystem_SetPitch(_pstSound->pstData, pstBus->fGlobalPitch * _pstSound->fPitchModifier * _fPitch);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Stores it */
      _pstSound->fPitch = _fPitch;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a sound time (ie. cursor/play position from beginning)
 * @param[in]   _pstSound                             Concerned sound
 * @param[in]   _fTime                                Time, in seconds
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetTime(orxSOUND *_pstSound, orxFLOAT _fTime)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Valid? */
    if((_fTime >= orxFLOAT_0)
    && ((orxStructure_TestFlags(_pstSound, orxSOUND_KU32_FLAG_HAS_STREAM))
     || (_fTime < orxSound_GetDuration(_pstSound))))
    {
      /* Sets its time */
      eResult = orxSoundSystem_SetTime(_pstSound->pstData, _fTime);
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Sound " orxANSI_KZ_COLOR_FG_GREEN "[%s]" orxANSI_KZ_COLOR_FG_DEFAULT ": " orxANSI_KZ_COLOR_FG_RED "Can't set time" orxANSI_KZ_COLOR_FG_DEFAULT " to <%g>: out of bound value, valid range is " orxANSI_KZ_COLOR_FG_YELLOW "[0, %g[" orxANSI_KZ_COLOR_FG_DEFAULT ", ignoring!", orxSound_GetName(_pstSound), _fTime, orxSound_GetDuration(_pstSound));
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets sound position
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _pvPosition     Desired position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetPosition(orxSOUND *_pstSound, const orxVECTOR *_pvPosition)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_pvPosition != orxNULL);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Sets its position */
    eResult = orxSoundSystem_SetPosition(_pstSound->pstData, _pvPosition);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a sound spatialization, with gain decreasing between the minimum and maximum distances, when enabled
 * @param[in] _pstSound                               Concerned Sound
 * @param[in] _fMinDistance                           Min distance, inside which the max gain will be used, strictly negative value to disable spatialization entirely
 * @param[in] _fMaxDistance                           Max distance, outside which the gain will stop decreasing, strictly negative value to disable spatialization entirely
 * @param[in] _fMinGain                               Min gain in [0.0f - 1.0f]
 * @param[in] _fMaxGain                               Max gain in [0.0f - 1.0f]
 * @param[in] _fRollOff                               RollOff factor applied when interpolating the gain between inner and outer distances, defaults to 1.0f
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetSpatialization(orxSOUND *_pstSound, orxFLOAT _fMinDistance, orxFLOAT _fMaxDistance, orxFLOAT _fMinGain, orxFLOAT _fMaxGain, orxFLOAT _fRollOff)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_fMaxDistance >= _fMinDistance);
  orxASSERT((_fMinGain >= orxFLOAT_0) && (_fMinGain <= orxFLOAT_1));
  orxASSERT((_fMaxGain >= orxFLOAT_0) && (_fMaxGain <= orxFLOAT_1));
  orxASSERT(_fRollOff >= orxFLOAT_0);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Sets its position */
    eResult = orxSoundSystem_SetSpatialization(_pstSound->pstData, _fMinDistance, _fMaxDistance, _fMinGain, _fMaxGain, _fRollOff);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Sets a sound panning
 * @param[in] _pstSound panning
 * @param[in] _fPanning                               Sound panning, -1.0f for full left, 0.0f for center, 1.0f for full right
 * @param[in] _bMix                                   Left/Right channels will be mixed if orxTRUE or act like a balance otherwise
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetPanning(orxSOUND *_pstSound, orxFLOAT _fPanning, orxBOOL _bMix)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Sets its position */
    eResult = orxSoundSystem_SetPanning(_pstSound->pstData, _fPanning, _bMix);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Loops sound
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _bLoop          orxTRUE / orxFALSE
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_Loop(orxSOUND *_pstSound, orxBOOL _bLoop)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Sets its looping status */
    eResult = orxSoundSystem_Loop(_pstSound->pstData, _bLoop);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets sound volume
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetVolume(const orxSOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    fResult = _pstSound->fVolume;
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetPitch(const orxSOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    fResult = _pstSound->fPitch;
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets a sound's time (ie. cursor/play position from beginning)
 * @param[in]   _pstSound                             Concerned sound
 * @return Sound's time (cursor/play position), in seconds
 */
orxFLOAT orxFASTCALL orxSound_GetTime(const orxSOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    fResult = orxSoundSystem_GetTime(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets sound position
 * @param[in]   _pstSound     Concerned Sound
 * @param[out]  _pvPosition   Sound's position
 * @return orxVECTOR / orxNULL
 */
orxVECTOR *orxFASTCALL orxSound_GetPosition(const orxSOUND *_pstSound, orxVECTOR *_pvPosition)
{
  orxVECTOR *pvResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_pvPosition != orxNULL);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    pvResult = orxSoundSystem_GetPosition(_pstSound->pstData, _pvPosition);
  }
  else
  {
    /* Updates result */
    pvResult = orxNULL;
  }

  /* Done! */
  return pvResult;
}

/** Gets a sound spatialization information
 * @param[in] _pstSound                               Concerned Sound
 * @param[out] _pfMinDistance                         Min distance, inside which the max gain will be used, will be strictly negative if the sound isn't spatialized
 * @param[out] _pfMaxDistance                         Max distance, outside which the gain will stop decreasing, will be strictly negative if the sound isn't spatialized
 * @param[out] _pfMinGain                             Min gain in [0.0f - 1.0f]
 * @param[out] _pfMaxGain                             Max gain in [0.0f - 1.0f]
 * @param[out] _pfRollOff                             RollOff factor applied when interpolating the gain between inner and outer distances, defaults to 1.0f
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_GetSpatialization(const orxSOUND *_pstSound, orxFLOAT *_pfMinDistance, orxFLOAT *_pfMaxDistance, orxFLOAT *_pfMinGain, orxFLOAT *_pfMaxGain, orxFLOAT *_pfRollOff)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_pfMinDistance != orxNULL);
  orxASSERT(_pfMaxDistance != orxNULL);
  orxASSERT(_pfMinGain != orxNULL);
  orxASSERT(_pfMaxGain != orxNULL);
  orxASSERT(_pfRollOff != orxNULL);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    eResult = orxSoundSystem_GetSpatialization(_pstSound->pstData, _pfMinDistance, _pfMaxDistance, _pfMinGain, _pfMaxGain, _pfRollOff);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets a sound panning
 * @param[in] _pstSound                               Concerned Sound
 * @param[out] _pfPanning                             Sound panning, -1.0f for full left, 0.0f for center, 1.0f for full right
 * @param[out] _pbMix                                 Left/Right channels are be mixed if orxTRUE or act like a balance otherwise
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_GetPanning(const orxSOUND *_pstSound, orxFLOAT *_pfPanning, orxBOOL *_pbMix)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT(_pfPanning != orxNULL);
  orxASSERT(_pbMix != orxNULL);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    eResult = orxSoundSystem_GetPanning(_pstSound->pstData, _pfPanning, _pbMix);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Is sound looping?
 * @param[in] _pstSound       Concerned Sound
 * @return orxTRUE / orxFALSE
 */
orxBOOL orxFASTCALL orxSound_IsLooping(const orxSOUND *_pstSound)
{
  orxBOOL bResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    bResult = orxSoundSystem_IsLooping(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    bResult = orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Gets sound duration
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetDuration(const orxSOUND *_pstSound)
{
  orxFLOAT fResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Updates result */
    fResult = orxSoundSystem_GetDuration(_pstSound->pstData);
  }
  else
  {
    /* Updates result */
    fResult = orxFLOAT_0;
  }

  /* Done! */
  return fResult;
}

/** Gets sound status
 * @param[in] _pstSound       Concerned Sound
 * @return orxSOUND_STATUS
 */
orxSOUND_STATUS orxFASTCALL orxSound_GetStatus(const orxSOUND *_pstSound)
{
  orxSOUND_STATUS eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Has sound? */
  if(_pstSound->pstData != orxNULL)
  {
    /* Depending on sound system status */
    switch(orxSoundSystem_GetStatus(_pstSound->pstData))
    {
      case orxSOUNDSYSTEM_STATUS_PLAY:
      {
        /* Updates result */
        eResult = orxSOUND_STATUS_PLAY;

        break;
      }

      case orxSOUNDSYSTEM_STATUS_PAUSE:
      {
        /* Updates result */
        eResult = orxSOUND_STATUS_PAUSE;

        break;
      }

      default:
      case orxSOUNDSYSTEM_STATUS_STOP:
      {
        /* Updates result */
        eResult = orxSOUND_STATUS_STOP;

        break;
      }
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSOUND_STATUS_NONE;
  }

  /* Done! */
  return eResult;
}

/** Gets sound config name
 * @param[in]   _pstSound     Concerned sound
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxSound_GetName(const orxSOUND *_pstSound)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Updates result */
  zResult = (_pstSound->zReference != orxNULL) ? _pstSound->zReference : orxSTRING_EMPTY;

  /* Done! */
  return zResult;
}

/** Gets master bus ID
 * @return      Master bus ID
 */
orxSTRINGID orxFASTCALL orxSound_GetMasterBusID()
{
  orxSTRINGID stResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);

  /* Updates result */
  stResult = sstSound.stMasterBusID;

  /* Done! */
  return stResult;
}

/** Gets sound's bus ID
 * @param[in]   _pstSound      Concerned sound
 * @return      Sound's bus ID
 */
orxSTRINGID orxFASTCALL orxSound_GetBusID(const orxSOUND *_pstSound)
{
  orxSTRINGID stResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);

  /* Updates result */
  stResult = _pstSound->stBusID;

  /* Done! */
  return stResult;
}

/** Sets sound's bus ID
 * @param[in]   _pstSound      Concerned sound
 * @param[in]   _stBusID       Bus ID to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetBusID(orxSOUND *_pstSound, orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstSound);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Removes sound from its current bus */
  if(orxLinkList_GetList(&(_pstSound->stBusNode)) != orxNULL)
  {
    orxLinkList_Remove(&(_pstSound->stBusNode));
  }

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxTRUE);

  /* Adds sound to end of list */
  orxLinkList_AddEnd(&(pstBus->stList), &(_pstSound->stBusNode));

  /* Stores bus ID */
  _pstSound->stBusID = _stBusID;

  /* Sets its internal bus */
  orxSoundSystem_SetBus(_pstSound->pstData, pstBus->hData);

  /* Done! */
  return eResult;
}

/** Gets next sound in bus
 * @param[in]   _pstSound     Concerned sound, orxNULL to get the first one
 * @param[in]   _stBusID      Bus ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxSOUND / orxNULL
 */
orxSOUND *orxFASTCALL orxSound_GetNext(const orxSOUND *_pstSound, orxSTRINGID _stBusID)
{
  orxSOUND *pstResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_pstSound == orxNULL) || (orxStructure_GetID((orxSTRUCTURE *)_pstSound) < orxSTRUCTURE_ID_NUMBER));
  orxASSERT((_pstSound == orxNULL) || (_stBusID == orxSTRINGID_UNDEFINED) || (_pstSound->stBusID == _stBusID));

  /* Has bus? */
  if(_stBusID != orxSTRINGID_UNDEFINED)
  {
    orxSOUND_BUS *pstBus;

    /* Gets bus */
    pstBus = orxSound_GetBus(_stBusID, orxFALSE);

    /* Valid? */
    if(pstBus != orxNULL)
    {
      orxLINKLIST_NODE *pstNode;

      /* Checks */
      orxASSERT((_pstSound == orxNULL) || (orxLinkList_GetList(&(_pstSound->stBusNode)) == &(pstBus->stList)));

      /* Gets node */
      pstNode = (_pstSound == orxNULL) ? orxLinkList_GetFirst(&(pstBus->stList)) : orxLinkList_GetNext(&(_pstSound->stBusNode));

      /* Valid? */
      if(pstNode != orxNULL)
      {
        /* Updates result */
        pstResult = orxSTRUCT_GET_FROM_FIELD(orxSOUND, stBusNode, pstNode);
      }
      else
      {
        /* Updates result */
        pstResult = orxNULL;
      }
    }
    else
    {
      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Updates result */
    pstResult = (_pstSound == orxNULL) ? orxSOUND(orxStructure_GetFirst(orxSTRUCTURE_ID_SOUND)) : orxSOUND(orxStructure_GetNext(_pstSound));
  }

  /* Done! */
  return pstResult;
}

/** Gets bus parent
 * @param[in]   _stBusID      Concerned bus ID
 * @return      Parent bus ID / orxSTRINGID_UNDEFINED
 */
orxSTRINGID orxFASTCALL orxSound_GetBusParent(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxSTRINGID   stResult = orxSTRINGID_UNDEFINED;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    orxTREE_NODE *pstParentNode;

    /* Gets its parent node */
    pstParentNode = orxTree_GetParent(&(pstBus->stNode));

    /* Found? */
    if(pstParentNode != orxNULL)
    {
      /* Updates result */
      stResult = orxSTRUCT_GET_FROM_FIELD(orxSOUND_BUS, stNode, pstParentNode)->stID;
    }
  }

  /* Done! */
  return stResult;
}

/** Gets bus child
 * @param[in]   _stBusID      Concerned bus ID
 * @return      Child bus ID / orxSTRINGID_UNDEFINED
 */
orxSTRINGID orxFASTCALL orxSound_GetBusChild(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxSTRINGID   stResult = orxSTRINGID_UNDEFINED;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    orxTREE_NODE *pstChildNode;

    /* Gets its child node */
    pstChildNode = orxTree_GetChild(&(pstBus->stNode));

    /* Found? */
    if(pstChildNode != orxNULL)
    {
      /* Updates result */
      stResult = orxSTRUCT_GET_FROM_FIELD(orxSOUND_BUS, stNode, pstChildNode)->stID;
    }
  }

  /* Done! */
  return stResult;
}

/** Gets bus sibling
 * @param[in]   _stBusID      Concerned bus ID
 * @return      Sibling bus ID / orxSTRINGID_UNDEFINED
 */
orxSTRINGID orxFASTCALL orxSound_GetBusSibling(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxSTRINGID   stResult = orxSTRINGID_UNDEFINED;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    orxTREE_NODE *pstSiblingNode;

    /* Gets its sibling node */
    pstSiblingNode = orxTree_GetSibling(&(pstBus->stNode));

    /* Found? */
    if(pstSiblingNode != orxNULL)
    {
      /* Updates result */
      stResult = orxSTRUCT_GET_FROM_FIELD(orxSOUND_BUS, stNode, pstSiblingNode)->stID;
    }
  }

  /* Done! */
  return stResult;
}

/** Sets a bus parent
 * @param[in]   _stBusID      Concerned bus ID, will create it if not already existing
 * @param[in]   _u32ParentBusID  ID of the bus to use as parent, will create it if not already existing
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetBusParent(orxSTRINGID _stBusID, orxSTRINGID _u32ParentBusID)
{
  orxSOUND_BUS *pstBus, *pstParentBus;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));
  orxASSERT((_u32ParentBusID != 0) && (_u32ParentBusID != orxSTRINGID_UNDEFINED));

  /* Gets buses */
  pstParentBus = orxSound_GetBus(_u32ParentBusID, orxTRUE);
  pstBus = orxSound_GetBus(_stBusID, orxTRUE);

  /* Sets its parent */
  if(orxTree_MoveAsChild(&(pstParentBus->stNode), &(pstBus->stNode)) != orxSTATUS_FAILURE)
  {
    /* Sets its internal parent */
    orxSoundSystem_SetBusParent(pstBus->hData, pstParentBus->hData);

    /* Updates it */
    eResult = orxSound_UpdateBus(pstBus);
  }

  /* Done! */
  return eResult;
}

/** Gets bus volume (local, ie. unaffected by the whole bus hierarchy)
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetBusVolume(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxFLOAT      fResult = orxFLOAT_1;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    /* Updates result */
    fResult = pstBus->fLocalVolume;
  }

  /* Done! */
  return fResult;
}

/** Gets bus pitch (local, ie. unaffected by the whole bus hierarchy)
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetBusPitch(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxFLOAT      fResult = orxFLOAT_1;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    /* Updates result */
    fResult = pstBus->fLocalPitch;
  }

  /* Done! */
  return fResult;
}

/** Sets bus volume
 * @param[in]   _stBusID      Concerned bus ID, will create it if not already existing
 * @param[in]   _fVolume      Desired volume (0.0 - 1.0)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetBusVolume(orxSTRINGID _stBusID, orxFLOAT _fVolume)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxTRUE);

  /* New volume? */
  if(_fVolume != pstBus->fLocalVolume)
  {
    /* Stores it */
    pstBus->fLocalVolume = _fVolume;

    /* Updates bus */
    eResult = orxSound_UpdateBus(pstBus);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Sets bus pitch
 * @param[in]   _stBusID      Concerned bus ID, will create it if not already existing
 * @param[in]   _fPitch       Desired pitch (< 1.0 => lower pitch, = 1.0 => original pitch, > 1.0 => higher pitch). 0.0 is ignored.
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_SetBusPitch(orxSTRINGID _stBusID, orxFLOAT _fPitch)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxTRUE);

  /* New pitch? */
  if(_fPitch != pstBus->fLocalPitch)
  {
    /* Stores it */
    pstBus->fLocalPitch = _fPitch;

    /* Updates bus */
    eResult = orxSound_UpdateBus(pstBus);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Gets bus global volume, ie. taking into account the whole bus hierarchy
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetBusGlobalVolume(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxFLOAT      fResult = orxFLOAT_1;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    /* Updates result */
    fResult = pstBus->fGlobalVolume;
  }

  /* Done! */
  return fResult;
}

/** Gets bus global pitch, ie. taking into account the whole bus hierarchy
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
orxFLOAT orxFASTCALL orxSound_GetBusGlobalPitch(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxFLOAT      fResult = orxFLOAT_1;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Found? */
  if(pstBus != orxNULL)
  {
    /* Updates result */
    fResult = pstBus->fGlobalPitch;
  }

  /* Done! */
  return fResult;
}

/** Adds a filter to a bus (cascading)
 * @param[in]   _stBusID          Concerned bus ID
 * @param[in]   _pstFilterData    Concerned filter data
 * @param[in]   _bUseCustomParam  Filter uses custom parameters
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_AddBusFilter(orxSTRINGID _stBusID, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));
  orxASSERT(_pstFilterData != orxNULL);

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxTRUE);

  /* Valid? */
  if(pstBus != orxNULL)
  {
    /* Adds filter to it */
    eResult = orxSoundSystem_AddBusFilter(pstBus->hData, _pstFilterData, _bUseCustomParam);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes last added filter from a bus
 * @param[in]   _stBusID      Concerned bus ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_RemoveLastBusFilter(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Valid? */
  if(pstBus != orxNULL)
  {
    /* Removes last filter from it */
    eResult = orxSoundSystem_RemoveLastBusFilter(pstBus->hData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Removes all filters from a bus
 * @param[in]   _stBusID      Concerned bus ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_RemoveAllBusFilters(orxSTRINGID _stBusID)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxFALSE);

  /* Valid? */
  if(pstBus != orxNULL)
  {
    /* Removes all filters from it */
    eResult = orxSoundSystem_RemoveAllBusFilters(pstBus->hData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Adds a filter to a bus (cascading) from config
 * @param[in]   _stBusID          Concerned bus ID
 * @param[in]   _zFilterConfigID  Config ID of the filter to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxSound_AddBusFilterFromConfig(orxSTRINGID _stBusID, const orxSTRING _zFilterConfigID)
{
  orxSOUND_BUS *pstBus;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstSound.u32Flags & orxSOUND_KU32_STATIC_FLAG_READY);
  orxASSERT((_stBusID != 0) && (_stBusID != orxSTRINGID_UNDEFINED));
  orxASSERT((_zFilterConfigID != orxNULL) && (_zFilterConfigID != orxSTRING_EMPTY));

  /* Gets bus */
  pstBus = orxSound_GetBus(_stBusID, orxTRUE);

  /* Valid? */
  if(pstBus != orxNULL)
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zFilterConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zFilterConfigID) != orxSTATUS_FAILURE))
    {
      orxSOUND_FILTER_DATA stData;

      /* Processes filter config data */
      if(orxSound_ProcessFilterConfigData(&stData) != orxSTATUS_FAILURE)
      {
        /* Adds filter */
        eResult = orxSoundSystem_AddBusFilter(pstBus->hData, &stData, orxConfig_GetBool(orxSOUND_KZ_CONFIG_USE_CUSTOM_PARAM));
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "Couldn't find filter section (%s) in config.", _zFilterConfigID);
    }
  }

  /* Done! */
  return eResult;
}

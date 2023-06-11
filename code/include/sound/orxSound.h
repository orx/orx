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
 * @file orxSound.h
 * @date 13/07/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSound
 *
 * Sound module
 * Module that handles sound
 *
 * @{
 */


#ifndef _orxSOUND_H_
#define _orxSOUND_H_


#include "orxInclude.h"
#include "math/orxVector.h"
#include "sound/orxSoundSystem.h"


/** Misc defines
 */
#define orxSOUND_KZ_RESOURCE_GROUP            "Sound"
#define orxSOUND_KZ_LOCALE_GROUP              "Sound"

#define orxSOUND_KZ_MASTER_BUS                "master"


/** Sound status enum
 */
typedef enum __orxSOUND_STATUS_t
{
  orxSOUND_STATUS_PLAY = 0,
  orxSOUND_STATUS_PAUSE,
  orxSOUND_STATUS_STOP,

  orxSOUND_STATUS_NUMBER,

  orxSOUND_STATUS_NONE = orxENUM_NONE

} orxSOUND_STATUS;


/** Internal Sound structures
 */
typedef struct __orxSOUND_t                   orxSOUND;
typedef struct __orxSOUND_SAMPLE_t            orxSOUND_SAMPLE;


/** Event enum
 */
typedef enum __orxSOUND_EVENT_t
{
  orxSOUND_EVENT_START = 0,                   /**< Event sent when a sound starts */
  orxSOUND_EVENT_STOP,                        /**< Event sent when a sound stops */
  orxSOUND_EVENT_ADD,                         /**< Event sent when a sound is added */
  orxSOUND_EVENT_REMOVE,                      /**< Event sent when a sound is removed */
  orxSOUND_EVENT_PACKET,                      /**< Event sent when a sound packet is streamed. IMPORTANT: this event can be sent from a worker thread, do not call any orx API when handling it */
  orxSOUND_EVENT_RECORDING_START,             /**< Event sent when recording starts */
  orxSOUND_EVENT_RECORDING_STOP,              /**< Event sent when recording stops */
  orxSOUND_EVENT_RECORDING_PACKET,            /**< Event sent when a packet has been recorded */
  orxSOUND_EVENT_SET_FILTER_PARAM,            /**< Event sent when setting a filter's parameter */

  orxSOUND_EVENT_NUMBER,

  orxSOUND_EVENT_NONE = orxENUM_NONE

} orxSOUND_EVENT;

/** Sound stream info
 */
typedef struct __orxSOUND_STREAM_INFO_t
{
  const orxSTRING zName;                      /**< Stream name : 4 */
  orxU32          u32SampleRate;              /**< The sample rate, e.g. 44100 Hertz : 8 */
  orxU32          u32ChannelNumber;           /**< Number of channels, either mono (1) or stereo (2) : 12 */

} orxSOUND_STREAM_INFO;

/** Sound recording packet
 */
typedef struct __orxSOUND_STREAM_PACKET_t
{
  orxFLOAT *afSampleList;                     /**< List of samples for this packet : 4 */
  orxU32    u32SampleNumber;                  /**< Number of samples contained in this packet : 8 */
  orxFLOAT  fTimeStamp;                       /**< Packet's timestamp (system time): 12 */
  orxFLOAT  fTime;                            /**< Packet's time (cursor/play position): 16 */
  orxS32    s32ID;                            /**< Packet's ID : 20 */
  orxBOOL   bDiscard;                         /**< Write/play the packet? : 24 */
  orxBOOL   bLast;                            /**< Last packet before end of stream? : 28 */

} orxSOUND_STREAM_PACKET;

/** Sound event payload
 */
typedef struct __orxSOUND_EVENT_PAYLOAD_t
{
  orxSOUND                   *pstSound;     /**< Sound reference : 4 */

  struct
  {
    orxSOUND_STREAM_INFO      stInfo;       /**< Sound record info : 16 */
    orxSOUND_STREAM_PACKET    stPacket;     /**< Sound record packet : 32 */

  } stStream;

  struct
  {
    orxSOUND_FILTER_DATA      stData;       /**< Filter data : 32 */
    orxSTRINGID               stBusID;      /**< Bus ID : 40 */

  } stFilter;

} orxSOUND_EVENT_PAYLOAD;


/** Sound module setup
 */
extern orxDLLAPI void orxFASTCALL             orxSound_Setup();

/** Initializes the sound module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Init();

/** Exits from the sound module
 */
extern orxDLLAPI void orxFASTCALL             orxSound_Exit();


/** Creates an empty sound
 * @return      Created orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL        orxSound_Create();

/** Creates sound from config
 * @param[in]   _zConfigID    Config ID
 * @ return orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL        orxSound_CreateFromConfig(const orxSTRING _zConfigID);

/** Creates a sound with an empty stream (ie. you'll need to provide actual sound data for each packet sent to the sound card using the event system)
 * @param[in] _u32ChannelNumber Number of channels of the stream
 * @param[in] _u32SampleRate    Sampling rate of the stream (ie. number of frames per second)
 * @param[in] _zName            Name to associate with this sound
 * @return orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL        orxSound_CreateWithEmptyStream(orxU32 _u32ChannelNumber, orxU32 _u32SampleRate, const orxSTRING _zName);

/** Deletes sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Delete(orxSOUND *_pstSound);

/** Clears cache (if any sound sample is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_ClearCache();


/** Creates a sample
 * @param[in] _u32ChannelNumber Number of channels of the sample
 * @param[in] _u32FrameNumber   Number of frame of the sample (number of "samples" = number of frames * number of channels)
 * @param[in] _u32SampleRate    Sampling rate of the sample (ie. number of frames per second)
 * @param[in] _zName            Name to associate with the sample
 * @return orxSOUND_SAMPLE / orxNULL
 */
extern orxDLLAPI orxSOUND_SAMPLE *orxFASTCALL orxSound_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate, const orxSTRING _zName);

/** Deletes a sample
 * @param[in] _pstSample        Concerned sample
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_DeleteSample(orxSOUND_SAMPLE *_pstSample);

/** Gets sample info
 * @param[in]   _pstSample                    Concerned sample
 * @param[in]   _pu32ChannelNumber            Number of channels of the sample
 * @param[in]   _pu32FrameNumber              Number of frame of the sample (number of "samples" = number of frames * number of channels)
 * @param[in]   _pu32SampleRate               Sampling rate of the sample (ie. number of frames per second)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_GetSampleInfo(const orxSOUND_SAMPLE *_pstSample, orxU32 *_pu32ChannelNumber, orxU32 *_pu32FrameNumber, orxU32 *_pu32SampleRate);

/** Sets sample data
 * @param[in]   _pstSample                    Concerned sample
 * @param[in]   _afData                       Data to set (samples are expected to be signed/normalized)
 * @param[in]   _u32SampleNumber              Number of samples in the data array
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetSampleData(orxSOUND_SAMPLE *_pstSample, const orxFLOAT *_afData, orxU32 _u32SampleNumber);

/** Links a sample
 * @param[in]   _pstSound     Concerned sound
 * @param[in]   _zSampleName  Name of the sample to link (must already be loaded/created)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_LinkSample(orxSOUND *_pstSound, const orxSTRING _zSampleName);

/** Unlinks (and deletes if not used anymore) a sample
 * @param[in]   _pstSound     Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_UnlinkSample(orxSOUND *_pstSound);


/** Is a stream (ie. music)?
 * @param[in] _pstSound       Concerned Sound
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxSound_IsStream(orxSOUND *_pstSound);


/** Plays sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Play(orxSOUND *_pstSound);

/** Pauses sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Pause(orxSOUND *_pstSound);

/** Stops sound
 * @param[in] _pstSound       Concerned Sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Stop(orxSOUND *_pstSound);

/** Adds a filter to a sound (cascading)
 * @param[in]   _pstSound         Concerned sound
 * @param[in]   _pstFilterData    Concerned filter data
 * @param[in]   _bUseCustomParam  Filter uses custom parameters
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_AddFilter(orxSOUND *_pstSound, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam);

/** Removes last added filter from a sound
 * @param[in]   _pstSound     Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_RemoveLastFilter(orxSOUND *_pstSound);

/** Removes all filters from a sound
 * @param[in]   _pstSound     Concerned sound
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_RemoveAllFilters(orxSOUND *_pstSound);

/** Adds a filter to a sound (cascading) from config
 * @param[in]   _pstSound         Concerned sound
 * @param[in]   _zFilterConfigID  Config ID of the filter to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_AddFilterFromConfig(orxSOUND *_pstSound, const orxSTRING _zFilterConfigID);


/** Starts recording
 * @param[in] _zName             Name for the recorded sound/file
 * @param[in] _bWriteToFile      Should write to file?
 * @param[in] _u32SampleRate     Sample rate, 0 for default rate (48000Hz)
 * @param[in] _u32ChannelNumber  Channel number, 0 for default mono channel
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_StartRecording(const orxCHAR *_zName, orxBOOL _bWriteToFile, orxU32 _u32SampleRate, orxU32 _u32ChannelNumber);

/** Stops recording
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_StopRecording();

/** Is recording possible on the current system?
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxSound_HasRecordingSupport();


/** Sets sound volume
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fVolume        Desired volume (0.0 - 1.0)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetVolume(orxSOUND *_pstSound, orxFLOAT _fVolume);

/** Sets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fPitch         Desired pitch
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPitch(orxSOUND *_pstSound, orxFLOAT _fPitch);

/** Sets a sound time (ie. cursor/play position from beginning)
 * @param[in]   _pstSound                             Concerned sound
 * @param[in]   _fTime                                Time, in seconds
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetTime(orxSOUND *_pstSound, orxFLOAT _fTime);

/** Sets sound position
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _pvPosition     Desired position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPosition(orxSOUND *_pstSound, const orxVECTOR *_pvPosition);

/** Sets a sound spatialization, with gain decreasing between the minimum and maximum distances, when enabled
 * @param[in] _pstSound                               Concerned Sound
 * @param[in] _fMinDistance                           Min distance, inside which the max gain will be used, strictly negative value to disable spatialization entirely
 * @param[in] _fMaxDistance                           Max distance, outside which the gain will stop decreasing, strictly negative value to disable spatialization entirely
 * @param[in] _fMinGain                               Min gain in [0.0f - 1.0f]
 * @param[in] _fMaxGain                               Max gain in [0.0f - 1.0f]
 * @param[in] _fRollOff                               RollOff factor applied when interpolating the gain between inner and outer distances, defaults to 1.0f
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetSpatialization(orxSOUND *_pstSound, orxFLOAT _fMinDistance, orxFLOAT _fMaxDistance, orxFLOAT _fMinGain, orxFLOAT _fMaxGain, orxFLOAT _fRollOff);

/** Sets a sound panning
 * @param[in] _pstSound panning
 * @param[in] _fPanning                               Sound panning, -1.0f for full left, 0.0f for center, 1.0f for full right
 * @param[in] _bMix                                   Left/Right channels will be mixed if orxTRUE or act like a balance otherwise
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPanning(orxSOUND *_pstSound, orxFLOAT _fPanning, orxBOOL _bMix);

/** Loops sound
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _bLoop          orxTRUE / orxFALSE
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_Loop(orxSOUND *_pstSound, orxBOOL _bLoop);


/** Gets sound volume
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetVolume(const orxSOUND *_pstSound);

/** Gets sound pitch
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetPitch(const orxSOUND *_pstSound);

/** Gets a sound's time (ie. cursor/play position from beginning)
 * @param[in]   _pstSound                             Concerned sound
 * @return Sound's time (cursor/play position), in seconds
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetTime(const orxSOUND *_pstSound);

/** Gets sound position
 * @param[in]  _pstSound      Concerned Sound
 * @param[out] _pvPosition    Sound's position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxSound_GetPosition(const orxSOUND *_pstSound, orxVECTOR *_pvPosition);

/** Gets a sound spatialization information
 * @param[in] _pstSound                               Concerned Sound
 * @param[out] _pfMinDistance                         Min distance, inside which the max gain will be used, will be strictly negative if the sound isn't spatialized
 * @param[out] _pfMaxDistance                         Max distance, outside which the gain will stop decreasing, will be strictly negative if the sound isn't spatialized
 * @param[out] _pfMinGain                             Min gain in [0.0f - 1.0f]
 * @param[out] _pfMaxGain                             Max gain in [0.0f - 1.0f]
 * @param[out] _pfRollOff                             RollOff factor applied when interpolating the gain between inner and outer distances, defaults to 1.0f
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_GetSpatialization(const orxSOUND *_pstSound, orxFLOAT *_pfMinDistance, orxFLOAT *_pfMaxDistance, orxFLOAT *_pfMinGain, orxFLOAT *_pfMaxGain, orxFLOAT *_pfRollOff);

/** Gets a sound panning
 * @param[in] _pstSound                               Concerned Sound
 * @param[out] _pfPanning                             Sound panning, -1.0f for full left, 0.0f for center, 1.0f for full right
 * @param[out] _pbMix                                 Left/Right channels are be mixed if orxTRUE or act like a balance otherwise
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_GetPanning(const orxSOUND *_pstSound, orxFLOAT *_pfPanning, orxBOOL *_pbMix);

/** Is sound looping?
 * @param[in] _pstSound       Concerned Sound
 * @return orxTRUE / orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL          orxSound_IsLooping(const orxSOUND *_pstSound);


/** Gets sound duration
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetDuration(const orxSOUND *_pstSound);

/** Gets sound status
 * @param[in] _pstSound       Concerned Sound
 * @return orxSOUND_STATUS
 */
extern orxDLLAPI orxSOUND_STATUS orxFASTCALL  orxSound_GetStatus(const orxSOUND *_pstSound);

/** Gets sound config name
 * @param[in]   _pstSound     Concerned sound
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL  orxSound_GetName(const orxSOUND *_pstSound);

/** Gets master bus ID
 * @return      Master bus ID
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL      orxSound_GetMasterBusID();

/** Gets sound's bus ID
 * @param[in]   _pstSound     Concerned sound
 * @return      Sound's bus ID
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL      orxSound_GetBusID(const orxSOUND *_pstSound);

/** Sets sound's bus ID
 * @param[in]   _pstSound     Concerned sound
 * @param[in]   _stBusID      Bus ID to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetBusID(orxSOUND *_pstSound, orxSTRINGID _stBusID);

/** Gets next sound in bus
 * @param[in]   _pstSound     Concerned sound, orxNULL to get the first one
 * @param[in]   _stBusID      Bus ID to consider, orxSTRINGID_UNDEFINED for all
 * @return      orxSOUND / orxNULL
 */
extern orxDLLAPI orxSOUND *orxFASTCALL        orxSound_GetNext(const orxSOUND *_pstSound, orxSTRINGID _stBusID);

/** Gets bus parent
 * @param[in]   _stBusID      Concerned bus ID
 * @return      Parent bus ID / orxSTRINGID_UNDEFINED
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL      orxSound_GetBusParent(orxSTRINGID _stBusID);

/** Gets bus child
 * @param[in]   _stBusID      Concerned bus ID
 * @return      Child bus ID / orxSTRINGID_UNDEFINED
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL      orxSound_GetBusChild(orxSTRINGID _stBusID);

/** Gets bus sibling
 * @param[in]   _stBusID      Concerned bus ID
 * @return      Sibling bus ID / orxSTRINGID_UNDEFINED
 */
extern orxDLLAPI orxSTRINGID orxFASTCALL      orxSound_GetBusSibling(orxSTRINGID _stBusID);

/** Sets a bus parent
 * @param[in]   _stBusID      Concerned bus ID, will create it if not already existing
 * @param[in]   _stParentBusID  ID of the bus to use as parent, will create it if not already existing
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetBusParent(orxSTRINGID _stBusID, orxSTRINGID _stParentBusID);

/** Gets bus volume (local, ie. unaffected by the whole bus hierarchy)
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetBusVolume(orxSTRINGID _stBusID);

/** Gets bus pitch (local, ie. unaffected by the whole bus hierarchy)
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetBusPitch(orxSTRINGID _stBusID);

/** Sets bus volume
 * @param[in]   _stBusID      Concerned bus ID, will create it if not already existing
 * @param[in]   _fVolume      Desired volume (0.0 - 1.0)
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetBusVolume(orxSTRINGID _stBusID, orxFLOAT _fVolume);

/** Sets bus pitch
 * @param[in]   _stBusID      Concerned bus ID, will create it if not already existing
 * @param[in]   _fPitch       Desired pitch
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetBusPitch(orxSTRINGID _stBusID, orxFLOAT _fPitch);

/** Gets bus global volume, ie. taking into account the whole bus hierarchy
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetBusGlobalVolume(orxSTRINGID _stBusID);

/** Gets bus global pitch, ie. taking into account the whole bus hierarchy
 * @param[in]   _stBusID      Concerned bus ID
 * @return      orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetBusGlobalPitch(orxSTRINGID _stBusID);

/** Adds a filter to a bus (cascading)
 * @param[in]   _stBusID          Concerned bus ID
 * @param[in]   _pstFilterData    Concerned filter data
 * @param[in]   _bUseCustomParam  Filter uses custom parameters
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_AddBusFilter(orxSTRINGID _stBusID, const orxSOUND_FILTER_DATA *_pstFilterData, orxBOOL _bUseCustomParam);

/** Removes last added filter from a bus
 * @param[in]   _stBusID      Concerned bus ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_RemoveLastBusFilter(orxSTRINGID _stBusID);

/** Removes all filters from a bus
 * @param[in]   _stBusID      Concerned bus ID
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_RemoveAllBusFilters(orxSTRINGID _stBusID);

/** Adds a filter to a bus (cascading) from config
 * @param[in]   _stBusID          Concerned bus ID
 * @param[in]   _zFilterConfigID  Config ID of the filter to add
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_AddBusFilterFromConfig(orxSTRINGID _stBusID, const orxSTRING _zFilterConfigID);

#endif /*_orxSOUND_H_*/

/** @} */

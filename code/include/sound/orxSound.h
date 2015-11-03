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
#include "sound/orxSoundSystem.h"
#include "math/orxVector.h"


/** Misc defines
 */
#define orxSOUND_KZ_RESOURCE_GROUP            "Sound"


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


/** Internal Sound structure
 */
typedef struct __orxSOUND_t                   orxSOUND;


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

  orxSOUND_EVENT_NUMBER,

  orxSOUND_EVENT_NONE = orxENUM_NONE

} orxSOUND_EVENT;

/** Sound stream info
 */
typedef struct __orxSOUND_STREAM_INFO_t
{
  orxU32    u32SampleRate;                    /**< The sample rate, e.g. 44100 Hertz : 4 */
  orxU32    u32ChannelNumber;                 /**< Number of channels, either mono (1) or stereo (2) : 8 */

} orxSOUND_STREAM_INFO;

/** Sound recording packet
 */
typedef struct __orxSOUND_STREAM_PACKET_t
{
  orxU32    u32SampleNumber;                  /**< Number of samples contained in this packet : 4 */
  orxS16   *as16SampleList;                   /**< List of samples for this packet : 8 */
  orxBOOL   bDiscard;                         /**< Write/play the packet? : 12 */
  orxFLOAT  fTimeStamp;                       /**< Packet's timestamp : 16 */
  orxS32    s32ID;                            /**< Packet's ID : 20 */

} orxSOUND_STREAM_PACKET;

/** Sound event payload
 */
typedef struct __orxSOUND_EVENT_PAYLOAD_t
{
  union
  {
    orxSOUND                   *pstSound;     /**< Sound reference : 4 */

    struct
    {
      const orxSTRING           zSoundName;   /**< Sound name : 4 */
      orxSOUND_STREAM_INFO      stInfo;       /**< Sound record info : 12 */
      orxSOUND_STREAM_PACKET    stPacket;     /**< Sound record packet : 32 */
    } stStream;
  };                                          /**< Stream : 32 */

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


/** Creates a sample
 * @param[in] _u32ChannelNumber Number of channels of the sample
 * @param[in] _u32FrameNumber   Number of frame of the sample (number of "samples" = number of frames * number of channels)
 * @param[in] _u32SampleRate    Sampling rate of the sample (ie. number of frames per second)
 * @param[in] _zName            Name to associate with the sample
 * @return orxSOUNDSYSTEM_SAMPLE / orxNULL
 */
extern orxDLLAPI orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSound_CreateSample(orxU32 _u32ChannelNumber, orxU32 _u32FrameNumber, orxU32 _u32SampleRate, const orxSTRING _zName);

/** Gets a sample
 * @param[in] _zName            Sample's name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSOUNDSYSTEM_SAMPLE *orxFASTCALL orxSound_GetSample(const orxSTRING _zName);

/** Deletes a sample
 * @param[in] _zName            Sample's name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_DeleteSample(const orxSTRING _zName);

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


/** Starts recording
 * @param[in] _zName             Name for the recorded sound/file
 * @param[in] _bWriteToFile      Should write to file?
 * @param[in] _u32SampleRate     Sample rate, 0 for default rate (44100Hz)
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

/** Sets sound position
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _pvPosition     Desired position
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetPosition(orxSOUND *_pstSound, const orxVECTOR *_pvPosition);

/** Sets sound attenuation
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fAttenuation   Desired attenuation
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetAttenuation(orxSOUND *_pstSound, orxFLOAT _fAttenuation);

/** Sets sound reference distance
 * @param[in] _pstSound       Concerned Sound
 * @param[in] _fDistance      Within this distance, sound is perceived at its maximum volume
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        orxSound_SetReferenceDistance(orxSOUND *_pstSound, orxFLOAT _fDistance);

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

/** Gets sound position
 * @param[in]  _pstSound      Concerned Sound
 * @param[out] _pvPosition    Sound's position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL       orxSound_GetPosition(const orxSOUND *_pstSound, orxVECTOR *_pvPosition);

/** Gets sound attenuation
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetAttenuation(const orxSOUND *_pstSound);

/** Gets sound reference distance
 * @param[in] _pstSound       Concerned Sound
 * @return orxFLOAT
 */
extern orxDLLAPI orxFLOAT orxFASTCALL         orxSound_GetReferenceDistance(const orxSOUND *_pstSound);

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

#endif /*_orxSOUND_H_*/

/** @} */

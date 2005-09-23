/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/**
 * @file
 * @date begin       : 23/07/2002
 * @date API updated : 14/11/2003
 * @date API updated : 17/05/2005
 * @author (C) Arcallians
 * 
 * @todo Add Stream management and 3D sounds in the API
 */ 
 
/**
 * @addtogroup Sound
 * Module for sound management.
 * @{
 */


#ifndef _orxSOUND_H_
#define _orxSOUND_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


typedef struct __orxSOUND_SAMPLE_t orxSOUND_SAMPLE;

/* Define Channels Values */
#define orxSOUND_CHANNEL_KU32_NUMBER      4096        /**< Max number of channel */
#define orxSOUND_CHANNEL_KU32_SELECT_FREE 4097        /**< Automatically select a free channel */
#define orxSOUND_CHANNEL_KU32_SELECT_ALL  4098        /**< Select all channels at the same time */
#define orxSOUND_CHANNEL_KU32_ERROR       4099        /**< Error returned by play if channel can not be used */

/* Define channel flags */
#define orxSOUND_STATE_KU32_STARTED       0x00000001  /**< channel started by play */
#define orxSOUND_STATE_KU32_PAUSED        0x00000002  /**< channel paused */
#define orxSOUND_STATE_KU32_LOOP          0x00000004  /**< channel loop */


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Sound module setup. */
extern orxDLLAPI orxVOID                              orxSound_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxSound_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxSound_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxSound_SampleLoadFromFile, orxSOUND_SAMPLE*, orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxSound_SampleLoadFromMemory, orxSOUND_SAMPLE*, orxCONST orxVOID *, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxSound_SampleUnload, orxVOID, orxSOUND_SAMPLE *);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxSound_SamplePlay, orxU32, orxU32, orxSOUND_SAMPLE *);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxSound_SampleTestFlags, orxBOOL, orxSOUND_SAMPLE *, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_3(orxSound_SampleSetFlags, orxVOID, orxSOUND_SAMPLE *, orxU32, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxSound_ChannelStop, orxSTATUS, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxSound_ChannelPause, orxSTATUS, orxU32, orxBOOL);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxSound_ChannelTestFlags, orxBOOL, orxU32, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_3(orxSound_ChannelSetFlags, orxVOID, orxU32, orxU32, orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION_2(orxSound_ChannelSetVolume, orxSTATUS, orxU32, orxU8);

/** Initialize the Sound Module
 * @return Returns the status of the module initialization
 */
orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxSound_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_Init)();
}

/** Uninitialize the Sound Module
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID orxSound_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_Exit)();
}

/** Load a sample From a file
 * @param[in] _zFileName        File name to load
 * @return Returns a sample
 */
orxSTATIC orxINLINE orxDLLAPI orxSOUND_SAMPLE* orxSound_SampleLoadFromFile(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_SampleLoadFromFile)(_zFileName);
}

/** Load a sample From memory
 * @param[in] _pMem             Memory address where is stored the sample
 * @param[in] _u32Size          Size of the data block to load
 * @return Returns a pointer on the loaded sample
 */
orxSTATIC orxINLINE orxDLLAPI orxSOUND_SAMPLE* orxSound_SampleLoadFromMemory(orxCONST orxVOID *_pMem, orxU32 _u32Size)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_SampleLoadFromMemory)(_pMem, _u32Size);
}

/** Unload a sample
 * @param[in] _pstSample        Pointer on the sample to unload
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID orxSound_SampleUnload(orxSOUND_SAMPLE *_pstSample)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_SampleUnload)(_pstSample);
}

/** Play a sample
 * @param[in] _u32Channel       Channel to use (from 0 to 4096). flags can be used to automatically select a free chanel or all the channel
 * @param[in] _pstSample        Sample to play
 * @return Returns the channel used by the sample or orxSOUND_CHANNEL_KU32_ERROR if no channel available
 */
orxSTATIC orxINLINE orxDLLAPI orxU32 orxSound_SamplePlay(orxU32 _u32Channel, orxSOUND_SAMPLE *_pstSample)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_SamplePlay)(_u32Channel, _pstSample);
}

/** Stop a sample
 * @param[in] _u32Channel       Channel to stop (orxSOUND_CHANNEL_KU32_SELECT_ALL can be used to stop all channels)
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxSound_ChannelStop(orxU32 _u32Channel)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_ChannelStop)(_u32Channel);
}

/** Pause/UnPause a sample
 * @param[in] _u32Channel       Channel to pause
 * @param[in] _bPause           Pause the channel
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxSound_ChannelPause(orxU32 _u32Channel, orxBOOL _bPause)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_ChannelPause)(_u32Channel, _bPause);
}

/** Test Sample flags
 * @param[in] _u32Channel       Channel to test
 * @param[in] _u32FlagsToTest   Flags to tests
 * @return orxTRUE if tested flags are present
 */
orxSTATIC orxINLINE orxDLLAPI orxBOOL orxSound_ChannelTestFlags(orxU32 _u32Channel, orxU32 _u32FlagsToTest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_ChannelTestFlags)(_u32Channel, _u32FlagsToTest);
}

/** Set Sample flags
 * @param[in] _u32Channel       Channel to set
 * @param[in] _u32FlagsToRemove Flags to remove
 * @param[in] _u32FlagsToAdd    Flags to add
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID orxSound_ChannelSetFlags(orxU32 _u32Channel, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_ChannelSetFlags)(_u32Channel, _u32FlagsToRemove, _u32FlagsToAdd);
}

/** Set Channel volume
 * @param[in] _u32Channel       Channel to use
 * @param[in] _u8Volume         New volume
 */
orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxSound_ChannelSetVolume(orxU32 _u32Channel, orxU8 _u8Volume)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSound_ChannelSetVolume)(_u32Channel, _u8Volume);
}


#endif /* _orxSOUND_H_ */

/** @} */

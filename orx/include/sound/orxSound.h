/**
 * @file orxSound.h
 * 
 * Module for sound management.
 * 
 * @todo Add Stream management and 3D sounds in the API
 */ 

/***************************************************************************
 orxSound.h

 begin                : 23/07/2002
 API updated            14/11/2003
 API updated            17/05/2005

 author               : (C) Gdp
 email                : snegri@free.fr
                        iarwain@arcallians.org
                        bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxSOUND_H_
#define _orxSOUND_H_

#include "orxInclude.h"

typedef struct __orxSOUND_SAMPLE_t orxSOUND_SAMPLE;

/* Define Channels Values */
#define orxSOUND_CHANNEL_KU32_NUMBER      4096 /* Max number of channel */
#define orxSOUND_CHANNEL_KU32_SELECT_FREE 4097 /* Automatically select a free channel */
#define orxSOUND_CHANNEL_KU32_SELECT_ALL  4098 /* Select all channels at the same time */
#define orxSOUND_CHANNEL_KU32_ERROR       4099 /* Error returned by play if channel can not be used */

/* Define sample flags */
#define orxSOUND_SAMPLE_KU32_STARTED      0x00000001 /* Sample started by play */
#define orxSOUND_SAMPLE_KU32_PAUSED       0x00000002 /* Sample paused */
#define orxSOUND_SAMPLE_KU32_LOOP         0x00000004 /* Sample loop */

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Function that initialize the Sound plugin module
 */
extern orxDLLAPI orxVOID          orxFASTCALL orxSound_Plugin_Init();

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Initialize the Sound Module
 * @return Returns the status of the module initialization
 */
extern orxDLLAPI orxSTATUS        (*orxSound_Init)();

/** Uninitialize the Sound Module
 */
extern orxDLLAPI orxVOID          (*orxSound_Exit)();

/** Load a sample From a file
 * @param _zFileName        (IN)  File name to load
 * @return Returns a sample
 */
extern orxDLLAPI orxSOUND_SAMPLE* (*orxSound_SampleLoadFromFile)(orxCONST orxSTRING _zFileName);

/** Load a sample From memory
 * @param _pMem             (IN)  Memory address where is stored the sample
 * @param _u32Size          (IN)  Size of the data block to load
 */
extern orxDLLAPI orxSOUND_SAMPLE* (*orxSound_SampleLoadFromMemory)(orxCONST orxVOID *_pMem, orxU32 _u32Size);

/** Unload a sample
 * @param _pstSample        (IN)  Pointer on the sample to unload
 */
extern orxDLLAPI orxVOID          (*orxSound_SampleUnload)(orxSOUND_SAMPLE *_pstSample);

/** Play a sample
 * @param _u32Channel       (IN)  Channel to use (from 0 to 4096). flags can be used to automatically select a free chanel or all the channel
 * @param _pstSample        (IN)  Sample to play
 * @return Returns the channel used by the sample or orxSOUND_CHANNEL_KU32_ERROR if no channel available
 */
extern orxDLLAPI orxU32           (*orxSound_PlaySample)(orxU32 _u32Channel, orxSOUND_SAMPLE *_pstSample);

/** Stop a sample
 * @param _u32Channel       (IN)  Channel to stop (orxSOUND_CHANNEL_KU32_SELECT_ALL can be used to stop all channels)
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS        (*orxSound_Stop)(orxU32 _u32Channel);

/** Pause/UnPause a sample
 * @param _u32Channel       (IN)  Channel to pause
 * @param _bPause           (IN)  Pause the channel
 */
extern orxDLLAPI orxSTATUS        (*orxSound_Pause)(orxU32 _u32Channel, orxBOOL _bPause);

/** Test Sample flags
 * @param _u32Channel       (IN)  Channel to test
 * @param _u32FlagsToTest   (IN)  Flags to tests
 * @return orxTRUE if tested flags are present
 */
extern orxDLLAPI orxBOOL          (*orxSound_TestFlags)(orxU32 _u32Channel, orxU32 _u32FlagsToTest);

/** Set Sample flags
 * @param _u32Channel       (IN)  Channel to set
 * @param _u32FlagsToRemove (IN)  Flags to remove
 * @param _u32FlagsToRemove (IN)  Flags to add
 */
extern orxDLLAPI orxVOID          (*orxSound_SetFlags)(orxU32 _u32Channel, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd);

/** Set Channel volume
 * @param _u32Channel       (IN)  Channel to use
 * @param _u8Volume         (IN)  New volume
 */
extern orxDLLAPI orxVOID          (*orxSound_SetVolume)(orxU32 _u32Channel, orxU8 _u8Volume);

#endif /* _orxSOUND_H_ */

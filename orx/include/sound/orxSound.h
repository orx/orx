/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

/**
 * @file orxSound.h
 * @date 13/07/2008
 * @author (C) Arcallians
 */

/**
 * @addtogroup Sound
 * 
 * Sound module
 * Module that handles sound
 *
 * @{
 */


#ifndef _orxSOUND_H_
#define _orxSOUND_H_


#include "orxInclude.h"

/** Sound module setup
 */
extern orxDLLAPI orxVOID                orxSound_Setup();

/** Initializes the sound module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxSound_Init();

/** Exits from the sound module
 */
extern orxDLLAPI orxVOID                orxSound_Exit();

#endif /*_orxSOUND_H_*/

/** @} */

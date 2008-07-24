/**
 * \file orxSystem.h
 */

/***************************************************************************
 begin                : 25/05/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxSYSTEM_H_
#define _orxSYSTEM_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


typedef orxU32                      orxDATE;


/** Event enum
 */
typedef enum __orxSYSTEM_EVENT_t
{
  orxSYSTEM_EVENT_CLOSE = 0,
  orxSYSTEM_EVENT_FOCUS_GAINED,
  orxSYSTEM_EVENT_FOCUS_LOST,

  orxSYSTEM_EVENT_NUMBER,

  orxSYSTEM_EVENT_NONE = orxENUM_NONE

} orxSYSTEM_EVENT;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** System module setup */
extern orxDLLAPI orxVOID            orxSystem_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the system module.
 * @return The status of the operation.
 */
extern orxDLLAPI orxSTATUS          orxSystem_Init();

/** Exits from the system module.
 * @return The status of the operation.
 */
extern orxDLLAPI orxVOID            orxSystem_Exit();

/** Gets time.
 * @return Current time.
 */
extern orxDLLAPI orxFLOAT           orxSystem_GetTime();

/** Delay the program for given number of milliseconds.
 * @param[in] _fSeconds Number of seconds to wait.
 */
extern orxDLLAPI orxVOID            orxSystem_Delay(orxFLOAT _fSeconds);

#endif /* _orxSYSTEM_H_ */

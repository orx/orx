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
 * @file orxSystem.h
 * @date 25/05/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxSystem
 * 
 * Module file
 * Code that handles modules and their dependencies
 *
 * @{
 */


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

/** Setups the system module */
extern orxDLLAPI orxVOID            orxSystem_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Inits the system module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS          orxSystem_Init();

/** Exits from the system module
 */
extern orxDLLAPI orxVOID            orxSystem_Exit();

/** Gets current time (elapsed from the beginning of the application)
 * @return Current time
 */
extern orxDLLAPI orxFLOAT           orxSystem_GetTime();

/** Delay the program for given number of seconds
 * @param[in] _fSeconds             Number of seconds to wait
 */
extern orxDLLAPI orxVOID            orxSystem_Delay(orxFLOAT _fSeconds);

#endif /* _orxSYSTEM_H_ */

/** @} */

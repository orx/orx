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


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** System module setup */
extern orxDLLAPI orxVOID            orxSystem_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxSystem_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSystem_Exit, orxVOID);

orxPLUGIN_DECLARE_CORE_FUNCTION(orxSystem_GetTime, orxFLOAT);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxSystem_Delay, orxVOID, orxFLOAT);


/** Inits the system module.
 * @return The status of the operation.
 */
orxSTATIC orxINLINE orxSTATUS orxSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_Init)();
}

/** Exits from the system module.
 * @return The status of the operation.
 */
orxSTATIC orxINLINE orxVOID orxSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_Exit)();
}

/** Gets time.
 * @return Current time.
 */
orxSTATIC orxINLINE orxFLOAT orxSystem_GetTime()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_GetTime)();
}

/** Delay the program for given number of milliseconds.
 * @param[in] _fSeconds Number of seconds to wait.
 */
orxSTATIC orxINLINE orxVOID orxSystem_Delay(orxFLOAT _fSeconds)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxSystem_Delay)(_fSeconds);
}

#endif /* _orxSYSTEM_H_ */

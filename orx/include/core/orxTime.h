/**
 * \file orxTime.h
 */

/***************************************************************************
 begin                : 25/05/2005
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxTIME_H_
#define _orxTIME_H_

#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


typedef orxU32                      orxDATE;

typedef orxU32                      orxTIME;


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxTime_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxTime_Exit, orxVOID);

orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxTime_GetTime, orxTIME);
orxPLUGIN_DECLARE_CORE_FUNCTION_0(orxTime_GetDate, orxDATE);
orxPLUGIN_DECLARE_CORE_FUNCTION_1(orxTime_Delay, orxVOID, orxTIME);


/** Inits the time module.
 * @return The status of the operation.
 */
orxSTATIC orxINLINE orxDLLAPI orxSTATUS orxTime_Init()
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxTime_Init)();
}

/** Exits from the time module.
 * @return The status of the operation.
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID orxTime_Exit()
{
  orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxTime_Exit)();
}

/** Gets time.
 * @return Current time.
 */
orxSTATIC orxINLINE orxDLLAPI orxTIME orxTime_GetTime()
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxTime_GetTime)();
}

/** Gets date.
 * @return Current date.
 */
orxSTATIC orxINLINE orxDLLAPI orxDATE orxTime_GetDate()
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxTime_GetDate)();
}

/** Delay the program for given number of milliseconds.
 * @param[in] _stTime Number of milliseconds to wait.
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID orxTime_Delay(orxTIME _stTime)
{
  return orxPLUGIN_BODY_CORE_FUNCTION_NAME(orxTime_Delay)(_stTime);
}

#endif /* _orxTIME_H_ */

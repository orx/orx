/** 
 * \file orxPhysics.h
 * 
 * Physics module
 * Handles physics & collision requests.
 * 
 * \todo
 */


/***************************************************************************
 orxPhysics.h
 Physics module
 
 begin                : 24/03/2008
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


#ifndef _orxPHYSICS_H_
#define _orxPHYSICS_H_


#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


/** Internal physics shape structure
 */
typedef struct __orxPHYSICS_SHAPE_t orxPHYSICS_SHAPE;

#define orxPHYSICS_KZ_CONFIG_SECTION      "Physics"
#define orxPHYSICS_KZ_CONFIG_GRAVITY      "Gravity"
#define orxPHYSICS_KZ_CONFIG_ALLOW_SLEEP  "AllowSleep"
#define orxPHYSICS_KZ_CONFIG_WORLD_LOWER  "WorldLowerBound"
#define orxPHYSICS_KZ_CONFIG_WORLD_UPPER  "WorldUpperBound"
#define orxPHYSICS_KZ_CONFIG_ITERATIONS   "IterationsPerStep"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Physics module setup */
extern orxDLLAPI orxVOID            orxPhysics_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxPhysics_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPhysics_Exit, orxVOID);


orxSTATIC orxINLINE orxSTATUS orxPhysics_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Init)();
}

orxSTATIC orxINLINE orxVOID orxPhysics_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPhysics_Exit)();
}


#endif /* _orxPHYSICS_H_ */

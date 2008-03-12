/**
 * \file orxSystem.c
 */

/***************************************************************************
 begin                : 26/05/2005
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


#include "core/orxSystem.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 orxSystem_Setup
 System module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_SYSTEM, orxMODULE_ID_TREE);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(SYSTEM)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, INIT, orxSystem_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, EXIT, orxSystem_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, GET_TIME, orxSystem_GetTime)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(SYSTEM, DELAY, orxSystem_Delay)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(SYSTEM)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_GetTime, orxFLOAT);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxSystem_Delay, orxVOID, orxFLOAT);

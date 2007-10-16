/**
 * \file orxTime.c
 */

/***************************************************************************
 begin                : 26/05/2005
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

#include "core/orxTime.h"
#include "plugin/orxPluginCore.h"


/***************************************************************************
 orxTime_Setup
 Time module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxTime_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_TIME, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_TIME, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_TIME, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_TIME, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_TIME, orxMODULE_ID_TREE);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(TIME)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, INIT, orxTime_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, EXIT, orxTime_Exit)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, GET_TIME, orxTime_GetTime)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(TIME, DELAY, orxTime_Delay)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(TIME)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxTime_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxTime_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxTime_GetTime, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxTime_Delay, orxVOID, orxU32);

/**
 * @file orxPhysics.c
 * 
 * Physics module
 * 
 */

 /***************************************************************************
 orxPhysics.c
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


#include "physics/orxPhysics.h"

#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Render module setup
 */
orxVOID orxPhysics_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_PHYSICS, orxMODULE_ID_CLOCK);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PHYSICS)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, INIT, orxPhysics_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, EXIT, orxPhysics_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, CREATE_BODY, orxPhysics_CreateBody)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PHYSICS, DELETE_BODY, orxPhysics_DeleteBody)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(PHYSICS)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_CreateBody, orxPHYSICS_BODY *, orxCONST orxBODY_DEF *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPhysics_DeleteBody, orxVOID, orxPHYSICS_BODY *);

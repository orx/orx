/**
 * @file orxRender.c
 * 
 * Render module
 * 
 */

 /***************************************************************************
 orxRender.c
 Render module
 
 begin                : 25/09/2007
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


#include "render/orxRender.h"

#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Render module setup
 */
orxVOID orxRender_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_OBJECT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_VIEWPORT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_DISPLAY);

  return;
}


/********************
 *  Plugin Related  *
 ********************/

/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(RENDER)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, INIT, orxRender_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, EXIT, orxRender_Exit)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(RENDER)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Exit, orxVOID);

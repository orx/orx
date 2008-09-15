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
 * @file orxRender.c
 * @date 25/09/2007
 * @author iarwain@orx-project.org
 *
 * @todo
 */


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
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_FPS);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_OBJECT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_VIEWPORT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_DISPLAY);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Init, orxSTATUS, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Exit, orxVOID, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_GetWorldPosition, orxVECTOR *, orxCONST orxVECTOR *, orxVECTOR *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(RENDER)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, INIT, orxRender_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, EXIT, orxRender_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, GET_WORLD_POSITION, orxRender_GetWorldPosition)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(RENDER)


/* *** Core function implementations *** */

orxSTATUS orxRender_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Init)();
}

orxVOID orxRender_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Exit)();
}

orxVECTOR *orxRender_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_GetWorldPosition)(_pvScreenPosition, _pvWorldPosition);
}

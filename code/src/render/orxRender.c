/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxRender.c
 * @date 25/09/2007
 * @author iarwain@orx-project.org
 *
 */


#include "render/orxRender.h"

#include "plugin/orxPluginCore.h"


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Render module setup
 */
void orxFASTCALL orxRender_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_FPS);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_VIEWPORT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_SHADERPOINTER);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Exit, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_GetWorldPosition, orxVECTOR *, const orxVECTOR *, orxVECTOR *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(RENDER)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, INIT, orxRender_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, EXIT, orxRender_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, GET_WORLD_POSITION, orxRender_GetWorldPosition)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(RENDER)


/* *** Core function implementations *** */

orxSTATUS orxFASTCALL orxRender_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Init)();
}

void orxFASTCALL orxRender_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Exit)();
}

orxVECTOR *orxFASTCALL orxRender_GetWorldPosition(const orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_GetWorldPosition)(_pvScreenPosition, _pvWorldPosition);
}

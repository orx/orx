/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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

#include "core/orxCommand.h"


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Command: GetWorldPosition
 */
void orxFASTCALL orxRender_CommandGetWorldPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Gets world position */
  orxRender_GetWorldPosition(&(_astArgList[0].vValue), orxNULL, &(_pstResult->vValue));

  /* Done! */
  return;
}

/** Command: GetScreenPosition
 */
void orxFASTCALL orxRender_CommandGetScreenPosition(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Gets screen position */
  orxRender_GetScreenPosition(&(_astArgList[0].vValue), orxNULL, &(_pstResult->vValue));

  /* Done! */
  return;
}

/** Registers all the render commands
 */
static orxINLINE void orxRender_RegisterCommands()
{
  /* Command: GetWorldPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Render, GetWorldPosition, "WorldPos", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"ScreenPos", orxCOMMAND_VAR_TYPE_VECTOR});
  /* Command: GetScreenPosition */
  orxCOMMAND_REGISTER_CORE_COMMAND(Render, GetScreenPosition, "ScreenPos", orxCOMMAND_VAR_TYPE_VECTOR, 1, 0, {"WorldPos", orxCOMMAND_VAR_TYPE_VECTOR});
}

/** Unregisters all the render commands
 */
static orxINLINE void orxRender_UnregisterCommands()
{
  /* Command: GetWorldPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Render, GetWorldPosition);
  /* Command: GetScreenPosition */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Render, GetScreenPosition);
}


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
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_VIEWPORT);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_RENDER, orxMODULE_ID_SHADERPOINTER);

  orxModule_AddOptionalDependency(orxMODULE_ID_RENDER, orxMODULE_ID_CONSOLE);

  /* Done! */
  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Init, orxSTATUS, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_Exit, void, void);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_GetWorldPosition, orxVECTOR *, const orxVECTOR *, const orxVIEWPORT *, orxVECTOR *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxRender_GetScreenPosition, orxVECTOR *, const orxVECTOR *, const orxVIEWPORT *, orxVECTOR *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(RENDER)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, INIT, orxRender_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, EXIT, orxRender_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, GET_WORLD_POSITION, orxRender_GetWorldPosition)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(RENDER, GET_SCREEN_POSITION, orxRender_GetScreenPosition)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(RENDER)


/* *** Core function implementations *** */

orxSTATUS orxFASTCALL orxRender_Init()
{
  orxSTATUS eResult;

  /* Updates result */
  eResult = orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Init)();

  /* Success? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Registers all commands */
    orxRender_RegisterCommands();
  }

  /* Done! */
  return eResult;
}

void orxFASTCALL orxRender_Exit()
{
  /* Unregisters commands */
  orxRender_UnregisterCommands();

  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Exit)();
}

orxVECTOR *orxFASTCALL orxRender_GetWorldPosition(const orxVECTOR *_pvScreenPosition, const orxVIEWPORT *_pstViewport, orxVECTOR *_pvWorldPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_GetWorldPosition)(_pvScreenPosition, _pstViewport, _pvWorldPosition);
}

orxVECTOR *orxFASTCALL orxRender_GetScreenPosition(const orxVECTOR *_pvWorldPosition, const orxVIEWPORT *_pstViewport, orxVECTOR *_pvScreenPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_GetScreenPosition)(_pvWorldPosition, _pstViewport, _pvScreenPosition);
}

/**
 * \file orxRender.h
 *
 * Render Module.
 * Renders visible objects on screen, using active renders.
 *
 * \todo
 * Optimize viewport list handling.
 */


/***************************************************************************
 orxRender.h
 Render module

 begin                : 15/12/2003
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


#ifndef _orxRENDER_H_
#define _orxRENDER_H_


#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#include "object/orxObject.h"


/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Render module setup */
extern orxDLLAPI orxVOID                orxRender_Setup();

/** Gets a world position from a screen one
 * @param[in]  _pvScreenPosition        Screen space position
 * @param[out] _pvWorldPosition         Corresponding world position
 * @return orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL orxVector_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition);


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxRender_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRender_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxRender_GetWorldPosition, orxVECTOR *, orxCONST orxVECTOR *, orxVECTOR *);


orxSTATIC orxINLINE orxSTATUS orxRender_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Init)();
}

orxSTATIC orxINLINE orxVOID orxRender_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_Exit)();
}

orxSTATIC orxINLINE orxVECTOR *orxRender_GetWorldPosition(orxCONST orxVECTOR *_pvScreenPosition, orxVECTOR *_pvWorldPosition)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxRender_GetWorldPosition)(_pvScreenPosition, _pvWorldPosition);
}


#endif /* _orxRENDER_H_ */

/**
 * @file orxBounce.c
 * 
 * Bounce demo
 * 
 */

 /***************************************************************************
 orxBounce.c
 Bounce demo
 
 begin                : 07/04/2008
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


#include "orx.h"


/** Inits the bounce demo
 */
orxSTATIC orxSTATUS orxBounce_Init()
{
  orxBODY_PART_DEF  stBodyPartTemplate;
  orxVIEWPORT      *pstViewport;
  orxCAMERA        *pstCamera;
  orxFLOAT          fScreenWidth, fScreenHeight;
  orxOBJECT        *pstObject;
  orxVECTOR         vPosition;
  orxSTATUS         eResult = orxSTATUS_SUCCESS;

  /* Gets screen dimensions */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Selects config section */
  orxConfig_SelectSection("Bounce");

  /* Inits and sets body part template */
  orxMemory_Set(&stBodyPartTemplate, 0, sizeof(orxBODY_PART_DEF));
  stBodyPartTemplate.fRestitution = orxFLOAT_1;
  stBodyPartTemplate.fFriction    = orxFLOAT_1;
  orxBody_SetPartTemplate(&stBodyPartTemplate);

  /* Creates ball */
  orxObject_CreateFromFile(orxConfig_GetString("BallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_SPHERE | orxOBJECT_KU32_FLAG_BODY_DYNAMIC);

  /* Creates the four walls */
//  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
//  orxVector_Set(&vPosition, orx2F(-0.5f) * fScreenWidth, orxFLOAT_0, orxFLOAT_0);
//  orxObject_SetPosition(pstObject, &vPosition);
//
//  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
//  orxVector_Set(&vPosition, orx2F(0.5f) * fScreenWidth, orxFLOAT_0, orxFLOAT_0);
//  orxObject_SetPosition(pstObject, &vPosition);
//
//  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
//  orxVector_Set(&vPosition, orxFLOAT_0, orx2F(-0.5f) * fScreenHeight, orxFLOAT_0);
//  orxObject_SetPosition(pstObject, &vPosition);
//
//  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
//  orxVector_Set(&vPosition, orxFLOAT_0, orx2F(0.5f) * fScreenHeight, orxFLOAT_0);
//  orxObject_SetPosition(pstObject, &vPosition);

  /* Creates & inits camera (using screen dimensions as frustrum will avoid scaling whichever resolution we are using) */
  pstCamera = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);
  orxCamera_SetFrustrum(pstCamera, fScreenWidth, fScreenHeight, orxFLOAT_0, orxFLOAT_1);

  /* Creates & inits viewport on screen */
  pstViewport = orxViewport_Create();
  orxViewport_SetCamera(pstViewport, pstCamera);

  /* Done! */
  return eResult;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(orxBounce_Init);

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

/** Update callback
 */
orxVOID orxFASTCALL orxBounce_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxSTATIC orxU32 su32Counter = 0;

  /* Clicking? */
  if((su32Counter < orxConfig_GetFloat("BallLimit")) && (orxMouse_IsButtonPressed(orxMOUSE_BUTTON_LEFT)))
  {
    orxS32      s32MouseX, s32MouseY;
    orxFLOAT    fScreenWidth, fScreenHeight;
    orxVECTOR   vPosition;
    orxOBJECT  *pstObject;

    /* Gets screen dimensions */
    orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

    /* Gets mouse coordinates */
    orxMouse_GetPosition(&s32MouseX, &s32MouseY);

    /* Gets on-screen position vector */
    orxVector_Set(&vPosition, orxS2F(s32MouseX) - orx2F(0.5f) * fScreenWidth, orxS2F(s32MouseY) - orx2F(0.5f) * fScreenHeight, orxFLOAT_0);

    /* Spawn a ball under the cursor with a random torque */
    pstObject = orxObject_CreateFromFile(orxConfig_GetString("BallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_SPHERE | orxOBJECT_KU32_FLAG_BODY_DYNAMIC);
    orxFLOAT f = orxFRAND(orxConfig_GetFloat("MinTorque"), orxConfig_GetFloat("MaxTorque"));
    orxObject_SetPosition(pstObject, &vPosition);
    orxObject_ApplyTorque(pstObject, f);

    /* Update counter */
    su32Counter++;
  }
}

/** Inits the bounce demo
 */
orxSTATIC orxSTATUS orxBounce_Init()
{
  orxBODY_PART_DEF  stBodyPartTemplate;
  orxVIEWPORT      *pstViewport;
  orxCAMERA        *pstCamera;
  orxCLOCK         *pstClock;
  orxFLOAT          fScreenWidth, fScreenHeight;
  orxOBJECT        *pstObject;
  orxVECTOR         vPosition;
  orxSTATUS         eResult;

  /* Gets screen dimensions */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Selects config section */
  orxConfig_SelectSection("Bounce");

  /* Inits and sets body part template */
  orxMemory_Set(&stBodyPartTemplate, 0, sizeof(orxBODY_PART_DEF));
  stBodyPartTemplate.fRestitution = orxConfig_GetFloat("Restitution");
  stBodyPartTemplate.fFriction    = orxConfig_GetFloat("Friction");
  orxBody_SetPartTemplate(&stBodyPartTemplate);

  /* Creates ball */
  pstObject = orxObject_CreateFromFile(orxConfig_GetString("BallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_SPHERE | orxOBJECT_KU32_FLAG_BODY_DYNAMIC);
  orxFLOAT f = orxFRAND(orxConfig_GetFloat("MinTorque"), orxConfig_GetFloat("MaxTorque"));
  orxObject_ApplyTorque(pstObject, f);

  /* Creates the four walls */
  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
  orxVector_Set(&vPosition, orx2F(-0.5f) * fScreenWidth, orxFLOAT_0, orxFLOAT_0);
  orxObject_SetPosition(pstObject, &vPosition);

  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
  orxVector_Set(&vPosition, orx2F(0.5f) * fScreenWidth, orxFLOAT_0, orxFLOAT_0);
  orxObject_SetPosition(pstObject, &vPosition);

  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
  orxVector_Set(&vPosition, orxFLOAT_0, orx2F(-0.5f) * fScreenHeight, orxFLOAT_0);
  orxObject_SetPosition(pstObject, &vPosition);
  orxObject_SetRotation(pstObject, orxMATH_KF_PI_BY_2);

  pstObject = orxObject_CreateFromFile(orxConfig_GetString("WallFile"), orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT | orxOBJECT_KU32_FLAG_BODY | orxOBJECT_KU32_FLAG_BODY_BOX);
  orxVector_Set(&vPosition, orxFLOAT_0, orx2F(0.5f) * fScreenHeight, orxFLOAT_0);
  orxObject_SetPosition(pstObject, &vPosition);
  orxObject_SetRotation(pstObject, orxMATH_KF_PI_BY_2);

  /* Creates & inits camera (using screen dimensions as frustrum will avoid scaling whichever resolution we are using) */
  pstCamera = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);
  orxCamera_SetFrustrum(pstCamera, fScreenWidth, fScreenHeight, orxFLOAT_0, orxFLOAT_1);

  /* Creates & inits viewport on screen */
  pstViewport = orxViewport_Create();
  orxViewport_SetCamera(pstViewport, pstCamera);

  /* Gets rendering clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_RENDER);

  /* Registers callback */
  eResult = orxClock_Register(pstClock, &orxBounce_Update, orxNULL, orxMODULE_ID_MAIN);

  /* Done! */
  return eResult;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(orxBounce_Init);

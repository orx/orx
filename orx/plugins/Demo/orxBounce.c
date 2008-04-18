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

  /* Selects config section */
  orxConfig_SelectSection("Bounce");

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

    /* Spawn a ball under the cursor */
    pstObject = orxObject_CreateFromConfig("Ball");
    orxObject_SetPosition(pstObject, &vPosition);

    /* Update counter */
    su32Counter++;
  }
}

/** Inits the bounce demo
 */
orxSTATIC orxSTATUS orxBounce_Init()
{
  orxCLOCK         *pstClock;
  orxFLOAT          fScreenWidth, fScreenHeight;
  orxSTATUS         eResult;

  /* Gets screen dimensions */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Loads config file and selects its section */
  orxConfig_Load("Bounce.ini");
  orxConfig_SelectSection("Bounce");
  orxConfig_Load(orxConfig_GetString("OverrideFile"));

  /* Creates ball */
  orxObject_CreateFromConfig("Ball");

  /* Creates walls */
  orxObject_CreateFromConfig("Wall1");
  orxObject_CreateFromConfig("Wall2");
  orxObject_CreateFromConfig("Wall3");
  orxObject_CreateFromConfig("Wall4");

  /* Creates viewport on screen */
  orxViewport_CreateFromConfig("BounceViewport");

  /* Gets rendering clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_RENDER);

  /* Registers callback */
  eResult = orxClock_Register(pstClock, &orxBounce_Update, orxNULL, orxMODULE_ID_MAIN);

  /* Done! */
  return eResult;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(orxBounce_Init);

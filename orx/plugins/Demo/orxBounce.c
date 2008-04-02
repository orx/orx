/**
 * @file orxBounce.c
 * 
 * Bounce demo
 * 
 */

 /***************************************************************************
 orxBounce.c
 Bounce demo
 
 begin                : 14/10/2007
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

/** Defines
 */
#define orxBOUNCE_KU32_TICK_SIZE              orxFLOAT_0


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxBOUNCE_STATIC_t
{
  orxCLOCK     *pstClock;
  orxOBJECT    *pstObject;
  orxCAMERA    *pstCamera;
  orxVIEWPORT  *pstViewport;

  orxVECTOR     vSpeed;

  orxFLOAT      fScreenWidth, fScreenHeight, fBallSide;

} orxBOUNCE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxBOUNCE_STATIC sstBounce;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxVOID orxFASTCALL orxBounce_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR vPos, vDiff;
  orxFLOAT fRot;

  orxObject_GetPosition(sstBounce.pstObject, &vPos);
  fRot = orxObject_GetRotation(sstBounce.pstObject);

  if(vPos.fY > sstBounce.fScreenHeight - (orx2F(0.5f) * sstBounce.fBallSide))
  {
    sstBounce.vSpeed.fY *= orx2F(-1.0f);
    vPos.fY = sstBounce.fScreenHeight - (orx2F(0.5f) * sstBounce.fBallSide);
  }

  sstBounce.vSpeed.fY += orx2F(1.0f);

  if((vPos.fX > sstBounce.fScreenWidth - (orx2F(0.5f) * sstBounce.fBallSide)) || (vPos.fX < (orx2F(0.5f) * sstBounce.fBallSide)))
  {
    sstBounce.vSpeed.fX *= orx2F(-1.0f);
  }

  fRot += sstBounce.vSpeed.fX * orx2F(0.01f);
  orxVector_Mulf(&vDiff, &sstBounce.vSpeed, orxFLOAT_1);
  orxVector_Add(&vPos, &vPos, &vDiff);

  orxObject_SetPosition(sstBounce.pstObject, &vPos);
  orxObject_SetRotation(sstBounce.pstObject, fRot);

  return;
}

orxSTATIC orxSTATUS orxBounce_Init()
{
  orxSTRING zFileName;
  orxSTATUS eResult;

  /* Cleans static controller */
  orxMemory_Set(&sstBounce, 0, sizeof(orxBOUNCE_STATIC));

  /* Selects config section */
  orxConfig_SelectSection("Bounce");

  /* Gets ball filename */
  zFileName = orxConfig_GetString("BallFile");

  /* Creates ball */
  sstBounce.pstObject = orxObject_CreateFromFile(zFileName, orxOBJECT_KU32_FLAG_2D | orxOBJECT_KU32_FLAG_GRAPHIC | orxOBJECT_KU32_FLAG_CENTERED_PIVOT);

  /* Valid? */
  if(sstBounce.pstObject != orxNULL)
  {
    orxVECTOR vCameraPosition, vObjectPosition;
    orxU32    u32ScreenWidth, u32ScreenHeight;

    /* Gets dimensions */
    orxObject_GetSize(sstBounce.pstObject, &sstBounce.fBallSide, &sstBounce.fBallSide);
    orxDisplay_GetBitmapSize(orxDisplay_GetScreenBitmap(), &u32ScreenWidth, &u32ScreenHeight);
    sstBounce.fScreenWidth  = orxU2F(u32ScreenWidth);
    sstBounce.fScreenHeight = orxU2F(u32ScreenHeight);

    /* Sets all vectors */
    orxVector_Set(&vObjectPosition, orx2F(0.5f) * sstBounce.fScreenWidth, orx2F(0.5f) * sstBounce.fScreenHeight, orx2F(2.0f));
    orxVector_Set(&sstBounce.vSpeed, orx2F(10.0f), orxFLOAT_0, orxFLOAT_0);

    /* Sets camera position */
    orxVector_Set(&vCameraPosition, orx2F(0.5f) * sstBounce.fScreenWidth, orx2F(0.5f) * sstBounce.fScreenHeight, orxFLOAT_0); 

    /* Creates & inits camera */
    sstBounce.pstCamera = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);
    orxCamera_SetFrustrum(sstBounce.pstCamera, sstBounce.fScreenWidth, sstBounce.fScreenHeight, orx2F(2.0f), orx2F(100.0f));
    orxCamera_SetPosition(sstBounce.pstCamera, &vCameraPosition);

    /* Creates & inits viewport on screen */
    sstBounce.pstViewport = orxViewport_Create();
    orxViewport_SetCamera(sstBounce.pstViewport, sstBounce.pstCamera);
    orxViewport_SetRelativeSize(sstBounce.pstViewport, orxFLOAT_1, orxFLOAT_1);
    orxViewport_SetPosition(sstBounce.pstViewport, 0, 0);

    /* Updates object position */
    orxObject_SetPosition(sstBounce.pstObject, &vObjectPosition);

    /* Creates rendering clock */
    sstBounce.pstClock = orxClock_Create(orxBOUNCE_KU32_TICK_SIZE, orxCLOCK_TYPE_USER);

    /* Registers rendering function */
    eResult = orxClock_Register(sstBounce.pstClock, orxBounce_Update, orxNULL, orxMODULE_ID_RENDER);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_DECLARE_ENTRY_POINT(orxBounce_Init);

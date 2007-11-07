/**
 * @file orxCave.c
 * 
 * Cave demo
 * 
 */

 /***************************************************************************
 orxCave.c
 Cave demo
 
 begin                : 07/11/2007
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
#define orxCAVE_KZ_DATA_FOLDER              "data\\cave\\"
#define orxCAVE_KF_SPEED                    orx2F(180.0f)
#define orxCAVE_KZ_BACKGROUND               orxCAVE_KZ_DATA_FOLDER"background.png"
#define orxCAVE_KZ_BACKGROUND_SHADOW        orxCAVE_KZ_DATA_FOLDER"background-shadow.png"
#define orxCAVE_KZ_BALL                     orxCAVE_KZ_DATA_FOLDER"ball.png"
#define orxCAVE_KZ_BALL_REFLECT             orxCAVE_KZ_DATA_FOLDER"ball-reflect.png"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Resource enum
 */
typedef enum __orxCAVE_RESOURCE_t
{
  orxCAVE_RESOURCE_BACKGROUND = 0,
  orxCAVE_RESOURCE_BACKGROUND_SHADOW,
  orxCAVE_RESOURCE_BALL,
  orxCAVE_RESOURCE_BALL_REFLECT,

  orxCAVE_RESOURCE_NUMBER,

  orxCAVE_RESOURCE_NONE = orxENUM_NONE

} orxCAVE_RESOURCE;

/** Resource info structure 
 */
typedef struct __orxCAVE_RESOURCE_INFO_t
{
  orxSTRING zFileName;
  orxFLOAT  fZ;

} orxCAVE_RESOURCE_INFO;

/** Resource data structure
 */
typedef struct __orxCAVE_RESOURCE_DATA_t
{
  orxGRAPHIC   *pstGraphic;
  orxTEXTURE   *pstTexture;
  orxOBJECT    *pstObject;

} orxCAVE_RESOURCE_DATA;

/** Static structure
 */
typedef struct __orxCAVE_STATIC_t
{
  orxCAMERA                *pstMainCamera, *pstTVCamera;
  orxVIEWPORT              *pstMainViewport, *pstTVViewport;
  orxFRAME                 *pstBallFrame;

  orxCAVE_RESOURCE_DATA     astData[orxCAVE_RESOURCE_NUMBER];

  orxFLOAT                  fScreenWidth, fScreenHeight, fBackgroundWidth, fBackgroundHeight;

} orxCAVE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxCAVE_STATIC sstCave;

/** Resource info
 */
orxSTATIC orxCAVE_RESOURCE_INFO sastInfo[orxCAVE_RESOURCE_NUMBER] =
{
    {
      orxCAVE_KZ_DATA_FOLDER"background.png",
      orx2F(1.0f)
    },
    {
      orxCAVE_KZ_DATA_FOLDER"background-shadow.png",
      orx2F(0.0f)
    },
    {
      orxCAVE_KZ_DATA_FOLDER"ball.png",
      orx2F(0.5f)
    },
    {
      orxCAVE_KZ_DATA_FOLDER"ball-reflect.png",
      orx2F(0.1f)
    }
};


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxVOID orxFASTCALL orxCave_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR vPos;
  orxFRAME *pstFrame;

  /* Gets ball texture frame */
  pstFrame = (orxFRAME *)orxObject_GetStructure(sstCave.astData[orxCAVE_RESOURCE_BALL].pstObject, orxSTRUCTURE_ID_FRAME);

  /* Updates its rotation */
  orxFrame_SetRotation(pstFrame, orx2F(4.0f) * _pstClockInfo->fTime);

  /* Updates main ball frame */
  orxFrame_GetPosition(sstCave.pstBallFrame, orxFRAME_SPACE_LOCAL, &vPos);
  vPos.fX += 100.0f * _pstClockInfo->fDT;
  orxFrame_SetPosition(sstCave.pstBallFrame, &vPos);

  return;
}

orxSTATIC orxSTATUS orxCave_Init()
{
  orxU32      u32ScreenWidth, u32ScreenHeight, i;
  orxVECTOR   vPos;
  orxFRAME   *pstFrame;
  orxSTATUS   eResult;

  /* Cleans static controller */
  orxMemory_Set(&sstCave, 0, sizeof(orxCAVE_STATIC));

  /* Gets screen dimensions */
  orxDisplay_GetBitmapSize(orxDisplay_GetScreenBitmap(), &u32ScreenWidth, &u32ScreenHeight);
  sstCave.fScreenWidth  = orxU2F(u32ScreenWidth);
  sstCave.fScreenHeight = orxU2F(u32ScreenHeight);

  /* Creates ball frame */
  sstCave.pstBallFrame  = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

  /* For all resources */
  for(i = 0; i < orxCAVE_RESOURCE_NUMBER; i++)
  {
    /* Loads textures */
    sstCave.astData[i].pstTexture = orxTexture_CreateFromFile(sastInfo[i].zFileName);

    /* Valid? */
    if(sstCave.astData[i].pstTexture != orxNULL)
    {
      orxVECTOR vPivot;
      orxFLOAT  fWidth, fHeight;

      /* Gets texture size */
      orxTexture_GetSize(sstCave.astData[i].pstTexture, &fWidth, &fHeight);

      /* Creates its frame */
      pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

      /* Is it a ball? */
      if(i >= orxCAVE_RESOURCE_BALL)
      {
        /* Sets the ball frame as parent */
        orxFrame_SetParent(pstFrame, sstCave.pstBallFrame);

        /* Inits pivot */
        orxVector_Set(&vPivot, orx2F(0.5f) * fWidth, orx2F(0.5f) * fHeight, orxFLOAT_0);
      }
      else
      {
        /* Inits pivot */
        orxVector_Copy(&vPivot, &orxVECTOR_0);
      }

      /* Creates & inits 2D graphic objet from texture */
      sstCave.astData[i].pstGraphic = orxGraphic_Create();
      orxGraphic_SetData(sstCave.astData[i].pstGraphic, (orxSTRUCTURE *)sstCave.astData[i].pstTexture);
      orxGraphic_SetPivot(sstCave.astData[i].pstGraphic, &vPivot);

      /* Updates frame position */
      orxVector_Set(&vPos, orxFLOAT_0, orxFLOAT_0, sastInfo[i].fZ);
      orxFrame_SetPosition(pstFrame, &vPos);

      /* Creates & inits object */
      sstCave.astData[i].pstObject = orxObject_Create();
      orxObject_LinkStructure(sstCave.astData[i].pstObject, (orxSTRUCTURE *)sstCave.astData[i].pstGraphic);
      orxObject_LinkStructure(sstCave.astData[i].pstObject, (orxSTRUCTURE *)pstFrame);
    }
  }

  /* Creates main camera */
  sstCave.pstMainCamera = orxCamera_Create();

  /* Sets camera frustrum */
  orxCamera_SetFrustrum(sstCave.pstMainCamera, sstCave.fScreenWidth, sstCave.fScreenHeight, orxFLOAT_0, orxFLOAT_1);

  /* Gets background size */
  orxTexture_GetSize(sstCave.astData[orxCAVE_RESOURCE_BACKGROUND].pstTexture, &(sstCave.fBackgroundWidth), &(sstCave.fBackgroundHeight));

  /* Sets camera position */
  orxVector_Set(&vPos, orx2F(0.5f) * sstCave.fScreenWidth, orx2F(0.5f) * sstCave.fBackgroundHeight, orxFLOAT_0);
  orxCamera_SetPosition(sstCave.pstMainCamera, &vPos);
  
  /* Creates & inits main viewport */
  sstCave.pstMainViewport = orxViewport_Create();
  orxViewport_SetCamera(sstCave.pstMainViewport, sstCave.pstMainCamera);
  orxViewport_SetRelativeSize(sstCave.pstMainViewport, orxFLOAT_1, orxFLOAT_1);
  orxViewport_SetRelativePosition(sstCave.pstMainViewport, orxVIEWPORT_KU32_FLAG_ALIGN_CENTER);

  /* Creates & inits TV viewport */
  sstCave.pstTVViewport = orxViewport_Create();
//  orxViewport_SetCamera(sstCave.pstTVViewport, sstCave.pstMainCamera);
  orxViewport_SetSize(sstCave.pstTVViewport, 132.0f, 80.0f);
  orxViewport_SetPosition(sstCave.pstTVViewport, orx2F(442.0f) + (orx2F(0.5f) * (sstCave.fScreenHeight - sstCave.fBackgroundHeight)), orx2F(266.0f));

  /* Registers update function on render clock */
  eResult = orxClock_Register(orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_RENDER), orxCave_Update, orxNULL, orxMODULE_ID_MAIN);

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

extern orxDLLEXPORT orxSTATUS orxPLUGIN_K_INIT_FUNCTION_NAME(orxS32 *_ps32Number, orxPLUGIN_USER_FUNCTION_INFO **_ppstInfo)
{
  orxSTATUS eResult;

  /* Inits bounce */
  eResult = orxCave_Init();

  /* Updates parameters */
  *_ps32Number  = 0;
  *_ppstInfo    = orxNULL; 
  
  /* Done! */
  return eResult;
}

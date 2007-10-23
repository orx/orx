/**
 * @file orxScroll.c
 * 
 * Scrolling demo
 * 
 */

 /***************************************************************************
 orxScroll.c
 Scrolling demo
 
 begin                : 22/10/2007
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
#define orxSCROLL_KU32_TICK_SIZE              10


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Resource enum
 */
typedef enum __orxSCROLL_RESOURCE_t
{
  orxSCROLL_RESOURCE_FUJI = 0,
  orxSCROLL_RESOURCE_CANYON1,
  orxSCROLL_RESOURCE_CANYON2,
  orxSCROLL_RESOURCE_CLOUD,
  orxSCROLL_RESOURCE_WAVE,

  orxSCROLL_RESOURCE_NUMBER,

  orxSCROLL_RESOURCE_NONE = orxENUM_NONE

} orxSCROLL_RESOURCE;

/** Resource info structure 
 */
typedef struct __orxSCROLL_RESOURCE_INFO_t
{
  orxSTRING zFileName;
  orxVECTOR vMin, vMax;

} orxSCROLL_RESOURCE_INFO;

/** Resource data structure
 */
typedef struct __orxSCROLL_RESOURCE_DATA_t
{
  orxGRAPHIC   *pstGraphic;
  orxTEXTURE   *pstTexture;

} orxSCROLL_RESOURCE_DATA;

/** Static structure
 */
typedef struct __orxSCROLL_STATIC_t
{
  orxCLOCK                 *pstClock;
  orxCAMERA                *pstCamera;
  orxVIEWPORT              *pstViewport;

  orxSCROLL_RESOURCE_DATA   astData[orxSCROLL_RESOURCE_NUMBER];

  orxFLOAT                  fScreenWidth, fScreenHeight, fScrollSpeed;

} orxSCROLL_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxSCROLL_STATIC sstScroll;

/** Resource info
 */
orxSTATIC orxSCROLL_RESOURCE_INFO sastInfo[orxSCROLL_RESOURCE_NUMBER] =
{
    {
        "fuji.png",
        {orx2F(0.0f), orx2F(600.0f), orx2F(100.0f)},
        {orx2F(10000.0f), orx2F(900.0f), orx2F(200.0f)}
    },
    {
        "canyon1.png",
        {orx2F(0.0f), orx2F(600.0f), orx2F(100.0f)},
        {orx2F(10000.0f), orx2F(650.0f), orx2F(200.0f)}
    },
    {
        "canyon2.png",
        {orx2F(0.0f), orx2F(600.0f), orx2F(100.0f)},
        {orx2F(10000.0f), orx2F(750.0f), orx2F(200.0f)}
    },
    {
        "cloud.png",
        {orx2F(0.0f), orx2F(50.0f), orx2F(100.0f)},
        {orx2F(10000.0f), orx2F(300.0f), orx2F(200.0f)}
    },
    {
        "wave.png",
        {orx2F(0.0f), orx2F(600.0f), orx2F(100.0f)},
        {orx2F(10000.0f), orx2F(800.0f), orx2F(200.0f)}
    }
};


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxVOID orxFASTCALL orxScroll_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxFRAME *pstFrame;
  orxVECTOR vPos;

  pstFrame = orxCamera_GetFrame(sstScroll.pstCamera);

  orxFrame_GetPosition(pstFrame, orxFRAME_SPACE_LOCAL, &vPos);

  vPos.fX += orx2F(.1f) * orxU2F(_pstClockInfo->u32StableDT);
  
  orxFrame_SetPosition(pstFrame, &vPos);

  return;
}

orxSTATIC orxSTATUS orxScroll_Init()
{
  orxU32    u32ScreenWidth, u32ScreenHeight, i;
  orxSTATUS eResult;

  /* Cleans static controller */
  orxMemory_Set(&sstScroll, 0, sizeof(orxSCROLL_STATIC));

  /* Gets screen dimensions */
  orxDisplay_GetBitmapSize(orxDisplay_GetScreenBitmap(), &u32ScreenWidth, &u32ScreenHeight);
  sstScroll.fScreenWidth  = orxU2F(u32ScreenWidth);
  sstScroll.fScreenHeight = orxU2F(u32ScreenHeight);

  /* For all resources */
  for(i = 0; i < orxSCROLL_RESOURCE_NUMBER; i++)
  {
    orxU32 j;

    /* Loads textures */
    sstScroll.astData[i].pstTexture = orxTexture_CreateFromFile(sastInfo[i].zFileName);

    /* Valid? */
    if(sstScroll.astData[i].pstTexture != orxNULL)
    {
      orxVECTOR vPivot;
      orxFLOAT  fWidth, fHeight;

      /* Gets texture size */
      orxTexture_GetSize(sstScroll.astData[i].pstTexture, &fWidth, &fHeight);

      /* Inits pivot */
      orxVector_Set(&vPivot, orxFLOAT_0, fHeight, orxFLOAT_0); 

      /* Sets it transparent color */
      orxDisplay_SetBitmapColorKey(orxTexture_GetBitmap(sstScroll.astData[i].pstTexture), orx2RGBA(0xFF, 0x40, 0xFF, 0x00), orxTRUE);

      /* Creates & inits 2D graphic objet from texture */
      sstScroll.astData[i].pstGraphic = orxGraphic_Create();
      orxGraphic_SetData(sstScroll.astData[i].pstGraphic, (orxSTRUCTURE *)sstScroll.astData[i].pstTexture);
      orxGraphic_SetPivot(sstScroll.astData[i].pstGraphic, &vPivot);
    }

    for(j = 0; j < 10; j++)
    {
      orxVECTOR   vPos;
      orxOBJECT  *pstObject;
      orxFRAME   *pstFrame;

      /* Inits position */
      orxVector_Set(&vPos, orxU2F(j) * orx2F(1000.0f), sstScroll.fScreenHeight, orx2F(10.0f) - orxU2F(i << 1));

      /* Creates & inits frame  with X differential scrolling */
      pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_SCROLL_X);
      orxFrame_SetPosition(pstFrame, &vPos);

      /* Creates & inits object */
      pstObject = orxObject_Create();
      orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)sstScroll.astData[i].pstGraphic);
      orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstFrame);
    }
  }

  /* Success? */
  if(i == orxSCROLL_RESOURCE_NUMBER)
  {
    orxVECTOR vCameraUL, vCameraBR;

    /* Inits camera vectors */
    orxVector_Set(&vCameraUL, orxFLOAT_0, orxFLOAT_0, orxFLOAT_0);
    orxVector_Set(&vCameraBR, sstScroll.fScreenWidth, sstScroll.fScreenHeight, orx2F(10.0f));

    /* Creates & inits camera */
    sstScroll.pstCamera = orxCamera_Create();
    orxCamera_SetFrustrum(sstScroll.pstCamera, &vCameraUL, &vCameraBR);

    /* Creates & inits viewport on screen */
    sstScroll.pstViewport = orxViewport_Create();
    orxViewport_SetCamera(sstScroll.pstViewport, sstScroll.pstCamera);
    orxViewport_SetRelativeSize(sstScroll.pstViewport, 1.0f, 1.0f);
    orxViewport_SetRelativePosition(sstScroll.pstViewport, orxVIEWPORT_KU32_FLAG_ALIGN_CENTER);

    /* Creates rendering clock */
    sstScroll.pstClock = orxClock_Create(orxSCROLL_KU32_TICK_SIZE, orxCLOCK_TYPE_USER);

    /* Registers update function */
    eResult = orxClock_Register(sstScroll.pstClock, orxScroll_Update, orxNULL, orxMODULE_ID_MAIN);
  }

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
  eResult = orxScroll_Init();

  /* Updates parameters */
  *_ps32Number  = 0;
  *_ppstInfo    = orxNULL; 
  
  /* Done! */
  return eResult;
}

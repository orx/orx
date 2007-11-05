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
#define orxSCROLL_KU32_TICK_SIZE              orxFLOAT_0
#define orxSCROLL_KF_MAX_Z                    orx2F(10000.0f)
#define orxSCROLL_KF_SPEED                    orx2F(180.0f)
#define orxSCROLL_KZ_BACKGROUND_NAME          "background.png"
#define orxSCROLL_KU32_WAVE_FRAME_NUMBER      4
#define orxSCROLL_KF_WAVE_ANGULAR_VELOCITY    (orx2F(0.5f) * orxMATH_KF_PI)
#define orxSCROLL_KF_WAVE_PHASIS              (orx2F(2.0f) * orxMATH_KF_PI / orx2F(orxSCROLL_KU32_WAVE_FRAME_NUMBER))


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
  orxU32    u32Number;

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
  orxOBJECT                *pstBackgroundObject;
  orxVIEWPORT              *pstViewport;
  orxFRAME                 *apstWaveFrame[orxSCROLL_KU32_WAVE_FRAME_NUMBER];

  orxSCROLL_RESOURCE_DATA   astData[orxSCROLL_RESOURCE_NUMBER];

  orxFLOAT                  fScreenWidth, fScreenHeight, fScrollSpeed, fViewportSize, fWaveAmplitude;

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
        {orx2F(0.0f), orx2F(1.1f), orx2F(1.0f)},
        {orx2F(11.0f), orx2F(1.3f), orx2F(1.0f)},
        80
    },
    {
        "boat1.png",
        {orx2F(0.0f), orx2F(1.0f), orx2F(0.6f)},
        {orx2F(11.0f), orx2F(1.25f), orx2F(0.8f)},
        15
    },
    {
        "boat2.png",
        {orx2F(0.0f), orx2F(1.0f), orx2F(0.2f)},
        {orx2F(11.0f), orx2F(1.15f), orx2F(0.5f)},
        20
    },
    {
        "cloud.png",
        {orx2F(0.0f), orx2F(0.1f), orx2F(0.2f)},
        {orx2F(11.0f), orx2F(0.7f), orx2F(0.7f)},
        70
    },
    {
        "wave.png",
        {orx2F(0.0f), orx2F(1.3f), orx2F(0.2f)},
        {orx2F(11.0f), orx2F(1.6f), orx2F(0.8f)},
        300
    }
};


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxVOID orxFASTCALL orxScroll_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR vPos;
  orxU32    i;

  /*** Camera update ***/

  /* Gets camera position */
  orxCamera_GetPosition(sstScroll.pstCamera, &vPos);

  /* Should reverse? */
  if(((vPos.fX < sstScroll.fScreenWidth) && (sstScroll.fScrollSpeed < orxFLOAT_0))
  || ((vPos.fX > orx2F(10.0f) * sstScroll.fScreenWidth) && (sstScroll.fScrollSpeed > orxFLOAT_0)))
  {
    /* Negates speed */
    sstScroll.fScrollSpeed *= orx2F(-1.0f);
  }

  /* Updates position vector */
  vPos.fX += sstScroll.fScrollSpeed * _pstClockInfo->fDT;

  /* Updates camera position */
  orxCamera_SetPosition(sstScroll.pstCamera, &vPos);

  /*** Updates waves ***/

  /* For all main wave frames */
  for(i = 0; i < orxSCROLL_KU32_WAVE_FRAME_NUMBER; i++)
  {
    /* Gets frame local position */
    orxFrame_GetPosition(sstScroll.apstWaveFrame[i], orxFRAME_SPACE_LOCAL, &vPos);

    /* Updates its Y coordinate along a sine with initial phasis */
    vPos.fY = sstScroll.fWaveAmplitude * sinf((orxU2F(i) * orxSCROLL_KF_WAVE_PHASIS) + (orxSCROLL_KF_WAVE_ANGULAR_VELOCITY * orxU2F(_pstClockInfo->fTime)));

    /* Applies it */
    orxFrame_SetPosition(sstScroll.apstWaveFrame[i], &vPos);
  }

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

  /* Inits wave amplitude */
  sstScroll.fWaveAmplitude = orx2F(0.8f) * sstScroll.fScreenHeight * (sastInfo[orxSCROLL_RESOURCE_WAVE].vMax.fY - sastInfo[orxSCROLL_RESOURCE_WAVE].vMin.fY);

  /* Creates main wave frame */
  for(i = 0; i < orxSCROLL_KU32_WAVE_FRAME_NUMBER; i++)
  {
    orxVECTOR vPos;

    /* Creates frame */
    sstScroll.apstWaveFrame[i] = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

    /* Inits position vector */
    orxVector_Set(&vPos, orxFLOAT_0, (orx2F(i) / orx2F(orxSCROLL_KU32_WAVE_FRAME_NUMBER)) * sstScroll.fWaveAmplitude, orxFLOAT_0);

    /* Updates its position */
    orxFrame_SetPosition(sstScroll.apstWaveFrame[i], &vPos);
  }
  
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

      /* Creates & inits 2D graphic objet from texture */
      sstScroll.astData[i].pstGraphic = orxGraphic_Create();
      orxGraphic_SetData(sstScroll.astData[i].pstGraphic, (orxSTRUCTURE *)sstScroll.astData[i].pstTexture);
      orxGraphic_SetPivot(sstScroll.astData[i].pstGraphic, &vPivot);
    }

    /* For all requested instances */
    for(j = 0; j < sastInfo[i].u32Number; j++)
    {
      orxVECTOR   vPos;
      orxOBJECT  *pstObject;
      orxFRAME   *pstFrame;

      /* Inits position */
      orxVector_Set(&vPos,
                    sstScroll.fScreenWidth * orxFRAND(sastInfo[i].vMin.fX, sastInfo[i].vMax.fX),
                    sstScroll.fScreenHeight * orxFRAND(sastInfo[i].vMin.fY, sastInfo[i].vMax.fY),
                    orxSCROLL_KF_MAX_Z * orxFRAND(sastInfo[i].vMin.fZ, sastInfo[i].vMax.fZ));

      /* Creates frame with X differential scrolling */
      pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_SCROLL_X);

      /* Are we creating waves? */
      if(i == orxSCROLL_RESOURCE_WAVE)
      {
        /* Sets one of the main wave frame as parent */
        orxFrame_SetParent(pstFrame, sstScroll.apstWaveFrame[j % orxSCROLL_KU32_WAVE_FRAME_NUMBER]);
      }

      /* Updates frame position */
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
    orxVECTOR   vPosition;
    orxTEXTURE *pstBackgroundTexture;

    /* Sets camera position */
    orxVector_Set(&vPosition, orx2F(0.5f) * sstScroll.fScreenWidth, orx2F(0.5f) * sstScroll.fScreenHeight, orxFLOAT_0); 

    /* Creates & inits camera */
    sstScroll.pstCamera = orxCamera_Create();
    orxCamera_SetFrustrum(sstScroll.pstCamera, sstScroll.fScreenWidth, sstScroll.fScreenHeight, orxFLOAT_0, orxSCROLL_KF_MAX_Z);
    orxCamera_SetPosition(sstScroll.pstCamera, &vPosition);

    /* Sets camera speed */
    sstScroll.fScrollSpeed = orxSCROLL_KF_SPEED;
    
    /* Creates & inits viewport on screen */
    sstScroll.fViewportSize = orxFLOAT_1;
    sstScroll.pstViewport   = orxViewport_Create();
    orxViewport_SetCamera(sstScroll.pstViewport, sstScroll.pstCamera);
    orxViewport_SetRelativeSize(sstScroll.pstViewport, sstScroll.fViewportSize, sstScroll.fViewportSize);
    orxViewport_SetRelativePosition(sstScroll.pstViewport, orxVIEWPORT_KU32_FLAG_ALIGN_CENTER);

    /* Creates rendering clock */
    sstScroll.pstClock = orxClock_Create(orxSCROLL_KU32_TICK_SIZE, orxCLOCK_TYPE_USER);

    /* Registers update function */
    eResult = orxClock_Register(sstScroll.pstClock, orxScroll_Update, orxNULL, orxMODULE_ID_MAIN);

    /* Loads background texture */
    pstBackgroundTexture = orxTexture_CreateFromFile(orxSCROLL_KZ_BACKGROUND_NAME);

    /* Valid? */
    if(pstBackgroundTexture != orxNULL)
    {
      orxVECTOR   vBackgroundPos;
      orxFRAME   *pstBackgroundFrame;
      orxGRAPHIC *pstBackgroundGraphic;
      orxFLOAT    fBackgroundWidth, fBackgroundHeight;

      /* Gets texture size */
      orxTexture_GetSize(pstBackgroundTexture, &fBackgroundWidth, &fBackgroundHeight);

      /* Creates & inits 2D graphic objet from texture */
      pstBackgroundGraphic = orxGraphic_Create();
      orxGraphic_SetData(pstBackgroundGraphic, (orxSTRUCTURE *)pstBackgroundTexture);

      /* Creates background frame */
      pstBackgroundFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

      /* Links it to camera frame */
      orxFrame_SetParent(pstBackgroundFrame, orxCamera_GetFrame(sstScroll.pstCamera));

      /* Sets its local position */
      orxVector_Set(&vBackgroundPos, orx2F(-0.5f) * sstScroll.fScreenWidth, orx2F(-0.5f) * sstScroll.fScreenHeight, orxSCROLL_KF_MAX_Z);
      orxFrame_SetPosition(pstBackgroundFrame, &vBackgroundPos);

      /* Sets its scale */
      orxFrame_SetScale(pstBackgroundFrame, sstScroll.fScreenWidth / fBackgroundWidth, sstScroll.fScreenHeight / fBackgroundHeight);

      /* Creates & inits object */
      sstScroll.pstBackgroundObject = orxObject_Create();
      orxObject_LinkStructure(sstScroll.pstBackgroundObject, (orxSTRUCTURE *)pstBackgroundGraphic);
      orxObject_LinkStructure(sstScroll.pstBackgroundObject, (orxSTRUCTURE *)pstBackgroundFrame);
    }
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

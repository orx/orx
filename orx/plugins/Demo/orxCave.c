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
#define orxCAVE_KZ_DATA_FOLDER              "data/cave/"
#define orxCAVE_KF_CAMERA_SPEED             orx2F(120.0f)
#define orxCAVE_KF_BALL_SPEED               orx2F(180.0f)
#define orxCAVE_KF_BALL_ANGULAR_VELOCITY    orx2F(-5.0f)
#define orxCAVE_KF_FADE_IN_SPEED            orxFLOAT_1
#define orxCAVE_KF_BULB_KEY_DURATION        orx2F(0.03f)


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Resource enum
 */
typedef enum __orxCAVE_RESOURCE_t
{
  orxCAVE_RESOURCE_BACKGROUND = 0,
  orxCAVE_RESOURCE_BACKGROUND_SHADOW,
  orxCAVE_RESOURCE_BOX,
  orxCAVE_RESOURCE_BALL,
  orxCAVE_RESOURCE_BALL_REFLECT,
  orxCAVE_RESOURCE_BALL_SHADOW,

  orxCAVE_RESOURCE_BULB_FIRST,
  orxCAVE_RESOURCE_BULB_1 = orxCAVE_RESOURCE_BULB_FIRST,
  orxCAVE_RESOURCE_BULB_2,
  orxCAVE_RESOURCE_BULB_3,
  orxCAVE_RESOURCE_BULB_LAST = orxCAVE_RESOURCE_BULB_3,

  orxCAVE_RESOURCE_NUMBER,

  orxCAVE_RESOURCE_NONE = orxENUM_NONE

} orxCAVE_RESOURCE;

/** Execution step
 */
typedef enum __orxCAVE_STEP_t
{
  orxCAVE_STEP_FADE_IN = 0,
  orxCAVE_STEP_CAMERA_SCROLLING,
  orxCAVE_STEP_TRIGGER_TV,
  orxCAVE_STEP_BALL_SCROLLING,

  orxCAVE_STEP_NUMBER,

  orxCAVE_STEP_NONE = orxENUM_NONE

} orxCAVE_STEP;

/** Resource info structure
 */
typedef struct __orxCAVE_RESOURCE_INFO_t
{
  orxSTRING zFileName;
  orxVECTOR vInitialPos;
  orxVECTOR vRelativePivot;

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
  orxCLOCK                 *pstClock;

  orxCAVE_RESOURCE_DATA     astData[orxCAVE_RESOURCE_NUMBER];

  orxFLOAT                  fScreenWidth, fScreenHeight, fBackgroundWidth, fBackgroundHeight, fStepDelay;

  orxCAVE_STEP              eStep;

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
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(1.0f)}},
      {{orx2F(0.0f)}, {orx2F(0.85f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"background-shadow.png",
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}},
      {{orx2F(0.0f)}, {orx2F(0.85f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"box.png",
      {{orx2F(424.0f)}, {orx2F(-171.0f)}, {orx2F(0.1f)}},
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"ball.png",
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.3f)}},
      {{orx2F(0.5f)}, {orx2F(0.5f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"ball-reflect.png",
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.2f)}},
      {{orx2F(0.5f)}, {orx2F(0.5f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"ball-shadow.png",
      {{orx2F(-35.0f)}, {orx2F(32.0f)}, {orx2F(1.0f)}},
      {{orx2F(0.5f)}, {orx2F(0.5f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"bulb1.png",
      {{orx2F(324.0f)}, {orx2F(-400.0f)}, {orx2F(0.4f)}},
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"bulb2.png",
      {{orx2F(324.0f)}, {orx2F(-400.0f)}, {orx2F(0.4f)}},
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}}
    },
    {
      orxCAVE_KZ_DATA_FOLDER"bulb3.png",
      {{orx2F(324.0f)}, {orx2F(-400.0f)}, {orx2F(0.4f)}},
      {{orx2F(0.0f)}, {orx2F(0.0f)}, {orx2F(0.0f)}}
    }
};

/** Step delays
 */
orxSTATIC orxFLOAT sastStepDelay[orxCAVE_STEP_NUMBER] =
{
    orx2F(0.0f),
    orx2F(0.5f),
    orx2F(1.0f),
    orx2F(1.0f)
};

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

orxSTATIC orxINLINE orxSTATUS orxCave_UpdateMainCamera(orxFLOAT _fDT)
{
  orxVECTOR vPos;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Gets camera position */
  orxCamera_GetPosition(sstCave.pstMainCamera, &vPos);

  /* Updates its scrolling */
  vPos.fX -= orxCAVE_KF_CAMERA_SPEED * _fDT;

  /* Stop? */
  if(vPos.fX <= orx2F(0.5f) * sstCave.fScreenWidth)
  {
    /* Resets position */
    vPos.fX = orx2F(0.5f) * sstCave.fScreenWidth;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Updates camera position */
  orxCamera_SetPosition(sstCave.pstMainCamera, &vPos);

  /* Done! */
  return eResult;
}

orxSTATIC orxINLINE orxSTATUS orxCave_UpdateFadeIn(orxFLOAT _fDT)
{
  orxFLOAT  fRelativeSize;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Enables main viewport */
  orxViewport_Enable(sstCave.pstMainViewport, orxTRUE);

  /* Gets viewport relative size */
  orxViewport_GetRelativeSize(sstCave.pstMainViewport, &fRelativeSize, &fRelativeSize);

  /* Updates size */
  fRelativeSize += orxCAVE_KF_FADE_IN_SPEED * _fDT;

  /* Full size? */
  if(fRelativeSize >= orxFLOAT_1)
  {
    /* Updates size */
    fRelativeSize = orxFLOAT_1;

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Refreshs viewport */
  orxViewport_SetRelativeSize(sstCave.pstMainViewport, fRelativeSize, fRelativeSize);
  orxViewport_SetRelativePosition(sstCave.pstMainViewport, orxVIEWPORT_KU32_FLAG_ALIGN_CENTER);

  /* Done! */
  return eResult;
}

orxSTATIC orxINLINE orxSTATUS orxCave_UpdateBall(orxFLOAT _fDT)
{
  orxVECTOR   vPos;
  orxFLOAT    fRotation;
  orxFRAME   *pstFrame;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Gets ball texture frame */
  pstFrame = orxOBJECT_GET_STRUCTURE(sstCave.astData[orxCAVE_RESOURCE_BALL].pstObject, FRAME);

  /* Gets its rotation */
  fRotation = orxFrame_GetRotation(pstFrame, orxFRAME_SPACE_LOCAL);

  /* Updates it */
  fRotation += orxCAVE_KF_BALL_ANGULAR_VELOCITY * _fDT;

  /* Updates ball texture frame */
  orxFrame_SetRotation(pstFrame, fRotation);

  /* Gets TV camera frame position */
  orxCamera_GetPosition(sstCave.pstTVCamera, &vPos);

  /* Updates it */
  vPos.fX -= orxCAVE_KF_BALL_SPEED * _fDT;

  /* Arrived? */
  if(vPos.fX <= orx2F(620.0f))
  {
    /* Updates position */
    vPos.fX = orx2F(620.0f);

    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Updates TV camera */
  orxCamera_SetPosition(sstCave.pstTVCamera, &vPos);

  /* Done! */
  return eResult;
}

orxVOID orxFASTCALL orxCave_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxBOOL bNextStep = orxFALSE;

  /* Has to wait? */
  if(sstCave.fStepDelay > orxFLOAT_0)
  {
    /* Updates delay */
    sstCave.fStepDelay -= _pstClockInfo->fDT;
  }
  else
  {
    /* Depending on execution step */
    switch(sstCave.eStep)
    {
      case orxCAVE_STEP_FADE_IN:
      {
        /* Updates camera */
        bNextStep = (orxCave_UpdateFadeIn(_pstClockInfo->fDT) != orxSTATUS_SUCCESS);

        break;
      }

      case orxCAVE_STEP_CAMERA_SCROLLING:
      {
        /* Updates camera */
        bNextStep = (orxCave_UpdateMainCamera(_pstClockInfo->fDT) != orxSTATUS_SUCCESS);

        break;
      }

      case orxCAVE_STEP_TRIGGER_TV:
      {
        /* Enables TV viewport */
        orxViewport_Enable(sstCave.pstTVViewport, orxTRUE);

        /* Goes to next step */
        bNextStep = orxTRUE;

        break;
      }

      case orxCAVE_STEP_BALL_SCROLLING:
      {
        /* Updates camera */
        bNextStep = (orxCave_UpdateBall(_pstClockInfo->fDT) != orxSTATUS_SUCCESS);

        break;
      }
    }

    /* Updates step? */
    if(bNextStep != orxFALSE)
    {
      /* Increases step */
      sstCave.eStep++;

      /* Updates step delay */
      sstCave.fStepDelay = sastStepDelay[sstCave.eStep];
    }
  }

  return;
}

orxSTATIC orxVOID orxCave_InitBulb()
{
  orxOBJECT      *pstObject;
  orxFRAME       *pstFrame;
  orxANIMSET     *pstAnimSet;
  orxANIMPOINTER *pstAnimPointer;
  orxANIM        *pstAnim;
  orxHANDLE       hAnim;
  orxU32          i;

  /* Creates bulb object */
  pstObject   = orxObject_Create();

  /* Creates bulb's animation set */
  pstAnimSet  = orxAnimSet_Create(1);

  /* Creates bulb's animation */
  pstAnim     = orxAnim_Create(orxANIM_KU32_FLAG_2D, orxCAVE_RESOURCE_BULB_LAST - orxCAVE_RESOURCE_BULB_FIRST + 1);

  /* For all bulb resources */
  for(i = orxCAVE_RESOURCE_BULB_FIRST; i <= orxCAVE_RESOURCE_BULB_LAST; i++)
  {
    orxTEXTURE *pstTexture;
    orxGRAPHIC *pstGraphic;
    orxFLOAT    fWidth, fHeight;

    /* Loads textures */
    pstTexture = orxTexture_CreateFromFile(sastInfo[i].zFileName);

    /* Gets texture size */
    fWidth  = orxTexture_GetWidth(pstTexture);
    fHeight = orxTexture_GetHeight(pstTexture);

    /* Creates & inits 2D graphic objet from texture */
    pstGraphic = orxGraphic_Create(orxGRAPHIC_KU32_FLAG_2D);
    orxGraphic_SetData(pstGraphic, (orxSTRUCTURE *)pstTexture);
    orxGraphic_SetPivot(pstGraphic, &orxVECTOR_0);

    /* First resource? */
    if(i == orxCAVE_RESOURCE_BULB_FIRST)
    {
      /* Links it to object */
      orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstGraphic);
    }

    /* Adds it to animation */
    orxAnim_AddKey(pstAnim, (orxSTRUCTURE *)pstGraphic, orx2F(i - orxCAVE_RESOURCE_BULB_FIRST + 1) * orxCAVE_KF_BULB_KEY_DURATION);
  }

  /* Adds animation to animation set */
  hAnim = orxAnimSet_AddAnim(pstAnimSet, pstAnim);

  /* Loops it */
  orxAnimSet_AddLink(pstAnimSet, hAnim, hAnim);

  /* Creates bulb's animation pointer */
  pstAnimPointer = orxAnimPointer_Create(pstAnimSet);

  /* Creates its frame */
  pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

  /* Updates frame position */
  orxFrame_SetPosition(pstFrame, &(sastInfo[orxCAVE_RESOURCE_BULB_FIRST].vInitialPos));

  /* Links info to object */
  orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstAnimPointer);
  orxObject_LinkStructure(pstObject, (orxSTRUCTURE *)pstFrame);

  return;
}

orxSTATIC orxSTATUS orxCave_Init()
{
  orxU32      i;
  orxVECTOR   vPos;
  orxFRAME   *pstFrame;
  orxSTATUS   eResult;

  /* Cleans static controller */
  orxMemory_Zero(&sstCave, sizeof(orxCAVE_STATIC));

  /* Gets screen dimensions */
  orxDisplay_GetBitmapSize(orxDisplay_GetScreenBitmap(), &(sstCave.fScreenWidth), &(sstCave.fScreenHeight));

  /* For all resources */
  for(i = 0; i < orxCAVE_RESOURCE_NUMBER; i++)
  {
    /* Not a bulb animation? */
    if((i < orxCAVE_RESOURCE_BULB_FIRST)
    || (i > orxCAVE_RESOURCE_BULB_LAST))
    {
      /* Loads textures */
      sstCave.astData[i].pstTexture = orxTexture_CreateFromFile(sastInfo[i].zFileName);

      /* Valid? */
      if(sstCave.astData[i].pstTexture != orxNULL)
      {
        orxVECTOR vPivot;
        orxFLOAT  fWidth, fHeight;

        /* Gets texture size */
        fWidth  = orxTexture_GetWidth(sstCave.astData[i].pstTexture);
        fHeight = orxTexture_GetHeight(sstCave.astData[i].pstTexture);

        /* Creates its frame */
        pstFrame = orxFrame_Create(orxFRAME_KU32_FLAG_NONE);

        /* Inits pivot */
        orxVector_Set(&vPivot, fWidth, fHeight, orxFLOAT_0);
        orxVector_Mul(&vPivot, &vPivot, &(sastInfo[i].vRelativePivot));

        /* Creates & inits 2D graphic objet from texture */
        sstCave.astData[i].pstGraphic = orxGraphic_Create(orxGRAPHIC_KU32_FLAG_2D);
        orxGraphic_SetData(sstCave.astData[i].pstGraphic, (orxSTRUCTURE *)sstCave.astData[i].pstTexture);
        orxGraphic_SetPivot(sstCave.astData[i].pstGraphic, &vPivot);

         /* Updates frame position */
        orxFrame_SetPosition(pstFrame, &(sastInfo[i].vInitialPos));

        /* Creates & inits object */
        sstCave.astData[i].pstObject = orxObject_Create();
        orxObject_LinkStructure(sstCave.astData[i].pstObject, (orxSTRUCTURE *)sstCave.astData[i].pstGraphic);
        orxObject_LinkStructure(sstCave.astData[i].pstObject, (orxSTRUCTURE *)pstFrame);
      }
    }
  }

  /* Inits bulb */
  orxCave_InitBulb();

  /* Creates main & TV cameras */
  sstCave.pstMainCamera = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);
  sstCave.pstTVCamera = orxCamera_Create(orxCAMERA_KU32_FLAG_2D);

  /* Sets cameras frustum */
  orxCamera_SetFrustum(sstCave.pstMainCamera, sstCave.fScreenWidth, sstCave.fScreenHeight, orxFLOAT_0, orxFLOAT_1);
  orxCamera_SetFrustum(sstCave.pstTVCamera, orx2F(256.0f), orx2F(160.0f), orxFLOAT_0, orxFLOAT_1);

  /* Gets background size */
  sstCave.fBackgroundWidth  = orxTexture_GetWidth(sstCave.astData[orxCAVE_RESOURCE_BACKGROUND].pstTexture);
  sstCave.fBackgroundHeight = orxTexture_GetHeight(sstCave.astData[orxCAVE_RESOURCE_BACKGROUND].pstTexture);

  /* Sets cameras position */
  orxVector_Set(&vPos, orx2F(sstCave.fBackgroundWidth - (0.5f * sstCave.fScreenWidth)), orx2F(0.5f - sastInfo[orxCAVE_RESOURCE_BACKGROUND].vRelativePivot.fY) * sstCave.fBackgroundHeight, orxFLOAT_0);
  orxCamera_SetPosition(sstCave.pstMainCamera, &vPos);
  orxVector_Set(&vPos, orx2F(1400.0f), orx2F(-20.0f), orxFLOAT_0);
  orxCamera_SetPosition(sstCave.pstTVCamera, &vPos);

  /* Creates & inits TV viewport */
  sstCave.pstTVViewport = orxViewport_Create();
  orxViewport_SetCamera(sstCave.pstTVViewport, sstCave.pstTVCamera);
  orxViewport_SetSize(sstCave.pstTVViewport, orx2F(128.0f), orx2F(80.0f));
  orxViewport_SetPosition(sstCave.pstTVViewport, orx2F(445.0f), orx2F(400.0f / 768.0f) * sstCave.fScreenHeight);
  orxViewport_Enable(sstCave.pstTVViewport, orxFALSE);

  /* Creates & inits main viewport */
  sstCave.pstMainViewport = orxViewport_Create();
  orxViewport_SetCamera(sstCave.pstMainViewport, sstCave.pstMainCamera);
  orxViewport_SetRelativeSize(sstCave.pstMainViewport, orx2F(0.001f), orx2F(0.001f));
  orxViewport_SetRelativePosition(sstCave.pstMainViewport, orxVIEWPORT_KU32_FLAG_ALIGN_CENTER);
  orxViewport_Enable(sstCave.pstMainViewport, orxFALSE);

  /* Links balls to TV camera */
  orxFrame_SetParent(orxOBJECT_GET_STRUCTURE(sstCave.astData[orxCAVE_RESOURCE_BALL].pstObject, FRAME), orxCamera_GetFrame(sstCave.pstTVCamera));
  orxFrame_SetParent(orxOBJECT_GET_STRUCTURE(sstCave.astData[orxCAVE_RESOURCE_BALL_REFLECT].pstObject, FRAME), orxCamera_GetFrame(sstCave.pstTVCamera));
  orxFrame_SetParent(orxOBJECT_GET_STRUCTURE(sstCave.astData[orxCAVE_RESOURCE_BALL_SHADOW].pstObject, FRAME), orxCamera_GetFrame(sstCave.pstTVCamera));

  /* Gets render clock */
  sstCave.pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_RENDER);
  orxClock_SetModifier(sstCave.pstClock, orxCLOCK_MOD_TYPE_FIXED, orx2F(0.01666f));

  /* Registers update function on render clock */
  eResult = orxClock_Register(sstCave.pstClock, orxCave_Update, orxNULL, orxMODULE_ID_MAIN);

  /* Done! */
  return eResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/***************************************************************************
 * Plugin Related                                                          *
 ***************************************************************************/

orxPLUGIN_DECLARE_ENTRY_POINT(orxCave_Init);

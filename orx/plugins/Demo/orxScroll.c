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


/** Resource enum
 */
typedef enum __orxSCROLL_RESOURCE_t
{
  orxSCROLL_RESOURCE_FUJI = 0,
  orxSCROLL_RESOURCE_BOAT1,
  orxSCROLL_RESOURCE_BOAT2,
  orxSCROLL_RESOURCE_CLOUD,
  orxSCROLL_RESOURCE_WAVE,

  orxSCROLL_RESOURCE_NUMBER,

  orxSCROLL_RESOURCE_NONE = orxENUM_NONE

} orxSCROLL_RESOURCE;

/** Resource names
 */
orxSTATIC orxSTRING sazResourceNames[orxSCROLL_RESOURCE_NUMBER] =
{
 "Fuji",
 "Boat1",
 "Boat2",
 "Cloud",
 "Wave"
};

/** Waves groups
 */
orxSTATIC orxOBJECT **sapstWaveGroups;

/** Camera
 */
orxSTATIC orxCAMERA *spstCamera;

/** Scrolling speed
 */
orxSTATIC orxFLOAT sfScrollingSpeed;

/** Wave amplitude
 */
orxSTATIC orxFLOAT sfWaveAmplitude;

/** Update callback
 */
orxVOID orxFASTCALL orxScroll_Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR vPos;
  orxFLOAT  fScreenWidth, fScreenHeight;
  orxS32    i;

  /* Gets screen dimensions */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Gets camera position */
  orxCamera_GetPosition(spstCamera, &vPos);

  /* Updates position vector */
  vPos.fX += orxConfig_GetFloat("ScrollingSpeed") * _pstClockInfo->fDT;

  /* End of scrolling? */
  if(vPos.fX > orxConfig_GetFloat("ScrollingRelativeLength") * fScreenWidth)
  {
    /* Go back at beginning */
    vPos.fX = orx2F(0.5f) * fScreenWidth;
  }

  /* Updates camera position */
  orxCamera_SetPosition(spstCamera, &vPos);

  /* For all wave groups */
  for(i = 0; i < orxConfig_GetS32("WaveGroupNumber"); i++)
  {
    /* Gets object's position */
    orxObject_GetPosition(sapstWaveGroups[i], &vPos);

    /* Updates its Y coordinate along a sine with initial phasis */
    vPos.fY = sfWaveAmplitude * sinf(orxMATH_KF_PI_BY_2 * (orxU2F(i) + orxU2F(_pstClockInfo->fTime)));

    /* Applies it */
    orxObject_SetPosition(sapstWaveGroups[i], &vPos);
  }
}

/** Inits the scroll demo
 */
orxSTATIC orxSTATUS orxScroll_Init()
{
  orxS32    s32WaveGroupNumber, i;
  orxFLOAT  fScreenWidth, fScreenHeight, fScrollingDepth;
  orxVECTOR vMin, vMax;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Gets screen dimensions */
  orxDisplay_GetScreenSize(&fScreenWidth, &fScreenHeight);

  /* Loads config file and selects its section */
  orxConfig_Load("Scroll.ini");
  orxConfig_SelectSection("Scroll");

  /* Gets scrolling depth and speed and wave amplitude coef */
  fScrollingDepth   = orxConfig_GetFloat("ScrollingDepth");
  sfScrollingSpeed  = orxConfig_GetFloat("ScrollingSpeed");
  sfWaveAmplitude   = orxConfig_GetFloat("WaveAmplitude");

  /* Gets wave group number */
  s32WaveGroupNumber = orxConfig_GetS32("WaveGroupNumber");

  /* Selects wave section */
  orxConfig_SelectSection("Wave");

  /* Creates the wave groups */
  sapstWaveGroups = orxMemory_Allocate(s32WaveGroupNumber * sizeof(orxOBJECT *), orxMEMORY_TYPE_TEMP);

  /* Updates wave amplitude */
  sfWaveAmplitude *= fScreenHeight * (orxConfig_GetVector("MaxPos", &vMax)->fY - orxConfig_GetVector("MinPos", &vMin)->fY);

  /* Creates main wave groups */
  for(i = 0; i < s32WaveGroupNumber; i++)
  {
    orxVECTOR vPos;

    /* Creates frame */
    sapstWaveGroups[i] = orxObject_CreateFromConfig("WaveGroup");

    /* Inits its vertical position (phasis) */
    orxVector_Set(&vPos, orxFLOAT_0, (orx2F(i) / orx2F(s32WaveGroupNumber)) * sfWaveAmplitude, orxFLOAT_0);

    /* Sets it */
    orxObject_SetPosition(sapstWaveGroups[i], &vPos);
  }

  /* For all resources */
  for(i = 0; i < orxSCROLL_RESOURCE_NUMBER; i++)
  {
    orxS32 j;
    orxVECTOR vMin, vMax;

    /* Selects config section */
    orxConfig_SelectSection(sazResourceNames[i]);

    /* Gets its min & max placement vectors */
    orxConfig_GetVector("MinPos", &vMin);
    orxConfig_GetVector("MaxPos", &vMax);

    /* For all requested instances */
    for(j = 0; j < orxConfig_GetS32("Number"); j++)
    {
      orxVECTOR   vPos;
      orxOBJECT  *pstObject;

      /* Creates the object */
      pstObject = orxObject_CreateFromConfig(sazResourceNames[i]);

      /* Inits its position */
      orxVector_Set(&vPos,
                    fScreenWidth * orxFRAND(vMin.fX, vMax.fX),
                    fScreenHeight * orxFRAND(vMin.fY, vMax.fY),
                    fScrollingDepth * orxFRAND(vMin.fZ, vMax.fZ));

      /* Are we creating waves? */
      if((i == orxSCROLL_RESOURCE_WAVE) || (i == orxSCROLL_RESOURCE_BOAT1))
      {
        /* Assigns it to one of the wave group */
        orxObject_SetParent(pstObject, sapstWaveGroups[j % s32WaveGroupNumber]);
      }

      /* Sets its position */
      orxObject_SetPosition(pstObject, &vPos);
    }
  }

  /* Success? */
  if(i == orxSCROLL_RESOURCE_NUMBER)
  {
    orxCLOCK     *pstClock;
    orxOBJECT    *pstBackground;
    orxVIEWPORT  *pstViewport;

    /* Selects main section */
    orxConfig_SelectSection("Scroll");

    /* Creates viewport */
    pstViewport = orxViewport_CreateFromConfig("ScrollViewport");

    /* Stores camera pointer */
    spstCamera = orxViewport_GetCamera(pstViewport);

    /* Creates background */
    pstBackground = orxObject_CreateFromConfig("Background");

    /* Links it to camera frame */
    orxFrame_SetParent(orxOBJECT_GET_STRUCTURE(pstBackground, FRAME), orxCamera_GetFrame(spstCamera));

    /* Gets rendering clock */
    pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_RENDER);

    /* Registers update function */
    eResult = orxClock_Register(pstClock, orxScroll_Update, orxNULL, orxMODULE_ID_MAIN);
  }

  /* Done! */
  return eResult;
}

/** Declares the demo entry point */
orxPLUGIN_DECLARE_ENTRY_POINT(orxScroll_Init);

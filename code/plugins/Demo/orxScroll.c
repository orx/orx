/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxScroll.c
 * @date 22/10/2007
 * @author iarwain@orx-project.org
 *
 * Scroll demo
 *
 */


#include "orxPluginAPI.h"


/** Misc defines
 */
#define orxSCROLL_RESOURCE_NUMBER   5

/** Resource names
 */
static orxSTRING sazResourceNames[orxSCROLL_RESOURCE_NUMBER] =
{
 "Fuji",
 "Boat1",
 "Boat2",
 "Cloud",
 "Wave"
};


/** Camera
 */
static orxCAMERA *spstCamera;

/** Overlay (for fade in / fade out)
 */
static orxOBJECT *spstOverlay;

/** Root
 */
static orxOBJECT *spstRoot;


/** Update callback used to update the scrolling. N.B.: it could also have been done through FXs, doing the synchro in the .ini, and no clock would have been needed.
 */
static void orxFASTCALL orxScroll_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxVECTOR vPos;
  orxFLOAT  fMove;

  /* Selects scroll section */
  orxConfig_SelectSection("Scroll");

  /* Computes move delta */
  fMove = _pstClockInfo->fDT * orxConfig_GetFloat("ScrollingSpeed");

  /* Gets root position */
  orxObject_GetPosition(spstRoot, &vPos);

  /* End of scrolling? */
  if(vPos.fX >= orxConfig_GetFloat("ScrollingMax")
  && (vPos.fX - orxConfig_GetFloat("ScrollingMax") < fMove))
  {
    /* Adds fade FX on overlay */
    orxObject_AddFX(spstOverlay, "FadeFX");

    /* Adds reinit FX on root */
    orxObject_AddFX(spstRoot, "ReinitFX");
  }

  /* Updates scrolling */
  vPos.fX += fMove;

  /* Updates root position */
  orxObject_SetPosition(spstRoot, &vPos);
}

/** Inits the scroll demo
 */
static orxSTATUS orxScroll_Init()
{
  orxS32      i, s32WaveGroupNumber;
  orxOBJECT  *apstWaveGroupList[32];
  orxSTATUS   eResult = orxSTATUS_FAILURE;

  /* Loads config file and selects its section */
  orxConfig_Load("Scroll.ini");
  orxConfig_SelectSection("Scroll");

  /* Gets wave group number */
  s32WaveGroupNumber = orxMIN(orxConfig_GetS32("WaveGroupNumber"), 32);

  /* For all wave groups */
  for(i = 0; i < s32WaveGroupNumber; i++)
  {
    orxCHAR acWaveGroupID[32];

    /* Gets its name */
    orxString_Print(acWaveGroupID, "WaveGroup%d", i + 1);

    /* Creates it */
    apstWaveGroupList[i] = orxObject_CreateFromConfig(acWaveGroupID);

    /* Adds wave FX to it */
    orxObject_AddDelayedFX(apstWaveGroupList[i], "WaveFX", orxS2F(i));
  }

  /* For all resources */
  for(i = 0; i < orxSCROLL_RESOURCE_NUMBER; i++)
  {
    orxS32 j;

    /* Pushes config section */
    orxConfig_PushSection(sazResourceNames[i]);

    /* For all requested instances */
    for(j = 0; j < orxConfig_GetS32("Number"); j++)
    {
      orxOBJECT *pstObject;

      /* Creates the object */
      pstObject = orxObject_CreateFromConfig(sazResourceNames[i]);

      /* Selects resource section */
      orxConfig_SelectSection(sazResourceNames[i]);

      /* Is on wave? */
      if(orxConfig_GetBool("OnWave"))
      {
        /* Assigns it to one of the wave group */
        orxObject_SetParent(pstObject, apstWaveGroupList[j % s32WaveGroupNumber]);
      }
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Success? */
  if(i == orxSCROLL_RESOURCE_NUMBER)
  {
    orxCLOCK     *pstClock;
    orxVIEWPORT  *pstViewport;

    /* Creates viewport */
    pstViewport = orxViewport_CreateFromConfig("ScrollViewport");

    /* Creates root */
    spstRoot = orxObject_CreateFromConfig("Root");

    /* Creates background */
    orxObject_CreateFromConfig("Background");

    /* Creates overlay */
    spstOverlay = orxObject_CreateFromConfig("Overlay");

    /* Stores camera pointer */
    spstCamera = orxViewport_GetCamera(pstViewport);

    /* Links camera to root */
    orxCamera_SetParent(spstCamera, spstRoot);

    /* Gets rendering clock */
    pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

    /* Registers update function */
    eResult = orxClock_Register(pstClock, orxScroll_Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);
  }

  /* Done! */
  return eResult;
}

/** Declares the demo entry point */
orxPLUGIN_DECLARE_INIT_ENTRY_POINT(orxScroll_Init);

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxBounce.c
 * @date 07/04/2008
 * @author iarwain@orx-project.org
 *
 * Bounce demo
 *
 */


#include "orxPluginAPI.h"

#define             TRAIL_POINT_NUMBER                      80

static orxU32       su32VideoModeIndex                    = 0;
static orxBOOL      sbShaderEnabled                       = orxFALSE;
static orxSPAWNER  *spoBallSpawner                        = orxNULL;
static orxOBJECT   *spstWalls                             = orxNULL;
static orxFLOAT     sfShaderPhase                         = orx2F(0.0f);
static orxFLOAT     sfShaderAmplitude                     = orx2F(0.0f);
static orxFLOAT     sfShaderFrequency                     = orx2F(1.0f);
static orxVECTOR    svColor                               = {0};
static orxFLOAT     sfColorTime                           = orx2F(0.0f);
static orxFLOAT     sfTrailTimer                          = orx2F(0.0f);
static orxBOOL      sbRecord                              = orxFALSE;
static orxU32       su32TrailIndex                        = 0;
static orxVECTOR    savTrailPointList[TRAIL_POINT_NUMBER] = {0};
static orxVECTOR    savTrailSpeedList[TRAIL_POINT_NUMBER] = {0};

/** Applies current selected video mode
 */
static void orxBounce_ApplyCurrentVideoMode()
{
  orxDISPLAY_VIDEO_MODE stVideoMode;

  /* Gets desired video mode */
  orxDisplay_GetVideoMode(su32VideoModeIndex, &stVideoMode);

  /* Applies it */
  orxDisplay_SetVideoMode(&stVideoMode);
}

/** Displays trail
 */
static void orxBounce_DisplayTrail(const orxBITMAP *_pstBitmap)
{
#define STORE_VERTEX(INDEX, X, Y, U, V, RGBA) astVertexList[INDEX].fX = X; astVertexList[INDEX].fY = Y; astVertexList[INDEX].fU = U; astVertexList[INDEX].fV = V; astVertexList[INDEX].stRGBA = RGBA;

  static orxU16     sau16IndexList[(TRAIL_POINT_NUMBER - 1) * 2];
  static orxBOOL    sbInit = orxFALSE;
  orxDISPLAY_VERTEX astVertexList[(TRAIL_POINT_NUMBER - 1) * 2];
  orxDISPLAY_MESH   stMesh;
  orxVECTOR         vOffset;
  orxU32            i;

  /* Not initialized? */
  if(sbInit == orxFALSE)
  {
    /* Inits index buffer */
    for(i = 0; i < TRAIL_POINT_NUMBER - 1; i++)
    {
      /* Stores indices */
      sau16IndexList[i * 2]      = (orxU16)(i * 2);
      sau16IndexList[i * 2 + 1]  = (orxU16)(i * 2 + 1);
    }

    /* Updates status */
    sbInit = orxTRUE;
  }

  /* For all points */
  for(i = 0; i < TRAIL_POINT_NUMBER - 1; i++)
  {
    orxVECTOR vVertex1, vVertex2;
    orxU32    u32Index, u32NextIndex;

    /* Gets barrel indices */
    u32Index      = (i + su32TrailIndex) % TRAIL_POINT_NUMBER;
    u32NextIndex  = (i + 1 + su32TrailIndex) % TRAIL_POINT_NUMBER;

    /* Gets offset vector */
    orxVector_Mulf(&vOffset, orxVector_Normalize(&vOffset, orxVector_2DRotate(&vOffset, orxVector_Sub(&vOffset, &savTrailPointList[u32NextIndex], &savTrailPointList[u32Index]), orxMATH_KF_PI_BY_2)), orx2F(40.0f) / orxMath_Pow(orxS2F(i + 1), orx2F(0.6f)));

    /* Computes vertices positions */
    orxVector_Add(&vVertex1, &savTrailPointList[u32Index], &vOffset);
    orxVector_Sub(&vVertex2, &savTrailPointList[u32Index], &vOffset);

    /* Stores vertices */
    STORE_VERTEX(i * 2, vVertex1.fX, vVertex1.fY, orxFLOAT_0, orxU2F(i) / orxU2F(TRAIL_POINT_NUMBER - 1), orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF * i / (TRAIL_POINT_NUMBER + 50)));
    STORE_VERTEX(i * 2 + 1, vVertex2.fX, vVertex2.fY, orxFLOAT_1, orxU2F(i) / orxU2F(TRAIL_POINT_NUMBER - 1), orx2RGBA(0xFF, 0xFF, 0xFF, 0xFF * i / (TRAIL_POINT_NUMBER + 50)));
  }

  /* Inits mesh */
  orxMemory_Zero(&stMesh, sizeof(orxDISPLAY_MESH));
  stMesh.astVertexList    = astVertexList;
  stMesh.u32VertexNumber  = orxARRAY_GET_ITEM_COUNT(astVertexList);
  stMesh.au16IndexList    = sau16IndexList;
  stMesh.u32IndexNumber   = orxARRAY_GET_ITEM_COUNT(sau16IndexList);
  stMesh.ePrimitive       = orxDISPLAY_PRIMITIVE_TRIANGLE_STRIP;

  /* Draws trail */
  orxDisplay_DrawMesh(&stMesh, _pstBitmap, orxDISPLAY_SMOOTHING_ON, orxDISPLAY_BLEND_MODE_ALPHA);
}

/** Updates trail
 */
static void orxFASTCALL orxBounce_UpdateTrail(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxU32    i;
  orxVECTOR vMousePos;

  /* Gets mouse position */
  orxMouse_GetPosition(&vMousePos);

  /* Updates trail timer */
  sfTrailTimer -= _pstClockInfo->fDT;

  /* Should add new segment? */
  if(sfTrailTimer <= orxFLOAT_0)
  {
    /* Pushes bounce section */
    orxConfig_PushSection("Bounce");

    /* Resets trail timer */
    sfTrailTimer = orxConfig_GetFloat("TrailTimer");

    /* Adds it to the trail */
    orxVector_Copy(&savTrailPointList[su32TrailIndex], &vMousePos);

    /* Gets its speed */
    orxConfig_GetVector("TrailSpeed", &savTrailSpeedList[su32TrailIndex]);

    /* Pops config section */
    orxConfig_PopSection();

    /* Updates trail index */
    su32TrailIndex = (su32TrailIndex + 1) % TRAIL_POINT_NUMBER;
  }
  else
  {
    /* Keeps last created point on pos */
    orxVector_Copy(&savTrailPointList[(su32TrailIndex == 0) ? TRAIL_POINT_NUMBER - 1 : su32TrailIndex - 1], &vMousePos);
  }

  /* For all segments */
  for(i = 0; i < TRAIL_POINT_NUMBER; i++)
  {
    orxVECTOR vTemp;

    /* Updates its position */
    orxVector_Add(&savTrailPointList[i], &savTrailPointList[i], orxVector_Mulf(&vTemp, &savTrailSpeedList[i], _pstClockInfo->fDT));
  }
}

/** Bounce event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxBounce_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("Bounce_EventHandler");

  /* Checks */
  orxASSERT((_pstEvent->eType == orxEVENT_TYPE_PHYSICS)
         || (_pstEvent->eType == orxEVENT_TYPE_INPUT)
         || (_pstEvent->eType == orxEVENT_TYPE_SHADER)
         || (_pstEvent->eType == orxEVENT_TYPE_SOUND)
         || (_pstEvent->eType == orxEVENT_TYPE_DISPLAY)
         || (_pstEvent->eType == orxEVENT_TYPE_TIMELINE)
         || (_pstEvent->eType == orxEVENT_TYPE_RENDER));

  /* Depending on event type */
  switch(_pstEvent->eType)
  {
    /* Input */
    case orxEVENT_TYPE_INPUT:
    {
      /* Not a set selection and console not enabled? */
      if((_pstEvent->eID != orxINPUT_EVENT_SELECT_SET) && (orxConsole_IsEnabled() == orxFALSE))
      {
        orxINPUT_EVENT_PAYLOAD *pstPayload;

        /* Gets event payload */
        pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        /* Has a multi-input info? */
        if(pstPayload->aeType[1] != orxINPUT_TYPE_NONE)
        {
          /* Logs info */
          orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "[%s::%s]" orxANSI_KZ_COLOR_FG_DEFAULT " is %s (" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_FG_DEFAULT ":%g + " orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_FG_DEFAULT ":%g)", pstPayload->zSetName, pstPayload->zInputName, (_pstEvent->eID == orxINPUT_EVENT_ON) ? orxANSI_KZ_COLOR_FG_GREEN "ON " orxANSI_KZ_COLOR_FG_DEFAULT : orxANSI_KZ_COLOR_FG_RED "OFF" orxANSI_KZ_COLOR_FG_DEFAULT, orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0], pstPayload->aeMode[0]), pstPayload->afValue[0], orxInput_GetBindingName(pstPayload->aeType[1], pstPayload->aeID[1], pstPayload->aeMode[1]), pstPayload->afValue[1]);
        }
        else
        {
          /* Logs info */
          orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "[%s::%s]" orxANSI_KZ_COLOR_FG_DEFAULT " is %s (" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_FG_DEFAULT ":%g)", pstPayload->zSetName, pstPayload->zInputName, (_pstEvent->eID == orxINPUT_EVENT_ON) ? orxANSI_KZ_COLOR_FG_GREEN  "ON " orxANSI_KZ_COLOR_FG_DEFAULT : orxANSI_KZ_COLOR_FG_RED "OFF" orxANSI_KZ_COLOR_FG_DEFAULT, orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0], pstPayload->aeMode[0]), pstPayload->afValue[0]);
        }
      }

      break;
    }

    /* Physics */
    case orxEVENT_TYPE_PHYSICS:
    {
      /* Colliding? */
      if(_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
      {
        /* Adds bump FX on both objects */
        orxObject_AddUniqueFX(orxOBJECT(_pstEvent->hSender), "Bump");
        orxObject_AddUniqueFX(orxOBJECT(_pstEvent->hRecipient), "Bump");
      }

      break;
    }

    /* Shader */
    case orxEVENT_TYPE_SHADER:
    {
      orxSHADER_EVENT_PAYLOAD *pstPayload;

      /* Profiles */
      orxPROFILER_PUSH_MARKER("Bounce_EventHandler_Shader");

      /* Checks */
      orxASSERT(_pstEvent->eID == orxSHADER_EVENT_SET_PARAM);

      /* Gets its payload */
      pstPayload = (orxSHADER_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Enabled? */
      if(!orxString_Compare(pstPayload->zParamName, "enabled"))
      {
        /* Updates its value */
        pstPayload->fValue = (sbShaderEnabled != orxFALSE) ? orxFLOAT_1 : orxFLOAT_0;
      }
      /* Phase? */
      else if(!orxString_Compare(pstPayload->zParamName, "phase"))
      {
        /* Updates its value */
        pstPayload->fValue = sfShaderPhase;
      }
      else if(!orxString_Compare(pstPayload->zParamName, "color"))
      {
        orxVector_Copy(&pstPayload->vValue, &svColor);
      }
      /* Frequency? */
      else if(!orxString_Compare(pstPayload->zParamName, "frequency"))
      {
        /* Updates its value */
        pstPayload->fValue = sfShaderFrequency;
      }
      /* Amplitude? */
      else if(!orxString_Compare(pstPayload->zParamName, "amplitude"))
      {
        /* Updates its value */
        pstPayload->fValue = sfShaderAmplitude;
      }

      /* Profiles */
      orxPROFILER_POP_MARKER();

      break;
    }

    /* Sound */
    case orxEVENT_TYPE_SOUND:
    {
      /* Recording packet? */
      if(_pstEvent->eID == orxSOUND_EVENT_RECORDING_PACKET)
      {
        orxSOUND_EVENT_PAYLOAD *pstPayload;

        /* Gets event payload */
        pstPayload = (orxSOUND_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        /* Is recording input active? */
        if(orxInput_IsActive("Record") != orxFALSE)
        {
          orxU32 i;

          /* For all samples */
          for(i = 0; i < pstPayload->stStream.stPacket.u32SampleNumber / 2; i++)
          {
            /* Shorten the packets by half */
            pstPayload->stStream.stPacket.afSampleList[i] = pstPayload->stStream.stPacket.afSampleList[i * 2];
          }

          /* Updates sample number */
          pstPayload->stStream.stPacket.u32SampleNumber = pstPayload->stStream.stPacket.u32SampleNumber / 2;

          /* Asks for writing it */
          pstPayload->stStream.stPacket.bDiscard = orxFALSE;
        }
        else
        {
          /* Asks for not writing it */
          pstPayload->stStream.stPacket.bDiscard = orxTRUE;
        }
      }

      break;
    }

    /* Display */
    case orxEVENT_TYPE_DISPLAY:
    {
      /* New video mode? */
      if(_pstEvent->eID == orxDISPLAY_EVENT_SET_VIDEO_MODE)
      {
        orxDISPLAY_EVENT_PAYLOAD *pstPayload;
        orxCHAR                   acBuffer[1024];

        /* Gets payload */
        pstPayload = (orxDISPLAY_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        /* Updates title string */
        orxConfig_PushSection("Bounce");
        orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%s (%ux%u)@%uHz", orxConfig_GetString("Title"), pstPayload->stVideoMode.u32Width, pstPayload->stVideoMode.u32Height, pstPayload->stVideoMode.u32RefreshRate);
        acBuffer[sizeof(acBuffer) - 1] = orxCHAR_NULL;
        orxConfig_PopSection();

        /* Updates display module config content */
        orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
        orxConfig_SetString(orxDISPLAY_KZ_CONFIG_TITLE, acBuffer);
        orxConfig_PopSection();
      }

      break;
    }

    /* TimeLine */
    case orxEVENT_TYPE_TIMELINE:
    {
      /* New event triggered? */
      if(_pstEvent->eID == orxTIMELINE_EVENT_TRIGGER)
      {
        orxTIMELINE_EVENT_PAYLOAD *pstPayload;

        /* Gets event payload */
        pstPayload = (orxTIMELINE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

        /* Not first frame? (to prevent looping track flood) */
        if(pstPayload->fTimeStamp != orxFLOAT_0)
        {
          /* Logs info */
          orxLOG(orxANSI_KZ_COLOR_FG_YELLOW "[%s::%s::%s]" orxANSI_KZ_COLOR_FG_DEFAULT " has been triggered", orxObject_GetName(orxOBJECT(_pstEvent->hSender)), pstPayload->zTrackName, pstPayload->zEvent);
        }
      }

      break;
    }

    /* Render */
    case orxEVENT_TYPE_RENDER:
    {
      /* Object start? */
      if(_pstEvent->eID == orxRENDER_EVENT_OBJECT_START)
      {
        /* Is walls? */
        if(orxOBJECT(_pstEvent->hSender) == spstWalls)
        {
          /* Pushes config section */
          orxConfig_PushSection("Bounce");

          /* Should display trail */
          if(orxConfig_GetBool("DisplayTrail"))
          {
            /* Draws trail */
            orxBounce_DisplayTrail(orxTexture_GetBitmap(orxObject_GetWorkingTexture(orxOBJECT(_pstEvent->hSender))));
          }

          /* Pops config section */
          orxConfig_PopSection();

          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }

      break;
    }

    default:
    {
      break;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Update callback
 */
static void orxFASTCALL orxBounce_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR vMousePos;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("Bounce_Update");

  if((sbRecord == orxFALSE) && (orxInput_IsActive("Record") != orxFALSE))
  {
    /* Starts recording with default settings */
    orxSound_StartRecording("orxSoundRecording.wav", orxFALSE, 0, 1);

    /* Updates status */
    sbRecord = orxTRUE;
  }

  if(orxInput_HasBeenActivated("ToggleTrail"))
  {
    /* Toggles trail rendering */
    orxConfig_PushSection("Bounce");
    orxConfig_SetBool("DisplayTrail", !orxConfig_GetBool("DisplayTrail"));
    orxConfig_PopSection();
  }

  if(orxInput_HasBeenActivated("ToggleProfiler"))
  {
    /* Toggles profiler rendering */
    orxConfig_PushSection(orxRENDER_KZ_CONFIG_SECTION);
    orxConfig_SetBool(orxRENDER_KZ_CONFIG_SHOW_PROFILER, !orxConfig_GetBool(orxRENDER_KZ_CONFIG_SHOW_PROFILER));
    orxConfig_PopSection();
  }

  if(orxInput_HasBeenActivated("PreviousResolution"))
  {
    /* Updates video mode index */
    su32VideoModeIndex = (su32VideoModeIndex == 0) ? orxDisplay_GetVideoModeCount() - 1 : su32VideoModeIndex - 1;

    /* Applies it */
    orxBounce_ApplyCurrentVideoMode();
  }
  else if(orxInput_HasBeenActivated("NextResolution"))
  {
    /* Updates video mode index */
    su32VideoModeIndex = (su32VideoModeIndex >= orxDisplay_GetVideoModeCount() - 1) ? 0 : su32VideoModeIndex + 1;

    /* Applies it */
    orxBounce_ApplyCurrentVideoMode();
  }
  if(orxInput_HasBeenActivated("ToggleFullScreen"))
  {
    /* Toggles full screen display */
    orxDisplay_SetFullScreen(!orxDisplay_IsFullScreen());
  }

  /* Pushes config section */
  orxConfig_PushSection("Bounce");

  /* Updates shader values */
  sfShaderPhase    += orxConfig_GetFloat("ShaderPhaseSpeed") * _pstClockInfo->fDT;
  sfShaderFrequency = orxConfig_GetFloat("ShaderMaxFrequency") * orxMath_Sin(orxConfig_GetFloat("ShaderFrequencySpeed") * _pstClockInfo->fTime);
  sfShaderAmplitude = orxConfig_GetFloat("ShaderMaxAmplitude") * orxMath_Sin(orxConfig_GetFloat("ShaderAmplitudeSpeed") * _pstClockInfo->fTime);

  /* Updates color time */
  sfColorTime -= _pstClockInfo->fDT;

  /* Should update color */
  if(sfColorTime <= orxFLOAT_0)
  {
    orxConfig_PushSection("BounceShader");
    orxConfig_GetVector("color", &svColor);
    orxConfig_PopSection();

    sfColorTime += orx2F(3.0f);
  }

  /* Gets mouse world position */
  orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), orxNULL, &vMousePos);

  /* Updates position */
  vMousePos.fZ += orx2F(0.5f);

  /* Has ball spawner? */
  if(spoBallSpawner != orxNULL)
  {
    /* Updates its position */
    orxSpawner_SetPosition(spoBallSpawner, &vMousePos);
  }

  /* Clears ray hit */
  orxConfig_SetString("RayHit", orxSTRING_EMPTY);

  /* Spawning */
  if(orxInput_IsActive("Spawn"))
  {
    /* Spawns one ball */
    orxSpawner_Spawn(spoBallSpawner, orxU32_UNDEFINED);
  }
  /* Picking? */
  else if(orxInput_IsActive("Pick"))
  {
    orxOBJECT *pstObject;

    /* Updates mouse position */
    vMousePos.fZ -= orx2F(0.1f);

    /* Picks object under mouse */
    pstObject = orxObject_Pick(&vMousePos, orxSTRINGID_UNDEFINED);

    /* Found? */
    if(pstObject)
    {
      /* Adds FX */
      orxObject_AddUniqueFX(pstObject, "Pick");
    }
  }
  /* Raycasting? */
  else if(orxInput_IsActive("RayCast"))
  {
    orxOBJECT  *pstRayHit;
    orxU32      u32SelfFlags = 0, u32CheckMask = 0;
    orxS32      i;

    /* Gets flags & mask */
    for(i = 0; i < orxConfig_GetListCount("RaySelfFlags"); i++)
    {
      u32SelfFlags |= orxPhysics_GetCollisionFlagValue(orxConfig_GetListString("RaySelfFlags", i));
    }
    for(i = 0; i < orxConfig_GetListCount("RayCheckMask"); i++)
    {
      u32CheckMask |= orxPhysics_GetCollisionFlagValue(orxConfig_GetListString("RayCheckMask", i));
    }

    /* Casts ray */
    pstRayHit = orxObject_Raycast(&orxVECTOR_0, &vMousePos, (orxU16)u32SelfFlags, (orxU16)u32CheckMask, orxFALSE, orxNULL, orxNULL);

    /* Hit? */
    if(pstRayHit != orxNULL)
    {
      /* Stores its name */
      orxConfig_SetString("RayHit", orxObject_GetName(pstRayHit));
    }
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Toggle shader? */
  if(orxInput_HasBeenActivated("ToggleShader"))
  {
    /* Toggles shader status */
    sbShaderEnabled = !sbShaderEnabled;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();
}

/** Inits the bounce demo
 */
static orxSTATUS orxBounce_Init()
{
  orxSTATUS eResult;

  /* Loads config file and selects its section */
  orxConfig_Load("Bounce.ini");
  orxConfig_SelectSection("Bounce");

  /* Loads input */
  orxInput_Load(orxNULL);

  /* Creates ball spawner */
  spoBallSpawner = orxSpawner_CreateFromConfig("BallSpawner");

  /* Valid? */
  if(spoBallSpawner != orxNULL)
  {
    orxOBJECT  *pstParticleSource;
    orxCLOCK   *pstClock;
    orxU32      i;

    /* Creates particle source */
    pstParticleSource = orxObject_CreateFromConfig("ParticleSource");

    /* Valid? */
    if(pstParticleSource != orxNULL)
    {
      /* Sets its parent */
      orxObject_SetParent(pstParticleSource, spoBallSpawner);
    }

    /* Updates cursor */
    orxMouse_ShowCursor(orxConfig_GetBool("ShowCursor"));

    /* Creates walls */
    spstWalls = orxObject_CreateFromConfig("Walls");

    /* Inits trail */
    for(i = 0; i < TRAIL_POINT_NUMBER; i++)
    {
      orxMouse_GetPosition(&savTrailPointList[i]);
    }

    /* Creates all viewports */
    for(i = 0; i < (orxU32)orxConfig_GetListCount("ViewportList"); i++)
    {
      orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
    }

    /* Gets rendering clock */
    pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

    /* Registers callback */
    eResult = orxClock_Register(pstClock, &orxBounce_Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

    /* Registers update trail timer */
    eResult = ((eResult != orxSTATUS_FAILURE) && (orxClock_Register(pstClock, &orxBounce_UpdateTrail, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_LOW) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;

    /* Registers event handler */
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, orxBounce_EventHandler) : orxSTATUS_FAILURE;
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_INPUT, orxBounce_EventHandler) : orxSTATUS_FAILURE;
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_SHADER, orxBounce_EventHandler) : orxSTATUS_FAILURE;
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_SOUND, orxBounce_EventHandler) : orxSTATUS_FAILURE;
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_DISPLAY, orxBounce_EventHandler) : orxSTATUS_FAILURE;
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_TIMELINE, orxBounce_EventHandler) : orxSTATUS_FAILURE;
    eResult = (eResult != orxSTATUS_FAILURE) ? orxEvent_AddHandler(orxEVENT_TYPE_RENDER, orxBounce_EventHandler) : orxSTATUS_FAILURE;
  }
  else
  {
    /* Failure */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(orxBounce_Init);

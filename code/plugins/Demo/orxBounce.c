/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Publi
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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

static orxU32       su32BallCounter = 0;
static orxU32       su32VideoModeIndex = 0;
static orxSPAWNER  *spoBallSpawner;
static orxVIEWPORT *spstViewport;
static orxFLOAT     sfShaderPhase = orx2F(0.0f);
static orxFLOAT     sfShaderAmplitude = orx2F(0.0f);
static orxFLOAT     sfShaderFrequency = orx2F(1.0f);

/** Applies current selected video mode
 */
static void orxBounce_ApplyCurrentVideoMode()
{
  orxDISPLAY_VIDEO_MODE stVideoMode;
  orxFLOAT              fWidth, fHeight;
  orxCHAR               acBuffer[1024];

  /* Gets desired video mode */
  orxDisplay_GetVideoMode(su32VideoModeIndex, &stVideoMode);

  /* Updates title string */
  orxConfig_PushSection("Bounce");
  orxString_NPrint(acBuffer, 1024, "%s (%ldx%ld)", orxConfig_GetString("Title"), stVideoMode.u32Width, stVideoMode.u32Height);
  acBuffer[1023] = orxCHAR_NULL;
  orxConfig_PopSection();

  /* Updates display module config content */
  orxConfig_PushSection(orxDISPLAY_KZ_CONFIG_SECTION);
  orxConfig_SetString(orxDISPLAY_KZ_CONFIG_TITLE, acBuffer);
  orxConfig_PopSection();

  /* Gets current viewport relative size */
  orxViewport_GetRelativeSize(spstViewport, &fWidth, &fHeight);

  /* Applies new video mode */
  orxDisplay_SetVideoMode(&stVideoMode);

  /* Updates viewport to its previous relative size */
  orxViewport_SetRelativeSize(spstViewport, fWidth, fHeight);
}

/** Bounce event handler
 * @param[in]   _pstEvent                     Sent event
 * @return      orxSTATUS_SUCCESS if handled / orxSTATUS_FAILURE otherwise
 */
static orxSTATUS orxFASTCALL orxBounce_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT((_pstEvent->eType == orxEVENT_TYPE_PHYSICS) || (_pstEvent->eType == orxEVENT_TYPE_INPUT) || (_pstEvent->eType == orxEVENT_TYPE_SHADER));

  /* Depending on event type */
  switch(_pstEvent->eType)
  {
    /* Input */
    case orxEVENT_TYPE_INPUT:
    {
      orxINPUT_EVENT_PAYLOAD *pstPayload;

      /* Gets event payload */
      pstPayload = (orxINPUT_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      /* Has a multi-input info? */
      if(pstPayload->aeType[1] != orxINPUT_TYPE_NONE)
      {
        /* Logs info */
        orxLOG("[%s::%s] is %s (%s/v=%g + %s/v=%g)", pstPayload->zSetName, pstPayload->zInputName, (_pstEvent->eID == orxINPUT_EVENT_ON) ? "ON " : "OFF", orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0]), pstPayload->afValue[0], orxInput_GetBindingName(pstPayload->aeType[1], pstPayload->aeID[1]), pstPayload->afValue[1]);
      }
      else
      {
        /* Logs info */
        orxLOG("[%s::%s] is %s (%s/v=%g)", pstPayload->zSetName, pstPayload->zInputName, (_pstEvent->eID == orxINPUT_EVENT_ON) ? "ON " : "OFF", orxInput_GetBindingName(pstPayload->aeType[0], pstPayload->aeID[0]), pstPayload->afValue[0]);
      }

      break;
    }

    /* Physics */
    case orxEVENT_TYPE_PHYSICS:
    {
      /* Going out of world? */
      if(_pstEvent->eID == orxPHYSICS_EVENT_OUT_OF_WORLD)
      {
        /* Deletes corresponding object */
        orxObject_SetLifeTime(orxOBJECT(_pstEvent->hSender), orxFLOAT_0);

        /* Updates ball counter */
        su32BallCounter--;
      }
      /* Colliding? */
      else if(_pstEvent->eID == orxPHYSICS_EVENT_CONTACT_ADD)
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
      orxSHADER_EVENT_PARAM_PAYLOAD *pstPayload;

      /* Checks */
      orxASSERT(_pstEvent->eID == orxSHADER_EVENT_SET_PARAM);

      /* Gets its payload */
      pstPayload = (orxSHADER_EVENT_PARAM_PAYLOAD *)_pstEvent->pstPayload;

      /* Phase? */
      if(!orxString_Compare(pstPayload->zParamName, "phase"))
      {
        /* Updates its value */
        pstPayload->fValue = sfShaderPhase;
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

      break;
    }

    default:
    {
      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Update callback
 */
static void orxFASTCALL orxBounce_Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR vMousePos;
  orxBOOL   bInViewport;
  orxAABOX stBox;

  orxAABox_Set(&stBox, &orxVECTOR_1, &orxVECTOR_0);

  if(orxInput_IsActive("PreviousResolution") && orxInput_HasNewStatus("PreviousResolution"))
  {
    /* Updates video mode index */
    su32VideoModeIndex = (su32VideoModeIndex == 0) ? orxDisplay_GetVideoModeCounter() - 1 : su32VideoModeIndex - 1;

    /* Applies it */
    orxBounce_ApplyCurrentVideoMode();
  }
  else if(orxInput_IsActive("NextResolution") && orxInput_HasNewStatus("NextResolution"))
  {
    /* Updates video mode index */
    su32VideoModeIndex = (su32VideoModeIndex >= orxDisplay_GetVideoModeCounter() - 1) ? 0 : su32VideoModeIndex + 1;

    /* Applies it */
    orxBounce_ApplyCurrentVideoMode();
  }
  if(orxInput_IsActive("ToggleFullScreen") && orxInput_HasNewStatus("ToggleFullScreen"))
  {
    /* Toggles full screen display */
    orxDisplay_SetFullScreen(!orxDisplay_IsFullScreen());
  }

  /* Gets mouse world position */
  bInViewport = (orxRender_GetWorldPosition(orxMouse_GetPosition(&vMousePos), &vMousePos) != orxNULL) ? orxTRUE : orxFALSE;

  /* Is mouse in a viewport? */
  if(bInViewport != orxFALSE)
  {
    /* Selects config section */
    orxConfig_SelectSection("Bounce");

    /* Updates shader values */
    sfShaderPhase    += orxConfig_GetFloat("ShaderPhaseSpeed") * _pstClockInfo->fDT;
    sfShaderFrequency = orxConfig_GetFloat("ShaderMaxFrequency") * orxMath_Sin(orxConfig_GetFloat("ShaderFrequencySpeed") * _pstClockInfo->fTime);
    sfShaderAmplitude = orxConfig_GetFloat("ShaderMaxAmplitude") * orxMath_Sin(orxConfig_GetFloat("ShaderAmplitudeSpeed") * _pstClockInfo->fTime);

    /* Updates position */
    vMousePos.fZ += orx2F(0.5f);

    /* Has ball spawner? */
    if(spoBallSpawner != orxNULL)
    {
      /* Updates its position */
      orxSpawner_SetPosition(spoBallSpawner, &vMousePos);
    }

    /* Spawning */
    if(orxInput_IsActive("Spawn"))
    {
      /* Spawn one ball */
      orxSpawner_Spawn(spoBallSpawner, 1);
    }
    /* Picking? */
    else if(orxInput_IsActive("Pick"))
    {
      orxOBJECT *pstObject;

      /* Updates mouse position */
      vMousePos.fZ -= orx2F(0.1f);

      /* Picks object under mouse */
      pstObject = orxObject_Pick(&vMousePos);

      /* Found? */
      if(pstObject)
      {
        /* Adds FX */
        orxObject_AddUniqueFX(pstObject, "Pick");
      }
    }
  }

  /* Toggle shader? */
  if(orxInput_IsActive("ToggleShader") && (orxInput_HasNewStatus("ToggleShader")))
  {
    /* Toggles shader */
    orxViewport_EnableShader(spstViewport, !orxViewport_IsShaderEnabled(spstViewport));
  }
}

/** Inits the bounce demo
 */
static orxSTATUS orxBounce_Init()
{
  orxCLOCK   *pstClock;
  orxSTATUS   eResult;

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
    orxOBJECT *pstParticleSource;

    /* Creates particle source */
    pstParticleSource = orxObject_CreateFromConfig("ParticleSource");

    /* Valid? */
    if(pstParticleSource != orxNULL)
    {
      /* Sets its parent */
      orxObject_SetParent(pstParticleSource, spoBallSpawner);
    }

    /* Should hide cursor */
    if(orxConfig_GetBool("ShowCursor") == orxFALSE)
    {
      orxMouse_ShowCursor(orxFALSE);
    }

    /* Creates walls */
    orxObject_CreateFromConfig("Walls");

    /* Creates viewport on screen */
    spstViewport = orxViewport_CreateFromConfig("BounceViewport");
    orxViewport_EnableShader(spstViewport, orxFALSE);

    /* Gets rendering clock */
    pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

    /* Registers callback */
    eResult = orxClock_Register(pstClock, &orxBounce_Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

    /* Registers event handler */
    eResult = ((eResult != orxSTATUS_FAILURE) && (orxEvent_AddHandler(orxEVENT_TYPE_PHYSICS, orxBounce_EventHandler) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    eResult = ((eResult != orxSTATUS_FAILURE) && (orxEvent_AddHandler(orxEVENT_TYPE_INPUT, orxBounce_EventHandler) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
    eResult = ((eResult != orxSTATUS_FAILURE) && (orxEvent_AddHandler(orxEVENT_TYPE_SHADER, orxBounce_EventHandler) != orxSTATUS_FAILURE)) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
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

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2010 Orx-Project
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
 * @file 06_Sound.c
 * @date 12/08/2008
 * @author iarwain@orx-project.org
 *
 * Sound tutorial
 */


#include "orxPluginAPI.h"


/* This is a basic C tutorial creating sounds effects and musics (streams).
 * As we are using the default executable for this tutorial, this code
 * will be loaded and executed as a runtime plugin.
 *
 * In addition, some basics are handled for us by the main executable.
 * First of all, it will load all available plugins and modules. If you
 * require only some of those, then it's better to write your own executable
 * instead of a plugin. This will be covered in a later tutorial.
 *
 * The main executable also handles some keys:
 * - F11 as vertical sync toggler
 * - Escape as exit key
 * - F12 to capture a screenshot
 * - Backspace to reload all configuration files (provided that config history is turned on)
 * It also exits if the orxSYSTEM_EVENT_CLOSE signal is sent.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation and viewport handling.
 *
 * This tutorial shows how to play sounds (samples) and musics (streams).
 * As with other features from previous behavior, it only requires, most of the time,
 * a single line of code, everything being data driven.
 *
 * It also demonstrates how to alter their settings in real time, using the soldier as a visual feedback.
 *
 * If you press up & down arrows, the music volume will change accordingly. The soldier will be scale in consequence.
 * By pressing left & right arrows, the music pitch (frequency) will change. The soldier will rotate like a knob.
 *
 * Left control key will play the music (and activate the soldier) if the music was paused,
 * otherwise it'll pause the music (and deactivate the soldier).
 *
 * Lastly, enter and space will play a sound effect on the soldier. Space triggered sound effect
 * is the same as enter except that its volume and pitch are randomly defined in the default config file.
 *
 * This allows to easily generate step or hit sounds with variety with no extra line of code.
 * We randomly change the soldier's color to illustrate this.
 * NB: The sound effect will only be added and played on an active soldier.
 * If you want to play a sound effect with no object as support,
 * you can do it the same way we create the music in this tutorial.
 * However, playing a sound on an object will allow spatial sound positioning (not covered by this tutorial).
 *
 * Many sound effects can be played at the same time on a single object.
 *
 * The sound config attribute KeepDataInCache allows to keep the sound sample
 * in memory instead of rereading it from file every time.
 * This only works for non-streamed data (ie. not for musics).
 * If it's set to false, the sample will be reloaded from file, unless there's currently
 * another sound effect of the same type being played.
 *
 * We also register to the sound events to display when sound effects are played and stopped.
 * These events are only sent for sound effects played on objects.
 */


/** Tutorial objects
 */
orxOBJECT *pstSoldier;
orxSOUND  *pstMusic;


/** Event handler
 */
orxSTATUS orxFASTCALL EventHandler(const orxEVENT *_pstEvent)
{
  orxSOUND_EVENT_PAYLOAD *pstPayload;

  /* Gets event payload */
  pstPayload = (orxSOUND_EVENT_PAYLOAD *)_pstEvent->pstPayload;

  /* Depending on event type */
  switch(_pstEvent->eID)
  {
    case orxSOUND_EVENT_START:
    {
      /* Logs info */
      orxLOG("Sound <%s>@<%s> has started!", pstPayload->zSoundName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }

    case orxSOUND_EVENT_STOP:
    {
      /* Logs info */
      orxLOG("Sound <%s>@<%s> has stopped!", pstPayload->zSoundName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Update callback
 */
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxVECTOR v;
  orxCOLOR  stColor;

  /* *** SOUND FX CONTROLS *** */

  /* Random SFX? */
  if(orxInput_IsActive("RandomSFX") && orxInput_HasNewStatus("RandomSFX"))
  {
    /* Adds a sound FX on soldier */
    orxObject_AddSound(pstSoldier, "RandomBip");

    /* Pushes config tutorial section */
    orxConfig_PushSection("Tutorial");

    /* Sets a random color on soldier */
    orxObject_SetColor(pstSoldier, orxColor_Set(&stColor, orxConfig_GetVector("RandomColor", &v), orxFLOAT_1));

    /* Pops config section */
    orxConfig_PopSection();
  }
  /* Default SFX? */
  if(orxInput_IsActive("DefaultSFX") && orxInput_HasNewStatus("DefaultSFX"))
  {
    /* Adds a sound FX on soldier */
    orxObject_AddSound(pstSoldier, "DefaultBip");

    /* Resets color on soldier */
    orxObject_SetColor(pstSoldier, orxColor_Set(&stColor, &orxVECTOR_WHITE, orxFLOAT_1));
  }

  /* *** MUSIC CONTROLS *** */

  /* Toggle music? */
  if(orxInput_IsActive("ToggleMusic") && orxInput_HasNewStatus("ToggleMusic"))
  {
    /* Not playing? */
    if(orxSound_GetStatus(pstMusic) != orxSOUND_STATUS_PLAY)
    {
      /* Plays music */
      orxSound_Play(pstMusic);

      /* Activates soldier */
      orxObject_Enable(pstSoldier, orxTRUE);
    }
    else
    {
      /* Pauses music */
      orxSound_Pause(pstMusic);

      /* Deactivates soldier */
      orxObject_Enable(pstSoldier, orxFALSE);
    }
  }

  /* Pitch up? */
  if(orxInput_IsActive("PitchUp"))
  {
    /* Speeds the music up */
    orxSound_SetPitch(pstMusic, orxSound_GetPitch(pstMusic) + orx2F(0.01f));

    /* Rotates soldier CW */
    orxObject_SetRotation(pstSoldier, orxObject_GetRotation(pstSoldier) + orx2F(4.0f) * _pstClockInfo->fDT);
  }
  /* Pitch down? */
  if(orxInput_IsActive("PitchDown"))
  {
    /* Slows the music down */
    orxSound_SetPitch(pstMusic, orxSound_GetPitch(pstMusic) - orx2F(0.01f));

    /* Rotates soldier CCW */
    orxObject_SetRotation(pstSoldier, orxObject_GetRotation(pstSoldier) - orx2F(4.0f) * _pstClockInfo->fDT);
  }
  /* Volume down? */
  if(orxInput_IsActive("VolumeDown"))
  {
    /* Turns down music's volume */
    orxSound_SetVolume(pstMusic, orxSound_GetVolume(pstMusic) - orx2F(0.05f));

    /* Scales down the soldier */
    orxObject_SetScale(pstSoldier, orxVector_Mulf(&v, orxObject_GetScale(pstSoldier, &v), orx2F(0.98f)));
  }
  /* Volume up? */
  if(orxInput_IsActive("VolumeUp"))
  {
    /* Turns up music's volume */
    orxSound_SetVolume(pstMusic, orxSound_GetVolume(pstMusic) + orx2F(0.05f));

    /* Scales up the soldier */
    orxObject_SetScale(pstSoldier, orxVector_Mulf(&v, orxObject_GetScale(pstSoldier, &v), orx2F(1.02f)));
  }
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK       *pstClock;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  const orxSTRING zInputVolumeUp;
  const orxSTRING zInputVolumeDown;
  const orxSTRING zInputPitchUp;
  const orxSTRING zInputPitchDown;
  const orxSTRING zInputToggleMusic;
  const orxSTRING zInputRandomSFX;
  const orxSTRING zInputDefaultSFX;

  /* Loads config file */
  orxConfig_Load("../06_Sound.ini");

  /* Reloads inputs */
  orxInput_Load(orxSTRING_EMPTY);

  /* Gets input binding names */
  orxInput_GetBinding("VolumeUp", 0, &eType, &eID);
  zInputVolumeUp = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("VolumeDown", 0, &eType, &eID);
  zInputVolumeDown = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("PitchUp", 0, &eType, &eID);
  zInputPitchUp = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("PitchDown", 0, &eType, &eID);
  zInputPitchDown = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("ToggleMusic", 0, &eType, &eID);
  zInputToggleMusic = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("RandomSFX", 0, &eType, &eID);
  zInputRandomSFX = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("DefaultSFX", 0, &eType, &eID);
  zInputDefaultSFX = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' & '%s' will change the music volume (+ soldier size)"
         "\n- '%s' & '%s' will change the music pitch (+ soldier rotation)"
         "\n- '%s' will toggle music (+ soldier display)"
         "\n- '%s' will play a random SFX on the soldier (+ change its color)"
         "\n- '%s' will the default SFX on the soldier (+ restore its color)"
         "\n! The sound effect will be played only if the soldier is active",
         zInputVolumeUp, zInputVolumeDown, zInputPitchUp, zInputPitchDown, zInputToggleMusic, zInputRandomSFX, zInputDefaultSFX);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates soldier */
  pstSoldier = orxObject_CreateFromConfig("Soldier");

  /* Gets main clock */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Creates background music (streamed) */
  pstMusic = orxSound_CreateFromConfig("Music");

  /* Plays it */
  orxSound_Play(pstMusic);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_SOUND, EventHandler);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);

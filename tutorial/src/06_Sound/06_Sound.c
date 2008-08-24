/* Orx - Portable Game Engine
 *
 * Orx is the legal property of its developers, whose names
 * are listed in the COPYRIGHT file distributed 
 * with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
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
 * @file 06_Sound.c
 * @date 12/08/2008
 * @author iarwain@orx-project.org
 *
 * Sound tutorial
 */


#include "orx.h"


/* This is a basic C tutorial creating sounds FXs and musics (streams).
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
 * Left clicking will play the music (and activate the soldier) if the music was paused.
 * The right click is used to pause the music (and deactivate the soldier).
 *
 * Lastly, enter and space will play a sound effect on the soldier. Space triggered sound effect
 * is the same as enter except that its volume and pitch are randomly defined in the default config file.
 *
 * This allows to easily generate step or hit sounds with variety with no extra line of code.
 * We randomly change the soldier's color to illustrate this.
 * NB: The sound effect will only be added and played on an active soldier.
 * If you want to play a sound effect with no object as support,
 * you can do it the same way we create the music in this tutorial.
 * However, playing a sound on an object will allow spatial sound positioning (WIP).
 *
 * Many sound effects can be played at the same time on a single object,
 * this tutorial contains some logic to only allow a single sound effect at a time.
 * This behavior is tweakable through the config system.
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
orxFASTCALL orxSTATUS EventHandler(orxCONST orxEVENT *_pstEvent)
{
  orxSOUND_EVENT_PAYLOAD *pstPayload;

  /* Gets event payload */
  pstPayload = _pstEvent->pstPayload;

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
      orxLOG("Sound <%s>@<%s> has stoped!", pstPayload->zSoundName, orxObject_GetName(orxOBJECT(_pstEvent->hRecipient)));

      break;
    }
  }

  /* Done! */
  return orxSTATUS_SUCCESS;
}


/** Update callback
 */
orxVOID orxFASTCALL Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxVECTOR v;
  orxCOLOR  stColor;

  /* *** SOUND FX CONTROLS *** */

  /* Selects config tutorial section */
  orxConfig_SelectSection("Tutorial");

  /* Is space pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_SPACE))
  {
    /* No sound FX playing or multiple sound FXs allowed? */
    if(orxConfig_GetBool("AllowMultipleSoundFX") || !orxObject_GetLastAddedSound(pstSoldier))
    {
      /* Adds a sound FX on soldier */
      orxObject_AddSound(pstSoldier, "RandomBip");

      /* Sets a random color on soldier */
      orxObject_SetColor(pstSoldier, orxColor_Set(&stColor, orxConfig_GetVector("RandomColor", &v), orxFLOAT_1));
    }
  }
  /* Is enter pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RETURN))
  {
    /* No sound FX playing or multiple sound FXs allowed? */
    if(orxConfig_GetBool("AllowMultipleSoundFX") || !orxObject_GetLastAddedSound(pstSoldier))
    {
      /* Adds a sound FX on soldier */
      orxObject_AddSound(pstSoldier, "DefaultBip");

      /* Resets color on soldier */
      orxObject_SetColor(pstSoldier, orxColor_Set(&stColor, &orxVECTOR_WHITE, orxFLOAT_1));
    }
  }

  /* *** MUSIC CONTROLS *** */

  /* Is left mouse button pressed? */
  if(orxMouse_IsButtonPressed(orxMOUSE_BUTTON_LEFT))
  {
    /* Not playing? */
    if(orxSound_GetStatus(pstMusic) != orxSOUND_STATUS_PLAY)
    {
      /* Plays music */
      orxSound_Play(pstMusic);

      /* Activates soldier */
      orxObject_Enable(pstSoldier, orxTRUE);
    }
  }
  /* Is right mouse button pressed? */
  if(orxMouse_IsButtonPressed(orxMOUSE_BUTTON_RIGHT))
  {
    /* Pauses music */
    orxSound_Pause(pstMusic);

    /* Deactivates soldier */
    orxObject_Enable(pstSoldier, orxFALSE);
  }

  /* Is right arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_RIGHT))
  {
    /* Speeds the music up */
    orxSound_SetPitch(pstMusic, orxSound_GetPitch(pstMusic) + orx2F(0.01f));

    /* Rotates soldier CW */
    orxObject_SetRotation(pstSoldier, orxObject_GetRotation(pstSoldier) + orx2F(4.0f) * _pstClockInfo->fDT);    
  }
  /* Is left arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_LEFT))
  {
    /* Slows the music down */
    orxSound_SetPitch(pstMusic, orxSound_GetPitch(pstMusic) - orx2F(0.01f));

    /* Rotates soldier CCW */
    orxObject_SetRotation(pstSoldier, orxObject_GetRotation(pstSoldier) - orx2F(4.0f) * _pstClockInfo->fDT);    
  }
  /* Is down arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_DOWN))
  {
    /* Turns down music's volume */
    orxSound_SetVolume(pstMusic, orxSound_GetVolume(pstMusic) - orx2F(0.05f));

    /* Scales down the soldier */
    orxObject_SetScale(pstSoldier, orxVector_Mulf(&v, orxObject_GetScale(pstSoldier, &v), orx2F(0.98f)));
  }
  /* Is up arrow pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_UP))
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
  orxCLOCK *pstClock;

  /* Displays a small hint in console */
  orxLOG("\n- Up/down arrow keys to change the music volume (and the soldier size)"
         "\n- Left/right arrow keys to change the music pitch (and the soldier rotation)"
         "\n- Left mouse button will play music if paused and activate the soldier"
         "\n- Right mouse button will pause the music and deactivate the soldier"
         "\n- Space will play a random sound effect on the soldier (and change its color)"
         "\n- Enter will a default sound effect on the soldier (and restore its color)"
         "\n! The sound effect will be played only if the soldier is active");

  /* Loads config file and selects main section */
  orxConfig_Load("../06_Sound.ini");

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates soldier */
  pstSoldier = orxObject_CreateFromConfig("Soldier");

  /* Creates a 100 Hz clock */
  pstClock = orxClock_Create(orx2F(0.01f), orxCLOCK_TYPE_USER);

  /* Creates background music (streamed) */
  pstMusic = orxSound_CreateFromConfig("Music");

  /* Plays it */
  orxSound_Play(pstMusic);

  /* Registers our update callback */
  orxClock_Register(pstClock, Update, orxNULL, orxMODULE_ID_MAIN);

  /* Registers event handler */
  orxEvent_AddHandler(orxEVENT_TYPE_SOUND, EventHandler);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);

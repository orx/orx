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
 * @file 02_Clock.c
 * @date 09/08/2008
 * @author iarwain@orx-project.org
 *
 * Clock tutorial
 */


#include "orxPluginAPI.h"


/* This is a basic C tutorial creating clocks and playing with them.
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
 * See tutorial 01_Object for more info about the basic object creation.
 * Here we register our callback on 2 different clocks for didactic purpose only. All objects
 * can of course be updated with only one clock, and the given clock context is also used here 
 * for demonstration only.
 * The first clock runs at 0.01s per tick (100 Hz) and the second one at 0.2s per tick (5 Hz).
 * If you press numpad '+', '-' and '*', you can alter the time of the first clock.
 * It'll still be updated at the same rate, but the time information that the clock will pass
 * to the callback will be stretched.
 * This provides an easy way of adding time distortion and having parts
 * of your logic code updated at different frequencies.
 * One clock can have as many callbacks registered as you want.
 *
 * For example, the FPS displayed in the top left corner is computed with a non-stretched clock
 * of tick size = 1 second.
 */


/** Update callback
 */
orxVOID orxFASTCALL Update(orxCONST orxCLOCK_INFO *_pstClockInfo, orxVOID *_pstContext)
{
  orxCLOCK  *pstClock;
  orxOBJECT *pstObject;


  /* *** LOG DISPLAY SECTION *** */

  /* Selects main config section */
  orxConfig_SelectSection("Main");

  /* Should display log? */
  if(orxConfig_GetBool("DisplayLog"))
  {
    /* Displays info in log and console */
    orxLOG("CLOCK<%p> : Time = %.3f / DT = %.3f", _pstClockInfo, _pstClockInfo->fTime, _pstClockInfo->fDT);
  }

  /* Is 'L' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_L))
  {
    /* Activates logging */
    orxConfig_SetBool("DisplayLog", orxTRUE);
  }
  /* Is 'S' pressed? */
  else if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_S))
  {
    /* Deactivates logging */
    orxConfig_SetBool("DisplayLog", orxFALSE);
  }


  /* *** OBJECT UPDATE SECTION *** */

  /* Gets object from context.
   * The helper macro orxOBJECT() will verify if the pointer we want to cast
   * is really an orxOBJECT.
   */
  pstObject = orxOBJECT(_pstContext);

  /* Rotates it according to ellapsed time (complete rotation every 2 seconds) */
  orxObject_SetRotation(pstObject, orxMATH_KF_PI * _pstClockInfo->fTime);


  /* *** CLOCK TIME STRETCHING SECTION *** */

  /* Finds first user created clock (clock1) */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_USER);

  /* Is '+' pressed? */
  if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_ADD))
  {
    /* Makes this clock go four time faster */
    orxClock_SetModifier(pstClock, orxCLOCK_MOD_TYPE_MULTIPLY, orx2F(4.0f));
  }
  /* Is '-' pressed? */
  else if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_SUBTRACT))
  {
    /* Makes this clock go four time slower */
    orxClock_SetModifier(pstClock, orxCLOCK_MOD_TYPE_MULTIPLY, orx2F(0.25f));
  }
  /* Is '*' pressed? */
  else if(orxKeyboard_IsKeyPressed(orxKEYBOARD_KEY_MULTIPLY))
  {
    /* Removes modifier from this clock */
    orxClock_SetModifier(pstClock, orxCLOCK_MOD_TYPE_NONE, orxFLOAT_0);
  }
}


/** Inits the tutorial
 */
orxSTATUS Init()
{
  orxCLOCK  *pstClock1, *pstClock2;
  orxOBJECT *pstObject1, *pstObject2;

  /* Displays a small hint in console */
  orxLOG("\n- Press 'L' to activate log and 'S' to stop it"
         "\n- To stretch time for the first clock (updating the box):"
         "\n . Press numpad '+' to set it 4 times faster"
         "\n . Press numpad '-' to set it 4 times slower"
         "\n . Press numpad '*' to set it back to normal");

  /* Loads config file and selects main section */
  orxConfig_Load("../02_Clock.ini");

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates objects */
  pstObject1 = orxObject_CreateFromConfig("Object1");
  pstObject2 = orxObject_CreateFromConfig("Object2");

  /* Creates two user clocks */
  pstClock1 = orxClock_Create(orx2F(0.01f), orxCLOCK_TYPE_USER);
  pstClock2 = orxClock_Create(orx2F(0.2f), orxCLOCK_TYPE_USER);

  /* Registers our update callback to these clocks with both object as context.
   * The module ID is used to skip the call to this callback if the corresponding module
   * is either not loaded or paused, which won't happen in this tutorial.
   */
  orxClock_Register(pstClock1, Update, pstObject1, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);
  orxClock_Register(pstClock2, Update, pstObject2, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);

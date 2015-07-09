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
 * @file 02_Clock.c
 * @date 09/08/2008
 * @author iarwain@orx-project.org
 *
 * Clock tutorial
 */


#include "orx.h"


/* This is a basic C tutorial creating clocks and playing with them.
 *
 * See tutorial 01_Object for more info about the basic object creation.
 * Here we register our callback on 2 different clocks for didactic purpose only. All objects
 * can of course be updated with only one clock, and the given clock context is also used here 
 * for demonstration only.
 * The first clock runs at 100 Hz and the second one at 5 Hz.
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
void orxFASTCALL Update(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxOBJECT *pstObject;

  /* *** LOG DISPLAY SECTION *** */

  /* Pushes main config section */
  orxConfig_PushSection("Main");

  /* Should display log? */
  if(orxConfig_GetBool("DisplayLog"))
  {
    /* Displays info in log and console */
    orxLOG("<%s>: Time = %.3f / DT = %.3f", orxClock_GetName(orxClock_GetFromInfo(_pstClockInfo)), _pstClockInfo->fTime, _pstClockInfo->fDT);
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* *** OBJECT UPDATE SECTION *** */

  /* Gets object from context.
   * The helper macro orxOBJECT() will verify if the pointer we want to cast
   * is really an orxOBJECT.
   */
  pstObject = orxOBJECT(_pstContext);

  /* Rotates it according to elapsed time (complete rotation every 2 seconds) */
  orxObject_SetRotation(pstObject, orxMATH_KF_PI * _pstClockInfo->fTime);
}

/** Input update callback
 */
void orxFASTCALL InputUpdate(const orxCLOCK_INFO *_pstClockInfo, void *_pstContext)
{
  orxCLOCK  *pstClock;

  /* *** LOG DISPLAY SECTION *** */

  /* Pushes main config section */
  orxConfig_PushSection("Main");

  /* Is log input newly active? */
  if(orxInput_IsActive("Log") && orxInput_HasNewStatus("Log"))
  {
    /* Toggles logging */
    orxConfig_SetBool("DisplayLog", !orxConfig_GetBool("DisplayLog"));
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* *** CLOCK TIME STRETCHING SECTION *** */
  
  /* Finds first user created clock (clock1).
   * We could have stored the clock at creation, of course, but this is done here for didactic purpose. */
  pstClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_USER);

  /* Is faster input active? */
  if(orxInput_IsActive("Faster"))
  {
    /* Makes this clock go four time faster */
    orxClock_SetModifier(pstClock, orxCLOCK_MOD_TYPE_MULTIPLY, orx2F(4.0f));
  }
  /* Is slower input active? */
  else if(orxInput_IsActive("Slower"))
  {
    /* Makes this clock go four time slower */
    orxClock_SetModifier(pstClock, orxCLOCK_MOD_TYPE_MULTIPLY, orx2F(0.25f));
  }
  /* Is normal input active? */
  else if(orxInput_IsActive("Normal"))
  {
    /* Removes modifier from this clock */
    orxClock_SetModifier(pstClock, orxCLOCK_MOD_TYPE_NONE, orxFLOAT_0);
  }
}


/** Inits the tutorial
 */
orxSTATUS orxFASTCALL Init()
{
  orxCLOCK       *pstClock1, *pstClock2, *pstMainClock;
  orxOBJECT      *pstObject1, *pstObject2;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  const orxSTRING zInputLog;
  const orxSTRING zInputFaster;
  const orxSTRING zInputSlower;
  const orxSTRING zInputNormal;

  /* Gets input binding names */
  orxInput_GetBinding("Log", 0, &eType, &eID, &eMode);
  zInputLog     = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("Faster", 0, &eType, &eID, &eMode);
  zInputFaster  = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("Slower", 0, &eType, &eID, &eMode);
  zInputSlower  = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("Normal", 0, &eType, &eID, &eMode);
  zInputNormal  = orxInput_GetBindingName(eType, eID, eMode);

  /* Displays a small hint in console */
  orxLOG("\n- Press '%s' to toggle log display"
         "\n- To stretch time for the first clock (updating the box):"
         "\n . Press numpad '%s' to set it 4 times faster"
         "\n . Press numpad '%s' to set it 4 times slower"
         "\n . Press numpad '%s' to set it back to normal", zInputLog,  zInputFaster, zInputSlower, zInputNormal);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates objects */
  pstObject1 = orxObject_CreateFromConfig("Object1");
  pstObject2 = orxObject_CreateFromConfig("Object2");

  /* Creates two user clocks: a 100Hz and a 5Hz */
  pstClock1 = orxClock_CreateFromConfig("Clock1");
  pstClock2 = orxClock_CreateFromConfig("Clock2");

  /* Registers our update callback to these clocks with both object as context.
   * The module ID is used to skip the call to this callback if the corresponding module
   * is either not loaded or paused, which won't happen in this tutorial.
   */
  orxClock_Register(pstClock1, Update, pstObject1, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);
  orxClock_Register(pstClock2, Update, pstObject2, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Gets main clock */
  pstMainClock = orxClock_FindFirst(orx2F(-1.0f), orxCLOCK_TYPE_CORE);

  /* Registers our input update callback to it
   * !!IMPORTANT!! *DO NOT* handle inputs in clock callbacks that are *NOT* registered to the main clock
   * you might miss input status updates if the user clock runs slower than the main one
   */
  orxClock_Register(pstMainClock, InputUpdate, orxNULL, orxMODULE_ID_MAIN, orxCLOCK_PRIORITY_NORMAL);

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/** Run function
 */
orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Should quit? */
  if(orxInput_IsActive("Quit"))
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Exit function
 */
void orxFASTCALL Exit()
{
  /* We're a bit lazy here so we let orx clean all our mess! :) */
}

/** Main function
 */
int main(int argc, char **argv)
{
  /* Executes a new instance of tutorial */
  orx_Execute(argc, argv, Init, Run, Exit);

  return EXIT_SUCCESS;
}


#ifdef __orxMSVC__

// Here's an example for a console-less program under windows with visual studio
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Inits and executes orx
  orx_WinExecute(Init, Run, Exit);

  // Done!
  return EXIT_SUCCESS;
}

#endif // __orxMSVC__

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
 * @file 10_StandAlone.cpp
 * @date 30/08/2008
 * @author iarwain@orx-project.org
 *
 * Stand alone tutorial
 */


/* This is a basic C++ tutorial showing how to write a stand alone executable using orx.
 * As we are *NOT* using the default executable anymore for this tutorial, the tutorial
 * code will be directly in the executable and not in an external library.
 *
 * This implies that we will *NOT* have the default hardcoded behavior:
 * - F11 will not affect vertical sync toggler
 * - Escape won't automagically exit
 * - F12 won't capture a screenshot
 * - Backspace won't reload configuration files
 * - the [Main] section in the config file won't be used to load a plugin ("GameFile" key)
 *
 * Our program, by default, will also *NOT* exit if it receives the orxSYSTEM_EVENT_CLOSE event.
 * To do so, we will either have to use the helper orx_Execute() function (see below)
 * or handle it ourselves.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation,
 * viewport, sound, FX, physics/collision and parallax scrolling handling.
 *
 * This tutorial shows how to create your stand alone game using orx as a library.
 *
 * As we're on our own here, we need to write the main function and initialize orx manually.
 * The good thing is that we can then specify which modules we want to use, and deactivates display
 * or any other module at will, if needed.
 *
 * If we still want a semi-automated initialization of orx, we can use the orx_Execute() function.
 * This tutorial will cover the use of orx with this helper function, but you can decide not to use
 * it if its behavior doesn't suit your needs.
 *
 * This helper function will take care of initializing everything correctly and exiting properly.
 * It will also make sure the clock module is constantly ticked (as it's part of orx's core) and that
 * we exit if the orxSYSTEM_EVENT_CLOSE event is sent.
 * This event is sent when closing the windows, for example, but it can also be sent under
 * your own criteria (escape key pressed, for example).
 *
 * This code is a basic C++ example to show how to use orx without having to write C code.
 *
 * This tutorial could have been architectured in a better way (cutting it into pieces with headers files, for example)
 * but we wanted to keep a single file per *basic* tutorial.
 *
 * This stand alone executable also creates a console (as does the default orx executable),
 * but you can have you own console-less program if you wish. In order to achieve that, you only need to provide
 * an argc/argv style parameter list that contains the executable name, otherwise the default loaded config file will be
 * orx.ini instead of being based on our executable name (ie. 10_StandAlone.ini).
 *
 * For visual studio users (windows), it can easily be achieved by writing a WinMain() function instead of main(),
 * and by getting the executable name (or hardcoding it).
 *
 * This tutorial simply display orx's logo and a localized legend. Press space to cycle through
 * all the availables languages for the legend's text.
 *
 * Some explanations about core elements that you can find in this tutorial:
 *
 * - Run function: Don't put *ANY* logic code here, it's only a backbone where you can handle default
 *   core behaviors (tracking exit or changing locale, for example) or profile some stuff.
 *   As it's directly called from the main loop and not part of the clock system, time consistency can't be enforced.
 *   For all your main game execution, please create (or use an existing) clock and register your callback to it.
 *
 * - Event handlers:
 *   When an event handler returns orxSTATUS_SUCCESS, no other handler will be called after it
 *   for the same event.
 *   On the other hand, if orxSTATUS_FAILURE is returned, event processing will continue for this event
 *   if other handlers are listening this event type.
 *   We'll monitor locale events to update our legend's text when the selected language is changed.
 *
 * - orx_Execute():
 *   Inits and executes orx using our self-defined functions (Init, Run and Exit).
 *   We can of course not use this helper and handles everything manually if its
 *   behavior doesn't suit our needs.
 *   You can have a look at the content of orx_Execute() (which is implemented in orx.h) to have
 *   a better idea on how to do this.
 */


// Include orx.h
#include "orx.h"

#include <iostream>

// !!! First we define our logo object !!!

// Our logo class
class Logo
{
private:
  // Our orxOBJECT self reference
  orxOBJECT *m_pstObject;

  // Our orxOBJECT reference to the legend
  orxOBJECT *m_pstLegend;

public:
  Logo();
  ~Logo();
};

// C-tor
Logo::Logo()
{
  // Creates and stores our orxOBJECT
  m_pstObject = orxObject_CreateFromConfig("Logo");

  // Adds ourselves to the orxOBJECT
  orxObject_SetUserData(m_pstObject, this);

  // Creates and stores our legend object
  m_pstLegend = orxObject_CreateFromConfig("Legend");
}

// D-tor
Logo::~Logo()
{
  // Deletes our orxOBJECTs
  orxObject_Delete(m_pstObject);
  orxObject_Delete(m_pstLegend);
}


// !!! Stand alone class that contains our core callbacks !!!


// Stand alone class
// NB: The callbacks could have been defined out of any class
// This is just to show how to do it if you need it
class StandAlone
{
public:
  static orxSTATUS orxFASTCALL  EventHandler(const orxEVENT *_pstEvent);
  static orxSTATUS orxFASTCALL  Init();
  static void orxFASTCALL       Exit();
  static orxSTATUS orxFASTCALL  Run();

  void SelectNextLanguage();

  StandAlone() : m_poLogo(NULL), s32LanguageIndex(0) {};
  ~StandAlone() {};

private:
  orxSTATUS                     InitGame();

  Logo  *m_poLogo;
  orxS32 s32LanguageIndex;
};


// Local instance
StandAlone soMyStandAloneGame;

// Select next language
void StandAlone::SelectNextLanguage()
{
  // Updates language index
  s32LanguageIndex = (s32LanguageIndex == orxLocale_GetLanguageCounter() - 1) ? 0 : s32LanguageIndex + 1;

  // Selects it
  orxLocale_SelectLanguage(orxLocale_GetLanguage(s32LanguageIndex));
}

// Init game function
orxSTATUS StandAlone::InitGame()
{
  orxSTATUS eResult;

  // We register our event handler
  eResult = orxEvent_AddHandler(orxEVENT_TYPE_LOCALE, EventHandler);

  // Successful?
  if(eResult != orxSTATUS_FAILURE)
  {
    // Creates logo
    m_poLogo = new Logo();

    // Displays all the available languages, C++ style
    std::cout << "The available languages are:" << std::endl;
    for(orxS32 i = 0; i < orxLocale_GetLanguageCounter(); i++)
    {
      std::cout << " - " << orxLocale_GetLanguage(i) << std::endl;
    }

    // Creates viewport
    orxViewport_CreateFromConfig("Viewport");
  }

  // Done!
  return eResult;
}

// Event handler
orxSTATUS orxFASTCALL StandAlone::EventHandler(const orxEVENT *_pstEvent)
{
  // Depending on event ID
  switch(_pstEvent->eID)
  {
    case orxLOCALE_EVENT_SELECT_LANGUAGE:
    {
      orxLOCALE_EVENT_PAYLOAD *pstPayload;

      // Gets locale payload
      pstPayload = (orxLOCALE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

      // Logs info
      orxLOG("Switching to '%s'.", pstPayload->zLanguage);

      break;
    }

    default:
    {
      break;
    }
  }

  // Done!
  return orxSTATUS_SUCCESS;
}

// Init function
orxSTATUS StandAlone::Init()
{
  orxSTATUS     eResult;
  orxINPUT_TYPE eType;
  orxENUM       eID;
  orxSTRING     zInputQuit, zInputCycle;

  /* Gets input binding names */
  orxInput_GetBinding("Quit", 0, &eType, &eID);
  zInputQuit = orxInput_GetBindingName(eType, eID);
  orxInput_GetBinding("CycleLanguage", 0, &eType, &eID);
  zInputCycle = orxInput_GetBindingName(eType, eID);

  // Logs
  orxLOG("\n- '%s' will exit from this tutorial"
         "\n- '%s' will cycle through all the available languages"
         "\n* The legend under the logo is always displayed in the current language", zInputQuit, zInputCycle);

  orxLOG("10_StandAlone Init() called!");

  // Inits our stand alone game
  eResult = soMyStandAloneGame.InitGame();

  // Done!
  return eResult;
}

// Exit function
void StandAlone::Exit()
{
  // Deletes our logo
  delete soMyStandAloneGame.m_poLogo;
  soMyStandAloneGame.m_poLogo = NULL;

  // Logs
  orxLOG("10_StandAlone Exit() called!");
}

// Run function
orxSTATUS StandAlone::Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Cycle action is active?
  if(orxInput_IsActive("CycleLanguage") && orxInput_HasNewStatus("CycleLanguage"))
  {
    // Selects next language
    soMyStandAloneGame.SelectNextLanguage();
  }

  // Is quit action active?
  if(orxInput_IsActive("Quit"))
  {
    // Logs
    orxLOG("Quit action triggered, exiting!");

    // Sets return value to orxSTATUS_FAILURE, meaning we want to exit
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}


// !!! Main program code !!!


// Main program function
int main(int argc, char **argv)
{
  // Inits and runs orx using our self-defined functions
  orx_Execute(argc, argv, StandAlone::Init, StandAlone::Run, StandAlone::Exit);

  // Done!
  return EXIT_SUCCESS;
}


#ifdef __orxMSVC__

#include "windows.h"

// Here's an example for a console-less program under windows with visual studio
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Boo! Hardcoded executable name. Command line parameters won't be passed to orx! ^^
  orxSTRING az[] =
  {
    "10_StandAlone.exe"
  };

  // Inits and executes orx
  orx_Execute(1, az, StandAlone::Init, StandAlone::Run, StandAlone::Exit);

  // Done!
  return EXIT_SUCCESS;
}

#endif // __orxMSVC__

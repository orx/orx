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
 * @file 10_Locale.cpp
 * @date 30/08/2008
 * @author iarwain@orx-project.org
 *
 * Locale / C++ tutorial
 */


/* This is a basic C++ tutorial showing how to write a program in C++ using orx.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation,
 * viewport, sound, FX, physics/collision and parallax scrolling handling.
 *
 * This tutorial shows how to create your game using C++ and orx as a library.
 * This code is a basic C++ example to show how to use orx without having to write C code.
 *
 * This tutorial could have been architectured in a better way (cutting it into pieces with headers files, for example)
 * but we wanted to keep a single file per *basic* tutorial.
 *
 * This program also creates a console (as does the default orx executable),
 * but you can have you own console-less program if you wish. In order to achieve that, you only need to provide
 * an argc/argv style parameter list that contains the executable name, otherwise the default loaded config file will be
 * orx.ini instead of being based on our executable name (ie. 10_Locale.ini).
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


// !!! Game class that contains our core callbacks !!!


// Game class
// NB: The callbacks could have been defined out of any class
// This is just to show how to do it if you need it
class Game
{
public:
  static orxSTATUS orxFASTCALL  EventHandler(const orxEVENT *_pstEvent);
  static orxSTATUS orxFASTCALL  Init();
  static void orxFASTCALL       Exit();
  static orxSTATUS orxFASTCALL  Run();

  void SelectNextLanguage();

  Game() : m_poLogo(NULL), u32LanguageIndex(0) {};
  ~Game() {};

private:
  orxSTATUS                     InitGame();

  Logo  *m_poLogo;
  orxU32 u32LanguageIndex;
};


// Local instance
Game soMyGame;

// Select next language
void Game::SelectNextLanguage()
{
  // Updates language index
  u32LanguageIndex = (u32LanguageIndex == orxLocale_GetLanguageCounter() - 1) ? 0 : u32LanguageIndex + 1;

  // Selects it
  orxLocale_SelectLanguage(orxLocale_GetLanguage(u32LanguageIndex));
}

// Init game function
orxSTATUS Game::InitGame()
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
    for(orxU32 i = 0; i < orxLocale_GetLanguageCounter(); i++)
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
orxSTATUS orxFASTCALL Game::EventHandler(const orxEVENT *_pstEvent)
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
orxSTATUS Game::Init()
{
  orxSTATUS       eResult;
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxINPUT_MODE   eMode;
  const orxSTRING zInputQuit;
  const orxSTRING zInputCycle;

  /* Gets input binding names */
  orxInput_GetBinding("Quit", 0, &eType, &eID, &eMode);
  zInputQuit = orxInput_GetBindingName(eType, eID, eMode);
  orxInput_GetBinding("CycleLanguage", 0, &eType, &eID, &eMode);
  zInputCycle = orxInput_GetBindingName(eType, eID, eMode);

  // Logs
  orxLOG("\n- '%s' will exit from this tutorial"
         "\n- '%s' will cycle through all the available languages"
         "\n* The legend under the logo is always displayed in the current language", zInputQuit, zInputCycle);

  orxLOG("10_Locale Init() called!");

  // Inits our game
  eResult = soMyGame.InitGame();

  // Done!
  return eResult;
}

// Exit function
void Game::Exit()
{
  // Deletes our logo
  delete soMyGame.m_poLogo;
  soMyGame.m_poLogo = NULL;

  // Logs
  orxLOG("10_Locale Exit() called!");
}

// Run function
orxSTATUS Game::Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Cycle action is active?
  if(orxInput_IsActive("CycleLanguage") && orxInput_HasNewStatus("CycleLanguage"))
  {
    // Selects next language
    soMyGame.SelectNextLanguage();
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
  orx_Execute(argc, argv, Game::Init, Game::Run, Game::Exit);

  // Done!
  return EXIT_SUCCESS;
}


#ifdef __orxMSVC__

// Here's an example for a console-less program under windows with visual studio
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  // Inits and executes orx
  orx_WinExecute(Game::Init, Game::Run, Game::Exit);

  // Done!
  return EXIT_SUCCESS;
}

#endif // __orxMSVC__

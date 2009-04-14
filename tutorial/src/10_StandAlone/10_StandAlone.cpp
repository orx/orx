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
 * We thus are able to specify what we need (physics or not, display or not, etc...).
 * However we will *NOT* have the default hardcoded behavior:
 * - F11 will not affect vertical sync toggler
 * - Escape won't exit
 * - Backspace won't reload configuration files
 * - the [Main] section in the config file won't be used to load a plugin ("GameFile" key)
 *
 * Our program, by default, will also *NOT* exit if it receives the orxSYSTEM_EVENT_CLOSE event.
 * To do so, we will either have to use the helper orx_Execute() function (see below)
 * or handle it ourselves.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation,
 * viewport, sound, FX, physics/collision and differentiel scrolling handling.
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
 * It will also make sure the clock module is constantly ticked (as it's orx's core) and that
 * we exit if the orxSYSTEM_EVENT_CLOSE event is sent.
 * This event is sent when closing the windows, for example, but it can also be sent under
 * your own criteria (escape key pressed, for example).
 * We'll see that even if this event is handled by default, we can still override it.
 * To give an example, this tutorial won't close using the default scheme as you will see in its code.
 *
 * This code is a basic C++ example to show how to use orx without having to write C code.
 *
 * This tutorial could have been architectured in a better way (cutting it into pieces with headers files, for example)
 * but we wanted to keep a single file per *basic* tutorial.
 *
 * This stand alone executable also creates a console (as does the default orx executable),
 * but you can have you own console-less program if you wish. You only need to provide an arc/argv style
 * parameter list that contains the executable name, otherwise the default config file will be
 * orx.ini instead of 10_StandAlone.ini here.
 *
 * For windows visual studio users, it can easily be achieved by writing a WinMain() function instead of main(),
 * and by getting the executable name (or hardcoding it).
 *
 * This tutorial simply display an orx logo, and will only exit if backspace is pressed. If you try to exit using
 * the windows 'X' button or by pressing alt-F4, a message will be displayed and the logo color will change.
 *
 * Some explanations about core elements you can find in this tutorial:
 *
 * - Run function: Don't put *ANY* logic code here, it's only a backbone where you can handle default
 *   core behaviors (tracking exit, for example) or profile some stuff.
 *   As it's directly called from the main loop and not part of the clock system, time consistency can't be achieved.
 *   For all your main game execution, please create (or use an existing) clock and register your callback to it.
 *
 * - Event handlers:
 *   When an event handler returns orxSTATUS_SUCCESS, no other handler will be called after it
 *   for the same event.
 *   For example, that's how we inhibit the default exit behavior for orxSYSTEM_EVENT_CLOSE event.
 *   On the other hand, if orxSTATUS_FAILURE is returned, event processing will continue for this one
 *   if other handlers are listening this event type.
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


// !!! First we define our logo object !!!

// Our logo class
class Logo
{
private:
  // Our orxOBJECT self reference
  orxOBJECT *m_pstObject;

  // Our internal color structure
  orxCOLOR  m_stMyColor;

public:
  Logo();
  ~Logo();

  void SetConfigColor();
};

// C-tor
Logo::Logo()
{
  // Creates and stores our orxOBJECT
  m_pstObject = orxObject_CreateFromConfig("Logo");

  // Adds ourselves to the orxOBJECT
  orxObject_SetUserData(m_pstObject, this);

  // Gets the default color
  orxObject_GetColor(m_pstObject, &m_stMyColor);
}

// D-tor
Logo::~Logo()
{
  // Deletes our orxOBJECT
  orxObject_Delete(m_pstObject);
}

// Sets config color
void Logo::SetConfigColor()
{
  // Selects tutorial section
  orxConfig_SelectSection("Tutorial");

  // Updates our color with config values
  m_stMyColor.fAlpha = orxConfig_GetFloat("LogoAlpha");
  orxConfig_GetVector("LogoColor", &(m_stMyColor.vRGB));
  orxVector_Mulf(&(m_stMyColor.vRGB), &(m_stMyColor.vRGB), orxRGBA_NORMALIZER);

  // Applies it
  orxObject_SetColor(m_pstObject, &m_stMyColor);
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

private:
  orxSTATUS                     InitGame();

  Logo *m_poLogo;
};


// Local instance
StandAlone soMyStandAloneGame;


// Init game function
orxSTATUS StandAlone::InitGame()
{
  // Creates logo
  m_poLogo = new Logo();

  // Creates viewport
  orxViewport_CreateFromConfig("Viewport");

  // We register our event handler
  return orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, EventHandler);
}

// Event handler
orxSTATUS orxFASTCALL StandAlone::EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult;

  // Depending on event ID
  switch(_pstEvent->eID)
  {
    // Close event?
    case orxSYSTEM_EVENT_CLOSE:
    {
      orxINPUT_TYPE eType;
      orxENUM       eID;
      orxSTRING     zInputQuit;

      /* Gets input binding name */
      orxInput_GetBinding("Quit", 0, &eType, &eID);
      zInputQuit = orxInput_GetBindingName(eType, eID);

      // Logs
      orxLOG("Default close/exit is inhibited.\nPlease press '%s' to exit!", zInputQuit);

      // Changes logo color
      soMyStandAloneGame.m_poLogo->SetConfigColor();

      // Inhibits further handling for this event
      eResult = orxSTATUS_SUCCESS;

      break;
    }

    default:
    {
      // Allows further handling for this event
      eResult = orxSTATUS_FAILURE;
      break;
    }
  }

  // Done!
  return eResult;
}

// Init function
orxSTATUS StandAlone::Init()
{
  orxSTATUS     eResult;
  orxINPUT_TYPE eType;
  orxENUM       eID;
  orxSTRING     zInputQuit;

  /* Gets input binding names */
  orxInput_GetBinding("Quit", 0, &eType, &eID);
  zInputQuit = orxInput_GetBindingName(eType, eID);

  // Logs
  orxLOG("\n- '%s' will exit from this tutorial"
         "\n* Alt-F4 and window closing button won't exit"
         "\n* If you try them, a message will be displayed and the logo color will change", zInputQuit);

  orxLOG("10_StandAlone Init() called!");

  // Inits our stand alone
  eResult = soMyStandAloneGame.InitGame();

  /* Done! */
  return eResult;
}

// Exit function
void StandAlone::Exit()
{
  // Deletes our logo
  delete soMyStandAloneGame.m_poLogo;

  // Logs
  orxLOG("10_StandAlone Exit() called!");
}

// Run function
orxSTATUS StandAlone::Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Is quit action active? */
  if(orxInput_IsActive("Quit"))
  {
    // Logs
    orxLOG("Quit action triggered, exiting!");

    // Sets return value to failure, meaning we want to exit
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

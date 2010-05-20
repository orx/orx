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
 * @file 01_Object.c
 * @date 04/08/2008
 * @author iarwain@orx-project.org
 *
 * Object creation tutorial
 */


#include "orxPluginAPI.h"


/* This is a basic C tutorial creating a viewport and an object.
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
 * As orx is data driven, here we just write 2 lines of code to create a viewport
 * and an object. All their properties can be defined in the config file (01_Object.ini).
 * As a matter of fact, the viewport is associated with a camera implicitly created from the
 * info given in the config file. You can also set their sizes, positions, the object colors,
 * scales, rotations, animations, physical properties, and so on. You can even request
 * random values for these without having to add a single line of code.
 * In a later tutorial we'll see how to generate your whole scene (all background
 * and landscape objects for example) with a simple for loop written in 3 lines of code.
 *
 * For now, you can try to uncomment some of the lines of 01_Object.ini, play with them,
 * then relaunch this tutorial. For an exhaustive list of options, please look at CreationTemplate.ini.
 */


/** Inits the tutorial
 */
orxSTATUS Init()
{
  /* Displays a small hint in console */
  orxLOG("\n* This tutorial creates a viewport/camera couple and an object"
         "\n* You can play with the config parameters in ../01_Object.ini"
         "\n* After changing them, relaunch the tutorial to see their effects");

  /* Loads config file */
  orxConfig_Load("../01_Object.ini");

  /* Reloads inputs */
  orxInput_Load(orxSTRING_EMPTY);

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates object */
  orxObject_CreateFromConfig("Object");

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);

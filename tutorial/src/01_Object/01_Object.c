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

  /* Creates viewport */
  orxViewport_CreateFromConfig("Viewport");

  /* Creates object */
  orxObject_CreateFromConfig("Object");

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/* Registers plugin entry */
orxPLUGIN_DECLARE_ENTRY_POINT(Init);

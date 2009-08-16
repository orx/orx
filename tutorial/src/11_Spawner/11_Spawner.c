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
 * @file 11_Spawner.c
 * @date 03/04/2009
 * @author iarwain@orx-project.org
 *
 * Spawner tutorial
 */


/* This is a basic C tutorial showing how to use spawners.
 * As we are *NOT* using the default executable anymore for this tutorial, the tutorial
 * code will be directly built into the executable and not into an external library.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation,
 * viewport, sound, FX, physics/collision, differentiel scrolling handling and stand alone creation.
 *
 * This tutorial shows how to create and use spawners for particle. It's only a tiny possibility
 * of what one can achieve using them. For example, they can be also used for generating monsters or
 * firing bullets.
 *
 * The code is only used for two tasks:
 * - creating 1 main objects and a viewport
 * - switching from one test to the other by reloading the appropriate config files
 *
 * Beside that, this tutorial is completely data-driven: the different test settings and the input definitions
 * are stored in config along with all the spawning/move/display logic.
 *
 * With this very few amount of lines of code, you can have an infinite number of results: playing with physics,
 * additive/multiply blend, masking, speed/acceleration of objects, ... is all up to you. All you need is changing
 * the config files. You can even test your changes without restarting this tutorial: config files will
 * be entirely reloaded when switching from a test to another.
 *
 * If there are too many particles displayed for your config, just turn down the amount of particles spawned
 * per wave and/or the frequency of the waves. To do so, search for the WaveNumber/WaveDelay attributes in the different
 * spawner sections. Have fun! =)
 *
 */


/* Include orx.h */
#include "orx.h"

/** Local storage
 */
static orxS32       ss32ConfigID  = 0;
static orxVIEWPORT *pstViewport   = orxNULL;
static orxOBJECT   *pstScene      = orxNULL;


/** Loads configuration
 */
static orxINLINE orxSTATUS LoadConfig()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Deletes our scene */
  if(pstScene)
  {
    orxObject_Delete(pstScene);
    pstScene = orxNULL;
  }

  /* Deletes our viewport */
  if(pstViewport)
  {
    orxViewport_Delete(pstViewport);
    pstViewport = orxNULL;
  }

  /* Clears all config data */
  orxConfig_Clear();

  /* Loads main config and selects tutorial section */
  orxConfig_Load(orxConfig_GetMainFileName());
  orxConfig_SelectSection("Tutorial");

  /* Is current ID valid? */
  if(ss32ConfigID < orxConfig_GetListCounter("ConfigList"))
  {
    orxSTRING zConfigFile;

    /* Gets config file */
    zConfigFile = orxConfig_GetListString("ConfigList", ss32ConfigID);

    /* Can load it? */
    if((eResult = orxConfig_Load(zConfigFile)) != orxSTATUS_FAILURE)
    {
      /* Creates viewport */
      pstViewport = orxViewport_CreateFromConfig("Viewport");

      /* Creates our scene */
      pstScene = orxObject_CreateFromConfig("Scene");
    }
  }

  /* Done! */
  return eResult;
}

/** Init function
 */
orxSTATUS orxFASTCALL Init()
{
  orxINPUT_TYPE   eType;
  orxENUM         eID;
  orxSTRING       zInputNextConfig, zInputPreviousConfig;

  /* Gets binding names */
  orxInput_GetBinding("NextConfig", 0, &eType, &eID);
  zInputNextConfig = orxInput_GetBindingName(eType, eID);

  orxInput_GetBinding("PreviousConfig", 0, &eType, &eID);
  zInputPreviousConfig = orxInput_GetBindingName(eType, eID);

  /* Displays a small hint in console */
  orxLOG("\n- '%s' will switch to the next config settings"
         "\n- '%s' will switch to the previous config settings"
         "\n* Config files are used with inheritance to provide all the combinations"
         "\n* All the tests use the same minimalist code (creating 1 object & 1 viewport)",
         zInputNextConfig, zInputPreviousConfig);

  /* Loads default configuration */
  return LoadConfig();
}

/** Run function
 */
orxSTATUS orxFASTCALL Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Next config requested? */
  if(orxInput_IsActive("NextConfig") && orxInput_HasNewStatus("NextConfig"))
  {
    /* Updates config ID */
    ss32ConfigID = (ss32ConfigID < orxConfig_GetListCounter("ConfigList") - 1) ? ss32ConfigID + 1 : 0;

    /* Loads it */
    LoadConfig();
  }
  /* Previous config requested? */
  else if(orxInput_IsActive("PreviousConfig") && orxInput_HasNewStatus("PreviousConfig"))
  {
    /* Updates config ID */
    ss32ConfigID = (ss32ConfigID > 0) ? ss32ConfigID - 1 : orxConfig_GetListCounter("ConfigList") - 1;

    /* Loads it */
    LoadConfig();
  }
  /* Should quit? */
  else if(orxInput_IsActive("Quit"))
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
}

/** Main function
 */
int main(int argc, char **argv)
{
  /* Executes a new instance of tutorial */
  orx_Execute(argc, argv, Init, Run, Exit);

  return EXIT_SUCCESS;
}

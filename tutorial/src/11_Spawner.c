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
 * @file 11_Spawner.c
 * @date 03/04/2009
 * @author iarwain@orx-project.org
 *
 * Spawner tutorial
 */


/* This is a basic C tutorial showing how to use spawners.
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
 * per wave and/or the frequency of the waves. To do so, search for the WaveSize/WaveDelay attributes in the different
 * spawner sections. Have fun! =)
 *
 */


/* Include orx.h */
#include "orx.h"

/** Local storage
 */
static orxS32       ss32ConfigID  = 0;
static orxOBJECT   *pstScene      = orxNULL;


/** Loads configuration
 */
static orxINLINE orxSTATUS LoadConfig()
{
  orxVIEWPORT  *pstViewport;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* Deletes our scene */
  if(pstScene)
  {
    orxObject_Delete(pstScene);
    pstScene = orxNULL;
  }

  /* For all the viewports */
  while((pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT))) != orxNULL)
  {
    /* Deletes it */
    orxViewport_Delete(pstViewport);
  }

  /* Clears all config data */
  orxConfig_Clear();

  /* Loads main config and selects tutorial section */
  orxConfig_Load(orxConfig_GetMainFileName());
  orxConfig_SelectSection("Tutorial");

  /* Is current ID valid? */
  if(ss32ConfigID < orxConfig_GetListCounter("ConfigList"))
  {
    const orxSTRING zConfigFile;

    /* Gets config file */
    zConfigFile = orxConfig_GetListString("ConfigList", ss32ConfigID);

    /* Can load it? */
    if((eResult = orxConfig_Load(zConfigFile)) != orxSTATUS_FAILURE)
    {
      orxS32 i;

      /* Pushes tutorial section */
      orxConfig_PushSection("Tutorial");

      /* For all defined viewports */
      for(i = 0; i < orxConfig_GetListCounter("ViewportList"); i++)
      {
        /* Creates it */
        orxViewport_CreateFromConfig(orxConfig_GetListString("ViewportList", i));
      }

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
  orxINPUT_MODE   eMode;
  const orxSTRING zInputNextConfig;
  const orxSTRING zInputPreviousConfig;

  /* Gets binding names */
  orxInput_GetBinding("NextConfig", 0, &eType, &eID, &eMode);
  zInputNextConfig = orxInput_GetBindingName(eType, eID, eMode);

  orxInput_GetBinding("PreviousConfig", 0, &eType, &eID, &eMode);
  zInputPreviousConfig = orxInput_GetBindingName(eType, eID, eMode);

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

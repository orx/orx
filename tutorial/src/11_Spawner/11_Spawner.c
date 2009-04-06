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
 * @date 03/04/2009
 * @author iarwain@orx-project.org
 *
 * Spawner tutorial
 */


/* This is a basic C tutorial showing how to use spawners.
 * As we are *NOT* using the default executable anymore for this tutorial, the tutorial
 * code will be directly in the executable and not in an external library.
 *
 * See previous tutorials for more info about the basic object creation, clock, animation,
 * viewport, sound, FX, physics/collision, differentiel scrolling handling and stand alone creation.
 *
 * This tutorial shows how to create and use spawners for particle. It's only a tiny possibility
 * of what one can achieve using them. For example, they can be also used for generating monsters or
 * firing bullets.
 *
 * The code is only used for two tasks:
 * - creating 2 main objects (spawner and optional mask) and a viewport
 * - switching from one test to the other by reloading the appropriate config files
 *
 * Beside that, this tutorial is completely data-driven: the different test settings and the input definitions
 * are stored in config along with all the spawning/move/display logic.
 *
 * With this very few amount of lines of code, you can have an infinite number of results: playing with physics,
 * additional/multiply blend, masking, speed/acceleration of objects, ... is all up to you. All you need is changing
 * the config files and you can even test your experience without restarting this tutorial as config files will
 * be reloaded when switching from a test to another.
 *
 * If there are too many particles displayed for your config, just turn down the amount of particles spawned
 * per wave and/or the waves frequency. For that, search for the WaveNumber/WaveDelay properties in the different
 * spawner sections. Have fun! =)
 *
 */


/* Include orx.h */
#include "orx.h"

/** Local storage
 */
static orxS32 ss32ConfigID = 0;


/** Loads configuration
 */
static orxINLINE orxSTATUS LoadConfig()
{
  orxOBJECT    *pstObject;
  orxVIEWPORT  *pstViewport;
  orxSTATUS     eResult = orxSTATUS_FAILURE;

  /* For all objects */
  while((pstObject = orxOBJECT(orxStructure_GetFirst(orxSTRUCTURE_ID_OBJECT))))
  {
    /* Deletes it */
    orxObject_Delete(pstObject);
  }

  /* For all viewports */
  while((pstViewport = orxVIEWPORT(orxStructure_GetFirst(orxSTRUCTURE_ID_VIEWPORT))))
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
  if(ss32ConfigID < orxConfig_GetListCounter("ConfigIDList"))
  {
    orxSTRING zFileList;

    /* Gets config file list */
    zFileList = orxConfig_GetListString("ConfigIDList", ss32ConfigID);

    // Valid?
    if(zFileList != orxSTRING_EMPTY)
    {
      orxS32 i, s32Number;

      /* For all defined config files */
      for(i = 0, s32Number = orxConfig_GetListCounter(zFileList); i < s32Number; i++)
      {
        /* Loads it */
        orxConfig_Load(orxConfig_GetListString(zFileList, i));
      }

      /* Creates viewport */
      orxViewport_CreateFromConfig("Viewport");

      /* Creates both objets */
      orxObject_CreateFromConfig("Mask");
      orxObject_CreateFromConfig("ParticleSpawner");

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Init function
 */
orxSTATUS Init()
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
         "\n* All the tests use the same minimalist code (creating 2 objects & 1 viewport)",
         zInputNextConfig, zInputPreviousConfig);

  /* Loads default configuration */
  return LoadConfig();
}

/** Run function
 */
orxSTATUS Run()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Next config requested? */
  if(orxInput_IsActive("NextConfig") && orxInput_HasNewStatus("NextConfig"))
  {
    /* Updates config ID */
    ss32ConfigID = (ss32ConfigID < orxConfig_GetListCounter("ConfigIDList") - 1) ? ss32ConfigID + 1 : 0;

    /* Loads it */
    LoadConfig();
  }
  /* Previous config requested? */
  else if(orxInput_IsActive("PreviousConfig") && orxInput_HasNewStatus("PreviousConfig"))
  {
    /* Updates config ID */
    ss32ConfigID = (ss32ConfigID > 0) ? ss32ConfigID - 1 : orxConfig_GetListCounter("ConfigIDList") - 1;

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
void Exit()
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

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2019 Orx-Project
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
 * @file [name].cpp
 * @date [date]
 *
 * Orx Game Template
 */


#include "orx.h"


/*
 * This is a basic code template to quickly and easily get started with a project or tutorial.
 */


/** Init function, is called when all orx's modules have been initialized
 */
orxSTATUS orxFASTCALL Init()
{
    // Display a small hint in console
    orxLOG("\n* This template project creates a viewport/camera couple and an object"
    "\n* You can play with the config parameters in ../data/config/[name].ini"
    "\n* After changing them, relaunch the executable to see the changes.");

    // Create the viewport
    orxViewport_CreateFromConfig("Viewport");

    // Create the object
    orxObject_CreateFromConfig("Object");

    // Done!
    return orxSTATUS_SUCCESS;
}

/** Run function, is called every clock cycle
 */
orxSTATUS orxFASTCALL Run()
{
    orxSTATUS eResult = orxSTATUS_SUCCESS;

    // Should quit?
    if(orxInput_IsActive("Quit"))
    {
        // Update result
        eResult = orxSTATUS_FAILURE;
    }

    // Done!
    return eResult;
}

/** Exit function, is called before exiting from orx
 */
void orxFASTCALL Exit()
{
    // Let Orx clean all our mess automatically. :)
}

/** Bootstrap function, called before config is initialized, allowing for early resource storage definition
 */
orxSTATUS orxFASTCALL Bootstrap()
{
    // Add a config storage to find the initial config file
    orxResource_AddStorage(orxCONFIG_KZ_RESOURCE_GROUP, "../data/config", orxFALSE);

    // Return orxSTATUS_FAILURE to prevent orx from loading the default config file
    return orxSTATUS_SUCCESS;
}

/** Main function
 */
int main(int argc, char **argv)
{
    // Set the bootstrap function to provide at least one resource storage before loading any config files
    orxConfig_SetBootstrap(Bootstrap);

    // Execute our game
    orx_Execute(argc, argv, Init, Run, Exit);

    // Done!
    return EXIT_SUCCESS;
}

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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
 * @file orxSoundSystem.c
 * @date 07/01/2009
 * @author simons.philippe@gmail.com
 *
 * Dummy sound system plugin implementation
 *
 */


#include "orxPluginAPI.h"

orxSTATUS orxFASTCALL orxSoundSystem_Dummy_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  orxDEBUG_PRINT(orxDEBUG_LEVEL_SOUND, "orxSoundSystem not yet implemented on this platform");
  
  /* Done! */
  return eResult;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

orxPLUGIN_USER_CORE_FUNCTION_START(SOUNDSYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxSoundSystem_Dummy_Init, SOUNDSYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_END();

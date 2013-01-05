/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
 * @file orxScreenshot.h
 * @date 07/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxScreenshot
 * 
 * Screenshot module
 * Module that captures screenshots
 *
 * @{
 */


#ifndef _orxSCREENSHOT_H_
#define _orxSCREENSHOT_H_

#include "orxInclude.h"


/** Misc
 */

#if defined(__orxIOS__)

  #define orxSCREENSHOT_KZ_DEFAULT_DIRECTORY_NAME "../Documents"
  #define orxSCREENSHOT_KZ_DEFAULT_BASE_NAME      "screenshot-"
  #define orxSCREENSHOT_KZ_DEFAULT_EXTENSION      "png"
  #define orxSCREENSHOT_KU32_DEFAULT_DIGITS       4

#elif defined(__orxANDROID__)

  #define orxSCREENSHOT_KZ_DEFAULT_DIRECTORY_NAME "."
  #define orxSCREENSHOT_KZ_DEFAULT_BASE_NAME      "screenshot-"
  #define orxSCREENSHOT_KZ_DEFAULT_EXTENSION      "png"
  #define orxSCREENSHOT_KU32_DEFAULT_DIGITS       4

#else

  #define orxSCREENSHOT_KZ_DEFAULT_DIRECTORY_NAME "."
  #define orxSCREENSHOT_KZ_DEFAULT_BASE_NAME      "screenshot-"
  #define orxSCREENSHOT_KZ_DEFAULT_EXTENSION      "tga"
  #define orxSCREENSHOT_KU32_DEFAULT_DIGITS       4

#endif


/** Screenshot module setup
 */
extern orxDLLAPI void orxFASTCALL       orxScreenshot_Setup();

/** Inits the screenshot module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxScreenshot_Init();

/** Exits from the screenshot module
 */
extern orxDLLAPI void orxFASTCALL       orxScreenshot_Exit();

/** Captures a screenshot
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxScreenshot_Capture();

#endif /* _orxSCREENSHOT_H_ */

/** @} */

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
 * @file orxScreenshot.h
 * @date 07/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Make a better init system for finding the next screenshot name
 *   (this one is broken is all screenshots aren't contiguous)
 */

/**
 * @addtogroup orxScreenshot
 * 
 * Screenshot module
 * Module that handles screenshots
 *
 * @{
 */


#ifndef _orxSCREENSHOT_H_
#define _orxSCREENSHOT_H_

#include "orxInclude.h"


/*****************************************************************************/

/* *** Misc *** */

#define orxSCREENSHOT_KZ_DIRECTORY      "."
#define orxSCREENSHOT_KZ_PREFIX         "shot"
#define orxSCREENSHOT_KZ_EXT            "bmp"


/*****************************************************************************/

/* *** Functions *** */

/** Screenshot module setup
 */
extern orxDLLAPI void                   orxScreenshot_Setup();

/** Inits the screenshot module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxScreenshot_Init();

/** Exits from the screenshot module
 */
extern orxDLLAPI void                   orxScreenshot_Exit();

/** Captures a screenshot
 */
extern orxDLLAPI void                   orxScreenshot_Capture();

#endif /* _orxSCREENSHOT_H_ */

/** @} */

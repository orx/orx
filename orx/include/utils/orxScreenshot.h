/** 
 * \file orxScreenshot.h
 * 
 * Screenshot Module.
 * Allows to take screenshots.
 * 
 * \todo
 * Make a better init system for finding the next screenshot name
 * (this one is broken is all screenshots aren't contiguous)
 */


/***************************************************************************
 orxScreenshot.h
 Screenshot module
 
 begin                : 07/12/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


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

/** Inits the screenshot system. */
extern orxDLLAPI orxSTATUS              orxScreenshot_Init();
/** Exits from the screenshot system. */
extern orxDLLAPI orxVOID                orxScreenshot_Exit();

/** Takes a screenshot. */
extern orxDLLAPI orxVOID                orxScreenshot_Take();

#endif /* _orxSCREENSHOT_H_ */

/**
 * \file orxMouse.h
 */

/***************************************************************************
 begin                : 22/11/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxMOUSE_H_
#define _orxMOUSE_H_

#include "orxInclude.h"

/** Init the mouse core plugin
 */
extern orxVOID   orxDLLAPI orxMouse_Plugin_Init();

/** Init the mouse module
 * @return Returns the status of the operation
 */
extern orxSTATUS orxDLLAPI (*orxMouse_Init)();

/** Exit the mouse module
 */
extern orxVOID   orxDLLAPI (*orxMouse_Exit)();

/** Measures how far the mouse has moved since the last call to this function
 * @param _ps32x (OUT)   X coordinates
 * @param _ps32y (OUT)   Y coordinates
 */
extern orxVOID   orxDLLAPI (*orxMouse_GetMove)(orxS32 *_ps32x, orxS32 *_ps32y);

#endif /* _orxMOUSE_H_ */

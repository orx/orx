/** 
 * \file render.h
 * 
 * Render Module.
 * Renders visible objects on screen, using active renders.
 * 
 * \todo
 * Optimize viewport list handling.
 */


/***************************************************************************
 render.h
 Render module
 
 begin                : 15/12/2003
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


#ifndef _RENDER_H_
#define _RENDER_H_

#include "include.h"

#include "camera/viewport.h"


/** Inits the render system. */
extern uint32 render_init();
/** Ends the render system. */
extern void   render_exit();

/** Renders all viewports. */
extern void   render_all();
/** Rendres given viewport. */
extern void   render_viewport();

#endif /* _RENDER_H_ */

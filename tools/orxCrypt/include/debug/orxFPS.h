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
 * @file orxFPS.h
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxFPS
 * 
 * FPS Module
 * Allows to compute & display FPS
 *
 * @{
 */


#ifndef _orxFPS_H_
#define _orxFPS_H_

#include "orxInclude.h"


/** Setups FPS module */
extern orxDLLAPI void orxFASTCALL                 orxFPS_Setup();

/** Inits the FPS module 
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL            orxFPS_Init();

/** Exits from the FPS module */
extern orxDLLAPI void orxFASTCALL                 orxFPS_Exit();

/** Increases internal frame counter */
extern orxDLLAPI void orxFASTCALL                 orxFPS_IncreaseFrameCounter();

/** Gets current FTP value
 * @return orxU32
 */
extern orxDLLAPI orxU32 orxFASTCALL               orxFPS_GetFPS();

#endif /* _orxFPS_H_ */

/** @} */

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
 * @file orxInput.h
 * @date 04/11/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxInput
 * 
 * Input module
 * Module that handles generalized inputs (keyboard, mouse, joystick, ...)
 *
 * @{
 */


#ifndef _orxINPUT_H_
#define _orxINPUT_H_


#include "orxInclude.h"


/** Input module setup
 */
extern orxDLLAPI orxVOID                orxInput_Setup();

/** Initializes Input module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS              orxInput_Init();

/** Exits from Input module
 */
extern orxDLLAPI orxVOID                orxInput_Exit();


#endif /*_orxINPUT_H_*/

/** @} */

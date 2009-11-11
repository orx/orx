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
 * @file orxPlugin_CoreID.h
 * @date 02/05/2005
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all the plugin IDs
 *
 * @{
 */


#ifndef _orxPLUGIN_COREID_H_
#define _orxPLUGIN_COREID_H_


/* *** Core plugin id enum *** */
typedef enum __orxPLUGIN_CORE_ID_t
{
  orxPLUGIN_CORE_ID_DISPLAY = 0,
  orxPLUGIN_CORE_ID_JOYSTICK,
  orxPLUGIN_CORE_ID_KEYBOARD,
  orxPLUGIN_CORE_ID_MOUSE,
  orxPLUGIN_CORE_ID_PHYSICS,
  orxPLUGIN_CORE_ID_RENDER,
  orxPLUGIN_CORE_ID_SOUNDSYSTEM,

  orxPLUGIN_CORE_ID_NUMBER,

  orxPLUGIN_CORE_ID_NONE = orxENUM_NONE
    
} orxPLUGIN_CORE_ID;

#endif /* _orxPLUGIN_COREID_H_ */

/** @} */

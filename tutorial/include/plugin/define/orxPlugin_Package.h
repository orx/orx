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
 * @file orxPlugin_Package.h
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup Plugin
 * 
 * Header that defines all IDs of the package plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_PACKAGE_H_
#define _orxPLUGIN_PACKAGE_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_t
{
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_OPEN,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_SET_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_TEST_FLAGS,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_COMMIT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_EXTRACT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_FIRST,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_NEXT,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_FIND_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_READ,

  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_PACKAGE_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_PACKAGE;

#endif /* _orxPLUGIN_PACKAGE_H_ */

/** @} */

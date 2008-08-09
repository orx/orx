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
 * @file orxPlugin_FileSystem.h
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all IDs of the file system plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_FILESYSTEM_H_
#define _orxPLUGIN_FILESYSTEM_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_t
{
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_FIND_FIRST,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_FIND_NEXT,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_FIND_CLOSE,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_INFO,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_COPY,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_RENAME,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_DELETE,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_CREATE_DIR,
  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_DELETE_DIR,

  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_FILESYSTEM;

#endif /* _orxPLUGIN_FILESYSTEM_H_ */

/** @} */

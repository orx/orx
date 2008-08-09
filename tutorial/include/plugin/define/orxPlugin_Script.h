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
 * @file orxPlugin_Script.h
 * @date 03/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
 * 
 * Header that defines all IDs of the script plugin
 *
 * @{
 */


#ifndef _orxPLUGIN_SCRIPT_H_
#define _orxPLUGIN_SCRIPT_H_

#include "plugin/define/orxPlugin_CoreID.h"


typedef enum __orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_t
{
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_INIT = 0,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_EXIT,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_RUN_FILE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_RUN_STRING,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_TYPE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_S32_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_FLOAT_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_GET_STRING_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_SET_S32_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_SET_FLOAT_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_SET_STRING_VALUE,
  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_REGISTER_FUNCTION,

  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_NUMBER,

  orxPLUGIN_FUNCTION_BASE_ID_SCRIPT_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_BASE_ID_SCRIPT;

#endif /* _orxPLUGIN_SCRIPT_H_ */

/** @} */

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
 * @file orxPluginType.h
 * @date 24/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup Plugin
 * 
 * Plugin type header
 * This header is used to define types for plugin module.
 *
 * @{
 */


#ifndef _orxPLUGIN_TYPE_H_
#define _orxPLUGIN_TYPE_H_

#include "orxInclude.h"


/*********************************************
 Constants / Defines
 *********************************************/

#define orxPLUGIN_KU32_FLAG_CORE_ID     0x10000000    /**< Plugin core ID flag */

#define orxPLUGIN_KU32_MASK_PLUGIN_ID   0x0000FF00    /**< Plugin ID mask */
#define orxPLUGIN_KU32_SHIFT_PLUGIN_ID  8             /**< Plugin ID bits shift */

#define orxPLUGIN_KU32_MASK_FUNCTION_ID 0x000000FF    /**< Plugin function mask */


/* Macro for getting plugin function id */
#define orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_ID, FUNCTION_BASE_ID)   \
  (orxPLUGIN_FUNCTION_ID)(((PLUGIN_ID << orxPLUGIN_KU32_SHIFT_PLUGIN_ID) & orxPLUGIN_KU32_MASK_PLUGIN_ID) | (FUNCTION_BASE_ID & orxPLUGIN_KU32_MASK_FUNCTION_ID))

/* Macro for getting core plugin function id */
#define orxPLUGIN_MAKE_CORE_FUNCTION_ID(PLUGIN_CORE_ID, FUNCTION_BASE_ID)   \
  (orxPLUGIN_FUNCTION_ID)(orxPLUGIN_KU32_FLAG_CORE_ID | orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_CORE_ID, FUNCTION_BASE_ID))


/*********************************************
 Structures
 *********************************************/
 
typedef enum __orxPLUGIN_FUNCTION_ID_t
{
  orxPLUGIN_FUNCTION_ID_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_ID;

typedef orxSTATUS (*orxPLUGIN_FUNCTION)();


/*********************************************
 Function prototypes
 *********************************************/

#endif /* _orxPLUGIN_TYPE_H_ */

/** @} */

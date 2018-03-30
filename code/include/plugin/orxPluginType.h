/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2018 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
 */

/**
 * @file orxPluginType.h
 * @date 24/04/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxPlugin
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

#define orxPLUGIN_KU32_FLAG_CORE_ID             0x10000000    /**< Plugin core ID flag */

#define orxPLUGIN_KU32_MASK_PLUGIN_ID           0x0000FF00    /**< Plugin ID mask */
#define orxPLUGIN_KU32_SHIFT_PLUGIN_ID          8             /**< Plugin ID bits shift */

#define orxPLUGIN_KU32_MASK_FUNCTION_ID         0x000000FF    /**< Plugin function mask */

/* Argument max size */
#define orxPLUGIN_KU32_FUNCTION_ARG_SIZE        128


/* Macro for getting plugin function id */
#define orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_ID, FUNCTION_BASE_ID)   \
  (orxPLUGIN_FUNCTION_ID)(((PLUGIN_ID << orxPLUGIN_KU32_SHIFT_PLUGIN_ID) & orxPLUGIN_KU32_MASK_PLUGIN_ID) | (FUNCTION_BASE_ID & orxPLUGIN_KU32_MASK_FUNCTION_ID))

/* Macro for getting core plugin function id */
#define orxPLUGIN_MAKE_CORE_FUNCTION_ID(PLUGIN_CORE_ID, FUNCTION_BASE_ID)   \
  (orxPLUGIN_FUNCTION_ID)(orxPLUGIN_KU32_FLAG_CORE_ID | orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_CORE_ID, FUNCTION_BASE_ID))

/* Defines plugin functions */
#define orxPLUGIN_K_INIT_FUNCTION_NAME                  orxPlugin_Entry_Init            /**< Plugin init function name */
#define orxPLUGIN_K_EXIT_FUNCTION_NAME                  orxPlugin_Entry_Exit            /**< Plugin exit function name */
#define orxPLUGIN_K_SWAP_FUNCTION_NAME                  orxPlugin_Entry_Swap            /**< Plugin swap function name */


/* Defines core plugin init function */
#ifdef __orxEMBEDDED__

#define orxPLUGIN_K_CORE_INIT_FUNCTION_NAME(SUFFIX)     orxPlugin_##SUFFIX##_Init       /**< Plugin core init function name */

#else /* __orxEMBEDDED__ */

#define orxPLUGIN_K_CORE_INIT_FUNCTION_NAME(SUFFIX)     orxPLUGIN_K_INIT_FUNCTION_NAME  /**< Plugin core init function name */

#endif /* __orxEMBEDDED__ */


/*********************************************
 Structures
 *********************************************/

typedef enum __orxPLUGIN_FUNCTION_ID_t
{
  orxPLUGIN_FUNCTION_ID_NONE = orxENUM_NONE

} orxPLUGIN_FUNCTION_ID;

typedef orxSTATUS (orxFASTCALL *orxPLUGIN_FUNCTION)();


/*********************************************
 Function prototypes
 *********************************************/

#endif /* _orxPLUGIN_TYPE_H_ */

/** @} */

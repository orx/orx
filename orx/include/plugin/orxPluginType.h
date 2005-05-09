/**
 * \file orxPluginType.h
 * This header is used to define types for plugin module.
 */

/*
 begin                : 24/04/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


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
  (orxPLUGIN_FUNCTION_ID)(orxPLUGIN_CORE_ID_FLAG | orxPLUGIN_MAKE_FUNCTION_ID(PLUGIN_CORE_ID, FUNCTION_BASE_ID))


/*********************************************
 Structures
 *********************************************/
 
typedef enum __orxPLUGIN_FUNCTION_ID_t
{
  orxPLUGIN_FUNCTION_ID_NONE = 0xFFFFFFFF
} orxPLUGIN_FUNCTION_ID;

typedef orxVOID(*orxPLUGIN_FUNCTION)();


/*********************************************
 Function prototypes
 *********************************************/


#endif /* _orxPLUGIN_TYPE_H_ */

/**
 * \file plugin_user.h
 * This header is used to define structures & functions for plugin registration.
 */

/*
 begin                : 23/04/2003
 author               : (C) Gdp
 email                : iarwain@ifrance.com
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _PLUGIN_USER_H_
#define _PLUGIN_USER_H_


/*********************************************
 Constants
 *********************************************/

#include <string.h>

#include "include.h"

#include "plugin/plugin_type.h"


/*
 * Includes all plugin related kernel headers
 */
#include "plugin/define/plugin_graph.h"             /* ID : 0x10000000 */
#include "plugin/define/plugin_sound.h"             /* ID : 0x10000100 */
#include "plugin/define/plugin_timer.h"             /* ID : 0x10000200 */
#include "plugin/define/plugin_keyboard.h"          /* ID : 0x10000300 */
#include "plugin/define/plugin_mouse.h"             /* ID : 0x10000400 */
#include "plugin/define/plugin_joystick.h"          /* ID : 0x10000500 */


/* Defines plugin registration info constants */
#define PLUGIN_USER_KZ_FUNCTION_INIT        "plugin_init" /**< Plugin init function name */

#define PLUGIN_KI_NAME_SIZE                 32
#define PLUGIN_KI_FUNCTION_ARG_SIZE         128


/*********************************************
 Structures
 *********************************************/

/* Macro for Structure Handling */
#define PLUGIN_USER_FUNCTION_START(STRUCTURE) \
  { \
    int32 i_plugin_user_function_count = 0; \
    plugin_user_st_function_info *pst_plugin_user_function_info = STRUCTURE;

#define PLUGIN_USER_CORE_FUNCTION_ADD(FUNCTION, TYPE, NAME) \
  PLUGIN_USER_FUNCTION_ADD(FUNCTION, PLUGIN_##TYPE##_KUL_ID_##NAME, TYPE##NAME, "")

#define PLUGIN_USER_FUNCTION_ADD(FUNCTION, ID, NAME, ARGS) \
  pst_plugin_user_function_info[i_plugin_user_function_count].pfn_function = (plugin_function) FUNCTION; \
  pst_plugin_user_function_info[i_plugin_user_function_count].u32_function_id = ID; \
  strcpy(pst_plugin_user_function_info[i_plugin_user_function_count].z_function_name, #NAME); \
  strcpy(pst_plugin_user_function_info[i_plugin_user_function_count].z_function_args, ARGS); \
  i_plugin_user_function_count++;

#define PLUGIN_USER_FUNCTION_END(NUMBER_ADDRESS, STRUCTURE_ADDRESS) \
  *NUMBER_ADDRESS = i_plugin_user_function_count; \
  *STRUCTURE_ADDRESS = pst_plugin_user_function_info; \
  }

typedef struct
{
  plugin_function pfn_function;                         /**< Function Address */
  char z_function_args[PLUGIN_KI_FUNCTION_ARG_SIZE];    /**< Function Argument Types */
  char z_function_name[PLUGIN_KI_NAME_SIZE];            /**< Function Name */
  unsigned long u32_function_id;                        /**< Function ID */
} plugin_user_st_function_info;


/*********************************************
 Function prototypes
 *********************************************/

#endif /* _PLUGIN_USER_H_ */

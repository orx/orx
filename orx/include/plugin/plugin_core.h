/**
 * \file plugin_core.h
 * This header is used to define structures & functions for core plugins info.
 */

/*
 begin                : 24/04/2003
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


#ifndef _PLUGIN_CORE_H_
#define _PLUGIN_CORE_H_


/*********************************************
 Constants
 *********************************************/

#include "orxInclude.h"

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
#include "plugin/define/orxPlugin_File.h"           /* ID : 0x10000600 */


/*** Plugin Definitions ***/
#define PLUGIN_CORE_KS32_NUMBER           7


/*********************************************
 Structures
 *********************************************/
 
typedef struct
{
  plugin_function *pfn_function;        /**< Function Address */
  orxU32 u32_function_id;        /**< Function ID */
} plugin_core_st_function;


/*********************************************
 Function prototypes
 *********************************************/

/** Add an info structure for the given core module.
 * Has to be called during a core module init.
 * \param _u32_core_id the numeric id of the core plugin
 * \param _pst_core_function the pointer on the core functions info array
 * \return nothing.
 */
extern orxVOID   plugin_core_info_add(orxU32 _u32_core_id, plugin_core_st_function *_pst_core_function, orxS32 _i_core_function_number);

/** Default core plugin function.
 * Needs to be referenced by all core functions at module init.
 */
extern orxVOID  *plugin_core_function_default(orxU8 *_zFunctionName, orxU8 *_zFileName, orxU32 _u32Line);

/*** Plugin Core Function Macro Definition *** */
#define PLUGIN_CORE_FUNCTION_DEFINE(FUNCTION_NAME, RETURN, ...)                     \
    orxSTATIC orxVOID *_DEFAULT_CORE_FUNCTION_##FUNCTION_NAME()                     \
    {                                                                               \
        return(plugin_core_function_default(#FUNCTION_NAME, __FILE__, __LINE__));   \
    }                                                                               \
                                                                                    \
    RETURN (*FUNCTION_NAME) (__VA_ARGS__) = (RETURN (*) (__VA_ARGS__)) (&_DEFAULT_CORE_FUNCTION_##FUNCTION_NAME)

#endif /* _PLUGIN_CORE_H_ */

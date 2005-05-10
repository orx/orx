/**
 * \file orxPlugin.h
 * This module provides functions for loading dynamic code into the core
 * engine and finding symbols within the dynamic modules.
 *
 * \todo
 * Has to :
 * - Add execute function for user register function + arg parser/checker
 * - make a Complete Function Info Archiving (Global Info Structure (hash table?))
 */

/*
 begin                : 06/09/2002
 author               : (C) Arcallians
 email                : david.anderson@calixo.net
                      : iarwain@arcallians.org (2003->)
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef _orxPLUGIN_H_
#define _orxPLUGIN_H_

#include "orxInclude.h"


/*********************************************
 Function prototypes
 *********************************************/

/** Initializes the plugin management engine.
 * \return orxSTATUS_SUCCESS/orxSTATUS_FAILED
 */
extern orxU32         orxPlugin_Init();

/** Shuts down the plugin management engine.
 * Calling this function will shutdown all the plugins, free all managment
 * ressources used, and place a silly message in place of the plugin error
 * string (see plugin_error() for info on how to retrieve it :).
 * \return nothing.
 */
extern orxVOID           plugin_exit();

/** Loads a plugin.
 * This function will load a plugin and allocate a node in the plugin
 * list for it.
 * \param _z_plugin_filename the complete path of the plugin file, including its extension
 * \param _z_plugin_name the name that the plugin will be given in the plugin list
 * \return plugin ID on success, orxU32_Undefined on failure (see plugin_error()).
 */
extern orxU32  plugin_load(orxU8 *_z_plugin_filename, orxU8 *_z_plugin_name);

/** Loads a plugin using OS common library extension.
 * This function will load a plugin adding OS library extension to the given name
 * and allocate a node in the plugin list for it.
 * \param _z_plugin_filename the complete path of the plugin file, without the library extension
 * \param _z_plugin_name the name that the plugin will be given in the plugin list
 * \return plugin ID on success, orxU32_Undefined on failure (see plugin_error()).
 */
extern orxU32  plugin_load_ext(orxU8 *_z_plugin_filename, orxU8 *_z_plugin_name);

/** Unloads a plugin.
 * This function will shut down a plugin and free the management ressources
 * it used.
 * \param _u32_plugin_id the numeric id of the plugin to unload
 * \return orxBOOLean orxTRUE on success, orxFALSE on error (see plugin_error() )
 */
extern orxBOOL           plugin_unload_by_id(orxU32 _u32_plugin_id);

/** Unloads a plugin.
 * This function will shut down a plugin and free the management ressources
 * it used.
 * \param _z_plugin_name the symbolic name of the plugin to unload
 * \return orxBOOLean orxTRUE on success, orxFALSE on error (see plugin_error() )
 */
extern orxBOOL           plugin_unload_by_name(orxU8 *_z_plugin_name);

/** Gets a function address from a plugin.
 * This function allows the core to retrieve a pointer to a function located
 * in a specified plugin module.
 * \param _u32_plugin_id the numeric id of the plugin
 * \param _zFunctionName the name of the function to find
 * \return a orxVOID pointer to the function (cast back to correct type in core),
 *  or orxNULL on error (see plugin_error() )
 */
extern orxVOID          *plugin_get_func_addr_by_id(orxU32 _u32_plugin_id,
                                  orxU8 *_zFunctionName);

/** Gets a function address from a plugin.
 * This function allows the core to retrieve a pointer to a function located
 * in a specified plugin module.
 * \param _z_plugin_name the symbolic name of the plugin
 * \param _zFunctionName the name of the function to find
 * \return a orxVOID pointer to the function (cast back to correct type in core),
 *  or orxNULL on error (see plugin_error() )
 */
extern orxVOID          *plugin_get_func_addr_by_name(orxU8 *_z_plugin_name,
                                        orxU8 *_zFunctionName);

/** Gets the numeric id of a plugin knowing its symbolic name.
 * This function allows the core to use the numeric identifier if it is going
 * to call several other plugin functions. This is good because the numeric
 * versions are faster than the 'byname' versions (strcmp needed in the latter
 * case).
 * \param _z_plugin_name the symbolic name of the plugin
 * \return numeric id on success, orxU32_Undefined on error
 */
extern orxU32  plugin_get_id_by_name(orxU8 *_z_plugin_name);

/** Gets the symbolic name of a plugin knowing its id.
 * This function is basically useful if you always use the numeric id
 * (which is a Good Thing(tm) ) and that you want to print a user friendly
 * message or something.
 * \param _u32_plugin_id the numeric id of the plugin
 * \return a pointer to the symbolic name string on success, orxNULL on error
 *  (see plugin_error() )
 */
extern orxU8          *plugin_get_name_by_id(orxU32 _u32_plugin_id);

#endif /* _orxPLUGIN_H_ */

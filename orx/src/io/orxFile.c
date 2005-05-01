/**
 * @file orxFile.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFile.c
 File / Directory management
 
 begin                : 01/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "io/orxFile.h"
#include "debug/orxDebug.h"
#include "plugin/plugin_core.h"

/********************
 *  Plugin Related  *
 ********************/
orxSTATIC plugin_core_st_function plugin_file_spst_function[orxPLUGIN_FILE_KU32_FUNCTION_NUMBER] =
{
  {(plugin_function *) &orxFile_Init,        orxPLUGIN_FILE_KU32_ID_INIT},
  {(plugin_function *) &orxFile_Exit,        orxPLUGIN_FILE_KU32_ID_EXIT},
  {(plugin_function *) &orxFile_FindFirst,   orxPLUGIN_FILE_KU32_ID_FIND_FIRST},
  {(plugin_function *) &orxFile_FindNext,    orxPLUGIN_FILE_KU32_ID_FIND_NEXT},
  {(plugin_function *) &orxFile_FindClose,   orxPLUGIN_FILE_KU32_ID_FIND_CLOSE},
  {(plugin_function *) &orxFile_Copy,        orxPLUGIN_FILE_KU32_ID_COPY},
  {(plugin_function *) &orxFile_Rename,      orxPLUGIN_FILE_KU32_ID_RENAME},
  {(plugin_function *) &orxFile_Delete,      orxPLUGIN_FILE_KU32_ID_DELETE},
  {(plugin_function *) &orxFile_DirCreate,   orxPLUGIN_FILE_KU32_ID_DIR_CREATE},
  {(plugin_function *) &orxFile_DirDelete,   orxPLUGIN_FILE_KU32_ID_DIR_DELETE}
};

/********************
 *   Core Related   *
 ********************/
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_Init, orxSTATUS);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_Exit, orxVOID);

PLUGIN_CORE_FUNCTION_DEFINE(orxFile_FindFirst, orxBOOL, orxSTRING, orxFILE_SEARCH_INFOS*);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_FindNext, orxBOOL, orxFILE_SEARCH_INFOS*);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_FindClose, orxVOID, orxFILE_SEARCH_INFOS*);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_Copy, orxSTATUS, orxSTRING, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_Rename, orxSTATUS, orxSTRING, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_Delete, orxSTATUS, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_DirCreate, orxSTATUS, orxSTRING);
PLUGIN_CORE_FUNCTION_DEFINE(orxFile_DirDelete, orxSTATUS, orxSTRING);

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Function that initialize the File plugin module
 */
orxVOID orxFile_Plugin_Init()
{
  plugin_core_info_add(orxPLUGIN_FILE_KU32_PLUGIN_ID, plugin_file_spst_function, orxPLUGIN_FILE_KU32_FUNCTION_NUMBER);  
}

/** Delete recursively a direcory and all its subfolders.
 * \param _zDirectory     (IN)     Name of the directory directory to remove (with subfolders)
 * \return status of the operation (orxSTATUS_FAILED or orxSTATUS_SUCCESS)
 */
orxSTATUS orxFile_Deltree(orxSTRING _zDirectory)
{
    /* TODO */
    return orxSTATUS_FAILED;
}

/** Returns orxTRUE if a file exists, else orxFALSE.
 * \param _zFileName     (IN)      Full File's name to test
 * \return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
orxBOOL orxFile_Exists(orxSTRING _zFile_name)
{
    /* TODO */
    return orxFALSE;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Debug function that print the list of all started search with extra informations.
 */
orxVOID orxFile_DebugPrint()
{
    /* TODO */
}

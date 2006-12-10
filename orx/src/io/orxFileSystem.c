/**
 * @file orxFileSystem.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFileSystem.c
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

#include "io/orxFileSystem.h"
#include "debug/orxDebug.h"
#include "plugin/orxPluginCore.h"

/********************
 *  Plugin Related  *
 ********************/
 
/***************************************************************************
 orxFileSysem_Setup
 File System module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxFileSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FILESYSTEM, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_FILESYSTEM, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FILESYSTEM, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_FILESYSTEM, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_FILESYSTEM, orxMODULE_ID_TREE);

  return;
}

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param _zFileName     (IN)      Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
orxBOOL orxFASTCALL orxFileSystem_Exists(orxCONST orxSTRING _zFileName)
{
	orxFILESYSTEM_INFO stInfos;
	return (orxFileSystem_Info(_zFileName, &stInfos) == orxSTATUS_SUCCESS);
}


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(FILESYSTEM)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, INIT, orxFileSystem_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, EXIT, orxFileSystem_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, FIND_FIRST, orxFileSystem_FindFirst)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, FIND_NEXT, orxFileSystem_FindNext)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, FIND_CLOSE, orxFileSystem_FindClose)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, INFO, orxFileSystem_Info)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, COPY, orxFileSystem_Copy)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, RENAME, orxFileSystem_Rename)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, DELETE, orxFileSystem_Delete)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, CREATE_DIR, orxFileSystem_CreateDir)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILESYSTEM, DELETE_DIR, orxFileSystem_DeleteDir)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(FILESYSTEM)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxFileSystem_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxFileSystem_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFileSystem_FindFirst, orxBOOL, orxSTRING, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFileSystem_FindNext, orxBOOL, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFileSystem_FindClose, orxVOID, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFileSystem_Info, orxSTATUS, orxSTRING, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFileSystem_Copy, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFileSystem_Rename, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFileSystem_Delete, orxSTATUS, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFileSystem_CreateDir, orxSTATUS, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFileSystem_DeleteDir, orxSTATUS, orxSTRING);


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

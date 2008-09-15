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
 * @file orxFileSystem.c
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */


#include "io/orxFileSystem.h"
#include "debug/orxDebug.h"
#include "plugin/orxPluginCore.h"

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/
 
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
	return(orxFileSystem_Info(_zFileName, &stInfos) == orxSTATUS_SUCCESS);
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_Init, orxSTATUS, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_Exit, orxVOID, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_FindFirst, orxBOOL, orxCONST orxSTRING, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_FindNext, orxBOOL, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_FindClose, orxVOID, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_Info, orxSTATUS, orxCONST orxSTRING, orxFILESYSTEM_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_Copy, orxSTATUS, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_Rename, orxSTATUS, orxCONST orxSTRING, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_Delete, orxSTATUS, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_CreateDir, orxSTATUS, orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFileSystem_DeleteDir, orxSTATUS, orxCONST orxSTRING);


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


/* *** Core function implementations *** */

/** Initialize the File Module
 */
orxSTATUS orxFileSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Init)();
}

/** Uninitialize the File Module
 */
orxVOID orxFileSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Exit)();
}

/** Starts a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param _zSearchPattern (IN)     Pattern to find
 * @param _pstFileInfos   (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
orxBOOL orxFileSystem_FindFirst(orxCONST orxSTRING _zSearchPattern, orxFILESYSTEM_INFO *_pstFileInfo)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_FindFirst)(_zSearchPattern, _pstFileInfo);
}

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFileSystem_FindFirst
 * @param _pstFileInfos   (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
orxBOOL orxFileSystem_FindNext(orxFILESYSTEM_INFO *_pstFileInfo)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_FindNext)(_pstFileInfo);
}

/** Closes a search (free the memory allocated for this search)
 * @param _pstFileInfos   (IN)     Informations returned during search
 */
orxVOID orxFileSystem_FindClose(orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_FindClose)(_pstFileInfo);
}

/** Retrieves informations about a file
 * @param _zFileName      (IN)      Files used to get informations
 * @param _pstFileInfos   (OUT)     Returned file's informations
 * @return Returns the status of the operation
 */
orxSTATUS orxFileSystem_Info(orxCONST orxSTRING _zFileName, orxFILESYSTEM_INFO *_pstFileInfo)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Info)(_zFileName, _pstFileInfo);
}

/** Copies a file
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
orxSTATUS orxFileSystem_Copy(orxCONST orxSTRING _zSource, orxCONST orxSTRING _zDest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Copy)(_zSource, _zDest);
}

/** Renames a file
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
orxSTATUS orxFileSystem_Rename(orxCONST orxSTRING _zSource, orxCONST orxSTRING _zDest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Rename)(_zSource, _zDest);
}

/** Deletes a file
 * @param _zFileName      (IN)     File's name to delete
 * @return The status of the operation
 */
orxSTATUS orxFileSystem_Delete(orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Delete)(_zFileName);
}

/** Creates a directory
 * @param _zDirName       (IN)     New directory's name
 * @return The status of the operation
 */
orxSTATUS orxFileSystem_CreateDir(orxCONST orxSTRING _zDirName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_CreateDir)(_zDirName);
}

/** Remove an empty directory
 * @param _zDirName       (IN)     Directory's name to delete
 * @return The status of the operation
 */
orxSTATUS orxFileSystem_DeleteDir(orxCONST orxSTRING _zDirName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_DeleteDir)(_zDirName);
}

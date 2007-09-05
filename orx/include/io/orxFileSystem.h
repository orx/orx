/**
 * @file orxFileSystem.h
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFileSystem.h
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
 
#ifndef _orxFILESYSTEM_H_
#define _orxFILESYSTEM_H_
 
#include "orxInclude.h"
#include "plugin/orxPluginCore.h"


#define orxFILESYSTEM_KU32_FLAG_INFO_NORMAL 0x00000001 /**< normal file */
#define orxFILESYSTEM_KU32_FLAG_INFO_RDONLY 0x00000002 /**< read-only file */
#define orxFILESYSTEM_KU32_FLAG_INFO_HIDDEN 0x00000004 /**< hidden file */
#define orxFILESYSTEM_KU32_FLAG_INFO_DIR    0x00000008 /**< directory */

#define orxFILESYSTEM_KU32_FLAG_OPEN_READ   0x00000001 /**< opened for read */
#define orxFILESYSTEM_KU32_FLAG_OPEN_WRITE  0x00000002 /**< opened for write */
#define orxFILESYSTEM_KU32_FLAG_OPEN_APPEND 0x00000004 /**< descriptor positioned at the end of file */


/** Store datas about the current file. */
typedef struct __orxFILESYSTEM_INFO_t
{
  orxU32 u32Flags;     /**< File attributes (see list of availables flags) */
  orxU32 u32TimeStamp; /**< Timestamp of the last modification */
  orxU32 u32Size;      /**< File's size (in bytes) */
  orxCHAR zName[256];  /**< File's name */
  orxCHAR zPath[1024]; /**< Directory's name where is stored the file */
  orxHANDLE hInternal; /**< Internal use handle */
} orxFILESYSTEM_INFO;

typedef struct __orxFILESYSTEM_t orxFILESYSTEM;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** File module setup */
extern orxDLLAPI orxVOID                orxFileSystem_Setup();

/** Delete recursively a direcory and all its subfolders.
 * @param _zDirectory     (IN)     Name of the directory directory to remove (with subfolders)
 * @return status of the operation (orxSTATUS_FAILURE or orxSTATUS_SUCCESS)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_Deltree(orxCONST orxSTRING _zDirectory);

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param _zFileName     (IN)      Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxFileSystem_Exists(orxCONST orxSTRING _zFileName);


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_Init, orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_Exit, orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_FindFirst, orxBOOL, orxSTRING, orxFILESYSTEM_INFO *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_FindNext, orxBOOL, orxFILESYSTEM_INFO *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_FindClose, orxVOID, orxFILESYSTEM_INFO *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_Info, orxSTATUS, orxSTRING, orxFILESYSTEM_INFO *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_Copy, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_Rename, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_Delete, orxSTATUS, orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_CreateDir, orxSTATUS, orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxFileSystem_DeleteDir, orxSTATUS, orxSTRING);


/** Initialize the File Module
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Init)();
}

/** Uninitialize the File Module
 */
orxSTATIC orxINLINE orxVOID orxFileSystem_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Exit)();
}


/** Start a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param _zSearchPattern (IN)     Pattern to find
 * @param _pstFileInfos   (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
orxSTATIC orxINLINE orxBOOL orxFileSystem_FindFirst(orxSTRING _zSearchPattern, orxFILESYSTEM_INFO *_pstFileInfo)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_FindFirst)(_zSearchPattern, _pstFileInfo);
}


/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFileSystem_FindFirst
 * @param _pstFileInfos   (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
orxSTATIC orxINLINE orxBOOL orxFileSystem_FindNext(orxFILESYSTEM_INFO *_pstFileInfo)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_FindNext)(_pstFileInfo);
}


/** Close a search (free the memory allocated for this search).
 * @param _pstFileInfos   (IN)     Informations returned during search
 */
orxSTATIC orxINLINE orxVOID orxFileSystem_FindClose(orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_FindClose)(_pstFileInfo);
}


/** Retrieves informations about a file
 * @param _zFileName      (IN)      Files used to get informations
 * @param _pstFileInfos   (OUT)     Returned file's informations
 * @return Returns the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_Info(orxSTRING _zFileName, orxFILESYSTEM_INFO *_pstFileInfo)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Info)(_zFileName, _pstFileInfo);
}


/** Copy a file.
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_Copy(orxSTRING _zSource, orxSTRING _zDest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Copy)(_zSource, _zDest);
}


/** Renames a file.
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_Rename(orxSTRING _zSource, orxSTRING _zDest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Rename)(_zSource, _zDest);
}


/** Deletes a file.
 * @param _zFileName      (IN)     File's name to delete
 * @return The status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_Delete(orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_Delete)(_zFileName);
}


/** Creates a directory
 * @param _zDirName       (IN)     New directory's name
 * @return The status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_CreateDir(orxSTRING _zDirName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_CreateDir)(_zDirName);
}


/** Remove an empty directory
 * @param _zDirName       (IN)     Directory's name to delete
 * @return The status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxFileSystem_DeleteDir(orxSTRING _zDirName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFileSystem_DeleteDir)(_zDirName);
}


#endif /* _orxFILESYSTEM_H_ */

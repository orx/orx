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
 * @file orxFileSystem.h
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxFileSystem
 * 
 * File system plugin module
 * Module that handles file system access
 *
 * @{
 */


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
  orxU32 u32Flags;          /**< File attributes (see list of availables flags) */
  orxU32 u32TimeStamp;      /**< Timestamp of the last modification */
  orxU32 u32Size;           /**< File's size (in bytes) */
  orxCHAR zName[256];       /**< File's name */
  orxCHAR zPath[1024];      /**< Directory's name where is stored the file */
  orxCHAR zFullName[1280];  /**< Full file name */
  orxCHAR zPattern[256];    /**< Search pattern */
  orxHANDLE hInternal;      /**< Internal use handle */
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

/** Initialize the File Module
 */
extern orxDLLAPI orxSTATUS orxFileSystem_Init();

/** Uninitialize the File Module
 */
extern orxDLLAPI orxVOID orxFileSystem_Exit();

/** Starts a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param _zSearchPattern (IN)     Pattern to find
 * @param _pstFileInfo    (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
extern orxDLLAPI orxBOOL orxFileSystem_FindFirst(orxCONST orxSTRING _zSearchPattern, orxFILESYSTEM_INFO *_pstFileInfo);

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFileSystem_FindFirst
 * @param _pstFileInfo    (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
extern orxDLLAPI orxBOOL orxFileSystem_FindNext(orxFILESYSTEM_INFO *_pstFileInfo);

/** Closes a search (free the memory allocated for this search).
 * @param _pstFileInfo    (IN)     Informations returned during search
 */
extern orxDLLAPI orxVOID orxFileSystem_FindClose(orxFILESYSTEM_INFO *_pstFileInfo);

/** Retrieves informations about a file
 * @param _zFileName      (IN)      Files used to get informations
 * @param _pstFileInfo    (OUT)     Returned file's informations
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFileSystem_Info(orxCONST orxSTRING _zFileName, orxFILESYSTEM_INFO *_pstFileInfo);

/** Copies a file
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFileSystem_Copy(orxCONST orxSTRING _zSource, orxCONST orxSTRING _zDest);

/** Renames a file
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFileSystem_Rename(orxCONST orxSTRING _zSource, orxCONST orxSTRING _zDest);

/** Deletes a file
 * @param _zFileName      (IN)     File's name to delete
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFileSystem_Delete(orxCONST orxSTRING _zFileName);

/** Creates a directory
 * @param _zDirName       (IN)     New directory's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFileSystem_CreateDir(orxCONST orxSTRING _zDirName);

/** Removes an empty directory
 * @param _zDirName       (IN)     Directory's name to delete
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFileSystem_DeleteDir(orxCONST orxSTRING _zDirName);

#endif /* _orxFILESYSTEM_H_ */

/** @} */

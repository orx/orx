/**
 * @file orxFile.h
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 * @todo Add functions to open / read / write / close a file
 */ 
 
 /***************************************************************************
 orxFile.h
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
 
#ifndef _orxFILE_H_
#define _orxFILE_H_
 
#include "orxInclude.h"

#define orxFILE_KU32_FLAGS_NORMAL 0x00000001 /**< normal file */
#define orxFILE_KU32_FLAGS_RDONLY 0x00000002 /**< read-only file */
#define orxFILE_KU32_FLAGS_HIDDEN 0x00000004 /**< hidden file */
#define orxFILE_KU32_FLAGS_DIR    0x00000008 /**< directory */


/** Store datas about the current file. */
typedef struct __orxFILE_SEARCH_INFOS_t
{
  orxU32 u32Flags;     /**< File attributes (see list of availables flags) */
  orxU32 u32TimeStamp; /**< Timestamp of the last modification */
  orxU32 u32Size;      /**< File's size (in bytes) */
  orxCHAR zName[256];  /**< File's name */
  orxCHAR zPath[1024]; /**< Directory's name where is stored the file */
  orxHANDLE hSearch;   /**< Search Handle (internal use) */
} orxFILE_SEARCH_INFOS;

/** Function that initialize the File plugin module
 */
extern orxDLLAPI orxVOID orxFile_Plugin_Init();

/** Initialize the File Module
 */
extern orxDLLAPI orxSTATUS (*orxFile_Init)();

/** Uninitialize the File Module
 */
extern orxDLLAPI orxVOID (*orxFile_Exit)();

/** Start a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param _zSearchPattern (IN)     Pattern to find
 * @param _pstFileInfos   (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
extern orxDLLAPI orxBOOL (*orxFile_FindFirst)(orxSTRING _zSearchPattern, orxFILE_SEARCH_INFOS *_pstFileInfos);

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFile_FindFirst
 * @param _pstFileInfos   (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
extern orxDLLAPI orxBOOL (*orxFile_FindNext)(orxFILE_SEARCH_INFOS *_pstFileInfos);

/** Close a search (free the memory allocated for this search).
 * @param _pstFileInfos   (IN)     Informations returned during search
 */
extern orxDLLAPI orxVOID (*orxFile_FindClose)(orxFILE_SEARCH_INFOS *_pstFileInfos);

/** Copy a file.
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxFile_Copy)(orxSTRING _zSource, orxSTRING _zDest);

/** Renames a file.
 * @param _zSource        (IN)     Source file's name
 * @param _zDest          (IN)     Destination file's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxFile_Rename)(orxSTRING _zSource, orxSTRING _zDest);

/** Deletes a file.
 * @param _zFileName      (IN)     File's name to delete
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxFile_Delete)(orxSTRING _zFileName);

/** Creates a directory
 * @param _zDirName       (IN)     New directory's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxFile_DirCreate)(orxSTRING _zDirName);

/** Remove an empty directory
 * @param _zDirName       (IN)     Directory's name to delete
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxFile_DirDelete)(orxSTRING _zDirName);

/** Delete recursively a direcory and all its subfolders.
 * \param _zDirectory     (IN)     Name of the directory directory to remove (with subfolders)
 * \return status of the operation (orxSTATUS_FAILED or orxSTATUS_SUCCESS)
 */
extern orxDLLAPI orxSTATUS orxFile_Deltree(orxSTRING _zDirectory);

/** Returns orxTRUE if a file exists, else orxFALSE.
 * \param _zFileName     (IN)      Full File's name to test
 * \return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
extern orxDLLAPI orxBOOL orxFile_Exists(orxSTRING _zFile_name);


/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Debug function that print the list of all started search with extra informations.
 */
extern orxDLLAPI orxVOID orxFile_DebugPrint();

#endif /* _orxFILE_H_ */

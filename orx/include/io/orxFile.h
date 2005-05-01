/**
 * @file orxFile.h
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
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

#define orxFILE_KU32_FLAGS_INFOS_NORMAL 0x00000001 /**< normal file */
#define orxFILE_KU32_FLAGS_INFOS_RDONLY 0x00000002 /**< read-only file */
#define orxFILE_KU32_FLAGS_INFOS_HIDDEN 0x00000004 /**< hidden file */
#define orxFILE_KU32_FLAGS_INFOS_DIR    0x00000008 /**< directory */

#define orxFILE_KU32_FLAGS_OPEN_READ    0x00000001 /**< opened for read */
#define orxFILE_KU32_FLAGS_OPEN_WRITE   0x00000002 /**< opened for write */
#define orxFILE_KU32_FLAGS_OPEN_APPEND  0x00000004 /**< descriptor positioned at the end of file */


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

typedef struct __orxFILE_t orxFILE;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Function that initialize the File plugin module
 */
extern orxDLLAPI orxVOID orxFile_Plugin_Init();

/** Delete recursively a direcory and all its subfolders.
 * @param _zDirectory     (IN)     Name of the directory directory to remove (with subfolders)
 * @return status of the operation (orxSTATUS_FAILED or orxSTATUS_SUCCESS)
 */
extern orxDLLAPI orxSTATUS orxFile_Deltree(orxSTRING _zDirectory);

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param _zFileName     (IN)      Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
extern orxDLLAPI orxBOOL orxFile_Exists(orxSTRING _zFile_name);

/** Open a file for later read or write operation.
 * @param _zPath         (IN)      Full file's path to open
 * @param _u32OpenFlags  (IN)      List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
extern orxDLLAPI orxFILE *orxFile_Open(orxSTRING _zPath, orxU32 _u32OpenFlags);

/** Read datas from a file
 * @param _pReadData     (OUT)     Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
extern orxDLLAPI orxU32 orxFile_Read(orxVOID *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** write datas to a file
 * @param _pDataToWrite  (IN)      Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
extern orxDLLAPI orxU32 orxFile_Write(orxCONST orxVOID *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** Close an oppened file
 * @param _pstFile       (IN)      File's pointer to close
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFile_Close(orxFILE *_pstFile);

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

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



/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Debug function that print the list of all started search with extra informations.
 */
extern orxDLLAPI orxVOID orxFile_DebugPrint();

#endif /* _orxFILE_H_ */

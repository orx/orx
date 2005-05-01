/**
 * @file orxPackage.h
 * 
 * Module for package management.
 * 
 */ 
 
 /***************************************************************************
 orxPackage.h
 Package management
 
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
 
#ifndef _orxPACKAGE_H_
#define _orxPACKAGE_H_
 
#include "orxInclude.h"
#include "io/orxFile.h"

#define orxPACKAGE_KU32_FLAGS_LOCATION_INTERN  0x00000001 /**< file inside the compiled archive */
#define orxPACKAGE_KU32_FLAGS_LOCATION_EXTERN  0x00000002 /**< file outside the compiled archive */

#define orxPACKAGE_KU32_FLAGS_OPEN_READ        0x00000004 /**< package opened in read mode */
#define orxPACKAGE_KU32_FLAGS_OPEN_WRITE       0x00000008 /**< package opened in write mode */

/** Store datas about the current file. */
typedef struct __orxPACKAGE_INFOS_t
{
  orxFILE_INFOS stFileInfos;    /**< File's information */
  orxU32 u32ExtraPackageFlags;  /**< Extra informations specific to package (location) */
  orxHANDLE hInternal;          /**< Internal use, do not modify */
} orxPACKAGE_INFOS;

typedef struct __orxPACKAGE_t orxPACKAGE;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Function that initialize the Package plugin module
 */
extern orxDLLAPI orxVOID orxPackage_Plugin_Init();

/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Initialize the Package Module
 */
extern orxDLLAPI orxSTATUS (*orxPackage_Init)();

/** Uninitialize the Package Module
 */
extern orxDLLAPI orxVOID (*orxPackage_Exit)();

/** Open a Package
 * @param _zDirPath         (IN)     Directory path where is stored the package to open
 * @param _zPackageName     (IN)     Name of the package
 * @param _u32OpenFlags     (IN)     Open flags (read/write, intern/extern, ...). The package will be created if it doesn't exists and is in write mode
 * @return a Pointer on a package, or orxNULL if an error has occured.
 */
extern orxDLLAPI orxPACKAGE* (*orxPackage_Open)(orxSTRING _zDirPath, orxSTRING _zPackageName, orxU32 _u32OpenFlags);

/** Close a package
 * @param _zPackage         (IN)     Package to close
 */
extern orxDLLAPI orxVOID (*orxPackage_Close)(orxPACKAGE *_pstPackage);


/** Set package flags
 * @param _pstPackage       (IN)     Package to use
 * @param _u32FlagsToRemove (IN)     List of flags to remove
 * @param _u32FlagsToAdd    (IN)     List of flags to add
 */
extern orxDLLAPI orxVOID (*orxPackage_SetFlags)(orxPACKAGE *_pstPackage, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd);

/** Test package flags
 * @param _pstPackage       (IN)     Package to use
 * @param _u32FlagsToTest   (IN)     List of flags to test
 * @return orxTRUE if flags are presents, else orxFALSE
 */
extern orxDLLAPI orxBOOL (*orxPackage_TestFlags)(orxPACKAGE *_pstPackage, orxU32 _u32FlagsToTest);

/** Commit a file in the package
 * @param _pstPackage       (IN)     Package to use
 * @param _zFileName        (IN)     File to commit (a pattern can be used (e.g : *.txt))
 * @return the status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxPackage_Commit)(orxPACKAGE *_pstPackage, orxSTRING _zFileName);

/** Extract a file from a package
 * @param _pstPackage       (IN)     Package to use
 * @param _zFileName        (IN)     File to extract (a pattern can be used (e.g : *.png))
 * @return the status of the operation
 */
extern orxDLLAPI orxSTATUS (*orxPackage_Extract)(orxPACKAGE *_pstPackage, orxSTRING _zFileName);

/** Start a new search. Find the first file that will match to the given pattern in the package
 * @param _pstPackage       (IN)     Package to use. Can be orxNULL (will search in all opened package)
 * @param _zSearchPattern   (IN)     Pattern to find
 * @param _pstFileInfos     (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
extern orxDLLAPI orxBOOL (*orxPackage_FindFirst)(orxPACKAGE *_pstPackage, orxSTRING _zSearchPattern, orxPACKAGE_INFOS *_pstFileInfos);

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFile_FindFirst
 * @param _pstFileInfos     (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
extern orxDLLAPI orxBOOL (*orxPackage_FindNext)(orxPACKAGE_INFOS *_pstFileInfos);

/** Close a search (free the memory allocated for this search).
 * @param _pstFileInfos     (IN)     Informations returned during search
 */
extern orxDLLAPI orxVOID (*orxPackage_FindClose)(orxPACKAGE_INFOS *_pstFileInfos);

/** Read data from a package and store it in memory
 * @param _pDataToWrite     (OUT)    Pointer where will be stored datas
 * @param _u32FileSize      (IN)     Size of the file (in bytes)
 * @param _pstPackage       (IN)     Pointer on the package descriptor
 * @param _zFileName        (IN)     File to read
 * @return Returns the number of written bytes
 */
extern orxDLLAPI orxU32 (*orxPackage_Read)(orxVOID *_pDataToWrite, orxU32 _u32FileSize, orxPACKAGE *_pstPackage, orxSTRING _zFileName);


/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Debug function that print informations about all opened packages.
 */
extern orxDLLAPI orxVOID orxPackage_DebugPrint();

#endif /* _orxPACKAGE_H_ */

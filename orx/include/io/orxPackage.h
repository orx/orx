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
#include "io/orxFileSystem.h"
#include "io/orxPackage.h"

#define orxPACKAGE_KU32_FLAGS_LOCATION_INTERN  0x00000001 /**< file inside the compiled archive */
#define orxPACKAGE_KU32_FLAGS_LOCATION_EXTERN  0x00000002 /**< file outside the compiled archive */

#define orxPACKAGE_KU32_FLAGS_OPEN_READ        0x00000004 /**< package opened in read mode */
#define orxPACKAGE_KU32_FLAGS_OPEN_WRITE       0x00000008 /**< package opened in write mode */

/** Store datas about the current file. */
typedef struct __orxPACKAGE_INFOS_t
{
  orxFILESYSTEM_INFO stFileInfos;     /**< File's information */
  orxU32 u32ExtraPackageFlags;  /**< Extra informations specific to package (location) */
  orxHANDLE hInternal;          /**< Internal use, do not modify */
} orxPACKAGE_INFOS;

typedef struct __orxPACKAGE_t orxPACKAGE;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** Package module setup */
extern orxDLLAPI orxVOID                              orxPackage_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Init,      orxSTATUS);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Exit,      orxVOID);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Close,     orxSTATUS,    orxPACKAGE *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_FindNext,  orxBOOL,      orxPACKAGE_INFOS *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_FindClose, orxVOID,      orxPACKAGE_INFOS *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_TestFlags, orxBOOL,      orxPACKAGE *,       orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Commit,    orxSTATUS,    orxPACKAGE *,       orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Extract,   orxSTATUS,    orxPACKAGE *,       orxCONST orxSTRING);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Open,      orxPACKAGE *, orxCONST orxSTRING, orxCONST orxSTRING,   orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_SetFlags,  orxVOID,      orxPACKAGE *,       orxU32,               orxU32);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_FindFirst, orxBOOL,      orxPACKAGE *,       orxCONST orxSTRING,   orxPACKAGE_INFOS *);
orxPLUGIN_DECLARE_CORE_FUNCTION(orxPackage_Read,      orxU32,       orxVOID *,          orxU32,               orxPACKAGE *,      orxCONST orxSTRING);

/** Initialize the Package Module
 * @return Returns the status of the initialization
 */
orxSTATIC orxINLINE orxSTATUS orxPackage_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Init)();
}

/** Uninitialize the Package Module
 */
orxSTATIC orxINLINE orxVOID orxPackage_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Exit)();
}

/** Open a Package
 * @param _zDirPath         (IN)     Directory path where is stored the package to open
 * @param _zPackageName     (IN)     Name of the package
 * @param _u32OpenFlags     (IN)     Open flags (read/write, intern/extern, ...). The package will be created if it doesn't exists and is in write mode
 * @return a Pointer on a package, or orxNULL if an error has occured.
 */
orxSTATIC orxINLINE orxPACKAGE *orxPackage_Open(orxCONST orxSTRING _zDirPath, orxCONST orxSTRING _zPackageName, orxU32 _u32OpenFlags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Open)(_zDirPath, _zPackageName, _u32OpenFlags);
}

/** Close a package
 * @param _zPackage         (IN)     Package to close
 */
orxSTATIC orxINLINE orxVOID orxPackage_Close(orxPACKAGE *_pstPackage)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Close)(_pstPackage);
}


/** Set package flags
 * @param _pstPackage       (IN)     Package to use
 * @param _u32FlagsToRemove (IN)     List of flags to remove
 * @param _u32FlagsToAdd    (IN)     List of flags to add
 */
orxSTATIC orxINLINE orxVOID orxPackage_SetFlags(orxPACKAGE *_pstPackage, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_SetFlags)(_pstPackage, _u32FlagsToRemove, _u32FlagsToAdd);
}

/** Test package flags
 * @param _pstPackage       (IN)     Package to use
 * @param _u32FlagsToTest   (IN)     List of flags to test
 * @return orxTRUE if flags are presents, else orxFALSE
 */
orxSTATIC orxINLINE orxBOOL orxPackage_TestFlags(orxPACKAGE *_pstPackage, orxU32 _u32FlagsToTest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_TestFlags)(_pstPackage, _u32FlagsToTest);
}

/** Commit a file in the package
 * @param _pstPackage       (IN)     Package to use
 * @param _zFileName        (IN)     File to commit (a pattern can be used (e.g : *.txt))
 * @return the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxPackage_Commit(orxPACKAGE *_pstPackage, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Commit)(_pstPackage, _zFileName);
}

/** Extract a file from a package
 * @param _pstPackage       (IN)     Package to use
 * @param _zFileName        (IN)     File to extract (a pattern can be used (e.g : *.png))
 * @return the status of the operation
 */
orxSTATIC orxINLINE orxSTATUS orxPackage_Extract(orxPACKAGE *_pstPackage, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Extract)(_pstPackage, _zFileName);
}

/** Start a new search. Find the first file that will match to the given pattern in the package
 * @param _pstPackage       (IN)     Package to use. Can be orxNULL (will search in all opened package)
 * @param _zSearchPattern   (IN)     Pattern to find
 * @param _pstFileInfos     (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
orxSTATIC orxINLINE orxBOOL orxPackage_FindFirst(orxPACKAGE *_pstPackage, orxCONST orxSTRING _zSearchPattern, orxPACKAGE_INFOS *_pstFileInfos)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_FindFirst)(_pstPackage, _zSearchPattern, _pstFileInfos);
}

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFile_FindFirst
 * @param _pstFileInfos     (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
orxSTATIC orxINLINE orxBOOL orxPackage_FindNext(orxPACKAGE_INFOS *_pstFileInfos)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_FindNext)(_pstFileInfos);
}

/** Close a search (free the memory allocated for this search).
 * @param _pstFileInfos     (IN)     Informations returned during search
 */
orxSTATIC orxINLINE orxVOID orxPackage_FindClose(orxPACKAGE_INFOS *_pstFileInfos)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_FindClose)(_pstFileInfos);
}

/** Read data from a package and store it in memory
 * @param _pDataToWrite     (OUT)    Pointer where will be stored datas
 * @param _u32FileSize      (IN)     Size of the file (in bytes)
 * @param _pstPackage       (IN)     Pointer on the package descriptor
 * @param _zFileName        (IN)     File to read
 * @return Returns the number of written bytes
 */
orxSTATIC orxINLINE orxU32 orxPackage_Read(orxVOID *_pDataToWrite, orxU32 _u32FileSize, orxPACKAGE *_pstPackage, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Read)(_pDataToWrite, _u32FileSize, _pstPackage, _zFileName);
}


#endif /* _orxPACKAGE_H_ */

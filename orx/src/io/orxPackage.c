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
 * @file orxPackage.c
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */


#include "io/orxPackage.h"
#include "debug/orxDebug.h"


/***************************************************************************
 orxPackage_Setup
 Package module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxPackage_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_PACKAGE, orxMODULE_ID_TREE);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Init,      orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Exit,      orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Close,     orxSTATUS,    orxPACKAGE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindNext,  orxBOOL,      orxPACKAGE_INFOS *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindClose, orxVOID,      orxPACKAGE_INFOS *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_TestFlags, orxBOOL,      orxPACKAGE *,       orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Commit,    orxSTATUS,    orxPACKAGE *,       orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Extract,   orxSTATUS,    orxPACKAGE *,       orxCONST orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Open,      orxPACKAGE *, orxCONST orxSTRING, orxCONST orxSTRING,   orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_SetFlags,  orxVOID,      orxPACKAGE *,       orxU32,               orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_FindFirst, orxBOOL,      orxPACKAGE *,       orxCONST orxSTRING,   orxPACKAGE_INFOS *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxPackage_Read,      orxU32,       orxVOID *,          orxU32,               orxPACKAGE *,      orxCONST orxSTRING);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(PACKAGE)
 
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, INIT, orxPackage_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, EXIT, orxPackage_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, OPEN, orxPackage_Open)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, CLOSE, orxPackage_Close)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, SET_FLAGS, orxPackage_SetFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, TEST_FLAGS, orxPackage_TestFlags)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, COMMIT, orxPackage_Commit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, EXTRACT, orxPackage_Extract)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, FIND_FIRST, orxPackage_FindFirst)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, FIND_NEXT, orxPackage_FindNext)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, FIND_CLOSE, orxPackage_FindClose)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(PACKAGE, READ, orxPackage_Read)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(PACKAGE)


/* *** Core function implementations *** */

/** Initializes the Package Module
 * @return Returns the status of the initialization
 */
orxSTATUS orxPackage_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Init)();
}

/** Uninitializes the Package Module
 */
orxVOID orxPackage_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Exit)();
}

/** Opens a Package
 * @param _zDirPath         (IN)     Directory path where is stored the package to open
 * @param _zPackageName     (IN)     Name of the package
 * @param _u32OpenFlags     (IN)     Open flags (read/write, intern/extern, ...). The package will be created if it doesn't exists and is in write mode
 * @return a Pointer on a package, or orxNULL if an error has occured.
 */
orxPACKAGE *orxPackage_Open(orxCONST orxSTRING _zDirPath, orxCONST orxSTRING _zPackageName, orxU32 _u32OpenFlags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Open)(_zDirPath, _zPackageName, _u32OpenFlags);
}

/** Closes a package
 * @param _zPackage         (IN)     Package to close
 */
orxVOID orxPackage_Close(orxPACKAGE *_pstPackage)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Close)(_pstPackage);
}

/** Sets package flags
 * @param _pstPackage       (IN)     Package to use
 * @param _u32FlagsToRemove (IN)     List of flags to remove
 * @param _u32FlagsToAdd    (IN)     List of flags to add
 */
orxVOID orxPackage_SetFlags(orxPACKAGE *_pstPackage, orxU32 _u32FlagsToRemove, orxU32 _u32FlagsToAdd)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_SetFlags)(_pstPackage, _u32FlagsToRemove, _u32FlagsToAdd);
}

/** Tests package flags
 * @param _pstPackage       (IN)     Package to use
 * @param _u32FlagsToTest   (IN)     List of flags to test
 * @return orxTRUE if flags are presents, else orxFALSE
 */
orxBOOL orxPackage_TestFlags(orxPACKAGE *_pstPackage, orxU32 _u32FlagsToTest)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_TestFlags)(_pstPackage, _u32FlagsToTest);
}

/** Commits a file in the package
 * @param _pstPackage       (IN)     Package to use
 * @param _zFileName        (IN)     File to commit (a pattern can be used (e.g : *.txt))
 * @return the status of the operation
 */
orxSTATUS orxPackage_Commit(orxPACKAGE *_pstPackage, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Commit)(_pstPackage, _zFileName);
}

/** Extracts a file from a package
 * @param _pstPackage       (IN)     Package to use
 * @param _zFileName        (IN)     File to extract (a pattern can be used (e.g : *.png))
 * @return the status of the operation
 */
orxSTATUS orxPackage_Extract(orxPACKAGE *_pstPackage, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Extract)(_pstPackage, _zFileName);
}

/** Starts a new search. Find the first file that will match to the given pattern in the package
 * @param _pstPackage       (IN)     Package to use. Can be orxNULL (will search in all opened package)
 * @param _zSearchPattern   (IN)     Pattern to find
 * @param _pstFileInfos     (OUT)    Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
orxBOOL orxPackage_FindFirst(orxPACKAGE *_pstPackage, orxCONST orxSTRING _zSearchPattern, orxPACKAGE_INFOS *_pstFileInfos)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_FindFirst)(_pstPackage, _zSearchPattern, _pstFileInfos);
}

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFile_FindFirst
 * @param _pstFileInfos     (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
orxBOOL orxPackage_FindNext(orxPACKAGE_INFOS *_pstFileInfos)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_FindNext)(_pstFileInfos);
}

/** Closes a search (free the memory allocated for this search)
 * @param _pstFileInfos     (IN)     Informations returned during search
 */
orxVOID orxPackage_FindClose(orxPACKAGE_INFOS *_pstFileInfos)
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_FindClose)(_pstFileInfos);
}

/** Reads data from a package and store it in memory
 * @param _pDataToWrite     (OUT)    Pointer where will be stored datas
 * @param _u32FileSize      (IN)     Size of the file (in bytes)
 * @param _pstPackage       (IN)     Pointer on the package descriptor
 * @param _zFileName        (IN)     File to read
 * @return Returns the number of written bytes
 */
orxU32 orxPackage_Read(orxVOID *_pDataToWrite, orxU32 _u32FileSize, orxPACKAGE *_pstPackage, orxCONST orxSTRING _zFileName)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxPackage_Read)(_pDataToWrite, _u32FileSize, _pstPackage, _zFileName);
}

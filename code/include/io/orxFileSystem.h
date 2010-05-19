/* Orx - Portable Game Engine
 *
 * Copyright (c) 2010 Orx-Project
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *    1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 *    2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 *    3. This notice may not be removed or altered from any source
 *    distribution.
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
 * File system module
 * Module that handles file system access
 *
 * @{
 */


#ifndef _orxFILESYSTEM_H_
#define _orxFILESYSTEM_H_
 
#include "orxInclude.h"


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
  orxHANDLE hInternal;      /**< Internal use handle */
  orxCHAR zName[256];       /**< File's name */
  orxCHAR zPattern[256];    /**< Search pattern */
  orxCHAR zPath[1024];      /**< Directory's name where is stored the file */
  orxCHAR zFullName[1280];  /**< Full file name */
} orxFILESYSTEM_INFO;


/** Internal FileSystem structure
 */
typedef struct __orxFILESYSTEM_t orxFILESYSTEM;


/** FileSystem module setup */
extern orxDLLAPI void orxFASTCALL       orxFileSystem_Setup();

/** Inits the FileSystem Module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_Init();

/** Exits from the FileSystem Module
 */
extern orxDLLAPI void orxFASTCALL       orxFileSystem_Exit();

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param[in] _zFileName           Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxFileSystem_Exists(const orxSTRING _zFileName);

/** Starts a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param[in] _zSearchPattern      Pattern to find
 * @param[out] _pstFileInfo        Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxFileSystem_FindFirst(const orxSTRING _zSearchPattern, orxFILESYSTEM_INFO *_pstFileInfo);

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFileSystem_FindFirst
 * @param[in,out] _pstFileInfo    (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxFileSystem_FindNext(orxFILESYSTEM_INFO *_pstFileInfo);

/** Closes a search (free the memory allocated for this search).
 * @param[in] _pstFileInfo         Informations returned during search
 */
extern orxDLLAPI void orxFASTCALL       orxFileSystem_FindClose(orxFILESYSTEM_INFO *_pstFileInfo);

/** Retrieves informations about a file
 * @param[in] _zFileName            Files used to get informations
 * @param[out] _pstFileInfo         Returned file's informations
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_Info(const orxSTRING _zFileName, orxFILESYSTEM_INFO *_pstFileInfo);

/** Copies a file
 * @param[in] _zSource             Source file's name
 * @param[in] _zDest               Destination file's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_Copy(const orxSTRING _zSource, const orxSTRING _zDest);

/** Renames a file
 * @param[in] _zSource             Source file's name
 * @param[in] _zDest               Destination file's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_Rename(const orxSTRING _zSource, const orxSTRING _zDest);

/** Deletes a file
 * @param[in] _zFileName           File's name to delete
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_Delete(const orxSTRING _zFileName);

/** Creates a directory
 * @param[in] _zDirName            New directory's name
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_CreateDir(const orxSTRING _zDirName);

/** Removes an empty directory
 * @param[in] _zDirName            Directory's name to delete
 * @return The status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFileSystem_DeleteDir(const orxSTRING _zDirName);

#endif /* _orxFILESYSTEM_H_ */

/** @} */

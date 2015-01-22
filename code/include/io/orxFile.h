/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxFile.h
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxFile
 *
 * File / file system module
 * Module that handles file / file system access
 *
 * @{
 */


#ifndef _orxFILE_H_
#define _orxFILE_H_

#include "orxInclude.h"


#define orxFILE_KU32_FLAG_INFO_NORMAL       0x00000001  /**< normal file */
#define orxFILE_KU32_FLAG_INFO_READONLY     0x00000002  /**< read-only file */
#define orxFILE_KU32_FLAG_INFO_HIDDEN       0x00000004  /**< hidden file */
#define orxFILE_KU32_FLAG_INFO_DIRECTORY    0x00000008  /**< directory */

#define orxFILE_KU32_FLAG_OPEN_READ         0x10000000  /**< opened for read */
#define orxFILE_KU32_FLAG_OPEN_WRITE        0x20000000  /**< opened for write */
#define orxFILE_KU32_FLAG_OPEN_APPEND       0x40000000  /**< descriptor positioned at the end of file */
#define orxFILE_KU32_FLAG_OPEN_BINARY       0x80000000  /**< binary file accessing */


/** File info structure */
typedef struct __orxFILE_INFO_t
{
  orxS64    s64Size;                                    /**< File's size (in bytes) */
  orxS64    s64TimeStamp;                               /**< Timestamp of the last modification */
  orxU32    u32Flags;                                   /**< File attributes (cf. list of available flags) */
  orxHANDLE hInternal;                                  /**< Internal use handle */
  orxCHAR   zName[256];                                 /**< File's name */
  orxCHAR   zPattern[256];                              /**< Search pattern */
  orxCHAR   zPath[1024];                                /**< Directory's name where is stored the file */
  orxCHAR   zFullName[1280];                            /**< Full file name */

} orxFILE_INFO;


/** Internal File structure
 */
typedef struct __orxFILE_t                  orxFILE;


/** File module setup */
extern orxDLLAPI void orxFASTCALL           orxFile_Setup();

/** Inits the File Module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_Init();

/** Exits from the File Module
 */
extern orxDLLAPI void orxFASTCALL           orxFile_Exit();

/** Gets current user's home directory (without trailing separator)
 * @param[in] _zSubPath                     Sub-path to append to the home directory, orxNULL for none
 * @return Current user's home directory, use it immediately or copy it as will be modified by the next call to orxFile_GetHomeDirectory() or orxFile_GetApplicationSaveDirectory()
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxFile_GetHomeDirectory(const orxSTRING _zSubPath);

/** Gets current user's application save directory (without trailing separator)
 * @param[in] _zSubPath                     Sub-path to append to the application save directory, orxNULL for none
 * @return Current user's application save directory, use it immediately or copy it as it will be modified by the next call to orxFile_GetHomeDirectory() or orxFile_GetApplicationSaveDirectory()
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxFile_GetApplicationSaveDirectory(const orxSTRING _zSubPath);

/** Checks if a file/directory exists
 * @param[in] _zFileName           Concerned file/directory
 * @return orxFALSE if _zFileName doesn't exist, orxTRUE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxFile_Exists(const orxSTRING _zFileName);

/** Starts a new file search: finds the first file/directory that will match to the given pattern (ex: /bin/foo*)
 * @param[in] _zSearchPattern      Pattern used for file/directory search
 * @param[out] _pstFileInfo        Information about the first file found
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_FindFirst(const orxSTRING _zSearchPattern, orxFILE_INFO *_pstFileInfo);

/** Continues a file search: finds the next occurrence of a pattern, the search has to be started with orxFile_FindFirst
 * @param[in,out] _pstFileInfo      Information about the last found file/directory
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_FindNext(orxFILE_INFO *_pstFileInfo);

/** Closes a search (frees the memory allocated for this search)
 * @param[in] _pstFileInfo         Information returned during search
 */
extern orxDLLAPI void orxFASTCALL           orxFile_FindClose(orxFILE_INFO *_pstFileInfo);

/** Retrieves a file/directory information
 * @param[in] _zFileName            Concerned file/directory name
 * @param[out] _pstFileInfo         Information of the file/directory
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_GetInfo(const orxSTRING _zFileName, orxFILE_INFO *_pstFileInfo);

/** Removes a file or an empty directory
 * @param[in] _zFileName            Concerned file / directory
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_Remove(const orxSTRING _zFileName);

/** Makes a directory, works recursively if needed
 * @param[in] _zName                Name of the directory to make
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_MakeDirectory(const orxSTRING _zName);

/** Opens a file for later read or write operation
 * @param[in] _zFileName           Full file's path to open
 * @param[in] _u32OpenFlags        List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occurred)
 */
extern orxDLLAPI orxFILE *orxFASTCALL       orxFile_Open(const orxSTRING _zFileName, orxU32 _u32OpenFlags);

/** Reads data from a file
 * @param[out] _pReadData          Buffer that will contain read data
 * @param[in] _s64ElemSize         Size of 1 element
 * @param[in] _s64NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
extern orxDLLAPI orxS64 orxFASTCALL         orxFile_Read(void *_pReadData, orxS64 _s64ElemSize, orxS64 _s64NbElem, orxFILE *_pstFile);

/** Writes data to a file
 * @param[in] _pDataToWrite        Buffer that contains the data to write
 * @param[in] _s64ElemSize         Size of 1 element
 * @param[in] _s64NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
extern orxDLLAPI orxS64 orxFASTCALL         orxFile_Write(const void *_pDataToWrite, orxS64 _s64ElemSize, orxS64 _s64NbElem, orxFILE *_pstFile);

/** Seeks to a position in the given file
 * @param[in] _pstFile              Concerned file
 * @param[in] _s64Position          Position (from start) where to set the indicator
 * @param[in] _eWhence              Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor position if successful, -1 otherwise
 */
extern orxDLLAPI orxS64 orxFASTCALL         orxFile_Seek(orxFILE *_pstFile, orxS64 _s64Position, orxSEEK_OFFSET_WHENCE _eWhence);

/** Tells the current position of the indicator in a file
 * @param[in] _pstFile              Concerned file
 * @return Returns the current position of the file indicator, -1 is invalid
 */
extern orxDLLAPI orxS64 orxFASTCALL         orxFile_Tell(const orxFILE *_pstFile);

/** Retrieves a file's size
 * @param[in] _pstFile              Concerned file
 * @return Returns the length of the file, <= 0 if invalid
 */
extern orxDLLAPI orxS64 orxFASTCALL         orxFile_GetSize(const orxFILE *_pstFile);

/** Retrieves a file's time of last modification
 * @param[in] _pstFile              Concerned file
 * @return Returns the time of the last modification, in seconds, since epoch
 */
extern orxDLLAPI orxS64 orxFASTCALL         orxFile_GetTime(const orxFILE *_pstFile);

/** Prints a formatted string to a file
 * @param[in] _pstFile             Pointer on the file descriptor
 * @param[in] _zString             Formatted string
 * @return Returns the number of written characters
 */
extern orxDLLAPI orxS32 orxCDECL            orxFile_Print(orxFILE *_pstFile, const orxSTRING _zString, ...);

/** Closes an oppened file
 * @param[in] _pstFile             File's pointer to close
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_Close(orxFILE *_pstFile);

#endif /* _orxFILE_H_ */

/** @} */

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2013 Orx-Project
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
#define orxFILE_KU32_FLAG_INFO_RDONLY       0x00000002  /**< read-only file */
#define orxFILE_KU32_FLAG_INFO_HIDDEN       0x00000004  /**< hidden file */
#define orxFILE_KU32_FLAG_INFO_DIR          0x00000008  /**< directory */

#define orxFILE_KU32_FLAG_OPEN_READ         0x00000001  /**< opened for read */
#define orxFILE_KU32_FLAG_OPEN_WRITE        0x00000002  /**< opened for write */
#define orxFILE_KU32_FLAG_OPEN_APPEND       0x00000004  /**< descriptor positioned at the end of file */
#define orxFILE_KU32_FLAG_OPEN_BINARY       0x00000008  /**< binary file accessing */


/** Store datas about the current file. */
typedef struct __orxFILE_INFO_t
{
  orxU32    u32Flags;                                   /**< File attributes (see list of availables flags) */
  orxU32    u32TimeStamp;                               /**< Timestamp of the last modification */
  orxU32    u32Size;                                    /**< File's size (in bytes) */
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

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param[in] _zFileName           Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxFile_Exists(const orxSTRING _zFileName);

/** Starts a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param[in] _zSearchPattern      Pattern to find
 * @param[out] _pstFileInfo        Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxFile_FindFirst(const orxSTRING _zSearchPattern, orxFILE_INFO *_pstFileInfo);

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFile_FindFirst
 * @param[in,out] _pstFileInfo    (IN/OUT) Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
extern orxDLLAPI orxBOOL orxFASTCALL        orxFile_FindNext(orxFILE_INFO *_pstFileInfo);

/** Closes a search (free the memory allocated for this search).
 * @param[in] _pstFileInfo         Informations returned during search
 */
extern orxDLLAPI void orxFASTCALL           orxFile_FindClose(orxFILE_INFO *_pstFileInfo);

/** Retrieves information about a file
 * @param[in] _zFileName            Files used to get information
 * @param[out] _pstFileInfo         Returned file's information
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL      orxFile_GetInfo(const orxSTRING _zFileName, orxFILE_INFO *_pstFileInfo);

/** Opens a file for later read or write operation
 * @param[in] _zFileName           Full file's path to open
 * @param[in] _u32OpenFlags        List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
extern orxDLLAPI orxFILE *orxFASTCALL       orxFile_Open(const orxSTRING _zFileName, orxU32 _u32OpenFlags);

/** Reads data from a file
 * @param[out] _pReadData          Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxFile_Read(void *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** writes data to a file
 * @param[in] _pDataToWrite        Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
extern orxDLLAPI orxU32 orxFASTCALL         orxFile_Write(void *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** Seeks to a position in the given file
 * @param[in] _pstFile              Concerned file
 * @param[in] _s32Position          Position (from start) where to set the indicator
 * @param[in] _eWhence              Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor positionif succesful, -1 otherwise
 */
extern orxDLLAPI orxS32 orxFASTCALL         orxFile_Seek(orxFILE *_pstFile, orxS32 _s32Position, orxSEEK_OFFSET_WHENCE _eWhence);

/** Tells the current position of the indicator in a file
 * @param[in] _pstFile              Concerned file
 * @return Returns the current position of the file indicator, -1 is invalid
 */
extern orxDLLAPI orxS32 orxFASTCALL         orxFile_Tell(const orxFILE *_pstFile);

/** Retrieves a file's size
 * @param[in] _pstFile              Concerned file
 * @return Returns the length of the file, <= 0 if invalid
 */
extern orxDLLAPI orxS32 orxFASTCALL         orxFile_GetSize(const orxFILE *_pstFile);

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

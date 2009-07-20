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
 * @file orxFile.h
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */

/**
 * @addtogroup orxFile
 * 
 * File plugin module
 * Module that handles file access
 *
 * @{
 */


#ifndef _orxFILE_H_
#define _orxFILE_H_
 
#include "orxInclude.h"


#define orxFILE_KU32_FLAG_OPEN_READ   0x00000001 /**< opened for read */
#define orxFILE_KU32_FLAG_OPEN_WRITE  0x00000002 /**< opened for write */
#define orxFILE_KU32_FLAG_OPEN_APPEND 0x00000004 /**< descriptor positioned at the end of file */
#define orxFILE_KU32_FLAG_OPEN_BINARY 0x00000008 /**< binary file accessing */


/** Internal File structure
 */
typedef struct __orxFILE_t orxFILE;


/** File module setup */
extern orxDLLAPI void orxFASTCALL       orxFile_Setup();

/** Inits the File Module
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFile_Init();

/** Exits from the File Module
 */
extern orxDLLAPI void orxFASTCALL       orxFile_Exit();

/** Opens a file for later read or write operation
 * @param[in] _zPath               Full file's path to open
 * @param[in] _u32OpenFlags        List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
extern orxDLLAPI orxFILE *orxFASTCALL   orxFile_Open(const orxSTRING _zPath, orxU32 _u32OpenFlags);

/** Reads data from a file
 * @param[out] _pReadData          Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxFile_Read(void *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** writes data to a file
 * @param[in] _pDataToWrite        Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
extern orxDLLAPI orxU32 orxFASTCALL     orxFile_Write(void *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** Prints a formatted string to a file
 * @param[in] _pstFile             Pointer on the file descriptor
 * @param[in] _zString             Formatted string
 * @return Returns the number of written characters
 */
extern orxDLLAPI orxS32 orxCDECL        orxFile_Print(orxFILE *_pstFile, orxSTRING _zString, ...);

/** Gets text line from a file
 * @param[out] _zBuffer             Pointer where will be stored datas
 * @param[in] _u32Size              Size of buffer
 * @param[in] _pstFile              Pointer on the file descriptor
 * @return Returns orxTRUE if a line has been read, else returns orxFALSE.
 */
extern orxDLLAPI orxBOOL orxFASTCALL    orxFile_ReadLine(orxSTRING _zBuffer, orxU32 _u32Size, orxFILE *_pstFile);

/** Closes an oppened file
 * @param[in] _pstFile             File's pointer to close
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxFile_Close(orxFILE *_pstFile);

#endif /* _orxFILE_H_ */

/** @} */

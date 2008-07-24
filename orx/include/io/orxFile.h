/**
 * @file orxFile.h
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFile.h
 File management
 
 begin                : 01/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   as published by the Free Software Foundation; either version 2.1      *
 *   of the License, or (at your option) any later version.                *
 *                                                                         *
 ***************************************************************************/
 
#ifndef _orxFILE_H_
#define _orxFILE_H_
 
#include "orxInclude.h"
#include "plugin/orxPluginCore.h"

#define orxFILE_KU32_FLAG_OPEN_READ   0x00000001 /**< opened for read */
#define orxFILE_KU32_FLAG_OPEN_WRITE  0x00000002 /**< opened for write */
#define orxFILE_KU32_FLAG_OPEN_APPEND 0x00000004 /**< descriptor positioned at the end of file */

typedef struct __orxFILE_t orxFILE;

/***************************************************************************
 * Functions directly implemented by orx core
 ***************************************************************************/

/** File module setup */
extern orxDLLAPI orxVOID                orxFile_Setup();


/***************************************************************************
 * Functions extended by plugins
 ***************************************************************************/

/** Initialize the File Module
 */
extern orxDLLAPI orxSTATUS              orxFile_Init();

/** Uninitialize the File Module
 */
extern orxDLLAPI orxVOID                orxFile_Exit();

/** Opens a file for later read or write operation
 * @param _zPath         (IN)      Full file's path to open
 * @param _u32OpenFlags  (IN)      List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
extern orxDLLAPI orxFILE*               orxFile_Open(orxCONST orxSTRING _zPath, orxU32 _u32OpenFlags);

/** Reads data from a file
 * @param _pReadData     (OUT)     Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
extern orxDLLAPI orxU32                 orxFile_Read(orxVOID *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** writes data to a file
 * @param _pDataToWrite  (IN)      Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
extern orxDLLAPI orxU32                 orxFile_Write(orxVOID *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile);

/** Gets text line from a file
 * @param _zBuffer  (OUT)     Pointer where will be stored datas
 * @param _u32Size  (IN)      Size of buffer
 * @param _pstFile  (IN)      Pointer on the file descriptor
 * @return Returns orxTRUE if a line has been read, else returns orxFALSE.
 */
extern orxDLLAPI orxBOOL                orxFile_ReadLine(orxSTRING _zBuffer, orxU32 _u32Size, orxFILE *_pstFile);

/** Closes an oppened file
 * @param _pstFile       (IN)      File's pointer to close
 * @return Returns the status of the operation
 */
extern orxDLLAPI orxSTATUS              orxFile_Close(orxFILE *_pstFile);

#endif /* _orxFILE_H_ */

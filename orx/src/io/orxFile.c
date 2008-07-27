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
 * @file orxFile.c
 * @date 01/05/2005
 * @author bestel@arcallians.org
 *
 * @todo
 */


#include "io/orxFile.h"
#include "debug/orxDebug.h"
#include "plugin/orxPluginCore.h"

/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/
 
/***************************************************************************
 orxFile_Setup
 File module setup.

 returns: nothing
 ***************************************************************************/
orxVOID orxFile_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_PLUGIN);
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_MEMORY);

  return;
}


/***************************************************************************
 * Plugin related                                                          *
 ***************************************************************************/

/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Open, orxFILE *, orxCONST orxSTRING, orxU32);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Read, orxU32, orxVOID *, orxU32, orxU32, orxFILE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Write, orxU32, orxVOID *, orxU32, orxU32, orxFILE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_ReadLine, orxSTATUS, orxSTRING, orxU32, orxFILE *);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Close, orxSTATUS, orxFILE *);


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(FILE)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, INIT, orxFile_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, EXIT, orxFile_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, OPEN, orxFile_Open)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, READ, orxFile_Read)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, WRITE, orxFile_Write)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, READ_LINE, orxFile_ReadLine)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, CLOSE, orxFile_Close)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(FILE)


/* *** Core function implementations *** */

/** Initialize the File Module
 */
orxSTATUS orxFile_Init()
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_Init)();
}

/** Uninitialize the File Module
 */
orxVOID orxFile_Exit()
{
  orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_Exit)();
}

/** Opens a file for later read or write operation
 * @param _zPath         (IN)      Full file's path to open
 * @param _u32OpenFlags  (IN)      List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
orxFILE *orxFile_Open(orxCONST orxSTRING _zPath, orxU32 _u32OpenFlags)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_Open)(_zPath, _u32OpenFlags);
}

/** Reads data from a file
 * @param _pReadData     (OUT)     Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
orxU32 orxFile_Read(orxVOID *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_Read)(_pReadData, _u32ElemSize, _u32NbElem, _pstFile);
}

/** writes data to a file
 * @param _pDataToWrite  (IN)      Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
orxU32 orxFile_Write(orxVOID *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_Write)(_pDataToWrite, _u32ElemSize, _u32NbElem, _pstFile);
}

/** Gets text line from a file
 * @param _zBuffer  (OUT)     Pointer where will be stored datas
 * @param _u32Size  (IN)      Size of buffer
 * @param _pstFile  (IN)      Pointer on the file descriptor
 * @return Returns orxTRUE if a line has been read, else returns orxFALSE.
 */
orxBOOL orxFile_ReadLine(orxSTRING _zBuffer, orxU32 _u32Size, orxFILE *_pstFile)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_ReadLine)(_zBuffer, _u32Size, _pstFile);
}

/** Closes an oppened file
 * @param _pstFile       (IN)      File's pointer to close
 * @return Returns the status of the operation
 */
orxSTATUS orxFile_Close(orxFILE *_pstFile)
{
  return orxPLUGIN_CORE_FUNCTION_POINTER_NAME(orxFile_Close)(_pstFile);
}

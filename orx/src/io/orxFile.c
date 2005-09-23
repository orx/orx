/**
 * @file orxFile.c
 * 
 * Module for file / directory management.
 * 
 * @todo Maybe find a way to be completly independent of hard disks name and directory seperator
 */ 
 
 /***************************************************************************
 orxFile.c
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

#include "io/orxFile.h"
#include "debug/orxDebug.h"
#include "plugin/orxPluginCore.h"

/********************
 *  Plugin Related  *
 ********************/
 
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
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_LINKLIST);
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_TREE);

  return;
}


/* *** Core function info array *** */

orxPLUGIN_BEGIN_CORE_FUNCTION_ARRAY(FILE)

orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, INIT, orxFile_Init)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, EXIT, orxFile_Exit)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, FIND_FIRST, orxFile_FindFirst)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, FIND_NEXT, orxFile_FindNext)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, FIND_CLOSE, orxFile_FindClose)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, INFO, orxFile_Info)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, COPY, orxFile_Copy)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, RENAME, orxFile_Rename)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, DELETE, orxFile_Delete)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, CREATE_DIR, orxFile_CreateDir)
orxPLUGIN_ADD_CORE_FUNCTION_ARRAY(FILE, DELETE_DIR, orxFile_DeleteDir)

orxPLUGIN_END_CORE_FUNCTION_ARRAY(FILE)


/* *** Core function definitions *** */

orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxFile_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION_0(orxFile_Exit, orxVOID);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFile_FindFirst, orxBOOL, orxSTRING, orxFILE_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFile_FindNext, orxBOOL, orxFILE_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFile_FindClose, orxVOID, orxFILE_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFile_Info, orxSTATUS, orxSTRING, orxFILE_INFO *);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFile_Copy, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_2(orxFile_Rename, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFile_Delete, orxSTATUS, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFile_CreateDir, orxSTATUS, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION_1(orxFile_DeleteDir, orxSTATUS, orxSTRING);


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Delete recursively a direcory and all its subfolders.
 * \param _zDirectory     (IN)     Name of the directory directory to remove (with subfolders)
 * \return status of the operation (orxSTATUS_FAILED or orxSTATUS_SUCCESS)
 */
orxSTATUS orxFASTCALL orxFile_Deltree(orxCONST orxSTRING _zDirectory)
{
  /* TODO */
  return orxSTATUS_FAILED;
}

/** Returns orxTRUE if a file exists, else orxFALSE.
 * \param _zFileName     (IN)      Full File's name to test
 * \return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
orxBOOL orxFASTCALL orxFile_Exists(orxCONST orxSTRING _zFileName)
{
  /* TODO */
  return orxFALSE;
}

/** Open a file for later read or write operation.
 * @param _zPath         (IN)      Full file's path to open
 * @param _u32OpenFlags  (IN)      List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
orxFILE *orxFASTCALL orxFile_Open(orxCONST orxSTRING _zPath, orxU32 _u32OpenFlags)
{
  /* TODO */
  return orxNULL;   
}

/** Read datas from a file
 * @param _pReadData     (OUT)     Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
orxU32 orxFASTCALL orxFile_Read(orxVOID *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* TODO */
  return 0;
}

/** write datas to a file
 * @param _pDataToWrite  (IN)      Pointer where will be stored datas
 * @param _u32ElemSize   (IN)      Size of 1 element
 * @param _u32NbElem     (IN)      Number of elements
 * @param _pstFile       (IN)      Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
orxU32 orxFASTCALL orxFile_Write(orxCONST orxVOID *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* TODO */
  return 0;
}

/** Close an oppened file
 * @param _pstFile       (IN)      File's pointer to close
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxFile_Close(orxFILE *_pstFile)
{
  /* TODO */
  return orxSTATUS_FAILED;
}

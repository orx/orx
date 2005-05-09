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
orxSTATIC orxCONST orxPLUGIN_CORE_FUNCTION sastFilePluginFunctionInfo[orxPLUGIN_FUNCTION_BASE_ID_FILE_NUMBER] =
{
  {(orxPLUGIN_FUNCTION *) &orxFile_Init,        orxPLUGIN_FUNCTION_BASE_ID_FILE_INIT},
  {(orxPLUGIN_FUNCTION *) &orxFile_Exit,        orxPLUGIN_FUNCTION_BASE_ID_FILE_EXIT},
  {(orxPLUGIN_FUNCTION *) &orxFile_FindFirst,   orxPLUGIN_FUNCTION_BASE_ID_FILE_FIND_FIRST},
  {(orxPLUGIN_FUNCTION *) &orxFile_FindNext,    orxPLUGIN_FUNCTION_BASE_ID_FILE_FIND_NEXT},
  {(orxPLUGIN_FUNCTION *) &orxFile_FindClose,   orxPLUGIN_FUNCTION_BASE_ID_FILE_FIND_CLOSE},
  {(orxPLUGIN_FUNCTION *) &orxFile_Infos,       orxPLUGIN_FUNCTION_BASE_ID_FILE_INFO},
  {(orxPLUGIN_FUNCTION *) &orxFile_Copy,        orxPLUGIN_FUNCTION_BASE_ID_FILE_COPY},
  {(orxPLUGIN_FUNCTION *) &orxFile_Rename,      orxPLUGIN_FUNCTION_BASE_ID_FILE_RENAME},
  {(orxPLUGIN_FUNCTION *) &orxFile_Delete,      orxPLUGIN_FUNCTION_BASE_ID_FILE_DELETE},
  {(orxPLUGIN_FUNCTION *) &orxFile_DirCreate,   orxPLUGIN_FUNCTION_BASE_ID_FILE_CREATE_DIR},
  {(orxPLUGIN_FUNCTION *) &orxFile_DirDelete,   orxPLUGIN_FUNCTION_BASE_ID_FILE_DELETE_DIR}
};

/********************
 *   Core Related   *
 ********************/
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Init, orxSTATUS);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Exit, orxVOID);

orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_FindFirst, orxBOOL, orxSTRING, orxFILE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_FindNext, orxBOOL, orxFILE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_FindClose, orxVOID, orxFILE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Infos, orxSTATUS, orxSTRING, orxFILE_INFOS*);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Copy, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Rename, orxSTATUS, orxSTRING, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_Delete, orxSTATUS, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_DirCreate, orxSTATUS, orxSTRING);
orxPLUGIN_DEFINE_CORE_FUNCTION(orxFile_DirDelete, orxSTATUS, orxSTRING);

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

struct __orxFILE_t
{
};

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Function that initialize the File plugin module
 */
orxVOID orxFile_Plugin_Init()
{
  orxPlugin_AddCoreInfo(orxPLUGIN_CORE_ID_FILE, sastFilePluginFunctionInfo, sizeof(sastFilePluginFunctionInfo) / sizeof(orxPLUGIN_CORE_FUNCTION));  
}

/** Delete recursively a direcory and all its subfolders.
 * \param _zDirectory     (IN)     Name of the directory directory to remove (with subfolders)
 * \return status of the operation (orxSTATUS_FAILED or orxSTATUS_SUCCESS)
 */
orxSTATUS orxFile_Deltree(orxSTRING _zDirectory)
{
  /* TODO */
  return orxSTATUS_FAILED;
}

/** Returns orxTRUE if a file exists, else orxFALSE.
 * \param _zFileName     (IN)      Full File's name to test
 * \return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
orxBOOL orxFile_Exists(orxSTRING _zFile_name)
{
  /* TODO */
  return orxFALSE;
}

/** Open a file for later read or write operation.
 * @param _zPath         (IN)      Full file's path to open
 * @param _u32OpenFlags  (IN)      List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
orxFILE *orxFile_Open(orxSTRING _zPath, orxU32 _u32OpenFlags)
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
orxU32 orxFile_Read(orxVOID *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
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
orxU32 orxFile_Write(orxCONST orxVOID *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* TODO */
  return 0;
}

/** Close an oppened file
 * @param _pstFile       (IN)      File's pointer to close
 * @return Returns the status of the operation
 */
orxSTATUS orxFile_Close(orxFILE *_pstFile)
{
  /* TODO */
  return orxSTATUS_FAILED;
}

/*******************************************************************************
 * DEBUG FUNCTION
 ******************************************************************************/

/** Debug function that print the list of all started search with extra informations.
 */
orxVOID orxFile_DebugPrint()
{
    /* TODO */
}

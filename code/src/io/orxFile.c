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
 * @file orxFile.c
 * @date 01/05/2005
 * @author bestel@arcallians.org
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxFile.h"
#include "memory/orxMemory.h"
#include "utils/orxString.h"
#include "debug/orxDebug.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifdef __orxWINDOWS__

  #include <io.h>

  #ifdef __orxMSVC__

    #pragma warning(disable : 4311 4312 4996)

  #endif /* __orxMSVC__ */

#else /* __orxWINDOWS__ */

  #ifdef __orxANDROID__

    #include <jni.h>
    #include "main/orxAndroid.h"

  #endif /* __orxANDROID__ */

  #include <dirent.h>
  #include <fnmatch.h>
  #include <unistd.h>

#endif /* __orxWINDOWS__ */


/** Module flags
 */
#define orxFILE_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxFILE_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** File structure
 */
struct __orxFILE_t
{
  FILE stFile;
};

/** Static structure
 */
typedef struct __orxFILE_STATIC_t
{
  orxU32 u32Flags;

} orxFILE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFILE_STATIC sstFile;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

#ifdef __orxWINDOWS__

static orxINLINE void orxFile_GetInfoFromData(const struct _finddata_t *_pstData, orxFILE_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Stores info */
  _pstFileInfo->u32Size       = _pstData->size;
  _pstFileInfo->u32TimeStamp  = (orxU32)_pstData->time_write;
  orxString_NCopy(_pstFileInfo->zName, (orxSTRING)_pstData->name, sizeof(_pstFileInfo->zName) - 1);
  _pstFileInfo->zName[sizeof(_pstFileInfo->zName) - 1] = orxCHAR_NULL;
  orxString_Copy(_pstFileInfo->zFullName + orxString_GetLength(_pstFileInfo->zPath), _pstFileInfo->zName);
  _pstFileInfo->u32Flags      = (_pstData->attrib == 0)
                                ? orxFILE_KU32_FLAG_INFO_NORMAL
                                : ((_pstData->attrib & _A_RDONLY) ? orxFILE_KU32_FLAG_INFO_RDONLY : 0)
                                | ((_pstData->attrib & _A_HIDDEN) ? orxFILE_KU32_FLAG_INFO_HIDDEN : 0)
                                | ((_pstData->attrib & _A_SUBDIR) ? orxFILE_KU32_FLAG_INFO_DIR : 0);

  return;
}

#else /* __orxWINDOWS__ */

static orxINLINE void orxFile_GetInfoFromData(const struct dirent *_pstData, orxFILE_INFO *_pstFileInfo)
{
  struct stat stStat;
  orxSTRING   zName;

  /* Checks */
  orxASSERT(_pstData != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Gets data name */
  zName = (orxSTRING)_pstData->d_name;

  /* Stores info */
  orxString_NCopy(_pstFileInfo->zName, zName, sizeof(_pstFileInfo->zName) - 1);
  _pstFileInfo->zName[sizeof(_pstFileInfo->zName) - 1] = orxCHAR_NULL;
  orxString_Copy(_pstFileInfo->zFullName + orxString_GetLength(_pstFileInfo->zPath), _pstFileInfo->zName);

  /* Gets file info */
  stat(_pstFileInfo->zFullName, &stStat);

  _pstFileInfo->u32Flags = 0;

  /* Read only file ? */
  if(access(_pstFileInfo->zFullName, R_OK | W_OK) == 0)
  {
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_NORMAL;
  }
  else if(access(_pstFileInfo->zFullName, R_OK) == 0)
  {
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_RDONLY;
  }

  /* Hidden ? */
  if(_pstFileInfo->zName[0] == '.')
  {
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_HIDDEN;
  }

  /* Dir ? */
  if(stStat.st_mode & S_IFDIR)
  {
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_DIR;
  }

  /* Normal file ? */
  if(_pstFileInfo->u32Flags != orxFILE_KU32_FLAG_INFO_NORMAL)
  {
    _pstFileInfo->u32Flags &= ~orxFILE_KU32_FLAG_INFO_NORMAL;
  }

  /* Sets time and last file access time */
  _pstFileInfo->u32Size       = stStat.st_size;
  _pstFileInfo->u32TimeStamp  = stStat.st_mtime;

  return;
}

#endif /* __orxWINDOWS__ */

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** File module setup */
void orxFASTCALL orxFile_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_FILE, orxMODULE_ID_STRING);

  return;
}

/** Inits the File Module
 */
orxSTATUS orxFASTCALL orxFile_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFile, sizeof(orxFILE_STATIC));

#ifdef __orxANDROID__

    if(chdir(orxAndroid_GetInternalStoragePath()) != 0)
    {
      orxDEBUG_PRINT(orxDEBUG_LEVEL_FILE, "could not chdir to %s !", orxAndroid_GetInternalStoragePath());
    }

#endif /* __orxANDROID__ */

    /* Updates status */
    sstFile.u32Flags |= orxFILE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the File Module
 */
void orxFASTCALL orxFile_Exit()
{
  /* Was initialized? */
  if(sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFile, sizeof(orxFILE_STATIC));
  }

  return;
}

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param[in] _zFileName           Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
orxBOOL orxFASTCALL orxFile_Exists(const orxSTRING _zFileName)
{
  orxFILE_INFO stInfo;

  /* Clears it */
  orxMemory_Zero(&stInfo, sizeof(orxFILE_INFO));

  /* Done! */
  return(orxFile_GetInfo(_zFileName, &(stInfo)) != orxSTATUS_FAILURE);
}

/** Starts a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param[in] _zSearchPattern      Pattern to find
 * @param[out] _pstFileInfo        Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
orxBOOL orxFASTCALL orxFile_FindFirst(const orxSTRING _zSearchPattern, orxFILE_INFO *_pstFileInfo)
{
  orxBOOL bResult = orxFALSE;

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32Handle;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Opens the search */
  s32Handle = (orxU32)_findfirst(_zSearchPattern, &stData);

  /* Valid? */
  if(s32Handle >= 0)
  {
    orxS32 s32LastSeparator, i;

    /* Gets last directory separator */
    for(s32LastSeparator = -1, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_LINUX, 0);
        i >= 0;
        s32LastSeparator = i, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_LINUX, i + 1));

    /* Not found? */
    if(s32LastSeparator < 0)
    {
      /* Gets last directory separator */
      for(s32LastSeparator = -1, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, 0);
          i >= 0;
          s32LastSeparator = i, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, i + 1));
    }

    /* Has directory? */
    if(s32LastSeparator >= 0)
    {
      orxU32 u32Index;

      /* Updates path & full name base */
      u32Index = orxMIN(s32LastSeparator + 1, sizeof(_pstFileInfo->zPath) - 1);
      orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, u32Index);
      _pstFileInfo->zPath[u32Index] = orxCHAR_NULL;
      orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

      /* Stores pattern */
      u32Index = orxMIN(orxString_GetLength(_zSearchPattern) - s32LastSeparator - 1, sizeof(_pstFileInfo->zPattern) - 1);
      orxString_NCopy(_pstFileInfo->zPattern, &_zSearchPattern[s32LastSeparator] + 1, u32Index);
      _pstFileInfo->zPattern[u32Index] = orxCHAR_NULL;
    }
    else
    {
      orxU32 u32Index;

      /* Clears vars */
      _pstFileInfo->zPath[0]      = orxCHAR_NULL;
      _pstFileInfo->zFullName[0]  = orxCHAR_NULL;

      /* Stores pattern */
      u32Index = orxMIN(orxString_GetLength(_zSearchPattern), sizeof(_pstFileInfo->zPattern) - 1);
      orxString_NCopy(_pstFileInfo->zPattern, _zSearchPattern, u32Index);
      _pstFileInfo->zPattern[u32Index] = orxCHAR_NULL;
    }

    /* Tranfers file info */
    orxFile_GetInfoFromData(&stData, _pstFileInfo);

    /* Stores handle */
    _pstFileInfo->hInternal = (orxHANDLE)s32Handle;

    /* Updates result */
    bResult = orxTRUE;
  }

#else /* __orxWINDOWS__ */

  /* Stores seach pattern */
  orxS32 s32LastSeparator, i;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Gets last directory separator */
  for(s32LastSeparator = -1, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_LINUX, 0);
      i >= 0;
      s32LastSeparator = i, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_LINUX, i + 1));

  /* Not found? */
  if(s32LastSeparator < 0)
  {
    /* Gets last directory separator */
    for(s32LastSeparator = -1, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, 0);
        i >= 0;
        s32LastSeparator = i, i = orxString_SearchCharIndex(_zSearchPattern, orxCHAR_DIRECTORY_SEPARATOR_WINDOWS, i + 1));
  }

  /* Has directory? */
  if(s32LastSeparator >= 0)
  {
    orxU32 u32Index;

    /* Updates path & full name base */
    u32Index = orxMIN((orxU32)(s32LastSeparator + 1), sizeof(_pstFileInfo->zPath) - 1);
    orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, u32Index);
    _pstFileInfo->zPath[u32Index] = orxCHAR_NULL;
    orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

    /* Stores pattern */
    u32Index = orxMIN(orxString_GetLength(_zSearchPattern) - s32LastSeparator - 1, sizeof(_pstFileInfo->zPattern) - 1);
    orxString_NCopy(_pstFileInfo->zPattern, &_zSearchPattern[s32LastSeparator] + 1, u32Index);
    _pstFileInfo->zPattern[u32Index] = orxCHAR_NULL;
  }
  else
  {
    orxU32 u32Index;

    /* Stores pattern */
    u32Index = orxMIN(orxString_GetLength(_zSearchPattern), sizeof(_pstFileInfo->zPattern) - 1);
    orxString_NCopy(_pstFileInfo->zPattern, _zSearchPattern, u32Index);
    _pstFileInfo->zPattern[u32Index] = orxCHAR_NULL;

    /* Clears vars */
    orxString_Print(_pstFileInfo->zPath, "./");
    orxString_Print(_pstFileInfo->zFullName, "./");
  }

  /* Open directory */
  DIR *pDir = opendir(_pstFileInfo->zPath);

  /* Valid ? */
  if(pDir != orxNULL)
  {
    /* Stores the DIR handle */
    _pstFileInfo->hInternal = (orxHANDLE)pDir;

    /* Retrieves info */
    bResult = orxFile_FindNext(_pstFileInfo);
  }

#endif /* __orxWINDOWS__ */

  /* Done! */
  return bResult;
}

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFile_FindFirst
 * @param[in,out] _pstFileInfo    Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
orxBOOL orxFASTCALL orxFile_FindNext(orxFILE_INFO *_pstFileInfo)
{
  orxBOOL bResult = orxFALSE;

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32FindResult;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Opens the search */
  s32FindResult = _findnext((orxS32)_pstFileInfo->hInternal, &stData);

  /* Valid? */
  if(s32FindResult == 0)
  {
    /* Tranfers file info */
    orxFile_GetInfoFromData(&stData, _pstFileInfo);

    /* Updates result */
    bResult = orxTRUE;
  }

#else /* __orxWINDOWS__ */

  struct dirent *pstDirEnt;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Updates full name */
  orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

  /* Reads directory */

  /* loop on entries until the pattern match */
  while((!bResult)
     && (pstDirEnt = readdir((DIR*)_pstFileInfo->hInternal)))
  {
    /* Gets file info */
    orxFile_GetInfoFromData(pstDirEnt, _pstFileInfo);

    /* Match ? */
    bResult = (fnmatch(_pstFileInfo->zPattern, _pstFileInfo->zName, 0) == 0);
  }

#endif /* __orxWINDOWS__ */

  /* Done! */
  return bResult;
}

/** Closes a search (free the memory allocated for this search)
 * @param[in] _pstFileInfo         Informations returned during search
 */
void orxFASTCALL orxFile_FindClose(orxFILE_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

#ifdef __orxWINDOWS__

  /* Has valid handle? */
  if(((orxS32)_pstFileInfo->hInternal) > 0)
  {
    /* Closes the search */
    _findclose((orxS32)_pstFileInfo->hInternal);
  }

#else /* __orxWINDOWS__ */

  /* Has valid handle? */
  if(((DIR *)_pstFileInfo->hInternal) != NULL)
  {
    /* Closes the search */
    closedir((DIR *)_pstFileInfo->hInternal);
  }

#endif /* __orxWINDOWS__ */

  /* Clears handle */
  _pstFileInfo->hInternal = 0;

  return;
}

/** Retrieves information about a file
 * @param[in] _zFileName            File used to get information
 * @param[out] _pstFileInfo         Returned file's information
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxFile_GetInfo(const orxSTRING _zFileName, orxFILE_INFO *_pstFileInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Looks for the first file */
  if(orxFile_FindFirst(_zFileName, _pstFileInfo) != orxFALSE)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;

    /* Closes the find request */
    orxFile_FindClose(_pstFileInfo);
  }

  /* Done! */
  return eResult;
}

/** Opens a file for later read or write operation
 * @param[in] _zFileName           Full file's path to open
 * @param[in] _u32OpenFlags        List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occured)
 */
orxFILE *orxFASTCALL orxFile_Open(const orxSTRING _zFileName, orxU32 _u32OpenFlags)
{
  /* Convert the open flags into a string */
  orxCHAR acMode[4];
  orxBOOL bBinaryMode;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Clears mode */
  acMode[0] = orxCHAR_NULL;

  /*** LIB C MODES :
   * r   : Open text file for reading.
   *       The stream is positioned at the beginning of the file.
   * r+  : Open for reading and writing.
   *       The stream is positioned at the beginning of the file.
   * w   : Truncate file to zero length or create text file for writing.
   *       The stream is positioned at the beginning of the file.
   * w+  : Open for reading and writing.
   *       The file is created if it does not exist, otherwise it is truncated.
   *       The stream is positioned at the beginning of the file.
   * a   : Open for appending (writing at end of file).
   *       The file is created if it does not exist.
   *       The stream is positioned at the end of the file.
   * a+  : Open for reading and appending (writing at end of file).
   *       The file is created if it does not exist.
   *       The initial file position for reading is at the beginning of the file, but output is always appended to the end of the file.
       *
   *** AVAILABLE CONVERSIONS :
   * READ | WRITE | APPEND | result
   *  X   |       |        | r
   *      |  X    |        | w+
   *      |       |   X    | a
   *      |  X    |   X    | a
   *  X   |  X    |        | r+
   *  X   |       |   X    | a+
   *  X   |  X    |   X    | a+
   */

  /* Binary? */
  if(_u32OpenFlags & orxFILE_KU32_FLAG_OPEN_BINARY)
  {
    /* Removes it */
    _u32OpenFlags &= ~orxFILE_KU32_FLAG_OPEN_BINARY;

    /* Updates binary status*/
    bBinaryMode = orxTRUE;
  }
  else
  {
    /* Updates binary status*/
    bBinaryMode = orxFALSE;
  }

  /* Read only? */
  if(_u32OpenFlags == orxFILE_KU32_FLAG_OPEN_READ)
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "rb");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "r");
    }
  }
  /* Write only ?*/
  else if(_u32OpenFlags == orxFILE_KU32_FLAG_OPEN_WRITE)
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "wb+");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "w+");
    }
  }
  /* Append only ? */
  else if((_u32OpenFlags == orxFILE_KU32_FLAG_OPEN_APPEND)
       || (_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_APPEND)))
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "ab");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "a");
    }
  }
  else if(_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_WRITE))
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "rb+");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "r+");
    }
  }
  else if((_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_APPEND))
       || (_u32OpenFlags == (orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_APPEND)))
  {
    /* Binary? */
    if(bBinaryMode != orxFALSE)
    {
      /* Sets literal mode */
      orxString_Print(acMode, "ab+");
    }
    else
    {
      /* Sets literal mode */
      orxString_Print(acMode, "a+");
    }
  }

  /* Open the file */
  return(orxFILE *)fopen(_zFileName, acMode);
}

/** Reads data from a file
 * @param[out] _pReadData          Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
orxU32 orxFASTCALL orxFile_Read(void *_pReadData, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* Default return value */
  orxU32 u32Ret = 0;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    u32Ret = (orxU32)fread(_pReadData, (size_t)_u32ElemSize, (size_t)_u32NbElem, (FILE *)_pstFile);
  }

  /* Returns the number of read elements */
  return u32Ret;
}

/** writes data to a file
 * @param[in] _pDataToWrite        Pointer where will be stored datas
 * @param[in] _u32ElemSize         Size of 1 element
 * @param[in] _u32NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
orxU32 orxFASTCALL orxFile_Write(const void *_pDataToWrite, orxU32 _u32ElemSize, orxU32 _u32NbElem, orxFILE *_pstFile)
{
  /* Default return value */
  orxU32 u32Ret = 0;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Checks inputs */
  orxASSERT(_pstFile != orxNULL);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    u32Ret = (orxU32)fwrite(_pDataToWrite, (size_t)_u32ElemSize, (size_t)_u32NbElem, (FILE *)_pstFile);
  }

  /* Returns the number of read elements */
  return u32Ret;
}

/** Seeks to a position in the given file
 * @param[in] _pstFile              Concerned file
 * @param[in] _s32Position          Position (from start) where to set the indicator
 * @param[in] _eWhence              Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor positionif succesful, -1 otherwise
 */
orxS32 orxFASTCALL orxFile_Seek(orxFILE *_pstFile, orxS32 _s32Position, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxS32 s32Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstFile != orxNULL)
  {
    fseek((FILE *)_pstFile, (size_t)_s32Position, _eWhence);

    /* Updates result */
    s32Result = orxFile_Tell(_pstFile);

  }
  else
  {
    /* Updates result */
    s32Result = -1;
  }

  /* Done! */
  return s32Result;
}

/** Tells the current position of the indicator in a file
 * @param[in] _pstFile              Concerned file
 * @return Returns the current position of the file indicator, -1 if invalid
 */
orxS32 orxFASTCALL orxFile_Tell(const orxFILE *_pstFile)
{
  orxS32 s32Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstFile != orxNULL)
  {
    /* Updates result */
    s32Result = ftell((FILE *)_pstFile);
  }
  else
  {
    /* Updates result */
    s32Result = -1;
  }

  /* Done! */
  return s32Result;
}

/** Retrieves a file's size
 * @param[in] _pstFile              Concerned file
 * @return Returns the length of the file, <= 0 if invalid
 */
orxS32 orxFASTCALL orxFile_GetSize(const orxFILE *_pstFile)
{
  struct stat stStat;
  orxS32      s32Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstFile != orxNULL)
  {
#ifdef __orxMSVC__

    /* Gets file stats */
    fstat(((FILE *)_pstFile)->_file, &stStat);

#else /* __orxMSVC__ */

    /* Gets file stats */
    fstat(fileno((FILE *)_pstFile), &stStat);

#endif /* __orxMSVC__ */

    /* Updates result */
    s32Result = stStat.st_size;
  }
  else
  {
    /* Updates result */
    s32Result = 0;
  }

  /* Done! */
  return s32Result;
}

/** Prints a formatted string to a file
 * @param[in] _pstFile             Pointer on the file descriptor
 * @param[in] _zString             Formatted string
 * @return Returns the number of written characters
 */
orxS32 orxCDECL orxFile_Print(orxFILE *_pstFile, const orxSTRING _zString, ...)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zString != orxNULL);

  /* Valid input? */
  if(_pstFile != orxNULL)
  {
    va_list stArgs;

    /* Gets variable arguments & print the string */
    va_start(stArgs, _zString);
    s32Result = vfprintf((FILE *)_pstFile, _zString, stArgs);
    va_end(stArgs);
  }

  /* Done! */
  return s32Result;
}

/** Closes an oppened file
 * @param[in] _pstFile             File's pointer to close
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxFile_Close(orxFILE *_pstFile)
{
  /* Default return value */
  orxSTATUS eRet = orxSTATUS_FAILURE;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Checks inputs */
  orxASSERT(_pstFile != orxNULL);

  /* valid ? */
  if(_pstFile != orxNULL)
  {
    /* Close file pointer */
    if(fclose((FILE *)_pstFile) == 0)
    {
      /* Success ! */
      eRet = orxSTATUS_SUCCESS;
    }
  }

  /* return success status */
  return eRet;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312 4996)

#endif /* __orxMSVC__ */

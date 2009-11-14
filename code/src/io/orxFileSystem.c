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
 * @file orxFileSystem.c
 * @date 01/05/2005
 * @author bestel@arcallians.org
 * @author iarwain@orx-project.org
 *
 */


#include "io/orxFileSystem.h"
#include "memory/orxMemory.h"
#include "utils/orxString.h"
#include "debug/orxDebug.h"

#ifdef __orxWINDOWS__

  #include <io.h>

#ifdef __orxMSVC__

  #pragma warning(disable : 4311 4312)

#endif /* __orxMSVC__ */

#else

  #include <sys/types.h>
  #include <dirent.h>
  #include <fnmatch.h>
  #include <sys/stat.h>
  #include <unistd.h>

#endif


/** Module flags
 */
#define orxFILESYSTEM_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxFILESYSTEM_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxFILESYSTEM_STATIC_t
{
  orxU32 u32Flags;

} orxFILESYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxFILESYSTEM_STATIC sstFileSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

#ifdef __orxWINDOWS__

static orxINLINE void orxFileSystem_GetInfoFromData(const struct _finddata_t *_pstData, orxFILESYSTEM_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT(_pstData != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Stores info */
  _pstFileInfo->u32Size       = _pstData->size;
  _pstFileInfo->u32TimeStamp  = (orxU32)_pstData->time_write;
  orxString_NCopy(_pstFileInfo->zName, (orxSTRING)_pstData->name, 255);
  _pstFileInfo->zName[255]    = orxCHAR_NULL;
  orxString_Copy(_pstFileInfo->zFullName + orxString_GetLength(_pstFileInfo->zPath), _pstFileInfo->zName);
  _pstFileInfo->u32Flags      = (_pstData->attrib == 0)
                                ? orxFILESYSTEM_KU32_FLAG_INFO_NORMAL
                                : ((_pstData->attrib & _A_RDONLY) ? orxFILESYSTEM_KU32_FLAG_INFO_RDONLY : 0)
                                | ((_pstData->attrib & _A_HIDDEN) ? orxFILESYSTEM_KU32_FLAG_INFO_HIDDEN : 0)
                                | ((_pstData->attrib & _A_SUBDIR) ? orxFILESYSTEM_KU32_FLAG_INFO_DIR : 0);

  return;
}

#else /* __orxWINDOWS__ */

static orxINLINE void orxFileSystem_GetInfoFromData(const struct dirent *_pstData, orxFILESYSTEM_INFO *_pstFileInfo)
{
  struct stat stStat;
  orxSTRING   zName;

  /* Checks */
  orxASSERT(_pstData != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Gets data name */
  zName = (orxSTRING)_pstData->d_name;

  /* Stores info */
  orxString_NCopy(_pstFileInfo->zName, zName, 255);
  _pstFileInfo->zName[255] = orxCHAR_NULL;
  orxString_Copy(_pstFileInfo->zFullName + orxString_GetLength(_pstFileInfo->zPath), _pstFileInfo->zName);

  /* Gets file info */
  stat(_pstFileInfo->zFullName, &stStat);

  _pstFileInfo->u32Flags = 0;

  /* Read only file ? */
  /* TODO : Update the way that read only is computed. It depends of the reader user/group */
  if((stStat.st_mode & S_IROTH) && !(stStat.st_mode & S_IWOTH))
  {
    _pstFileInfo->u32Flags |= orxFILESYSTEM_KU32_FLAG_INFO_RDONLY;
  }

  /* Hidden ? */
  if(_pstFileInfo->zName[0] == '.')
  {
    _pstFileInfo->u32Flags |= orxFILESYSTEM_KU32_FLAG_INFO_HIDDEN;
  }

  /* Dir ? */
  if(stStat.st_mode & S_IFDIR)
  {
    _pstFileInfo->u32Flags |= orxFILESYSTEM_KU32_FLAG_INFO_DIR;
  }

  /* Normal file ? */
  if(_pstFileInfo->u32Flags == 0)
  {
    _pstFileInfo->u32Flags = orxFILESYSTEM_KU32_FLAG_INFO_NORMAL;
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

/** FileSystem module setup */
void orxFASTCALL orxFileSystem_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_FILESYSTEM, orxMODULE_ID_MEMORY);

  return;
}

/** Inits the FileSystem Module
 */
orxSTATUS orxFASTCALL orxFileSystem_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFileSystem, sizeof(orxFILESYSTEM_STATIC));

    /* Updates status */
    sstFileSystem.u32Flags |= orxFILESYSTEM_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;
}

/** Exits from the FileSystem Module
 */
void orxFASTCALL orxFileSystem_Exit()
{
  /* Was initialized? */
  if(sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY)
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstFileSystem, sizeof(orxFILESYSTEM_STATIC));
  }

  return;
}

/** Returns orxTRUE if a file exists, else orxFALSE.
 * @param[in] _zFileName           Full File's name to test
 * @return orxFALSE if _zFileName doesn't exist, else orxTRUE
 */
orxBOOL orxFASTCALL orxFileSystem_Exists(const orxSTRING _zFileName)
{
  orxFILESYSTEM_INFO stInfo;

  /* Clears it */
  orxMemory_Zero(&stInfo, sizeof(orxFILESYSTEM_INFO));

  /* Done! */
	return(orxFileSystem_Info(_zFileName, &(stInfo)) == orxSTATUS_SUCCESS);
}

/** Starts a new search. Find the first file that will match to the given pattern (e.g : /bin/toto* or c:\*.*)
 * @param[in] _zSearchPattern      Pattern to find
 * @param[out] _pstFileInfo        Informations about the first file found
 * @return orxTRUE if a file has been found, else orxFALSE
 */
orxBOOL orxFASTCALL orxFileSystem_FindFirst(const orxSTRING _zSearchPattern, orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxBOOL bResult = orxFALSE;

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32Handle;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
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
      u32Index = orxMIN(s32LastSeparator + 1, 1023);
      orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, u32Index);
      _pstFileInfo->zPath[u32Index] = orxCHAR_NULL;
      orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

      /* Stores pattern */
      u32Index = orxMIN(orxString_GetLength(_zSearchPattern) - s32LastSeparator - 1, 255);
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
      u32Index = orxMIN(orxString_GetLength(_zSearchPattern), 255);
      orxString_NCopy(_pstFileInfo->zPattern, _zSearchPattern, u32Index);
      _pstFileInfo->zPattern[u32Index] = orxCHAR_NULL;
    }

    /* Tranfers file info */
    orxFileSystem_GetInfoFromData(&stData, _pstFileInfo);

    /* Stores handle */
    _pstFileInfo->hInternal = (orxHANDLE)s32Handle;

    /* Updates result */
    bResult = orxTRUE;
  }

#else /* __orxWINDOWS__ */

  /* Stores seach pattern */
  orxS32 s32LastSeparator, i;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
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
    u32Index = orxMIN(s32LastSeparator + 1, 1023);
    orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, u32Index);
    _pstFileInfo->zPath[u32Index] = orxCHAR_NULL;
    orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

    /* Stores pattern */
    u32Index = orxMIN(orxString_GetLength(_zSearchPattern) - s32LastSeparator - 1, 255);
    orxString_NCopy(_pstFileInfo->zPattern, &_zSearchPattern[s32LastSeparator] + 1, u32Index);
    _pstFileInfo->zPattern[u32Index] = orxCHAR_NULL;
  }
  else
  {
    orxU32 u32Index;

    /* Stores pattern */
    u32Index = orxMIN(orxString_GetLength(_zSearchPattern), 255);
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
    _pstFileInfo->hInternal = (orxHANDLE) pDir;

    /* Retrieves info */
    bResult = orxFileSystem_FindNext(_pstFileInfo);
  }

#endif /* __orxWINDOWS__ */

  /* Done! */
  return bResult;
}

/** Continues a search. Find the next occurence of a pattern. The search has to be started with orxFileSystem_FindFirst
 * @param[in,out] _pstFileInfo    Informations about the found file
 * @return orxTRUE, if the next file has been found, else returns orxFALSE
 */
orxBOOL orxFASTCALL orxFileSystem_FindNext(orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxBOOL bResult = orxFALSE;

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32FindResult;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Opens the search */
  s32FindResult = _findnext((orxS32)_pstFileInfo->hInternal, &stData);

  /* Valid? */
  if(s32FindResult == 0)
  {
    /* Tranfers file info */
    orxFileSystem_GetInfoFromData(&stData, _pstFileInfo);

    /* Updates result */
    bResult = orxTRUE;
  }

#else /* __orxWINDOWS__ */

  struct dirent *pstDirEnt;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Updates full name */
  orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

  /* Read directory */

  /* loop on entries until the pattern match */
  while((!bResult)
     && (pstDirEnt = readdir((DIR*)_pstFileInfo->hInternal)))
  {
    /* Gets file info */
    orxFileSystem_GetInfoFromData(pstDirEnt, _pstFileInfo);

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
void orxFASTCALL orxFileSystem_FindClose(orxFILESYSTEM_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

#ifdef __orxWINDOWS__

  /* Has valid handle? */
  if(((orxS32)_pstFileInfo->hInternal) > 0)
  {
    /* Closes the search */
    _findclose((orxS32)_pstFileInfo->hInternal);
  }

#else /* __orxWINDOWS__ */

  /* Closes the search */
  closedir((DIR *) _pstFileInfo->hInternal);

#endif /* __orxWINDOWS__ */

  /* Clears handle */
  _pstFileInfo->hInternal = 0;

  return;
}

/** Retrieves informations about a file
 * @param[in] _zFileName            Files used to get informations
 * @param[out] _pstFileInfo         Returned file's informations
 * @return Returns the status of the operation
 */
orxSTATUS orxFASTCALL orxFileSystem_Info(const orxSTRING _zFileName, orxFILESYSTEM_INFO *_pstFileInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Finds for the file */
  if(orxFileSystem_FindFirst(_zFileName, _pstFileInfo) != orxFALSE)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Closes the find */
  orxFileSystem_FindClose(_pstFileInfo);

  /* Done! */
  return eResult;
}

/** Copies a file
 * @param[in] _zSource             Source file's name
 * @param[in] _zDest               Destination file's name
 * @return The status of the operation
 */
orxSTATUS orxFASTCALL orxFileSystem_Copy(const orxSTRING _zSource, const orxSTRING _zDest)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

/** Renames a file
 * @param[in] _zSource             Source file's name
 * @param[in] _zDest               Destination file's name
 * @return The status of the operation
 */
orxSTATUS orxFASTCALL orxFileSystem_Rename(const orxSTRING _zSource, const orxSTRING _zDest)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

/** Deletes a file
 * @param[in] _zFileName           File's name to delete
 * @return The status of the operation
 */
orxSTATUS orxFASTCALL orxFileSystem_Delete(const orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

/** Creates a directory
 * @param[in] _zDirName            New directory's name
 * @return The status of the operation
 */
orxSTATUS orxFASTCALL orxFileSystem_CreateDir(const orxSTRING _zDirName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

/** Remove an empty directory
 * @param[in] _zDirName            Directory's name to delete
 * @return The status of the operation
 */
orxSTATUS orxFASTCALL orxFileSystem_DeleteDir(const orxSTRING _zDirName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);

  /* Not implemented yet */
  orxASSERT(orxFALSE && "Not implemented yet!");

  /* Done! */
  return eResult;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4311 4312)

#endif /* __orxMSVC__ */

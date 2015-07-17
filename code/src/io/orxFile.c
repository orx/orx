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
  #include <Shlobj.h>

  #ifdef __orxMSVC__

    #include <direct.h>

    #pragma warning(disable : 4311 4312 4996)

  #endif /* __orxMSVC__ */

#else /* __orxWINDOWS__ */

#define _FILE_OFFSET_BITS                       64

  #if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

    #include "main/orxAndroid.h"

  #endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */

  #include <dirent.h>
  #include <fnmatch.h>
  #include <unistd.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <pwd.h>

#endif /* __orxWINDOWS__ */


/** Module flags
 */
#define orxFILE_KU32_STATIC_FLAG_NONE           0x00000000  /**< No flags have been set */
#define orxFILE_KU32_STATIC_FLAG_READY          0x00000001  /**< The module has been initialized */

#ifdef __orxWINDOWS__
#ifdef __orxX86_64__
  #define orxFILE_CAST_HELPER                   (orxS64)
#else /* __orxX86_64__ */
  #define orxFILE_CAST_HELPER                   (orxS32)
#endif /* __orxX86_64__ */
#endif /* __orxWINDOWS__ */

/** Misc
 */
#define orxFILE_KU32_BUFFER_SIZE                512         /**< Buffer size */

#if defined(__orxLINUX__) || defined(__orxRASPBERRY_PI__)

#define orxFILE_KZ_APPLICATION_FOLDER           ".local/share"

#elif defined(__orxMAC__)

#define orxFILE_KZ_APPLICATION_FOLDER           "Library/Application Support"

#elif defined(__orxIOS__)

extern const orxSTRING orxiOS_GetDocumentsPath();

#endif


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
  orxCHAR acWorkDirectory[orxFILE_KU32_BUFFER_SIZE];
  orxU32  u32Flags;

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
  _pstFileInfo->s64Size       = (orxS64)_pstData->size;
  _pstFileInfo->s64TimeStamp  = (orxS64)_pstData->time_write;
  orxString_NCopy(_pstFileInfo->zName, (orxSTRING)_pstData->name, sizeof(_pstFileInfo->zName) - 1);
  _pstFileInfo->zName[sizeof(_pstFileInfo->zName) - 1] = orxCHAR_NULL;
  orxString_Copy(_pstFileInfo->zFullName + orxString_GetLength(_pstFileInfo->zPath), _pstFileInfo->zName);
  _pstFileInfo->u32Flags      = ((_pstData->attrib & (_A_RDONLY|_A_HIDDEN|_A_SUBDIR)) == 0)
                                ? orxFILE_KU32_FLAG_INFO_NORMAL
                                : ((_pstData->attrib & _A_RDONLY) ? orxFILE_KU32_FLAG_INFO_READONLY : 0)
                                | ((_pstData->attrib & _A_HIDDEN) ? orxFILE_KU32_FLAG_INFO_HIDDEN : 0)
                                | ((_pstData->attrib & _A_SUBDIR) ? orxFILE_KU32_FLAG_INFO_DIRECTORY : 0);

  /* Done! */
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
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_READONLY;
  }

  /* Hidden ? */
  if(_pstFileInfo->zName[0] == '.')
  {
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_HIDDEN;
  }

  /* Dir ? */
  if(stStat.st_mode & S_IFDIR)
  {
    _pstFileInfo->u32Flags |= orxFILE_KU32_FLAG_INFO_DIRECTORY;
  }

  /* Normal file ? */
  if(_pstFileInfo->u32Flags != orxFILE_KU32_FLAG_INFO_NORMAL)
  {
    _pstFileInfo->u32Flags &= ~orxFILE_KU32_FLAG_INFO_NORMAL;
  }

  /* Sets size and last file access time */
  _pstFileInfo->s64Size       = (orxS64)stStat.st_size;
  _pstFileInfo->s64TimeStamp  = (orxS64)stStat.st_mtime;

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

#ifdef __orxWINDOWS__

    /* Increases C runtime stdio limit */
    _setmaxstdio(2048);

#endif /* __orxWINDOWS__ */

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

/** Gets current user's home directory (without trailing separator)
 * @param[in] _zSubPath                     Sub-path to append to the home directory, orxNULL for none
 * @return Current user's home directory, use it immediately or copy it as will be modified by the next call to orxFile_GetHomeDirectory() or orxFile_GetApplicationSaveDirectory()
 */
const orxSTRING orxFASTCALL orxFile_GetHomeDirectory(const orxSTRING _zSubPath)
{
  orxS32 s32Index = -1;
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

#if defined(__orxWINDOWS__)

  {
    char acPath[MAX_PATH];

    /* Gets application folder */
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE, NULL, 0, acPath)))
    {
      /* For all characters */
      for(s32Index = 0; s32Index < MAX_PATH; s32Index++)
      {
        /* Copies it + replace windows separators by linux ones */
        sstFile.acWorkDirectory[s32Index] = (acPath[s32Index] != orxCHAR_DIRECTORY_SEPARATOR_WINDOWS) ? acPath[s32Index] : orxCHAR_DIRECTORY_SEPARATOR_LINUX;

        /* End of string? */
        if(acPath[s32Index] == orxCHAR_NULL)
        {
          /* Stops */
          break;
        }
      }
    }
  }

#elif defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxRASPBERRY_PI__)

  {
    const orxCHAR *zHome;

    /* Gets environment HOME variable */
    zHome = (orxCHAR *)getenv("HOME");

    /* Valid? */
    if(zHome != orxNULL)
    {
      /* Prints home directory */
      s32Index = orxString_NPrint(sstFile.acWorkDirectory, sizeof(sstFile.acWorkDirectory) - 1, "%s", zHome);
    }
    else
    {
      struct passwd *pstPasswd;

      /* Gets current user's passwd */
      pstPasswd = getpwuid(getuid());

      /* Valid? */
      if(pstPasswd != orxNULL)
      {
        /* Prints home directory */
        s32Index = orxString_NPrint(sstFile.acWorkDirectory, sizeof(sstFile.acWorkDirectory) - 1, "%s", pstPasswd->pw_dir);
      }
    }
  }

#endif

  /* Success? */
  if(s32Index >= 0)
  {
    /* Should add sub-path? */
    if(_zSubPath != orxNULL)
    {
      /* Appends folder name */
      s32Index += orxString_NPrint(sstFile.acWorkDirectory + s32Index, sizeof(sstFile.acWorkDirectory) - s32Index - 1, "%c%s", orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zSubPath);
    }

    /* Updates result */
    zResult = sstFile.acWorkDirectory;
  }

  /* Done! */
  return zResult;
}

/** Gets current user's application save directory (without trailing separator)
 * @param[in] _zSubPath                     Sub-path to append to the application save directory, orxNULL for none
 * @return Current user's application save directory, use it immediately or copy it as it will be modified by the next call to orxFile_GetHomeDirectory() or orxFile_GetApplicationSaveDirectory()
 */
const orxSTRING orxFASTCALL orxFile_GetApplicationSaveDirectory(const orxSTRING _zSubPath)
{
  orxS32 s32Index = -1;
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

#if defined(__orxWINDOWS__)

  {
    char acPath[MAX_PATH];

    /* Gets application folder */
    if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, acPath)))
    {
      /* For all characters */
      for(s32Index = 0; s32Index < MAX_PATH; s32Index++)
      {
        /* Copies it + replace windows separators by linux ones */
        sstFile.acWorkDirectory[s32Index] = (acPath[s32Index] != orxCHAR_DIRECTORY_SEPARATOR_WINDOWS) ? acPath[s32Index] : orxCHAR_DIRECTORY_SEPARATOR_LINUX;

        /* End of string? */
        if(acPath[s32Index] == orxCHAR_NULL)
        {
          /* Stops */
          break;
        }
      }
    }
  }

#elif defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxRASPBERRY_PI__)

  {
    const orxCHAR *zHome;

    /* Gets environment HOME variable */
    zHome = (orxCHAR *)getenv("HOME");

    /* Valid? */
    if(zHome != orxNULL)
    {
      /* Prints home directory */
      s32Index = orxString_NPrint(sstFile.acWorkDirectory, sizeof(sstFile.acWorkDirectory) - 1, "%s%c%s", zHome, orxCHAR_DIRECTORY_SEPARATOR_LINUX, orxFILE_KZ_APPLICATION_FOLDER);
    }
    else
    {
      struct passwd *pstPasswd;

      /* Gets current user's passwd */
      pstPasswd = getpwuid(getuid());

      /* Valid? */
      if(pstPasswd != orxNULL)
      {
        /* Prints home directory */
        s32Index = orxString_NPrint(sstFile.acWorkDirectory, sizeof(sstFile.acWorkDirectory) - 1, "%s%c%s", pstPasswd->pw_dir, orxCHAR_DIRECTORY_SEPARATOR_LINUX, orxFILE_KZ_APPLICATION_FOLDER);
      }
    }
  }

#elif defined(__orxIOS__)

  /* Prints documents directory */
  s32Index = orxString_NPrint(sstFile.acWorkDirectory, sizeof(sstFile.acWorkDirectory) - 1, "%s", orxiOS_GetDocumentsPath());

#elif defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

  /* Prints internal storage directory */
  s32Index = orxString_NPrint(sstFile.acWorkDirectory, sizeof(sstFile.acWorkDirectory) - 1, "%s", orxAndroid_GetInternalStoragePath());

#endif

  /* Success? */
  if(s32Index >= 0)
  {
    /* Should add sub-path? */
    if(_zSubPath != orxNULL)
    {
      /* Appends folder name */
      s32Index += orxString_NPrint(sstFile.acWorkDirectory + s32Index, sizeof(sstFile.acWorkDirectory) - s32Index - 1, "%c%s", orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zSubPath);
    }

    /* Updates result */
    zResult = sstFile.acWorkDirectory;
  }

  /* Done! */
  return zResult;
}

/** Checks if a file/directory exists
 * @param[in] _zFileName           Concerned file/directory
 * @return orxFALSE if _zFileName doesn't exist, orxTRUE otherwise
 */
orxBOOL orxFASTCALL orxFile_Exists(const orxSTRING _zFileName)
{
  orxFILE_INFO stInfo;

  /* Clears it */
  orxMemory_Zero(&stInfo, sizeof(orxFILE_INFO));

  /* Done! */
  return(orxFile_GetInfo(_zFileName, &(stInfo)) != orxSTATUS_FAILURE);
}

/** Starts a new file search: finds the first file/directory that will match to the given pattern (ex: /bin/foo*)
 * @param[in] _zSearchPattern      Pattern used for file/directory search
 * @param[out] _pstFileInfo        Information about the first file found
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFile_FindFirst(const orxSTRING _zSearchPattern, orxFILE_INFO *_pstFileInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxHANDLE           hHandle;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zSearchPattern != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Opens the search */
  hHandle = (orxHANDLE)_findfirst(_zSearchPattern, &stData);

  /* Valid? */
  if(orxFILE_CAST_HELPER hHandle > 0)
  {
    const orxSTRING zFileName;

    /* Gets base file name */
    zFileName = orxString_SkipPath(_zSearchPattern);

    /* Has directory? */
    if(zFileName != _zSearchPattern)
    {
      orxU32 u32Length;

      /* Updates path & full name base */
      u32Length = (orxU32)orxMIN(zFileName - _zSearchPattern, sizeof(_pstFileInfo->zPath) - 1);
      orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, u32Length);
      _pstFileInfo->zPath[u32Length] = orxCHAR_NULL;
      orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

      /* Stores pattern */
      u32Length = orxMIN(orxString_GetLength(zFileName), sizeof(_pstFileInfo->zPattern) - 1);
      orxString_NCopy(_pstFileInfo->zPattern, zFileName, u32Length);
      _pstFileInfo->zPattern[u32Length] = orxCHAR_NULL;
    }
    else
    {
      orxU32 u32Length;

      /* Clears vars */
      _pstFileInfo->zPath[0]      = orxCHAR_NULL;
      _pstFileInfo->zFullName[0]  = orxCHAR_NULL;

      /* Stores pattern */
      u32Length = orxMIN(orxString_GetLength(_zSearchPattern), sizeof(_pstFileInfo->zPattern) - 1);
      orxString_NCopy(_pstFileInfo->zPattern, _zSearchPattern, u32Length);
      _pstFileInfo->zPattern[u32Length] = orxCHAR_NULL;
    }

    /* Tranfers file info */
    orxFile_GetInfoFromData(&stData, _pstFileInfo);

    /* Stores handle */
    _pstFileInfo->hInternal = hHandle;

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

#else /* __orxWINDOWS__ */

  const orxSTRING zFileName;
  DIR            *pDir;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Gets base file name */
  zFileName = orxString_SkipPath(_zSearchPattern);

  /* Has directory? */
  if(zFileName != _zSearchPattern)
  {
    orxU32 u32Length;

    /* Updates path & full name base */
    u32Length = orxMIN((orxU32)(zFileName - _zSearchPattern), sizeof(_pstFileInfo->zPath) - 1);
    orxString_NCopy(_pstFileInfo->zPath, _zSearchPattern, u32Length);
    _pstFileInfo->zPath[u32Length] = orxCHAR_NULL;
    orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

    /* Stores pattern */
    u32Length = orxMIN(orxString_GetLength(zFileName), sizeof(_pstFileInfo->zPattern) - 1);
    orxString_NCopy(_pstFileInfo->zPattern, zFileName, u32Length);
    _pstFileInfo->zPattern[u32Length] = orxCHAR_NULL;
  }
  else
  {
    orxU32 u32Length;

    /* Stores pattern */
    u32Length = orxMIN(orxString_GetLength(_zSearchPattern), sizeof(_pstFileInfo->zPattern) - 1);
    orxString_NCopy(_pstFileInfo->zPattern, _zSearchPattern, u32Length);
    _pstFileInfo->zPattern[u32Length] = orxCHAR_NULL;

    /* Clears vars */
    orxString_Print(_pstFileInfo->zPath, "./");
    orxString_Print(_pstFileInfo->zFullName, "./");
  }

  /* Open directory */
  pDir = opendir(_pstFileInfo->zPath);

  /* Valid ? */
  if(pDir != orxNULL)
  {
    /* Stores the DIR handle */
    _pstFileInfo->hInternal = (orxHANDLE)pDir;

    /* Retrieves info */
    eResult = orxFile_FindNext(_pstFileInfo);

    /* Failure? */
    if(eResult == orxSTATUS_FAILURE)
    {
      /* Closes directory */
      closedir(pDir);

      /* Clears handle */
      _pstFileInfo->hInternal = 0;
    }
  }

#endif /* __orxWINDOWS__ */

  /* Done! */
  return eResult;
}

/** Continues a file search: finds the next occurrence of a pattern, the search has to be started with orxFile_FindFirst
 * @param[in,out] _pstFileInfo      Information about the last found file/directory
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFile_FindNext(orxFILE_INFO *_pstFileInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

#ifdef __orxWINDOWS__

  struct _finddata_t  stData;
  orxS32              s32FindResult;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Opens the search */
  s32FindResult = _findnext(orxFILE_CAST_HELPER _pstFileInfo->hInternal, &stData);

  /* Valid? */
  if(s32FindResult == 0)
  {
    /* Tranfers file info */
    orxFile_GetInfoFromData(&stData, _pstFileInfo);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

#else /* __orxWINDOWS__ */

  struct dirent *pstDirEnt;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Updates full name */
  orxString_Copy(_pstFileInfo->zFullName, _pstFileInfo->zPath);

  /* Reads directory */

  /* Loops over entries until the pattern matches */
  while((eResult == orxSTATUS_FAILURE)
     && (pstDirEnt = readdir((DIR*)_pstFileInfo->hInternal)))
  {
    /* Gets file info */
    orxFile_GetInfoFromData(pstDirEnt, _pstFileInfo);

    /* Match ? */
    eResult = (fnmatch(_pstFileInfo->zPattern, _pstFileInfo->zName, 0) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
  }

#endif /* __orxWINDOWS__ */

  /* Done! */
  return eResult;
}

/** Closes a search (frees the memory allocated for this search)
 * @param[in] _pstFileInfo         Information returned during search
 */
void orxFASTCALL orxFile_FindClose(orxFILE_INFO *_pstFileInfo)
{
  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_pstFileInfo != orxNULL);

#ifdef __orxWINDOWS__

  /* Has valid handle? */
  if(orxFILE_CAST_HELPER _pstFileInfo->hInternal > 0)
  {
    /* Closes the search */
    _findclose(orxFILE_CAST_HELPER _pstFileInfo->hInternal);
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

/** Retrieves a file/directory information
 * @param[in] _zFileName            Concerned file/directory name
 * @param[out] _pstFileInfo         Information of the file/directory
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFile_GetInfo(const orxSTRING _zFileName, orxFILE_INFO *_pstFileInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFileName != orxNULL);
  orxASSERT(_pstFileInfo != orxNULL);

  /* Looks for the first file */
  if(orxFile_FindFirst(_zFileName, _pstFileInfo) != orxSTATUS_FAILURE)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;

    /* Closes the find request */
    orxFile_FindClose(_pstFileInfo);
  }

  /* Done! */
  return eResult;
}

/** Removes a file or an empty directory
 * @param[in] _zFileName            Concerned file / directory
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFile_Remove(const orxSTRING _zFileName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zFileName != orxNULL);

  /* Tries to remove it as a file */
  if(remove(_zFileName) == 0)
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Tries to remove it as a directory */
#ifdef __orxWINDOWS__
    if(_rmdir(_zFileName) == 0)
#else /* __orxWINDOWS__ */
    if(rmdir(_zFileName) == 0)
#endif /* __orxWINDOWS__ */
    {
      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Done! */
  return eResult;
}

/** Makes a directory, works recursively if needed
 * @param[in] _zName                Name of the directory to make
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxFile_MakeDirectory(const orxSTRING _zName)
{
  const orxCHAR  *pcSrc;
  orxCHAR        *pcDst;
  orxCHAR         acBuffer[orxFILE_KU32_BUFFER_SIZE];
  orxSTATUS       eResult;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);
  orxASSERT(_zName != orxNULL);

  /* For all characters */
  for(pcSrc = _zName, pcDst = acBuffer; (*pcSrc != orxCHAR_NULL) && ((pcDst - acBuffer) < orxFILE_KU32_BUFFER_SIZE - 1); pcSrc++, pcDst++)
  {
    /* Is a directory separator? */
    if((*pcSrc == orxCHAR_DIRECTORY_SEPARATOR_LINUX) || (*pcSrc == orxCHAR_DIRECTORY_SEPARATOR_WINDOWS))
    {
      /* Ends buffer */
      *pcDst = orxCHAR_NULL;

      /* Makes intermediate directory */
#ifdef __orxWINDOWS__
      mkdir(acBuffer);
#else /* __orxWINDOWS__ */
      mkdir(acBuffer, S_IRWXU | S_IRWXG | S_IRWXO);
#endif /* __orxWINDOWS__ */

      /* Stores separator */
      *pcDst = orxCHAR_DIRECTORY_SEPARATOR_LINUX;
    }
    else
    {
      /* Copies character */
      *pcDst = *pcSrc;
    }
  }

  /* Success? */
  if(*pcSrc == orxCHAR_NULL)
  {
    /* Ends buffer */
    *pcDst = orxCHAR_NULL;

    /* Makes final directory */
#ifdef __orxWINDOWS__
    eResult = (mkdir(acBuffer) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
#else /* __orxWINDOWS__ */
    eResult = (mkdir(acBuffer, S_IRWXU | S_IRWXG | S_IRWXO) == 0) ? orxSTATUS_SUCCESS : orxSTATUS_FAILURE;
#endif /* __orxWINDOWS__ */
  }

  /* Done! */
  return eResult;
}

/** Opens a file for later read or write operation
 * @param[in] _zFileName           Full file's path to open
 * @param[in] _u32OpenFlags        List of used flags when opened
 * @return a File pointer (or orxNULL if an error has occurred)
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

  /* Write mode? */
  if(_u32OpenFlags & (orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_APPEND))
  {
    const orxSTRING zBaseName;

    /* Gets file base name */
    zBaseName = orxString_SkipPath(_zFileName);

    /* Has intermediate directories? */
    if(zBaseName > _zFileName)
    {
      orxCHAR acBuffer[orxFILE_KU32_BUFFER_SIZE];

      /* Is local buffer big enough? */
      if((orxU32)(zBaseName - _zFileName - 1) < sizeof(acBuffer) - 1)
      {
        /* Copies path locally */
        acBuffer[orxString_NPrint(acBuffer, sizeof(acBuffer) - 1, "%.*s", zBaseName - _zFileName - 1, _zFileName)] = orxCHAR_NULL;

        /* Makes sure path exists */
        orxFile_MakeDirectory(acBuffer);
      }
    }
  }

  /* Opens the file */
  return(orxFILE *)fopen(_zFileName, acMode);
}

/** Reads data from a file
 * @param[out] _pReadData          Buffer that will contain read data
 * @param[in] _s64ElemSize         Size of 1 element
 * @param[in] _s64NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of read elements (not bytes)
 */
orxS64 orxFASTCALL orxFile_Read(void *_pReadData, orxS64 _s64ElemSize, orxS64 _s64NbElem, orxFILE *_pstFile)
{
  /* Default return value */
  orxS64 s64Ret = 0;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    s64Ret = (orxS64)fread(_pReadData, (size_t)_s64ElemSize, (size_t)_s64NbElem, (FILE *)_pstFile);
  }

  /* Returns the number of read elements */
  return s64Ret;
}

/** Writes data to a file
 * @param[in] _pDataToWrite        Buffer that contains the data to write
 * @param[in] _s64ElemSize         Size of 1 element
 * @param[in] _s64NbElem           Number of elements
 * @param[in] _pstFile             Pointer on the file descriptor
 * @return Returns the number of written elements (not bytes)
 */
orxS64 orxFASTCALL orxFile_Write(const void *_pDataToWrite, orxS64 _s64ElemSize, orxS64 _s64NbElem, orxFILE *_pstFile)
{
  /* Default return value */
  orxS64 s64Ret = 0;

  /* Module initialized ? */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Checks inputs */
  orxASSERT(_pstFile != orxNULL);

  /* Valid input ? */
  if(_pstFile != orxNULL)
  {
    s64Ret = (orxS64)fwrite(_pDataToWrite, (size_t)_s64ElemSize, (size_t)_s64NbElem, (FILE *)_pstFile);
  }

  /* Returns the number of read elements */
  return s64Ret;
}

/** Seeks to a position in the given file
 * @param[in] _pstFile              Concerned file
 * @param[in] _s64Position          Position (from start) where to set the indicator
 * @param[in] _eWhence              Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor position if successful, -1 otherwise
 */
orxS64 orxFASTCALL orxFile_Seek(orxFILE *_pstFile, orxS64 _s64Position, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxS64 s64Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstFile != orxNULL)
  {
#if defined(__orxMSVC__) && defined(__orxX86_64__)
    (void)fseek((FILE *)_pstFile, (long)_s64Position, _eWhence);
#else /* __orxMSVC__ && __orxX86_64__ */
    (void)fseek((FILE *)_pstFile, (size_t)_s64Position, _eWhence);
#endif /* __orxMSVC__ && __orxX86_64__ */

    /* Updates result */
    s64Result = orxFile_Tell(_pstFile);

  }
  else
  {
    /* Updates result */
    s64Result = -1;
  }

  /* Done! */
  return s64Result;
}

/** Tells the current position of the indicator in a file
 * @param[in] _pstFile              Concerned file
 * @return Returns the current position of the file indicator, -1 if invalid
 */
orxS64 orxFASTCALL orxFile_Tell(const orxFILE *_pstFile)
{
  orxS64 s64Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstFile != orxNULL)
  {
    /* Updates result */
    s64Result = ftell((FILE *)_pstFile);
  }
  else
  {
    /* Updates result */
    s64Result = -1;
  }

  /* Done! */
  return s64Result;
}

/** Retrieves a file's size
 * @param[in] _pstFile              Concerned file
 * @return Returns the length of the file, <= 0 if invalid
 */
orxS64 orxFASTCALL orxFile_GetSize(const orxFILE *_pstFile)
{
  orxS64 s64Result;

  /* Checks */
  orxASSERT((sstFile.u32Flags & orxFILE_KU32_STATIC_FLAG_READY) == orxFILE_KU32_STATIC_FLAG_READY);

  /* Valid? */
  if(_pstFile != orxNULL)
  {
#ifdef __orxWINDOWS__

    struct _stat64 stStat;

    /* Gets file stats */
    _fstat64(((FILE *)_pstFile)->_file, &stStat);

#else /* __orxWINDOWS__ */

    struct stat stStat;

    /* Gets file stats */
    fstat(fileno((FILE *)_pstFile), &stStat);

#endif /* __orxWINDOWS__ */

    /* Updates result */
    s64Result = (orxS64)stStat.st_size;
  }
  else
  {
    /* Updates result */
    s64Result = 0;
  }

  /* Done! */
  return s64Result;
}

/** Retrieves a file's time of last modification
 * @param[in] _pstFile              Concerned file
 * @return Returns the time of the last modification, in seconds, since epoch
 */
orxS64 orxFASTCALL orxFile_GetTime(const orxFILE *_pstFile)
{
  struct stat stStat;
  orxS64      s64Result;

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
    s64Result = (orxS64)stStat.st_mtime;
  }
  else
  {
    /* Updates result */
    s64Result = 0;
  }

  /* Done! */
  return s64Result;
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

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008- Orx-Project
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
 * @file orxResource.c
 * @date 20/01/2013
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxResource.h"

#include "debug/orxDebug.h"
#include "core/orxClock.h"
#include "core/orxCommand.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxSystem.h"
#include "core/orxThread.h"
#include "debug/orxProfiler.h"
#include "io/orxFile.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "utils/orxHashTable.h"
#include "utils/orxLinkList.h"
#include "utils/orxString.h"

#if defined(__orxANDROID__)

#include "main/android/orxAndroid.h"

#endif /* __orxANDROID__ */

/** Module flags
 */
#define orxRESOURCE_KU32_STATIC_FLAG_NONE             0x00000000                      /**< No flags */

#define orxRESOURCE_KU32_STATIC_FLAG_READY            0x00000001                      /**< Ready flag */
#define orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED    0x00000002                      /**< Config loaded flag */
#define orxRESOURCE_KU32_STATIC_FLAG_WATCH_SET        0x00000004                      /**< Watch set flag */
#define orxRESOURCE_KU32_STATIC_FLAG_NOTIFY_SET       0x00000008                      /**< Notify set flag */
#define orxRESOURCE_KU32_STATIC_FLAG_WATCH_REGISTERED 0x00000010                      /**< Watch registered flag */
#define orxRESOURCE_KU32_STATIC_FLAG_EXIT             0x00000020                      /**< Exit flag */

#define orxRESOURCE_KU32_STATIC_MASK_ALL              0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxRESOURCE_KU32_CACHE_TABLE_SIZE             256                             /**< Resource info cache table size */

#define orxRESOURCE_KU32_RESOURCE_INFO_BANK_SIZE      256                             /**< Resource info bank size */

#define orxRESOURCE_KU32_STORAGE_BANK_SIZE            128                             /**< Storage bank size */
#define orxRESOURCE_KU32_GROUP_BANK_SIZE              8                               /**< Group bank size */
#define orxRESOURCE_KU32_TYPE_BANK_SIZE               8                               /**< Type bank size */

#define orxRESOURCE_KU32_MEMORY_BANK_SIZE             32                              /**< Memory bank size */

#define orxRESOURCE_KU32_OPEN_INFO_BANK_SIZE          64                              /**< Open resource info bank size */

#define orxRESOURCE_KU32_WATCH_ITERATION_LIMIT        2                               /**< Watch iteration limit */
#define orxRESOURCE_KU32_WATCH_TIME_UNINITIALIZED     -1                              /**< Watch time uninitialized */
#define orxRESOURCE_KF_WATCH_NOTIFICATION_DELAY       0.2                             /**< Watch notification delay */

#define orxRESOURCE_KU32_BUFFER_SIZE                  256                             /**< Buffer size */
#define orxRESOURCE_KU32_REQUEST_LIST_SIZE            8192                            /**< Request list size */

#define orxRESOURCE_KZ_THREAD_NAME                    "Resource"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Resource type
 */
typedef struct __orxRESOURCE_TYPE_t
{
  orxLINKLIST_NODE          stNode;                                                   /**< Linklist node */
  orxRESOURCE_TYPE_INFO     stInfo;                                                   /**< Type info */

} orxRESOURCE_TYPE;

/** Resource group
 */
typedef struct __orxRESOURCE_GROUP_t
{
  orxSTRINGID               stID;                                                     /**< Group name ID */
  orxLINKLIST               stStorageList;                                            /**< Group storage list */
  orxBANK                  *pstStorageBank;                                           /**< Group storage bank */
  orxHASHTABLE             *pstCacheTable;                                            /**< Group cache table */

} orxRESOURCE_GROUP;

/** Storage info
 */
typedef struct __orxRESOURCE_STORAGE_t
{
  orxLINKLIST_NODE          stNode;                                                   /**< Linklist node */
  orxSTRINGID               stID;                                                     /**< Literal storage ID */

} orxRESOURCE_STORAGE;

/** Resource info
 */
typedef struct __orxRESOURCE_INFO_t
{
  orxSTRING                 zLocation;                                                /**< Resource literal location */
  orxRESOURCE_TYPE_INFO    *pstTypeInfo;                                              /**< Resource type info */
  orxS64                    s64Time;                                                  /**< Resource modification time */
  orxSTRINGID               stGroupID;                                                /**< Group ID */
  orxSTRINGID               stStorageID;                                              /**< Storage ID */
  orxSTRINGID               stNameID;                                                 /**< Name ID */
  orxBOOL                   bPendingWatch;                                            /**< Pending watch */

} orxRESOURCE_INFO;

/** Open resource info
 */
typedef struct __orxRESOURCE_OPEN_INFO_t
{
  orxRESOURCE_TYPE_INFO    *pstTypeInfo;                                              /**< Resource type info */
  orxHANDLE                 hResource;                                                /**< Resource handle */
  orxSTRING                 zLocation;                                                /**< Resource location */
  volatile orxU32           u32OpCount;                                               /**< Operation count */

} orxRESOURCE_OPEN_INFO;

/** Request type enum
 */
typedef enum __orxRESOURCE_REQUEST_TYPE_t
{
  orxRESOURCE_REQUEST_TYPE_READ = 0,
  orxRESOURCE_REQUEST_TYPE_WRITE,
  orxRESOURCE_REQUEST_TYPE_CLOSE,
  orxRESOURCE_REQUEST_TYPE_GET_TIME,

  orxRESOURCE_REQUEST_TYPE_NUMBER,

  orxRESOURCE_REQUEST_TYPE_NONE = orxENUM_NONE

} orxRESOURCE_REQUEST_TYPE;

/** Request
 */
typedef struct __orxRESOURCE_REQUEST_t
{
  orxS64                    s64Size;                                                  /**< Request buffer size */
  void                     *pBuffer;                                                  /**< Request buffer */
  orxRESOURCE_OP_FUNCTION   pfnCallback;                                              /**< Request completion callback */
  void                     *pContext;                                                 /**< Request context */
  orxRESOURCE_OPEN_INFO    *pstResourceInfo;                                          /**< Request open resource info */
  orxRESOURCE_REQUEST_TYPE  eType;                                                    /**< Request type */

} orxRESOURCE_REQUEST;

/** Memory data
 */
typedef struct __orxRESOURCE_MEMORY_DATA_t
{
  orxSTRINGID               stNameID;                                                 /**< Memory Data Name ID */
  orxS64                    s64Size;                                                  /**< Memory Data Size */
  orxS64                    s64Time;                                                  /**< Memory Data Time */
  const orxU8              *pu8Buffer;                                                /**< Memory Data Buffer */
  orxU32                    u32RefCount;                                              /**< Memory Data Ref Count */

} orxRESOURCE_MEMORY_DATA;

/** Memory resource
 */
typedef struct __orxRESOURCE_MEMORY_RESOURCE_t
{
  orxS64                    s64Cursor;                                                /**< Memory Resource Cursor */
  orxRESOURCE_MEMORY_DATA  *pstData;                                                  /**< Memory Resource Data */

} orxRESOURCE_MEMORY_RESOURCE;

/** Static structure
 */
typedef struct __orxRESOURCE_STATIC_t
{
  orxBANK                  *pstGroupBank;                                             /**< Group bank */
  orxBANK                  *pstTypeBank;                                              /**< Type info bank */
  orxBANK                  *pstResourceInfoBank;                                      /**< Resource info bank */
  orxBANK                  *pstOpenInfoBank;                                          /**< Open resource table size */
  orxTHREAD_SEMAPHORE*      pstRequestSemaphore;                                      /**< Request semaphore */
  orxTHREAD_SEMAPHORE*      pstWorkerSemaphore;                                       /**< Worker semaphore */
  orxLINKLIST               stTypeList;                                               /**< Type list */
  orxSTRING                 zLastUncachedLocation;                                    /**< Last uncached location */
  orxSTRINGID               stLastWatchedGroupID;                                     /**< Last watched group ID */
  orxHASHTABLE             *pstMemoryDataTable;                                       /**< Memory data table */
  orxBANK                  *pstMemoryDataBank;                                        /**< Memory data bank */
  orxBANK                  *pstMemoryResourceBank;                                    /**< Memory resource bank */
  volatile orxSTATUS        eThreadResult;                                            /**< Thread result */
  volatile orxRESOURCE_REQUEST astRequestList[orxRESOURCE_KU32_REQUEST_LIST_SIZE];    /**< Request list */
  volatile orxU32           u32RequestInIndex;                                        /**< Request in index */
  volatile orxU32           u32RequestProcessIndex;                                   /**< Request process index */
  volatile orxU32           u32RequestOutIndex;                                       /**< Request out index */
  orxU32                    u32RequestThreadID;                                       /**< Request thread ID */
  orxU32                    u32Flags;                                                 /**< Control flags */

} orxRESOURCE_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxRESOURCE_STATIC sstResource;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

static const orxSTRING orxFASTCALL orxResource_File_Locate(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName, orxBOOL _bRequireExistence)
{
  static orxCHAR  sacFileLocationBuffer[orxRESOURCE_KU32_BUFFER_SIZE];
  orxFILE_INFO    stInfo;
  orxFILE        *pstFile = orxNULL;
  const orxSTRING zResult = orxNULL;

  /* Default storage? */
  if(orxString_Compare(_zStorage, orxRESOURCE_KZ_DEFAULT_STORAGE) == 0)
  {
    /* Uses name as path */
    orxString_NPrint(sacFileLocationBuffer, sizeof(sacFileLocationBuffer), "%s", _zName);
  }
  else
  {
    /* Composes full name */
    orxString_NPrint(sacFileLocationBuffer, sizeof(sacFileLocationBuffer), "%s%c%s", _zStorage, orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zName);
  }

  /* Exists or doesn't require existence and can be created? */
  if(((orxFile_GetInfo(sacFileLocationBuffer, &stInfo) != orxSTATUS_FAILURE)
   && !orxFLAG_TEST(stInfo.u32Flags, orxFILE_KU32_FLAG_INFO_DIRECTORY))
  || ((_bRequireExistence == orxFALSE)
   && ((pstFile = orxFile_Open(sacFileLocationBuffer, orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_BINARY)) != orxNULL)))
  {
    /* Updates result */
    zResult = sacFileLocationBuffer;

    /* Has temporary file? */
    if(pstFile != orxNULL)
    {
      /* Removes it */
      orxFile_Close(pstFile);
      orxFile_Delete(sacFileLocationBuffer);
    }
  }

  /* Done! */
  return zResult;
}

static orxHANDLE orxFASTCALL orxResource_File_Open(const orxSTRING _zLocation, orxBOOL _bEraseMode)
{
  orxFILE  *pstFile;
  orxHANDLE hResult;

  /* Opens file */
  pstFile = orxFile_Open(_zLocation, (_bEraseMode != orxFALSE) ? orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_BINARY : orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_BINARY);

  /* Not in erase mode and couldn't open it? */
  if((_bEraseMode == orxFALSE) && (pstFile == orxNULL))
  {
    /* Opens it in read-only mode */
    pstFile = orxFile_Open(_zLocation, orxFILE_KU32_FLAG_OPEN_READ | orxFILE_KU32_FLAG_OPEN_BINARY);
  }

  /* Updates result */
  hResult = (pstFile != orxNULL) ? (orxHANDLE)pstFile : orxHANDLE_UNDEFINED;

  /* Done! */
  return hResult;
}

static void orxFASTCALL orxResource_File_Close(orxHANDLE _hResource)
{
  orxFILE *pstFile;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Closes it */
  orxFile_Close(pstFile);

  /* Done! */
  return;
}

static orxS64 orxFASTCALL orxResource_File_GetSize(orxHANDLE _hResource)
{
  orxFILE  *pstFile;
  orxS64    s64Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s64Result = orxFile_GetSize(pstFile);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_File_GetTime(const orxSTRING _zLocation)
{
  orxFILE_INFO  stFileInfo;
  orxS64        s64Result;

  /* Gets file info */
  if(orxFile_GetInfo(_zLocation, &stFileInfo) != orxSTATUS_FAILURE)
  {
    /* Updates result */
    s64Result = stFileInfo.s64TimeStamp;
  }
  else
  {
    /* Updates result */
    s64Result = 0;
  }

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_File_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxFILE  *pstFile;
  orxS64    s64Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s64Result = orxFile_Seek(pstFile, _s64Offset, _eWhence);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_File_Tell(orxHANDLE _hResource)
{
  orxFILE  *pstFile;
  orxS64    s64Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s64Result = orxFile_Tell(pstFile);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_File_Read(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer)
{
  orxFILE  *pstFile;
  orxS64    s64Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s64Result = orxFile_Read(_pBuffer, sizeof(orxCHAR), _s64Size, pstFile);

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_File_Write(orxHANDLE _hResource, orxS64 _s64Size, const void *_pBuffer)
{
  orxFILE  *pstFile;
  orxS64    s64Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s64Result = orxFile_Write(_pBuffer, sizeof(orxCHAR), _s64Size, pstFile);

  /* Done! */
  return s64Result;
}

static orxSTATUS orxFASTCALL orxResource_File_Delete(const orxSTRING _zLocation)
{
  orxSTATUS eResult;

  /* Deletes file */
  eResult = orxFile_Delete(_zLocation);

  /* Done! */
  return eResult;
}

static const orxSTRING orxFASTCALL orxResource_Memory_Locate(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName, orxBOOL _bRequireExistence)
{
  static orxCHAR  sacFileLocationBuffer[orxRESOURCE_KU32_BUFFER_SIZE];
  const orxSTRING zResult = orxNULL;

  /* Composes full name */
  orxString_NPrint(sacFileLocationBuffer, sizeof(sacFileLocationBuffer), "%s%c%s%c%s", (_zGroup != orxNULL) ? _zGroup : orxSTRING_EMPTY, orxRESOURCE_KC_LOCATION_SEPARATOR, ((_zStorage != orxNULL) && (orxString_Compare(_zStorage, orxRESOURCE_KZ_DEFAULT_STORAGE) != 0)) ? _zStorage : orxSTRING_EMPTY, orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zName);

  /* Found in group? */
  if(orxHashTable_Get(sstResource.pstMemoryDataTable, orxString_Hash(sacFileLocationBuffer)) != orxNULL)
  {
    /* Updates result */
    zResult = sacFileLocationBuffer;
  }
  else
  {
    orxCHAR *zGenericName;

    /* Gets generic name */
    zGenericName = sacFileLocationBuffer + orxString_GetLength(_zGroup);

    /* Found? */
    if(orxHashTable_Get(sstResource.pstMemoryDataTable, orxString_Hash(zGenericName)) != orxNULL)
    {
      /* Updates result */
      zResult = zGenericName;
    }
  }

  /* Done! */
  return zResult;
}

static orxHANDLE orxFASTCALL orxResource_Memory_Open(const orxSTRING _zLocation, orxBOOL _bEraseMode)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Not in erase mode? */
  if(_bEraseMode == orxFALSE)
  {
    orxRESOURCE_MEMORY_DATA *pstData;

    /* Gets its data */
    pstData = (orxRESOURCE_MEMORY_DATA *)orxHashTable_Get(sstResource.pstMemoryDataTable, orxString_Hash(_zLocation));

    /* Found? */
    if(pstData != orxNULL)
    {
      orxRESOURCE_MEMORY_RESOURCE *pstResource;

      /* Allocates its internal resource */
      pstResource = (orxRESOURCE_MEMORY_RESOURCE *)orxBank_Allocate(sstResource.pstMemoryResourceBank);

      /* Success? */
      if(pstResource != orxNULL)
      {
        /* Inits it */
        orxMemory_Zero(pstResource, sizeof(orxRESOURCE_MEMORY_RESOURCE));
        pstData->u32RefCount++;
        pstResource->pstData = pstData;

        /* Updates result */
        hResult = (orxHANDLE)pstResource;
      }
    }
  }

  /* Done! */
  return hResult;
}

static void orxFASTCALL orxResource_Memory_Close(orxHANDLE _hResource)
{
  orxRESOURCE_MEMORY_RESOURCE *pstResource;

  /* Gets internal resource */
  pstResource = (orxRESOURCE_MEMORY_RESOURCE *)_hResource;

  /* Updates its data's ref count */
  orxASSERT(pstResource->pstData->u32RefCount > 0);
  pstResource->pstData->u32RefCount--;

  /* Frees it */
  orxBank_Free(sstResource.pstMemoryResourceBank, pstResource);

  /* Done! */
  return;
}

static orxS64 orxFASTCALL orxResource_Memory_GetSize(orxHANDLE _hResource)
{
  orxRESOURCE_MEMORY_RESOURCE  *pstResource;
  orxS64                        s64Result;

  /* Gets internal resource */
  pstResource = (orxRESOURCE_MEMORY_RESOURCE *)_hResource;

  /* Updates result */
  s64Result = pstResource->pstData->s64Size;

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_Memory_GetTime(const orxSTRING _zLocation)
{
  orxRESOURCE_MEMORY_DATA *pstData;
  orxS64                   s64Result = 0;

  /* Gets its data */
  pstData = (orxRESOURCE_MEMORY_DATA *)orxHashTable_Get(sstResource.pstMemoryDataTable, orxString_Hash(_zLocation));

  /* Found? */
  if(pstData != orxNULL)
  {
    /* Updates result */
    s64Result = pstData->s64Time;
  }

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_Memory_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxRESOURCE_MEMORY_RESOURCE  *pstResource;
  orxS64                        s64Cursor;

  /* Gets internal resource */
  pstResource = (orxRESOURCE_MEMORY_RESOURCE *)_hResource;

  // Depending on seek mode
  switch(_eWhence)
  {
    case orxSEEK_OFFSET_WHENCE_START:
    {
      // Computes cursor
      s64Cursor = _s64Offset;
      break;
    }

    case orxSEEK_OFFSET_WHENCE_CURRENT:
    {
      // Computes cursor
      s64Cursor = pstResource->s64Cursor + _s64Offset;
      break;
    }

    case orxSEEK_OFFSET_WHENCE_END:
    {
      // Computes cursor
      s64Cursor = pstResource->pstData->s64Size - _s64Offset;
      break;
    }

    default:
    {
      // Failure
      s64Cursor = -1;
      break;
    }
  }

  // Is cursor valid?
  if((s64Cursor >= 0) && (s64Cursor <= pstResource->pstData->s64Size))
  {
    // Updates cursor
    pstResource->s64Cursor = s64Cursor;
  }
  else
  {
    // Clears value
    s64Cursor = -1;
  }

  /* Done! */
  return s64Cursor;
}

static orxS64 orxFASTCALL orxResource_Memory_Tell(orxHANDLE _hResource)
{
  orxRESOURCE_MEMORY_RESOURCE  *pstResource;
  orxS64                        s64Result;

  /* Gets internal resource */
  pstResource = (orxRESOURCE_MEMORY_RESOURCE *)_hResource;

  /* Updates result */
  s64Result = pstResource->s64Cursor;

  /* Done! */
  return s64Result;
}

static orxS64 orxFASTCALL orxResource_Memory_Read(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer)
{
  orxRESOURCE_MEMORY_RESOURCE  *pstResource;
  orxS64                        s64CopySize;

  /* Gets internal resource */
  pstResource = (orxRESOURCE_MEMORY_RESOURCE *)_hResource;

  // Gets actual copy size to prevent any out-of-bound access
  s64CopySize = orxMIN(_s64Size, pstResource->pstData->s64Size - pstResource->s64Cursor);

  // Should copy content?
  if(s64CopySize != 0)
  {
    // Copies content
    orxMemory_Copy(_pBuffer, pstResource->pstData->pu8Buffer + pstResource->s64Cursor, (orxS32)s64CopySize);
  }

  // Updates cursor
  pstResource->s64Cursor += s64CopySize;

  // Done!
  return s64CopySize;
}

static orxINLINE void orxResource_DeleteGroup(orxRESOURCE_GROUP *_pstGroup)
{
  orxRESOURCE_INFO *pstResourceInfo;
  orxHANDLE         hIterator;

  /* For all cached resources */
  for(hIterator = orxHashTable_GetNext(_pstGroup->pstCacheTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstResourceInfo);
      hIterator != orxHANDLE_UNDEFINED;
      hIterator = orxHashTable_GetNext(_pstGroup->pstCacheTable, hIterator, orxNULL, (void **)&pstResourceInfo))
  {
    /* Deletes its location */
    orxMemory_Free(pstResourceInfo->zLocation);

    /* Frees it */
    orxBank_Free(sstResource.pstResourceInfoBank, pstResourceInfo);
  }

  /* Deletes cache table */
  orxHashTable_Delete(_pstGroup->pstCacheTable);

  /* Deletes storage bank */
  orxBank_Delete(_pstGroup->pstStorageBank);

  /* Deletes it */
  orxBank_Free(sstResource.pstGroupBank, _pstGroup);

  /* Done! */
  return;
}

static orxINLINE orxRESOURCE_GROUP *orxResource_CreateGroup(orxSTRINGID _stGroupID)
{
  orxRESOURCE_GROUP *pstResult;

  /* Creates group */
  pstResult = (orxRESOURCE_GROUP *)orxBank_Allocate(sstResource.pstGroupBank);

  /* Success? */
  if(pstResult != orxNULL)
  {
    orxRESOURCE_STORAGE *pstStorage;

    /* Inits it */
    pstResult->stID           = _stGroupID;
    pstResult->pstStorageBank = orxBank_Create(orxRESOURCE_KU32_STORAGE_BANK_SIZE, sizeof(orxRESOURCE_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    pstResult->pstCacheTable  = orxHashTable_Create(orxRESOURCE_KU32_CACHE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    orxMemory_Zero(&(pstResult->stStorageList), sizeof(orxLINKLIST));

    /* Creates storage */
    pstStorage = (orxRESOURCE_STORAGE *)orxBank_Allocate(pstResult->pstStorageBank);

    /* Success? */
    if(pstStorage != orxNULL)
    {
      /* Stores its content */
      pstStorage->stID = orxString_GetID(orxRESOURCE_KZ_DEFAULT_STORAGE);

      /* Clears node */
      orxMemory_Zero(&(pstStorage->stNode), sizeof(orxLINKLIST_NODE));

      /* Adds it first */
      orxLinkList_AddStart(&(pstResult->stStorageList), &(pstStorage->stNode));
    }
    else
    {
      /* Deletes it */
      orxResource_DeleteGroup(pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

static orxINLINE orxRESOURCE_GROUP *orxResource_FindGroup(orxSTRINGID _stGroupID)
{
  orxRESOURCE_GROUP *pstResult;

  /* Gets group */
  for(pstResult = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
      (pstResult != orxNULL) && (pstResult->stID != _stGroupID);
      pstResult = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstResult))
    ;

  /* Done! */
  return pstResult;
}

static void orxFASTCALL orxResource_NotifyRequest(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_NotifyRequest");

  /* While there are processed requests */
  while(sstResource.u32RequestOutIndex != sstResource.u32RequestProcessIndex)
  {
    volatile orxRESOURCE_REQUEST *pstRequest;

    /* Gets request */
    pstRequest = &(sstResource.astRequestList[sstResource.u32RequestOutIndex]);

    /* Decrements operation count */
    if(pstRequest->pstResourceInfo != orxNULL)
    {
      pstRequest->pstResourceInfo->u32OpCount--;
    }

    /* Has callback? */
    if(pstRequest->pfnCallback != orxNULL)
    {
      /* Notifies it */
      pstRequest->pfnCallback((orxHANDLE)pstRequest->pstResourceInfo, pstRequest->s64Size, pstRequest->pBuffer, pstRequest->pContext);
    }

    /* Updates request out index */
    orxMEMORY_BARRIER();
    sstResource.u32RequestOutIndex = (sstResource.u32RequestOutIndex + 1) & (orxRESOURCE_KU32_REQUEST_LIST_SIZE - 1);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

static orxSTATUS orxFASTCALL orxResource_ProcessRequests(void *_pContext)
{
  orxSTATUS eResult;

  /* Waits for worker semaphore */
  orxThread_WaitSemaphore(sstResource.pstWorkerSemaphore);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_ProcessRequests");

  /* While there are pending requests */
  while(sstResource.u32RequestProcessIndex != sstResource.u32RequestInIndex)
  {
    volatile orxRESOURCE_REQUEST *pstRequest;

    /* Gets request */
    pstRequest = &(sstResource.astRequestList[sstResource.u32RequestProcessIndex]);

    /* Depending on request type */
    switch(pstRequest->eType)
    {
      case orxRESOURCE_REQUEST_TYPE_READ:
      {
        /* Services it */
        pstRequest->s64Size = pstRequest->pstResourceInfo->pstTypeInfo->pfnRead(pstRequest->pstResourceInfo->hResource, pstRequest->s64Size, pstRequest->pBuffer);

        break;
      }

      case orxRESOURCE_REQUEST_TYPE_WRITE:
      {
        /* Services it */
        pstRequest->s64Size = pstRequest->pstResourceInfo->pstTypeInfo->pfnWrite(pstRequest->pstResourceInfo->hResource, pstRequest->s64Size, pstRequest->pBuffer);

        break;
      }

      case orxRESOURCE_REQUEST_TYPE_CLOSE:
      {
        /* Services it */
        pstRequest->pstResourceInfo->pstTypeInfo->pfnClose(pstRequest->pstResourceInfo->hResource);

        /* Deletes location */
        orxString_Delete(pstRequest->pstResourceInfo->zLocation);

        /* Frees open info */
        orxBank_Free(sstResource.pstOpenInfoBank, pstRequest->pstResourceInfo);
        pstRequest->pstResourceInfo = orxNULL;

        break;
      }

      case orxRESOURCE_REQUEST_TYPE_GET_TIME:
      {
        orxRESOURCE_INFO *pstResourceInfo;

        /* Gets resource info */
        pstResourceInfo = (orxRESOURCE_INFO *)pstRequest->pContext;

        /* Gets its modification time (cheating for the storage) */
        pstRequest->s64Size = pstResourceInfo->pstTypeInfo->pfnGetTime(pstResourceInfo->zLocation + orxString_GetLength(pstResourceInfo->pstTypeInfo->zTag) + 1);

        /* Updates its status */
        pstResourceInfo->bPendingWatch = orxFALSE;

        break;
      }

      default:
      {
        break;
      }
    }

    /* Updates request process index */
    orxMEMORY_BARRIER();
    sstResource.u32RequestProcessIndex = (sstResource.u32RequestProcessIndex + 1) & (orxRESOURCE_KU32_REQUEST_LIST_SIZE - 1);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Updates result */
  eResult = sstResource.eThreadResult;

  /* Done! */
  return eResult;
}

static void orxResource_AddRequest(orxRESOURCE_REQUEST_TYPE _eType, orxS64 _s64Size, void *_pBuffer, orxRESOURCE_OP_FUNCTION _pfnCallback, void *_pContext, orxRESOURCE_OPEN_INFO *_pstResourceInfo)
{
  orxU32                        u32NextRequestIndex;
  orxBOOL                       bAdd = orxTRUE;

  /* Checks */
  orxASSERT(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID);

  /* Not shutting down */
  if(!orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_EXIT))
  {
    /* Waits for semaphore */
    orxThread_WaitSemaphore(sstResource.pstRequestSemaphore);

    /* Gets next request index */
    u32NextRequestIndex = (sstResource.u32RequestInIndex + 1) & (orxRESOURCE_KU32_REQUEST_LIST_SIZE - 1);

    /* Time request? */
    if(_eType == orxRESOURCE_REQUEST_TYPE_GET_TIME)
    {
      orxU32 u32InIndex, u32ProcessIndex, u32FreeSlots;

      /* Gets indices */
      u32InIndex      = sstResource.u32RequestInIndex;
      u32ProcessIndex = sstResource.u32RequestProcessIndex;

      /* Gets number of free slots */
      u32FreeSlots = (u32InIndex >= u32ProcessIndex) ? orxRESOURCE_KU32_REQUEST_LIST_SIZE - u32InIndex + u32ProcessIndex : u32ProcessIndex - u32InIndex;

      /* More than a quarter of the slots are free? */
      if(u32FreeSlots >= orxRESOURCE_KU32_REQUEST_LIST_SIZE / 4)
      {
        /* Process addition */
        bAdd = orxTRUE;
      }
      else
      {
        /* Drops request */
        bAdd = orxFALSE;
      }
    }
    else
    {
      /* Waits for a free slot */
      while(u32NextRequestIndex == sstResource.u32RequestOutIndex)
      {
        /* Main thread? */
        if(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID)
        {
          /* Manually pumps some request notifications */
          orxResource_NotifyRequest(orxNULL, orxNULL);
        }
      }

      /* Process addition */
      bAdd = orxTRUE;
    }

    /* Should add request? */
    if(bAdd != orxFALSE)
    {
      volatile orxRESOURCE_REQUEST *pstRequest;

      /* Gets current request */
      pstRequest = &(sstResource.astRequestList[sstResource.u32RequestInIndex]);

      /* Inits it */
      if(_pstResourceInfo != orxNULL)
      {
        _pstResourceInfo->u32OpCount++;
      }
      pstRequest->s64Size         = _s64Size;
      pstRequest->pBuffer         = _pBuffer;
      pstRequest->pfnCallback     = _pfnCallback;
      pstRequest->pContext        = _pContext;
      pstRequest->pstResourceInfo = _pstResourceInfo;
      pstRequest->eType           = _eType;

      /* Commits request */
      orxMEMORY_BARRIER();
      sstResource.u32RequestInIndex = u32NextRequestIndex;

      /* Signals worker semaphore */
      orxThread_SignalSemaphore(sstResource.pstWorkerSemaphore);
    }

    /* Signals semaphore */
    orxThread_SignalSemaphore(sstResource.pstRequestSemaphore);
  }
}

static void orxFASTCALL orxResource_NotifyUpdateChange(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  orxRESOURCE_INFO         *pstResourceInfo;
  orxRESOURCE_EVENT_PAYLOAD stPayload;

  /* Gets resource info */
  pstResourceInfo = (orxRESOURCE_INFO *)_pContext;

  /* Clears payload */
  orxMemory_Zero(&stPayload, sizeof(orxRESOURCE_EVENT_PAYLOAD));

  /* Inits payload */
  stPayload.s64Time     = pstResourceInfo->s64Time;
  stPayload.zLocation   = pstResourceInfo->zLocation;
  stPayload.pstTypeInfo = pstResourceInfo->pstTypeInfo;
  stPayload.stGroupID   = pstResourceInfo->stGroupID;
  stPayload.stStorageID = pstResourceInfo->stStorageID;
  stPayload.stNameID    = pstResourceInfo->stNameID;

  /* Sends event */
  orxEVENT_SEND(orxEVENT_TYPE_RESOURCE, orxRESOURCE_EVENT_UPDATE, orxNULL, orxNULL, &stPayload);
}

static void orxFASTCALL orxResource_NotifyChange(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, void *_pContext)
{
  orxRESOURCE_INFO *pstResourceInfo;

  /* Gets resource info */
  pstResourceInfo = (orxRESOURCE_INFO *)_pContext;

  /* Has been modified since last access (using cheat storage)? */
  if(_s64Size != pstResourceInfo->s64Time)
  {
    /* Not first inspection? */
    if(pstResourceInfo->s64Time != orxRESOURCE_KU32_WATCH_TIME_UNINITIALIZED)
    {
      /* Removed or added? */
      if((_s64Size == 0) || (pstResourceInfo->s64Time == 0))
      {
        orxRESOURCE_EVENT_PAYLOAD stPayload;

        /* Stores its new modification time */
        pstResourceInfo->s64Time = _s64Size;

        /* Clears payload */
        orxMemory_Zero(&stPayload, sizeof(orxRESOURCE_EVENT_PAYLOAD));

        /* Inits payload */
        stPayload.s64Time     = _s64Size;
        stPayload.zLocation   = pstResourceInfo->zLocation;
        stPayload.pstTypeInfo = pstResourceInfo->pstTypeInfo;
        stPayload.stGroupID   = pstResourceInfo->stGroupID;
        stPayload.stStorageID = pstResourceInfo->stStorageID;
        stPayload.stNameID    = pstResourceInfo->stNameID;

        /* Removed? */
        if(_s64Size == 0)
        {
          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_RESOURCE, orxRESOURCE_EVENT_REMOVE, orxNULL, orxNULL, &stPayload);
        }
        /* Added */
        else
        {
          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_RESOURCE, orxRESOURCE_EVENT_ADD, orxNULL, orxNULL, &stPayload);
        }
      }
      else
      {
        /* Stores its new modification time */
        pstResourceInfo->s64Time = _s64Size;

        /* Removes potential pending update notification */
        orxClock_RemoveGlobalTimer(orxResource_NotifyUpdateChange, orxRESOURCE_KF_WATCH_NOTIFICATION_DELAY, _pContext);

        /* Defers update notification to cope with potential slow external resource writes */
        orxClock_AddGlobalTimer(orxResource_NotifyUpdateChange, orxRESOURCE_KF_WATCH_NOTIFICATION_DELAY, 1, _pContext);
      }
    }
    else
    {
      /* Stores its new modification time */
      pstResourceInfo->s64Time = _s64Size;
    }
  }

  /* Done! */
  return;
}

static void orxFASTCALL orxResource_Watch(const orxCLOCK_INFO *_pstClockInfo, void *_pContext)
{
  static orxS32 ss32GroupIndex = 0;
  orxS32        s32ListCount;
  orxU32        u32WatchCount = 0;
  orxBOOL       bAbort = orxFALSE;

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Watch");

  /* Pushes config section */
  orxConfig_PushSection(orxRESOURCE_KZ_CONFIG_SECTION);

  /* For all watched groups */
  for(s32ListCount = orxConfig_GetListCount(orxRESOURCE_KZ_CONFIG_WATCH_LIST); ss32GroupIndex < s32ListCount; ss32GroupIndex++)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets its ID */
    stGroupID = orxString_Hash(orxConfig_GetListString(orxRESOURCE_KZ_CONFIG_WATCH_LIST, ss32GroupIndex));

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Found? */
    if(pstGroup != orxNULL)
    {
      static orxHANDLE  shIterator;
      orxRESOURCE_INFO *pstResourceInfo;

      /* New group? */
      if(stGroupID != sstResource.stLastWatchedGroupID)
      {
        /* Resets iterator */
        shIterator = orxHANDLE_UNDEFINED;

        /* Stores group ID */
        sstResource.stLastWatchedGroupID = stGroupID;
      }

      /* For all its cached resources */
      for(shIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, shIterator, orxNULL, (void **)&pstResourceInfo);
          shIterator != orxHANDLE_UNDEFINED;
          shIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, shIterator, orxNULL, (void **)&pstResourceInfo))
      {
        /* Does its type support time? */
        if(pstResourceInfo->pstTypeInfo->pfnGetTime != orxNULL)
        {
          /* Not already queued? */
          if(pstResourceInfo->bPendingWatch == orxFALSE)
          {
            /* Updates its status */
            pstResourceInfo->bPendingWatch = orxTRUE;

            /* Adds request */
            orxResource_AddRequest(orxRESOURCE_REQUEST_TYPE_GET_TIME, 0, orxNULL, &orxResource_NotifyChange, pstResourceInfo, orxNULL);
          }

          /* Updates watch count */
          u32WatchCount++;

          /* Reached limit? */
          if(u32WatchCount >= orxRESOURCE_KU32_WATCH_ITERATION_LIMIT)
          {
            /* Aborts */
            bAbort = orxTRUE;

            break;
          }
        }
      }

      /* Abort? */
      if(bAbort != orxFALSE)
      {
        /* Stops */
        break;
      }
    }
  }

  /* Watched all resources? */
  if(ss32GroupIndex >= s32ListCount)
  {
    /* Reset */
    ss32GroupIndex = 0;
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return;
}

static void orxResource_UpdatePostInit()
{
  /* Isn't request notification callback set? */
  if(!orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_NOTIFY_SET))
  {
    /* Is clock module initialized? */
    if(orxModule_IsInitialized(orxMODULE_ID_CLOCK) != orxFALSE)
    {
      orxSTATUS eResult;

      /* Registers request notification callback */
      eResult = orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxResource_NotifyRequest, orxNULL, orxMODULE_ID_RESOURCE, orxCLOCK_PRIORITY_LOWEST);

      /* Checks */
      orxASSERT(eResult != orxSTATUS_FAILURE);

      /* Updates flags */
      orxFLAG_SET(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_NOTIFY_SET, orxRESOURCE_KU32_STATIC_FLAG_NONE);
    }
  }

  /* Isn't config already loaded? */
  if(!orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED))
  {
    /* Reloads storage */
    orxResource_ReloadStorage();
  }

  /* Is config loaded now? */
  if(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED))
  {
    /* Watch not already set? */
    if(!orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_WATCH_SET))
    {
      /* Is clock module initialized? */
      if(orxModule_IsInitialized(orxMODULE_ID_CLOCK) != orxFALSE)
      {
        orxBOOL bDebugLevelBackup;

        /* Enables config logs */
        bDebugLevelBackup = orxDEBUG_IS_LEVEL_ENABLED(orxDEBUG_LEVEL_CONFIG);
        orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, orxTRUE);

        /* Pushes resource config section */
        orxConfig_PushSection(orxRESOURCE_KZ_CONFIG_SECTION);

        /* Has watch list? */
        if(orxConfig_HasValue(orxRESOURCE_KZ_CONFIG_WATCH_LIST) != orxFALSE)
        {
          /* Registers watch callbacks */
          orxClock_Register(orxClock_Get(orxCLOCK_KZ_CORE), orxResource_Watch, orxNULL, orxMODULE_ID_RESOURCE, orxCLOCK_PRIORITY_LOWEST);

          /* Updates flags */
          orxFLAG_SET(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_WATCH_REGISTERED, orxRESOURCE_KU32_STATIC_FLAG_NONE);
        }

        /* Pops config section */
        orxConfig_PopSection();

        /* Restores config logs */
        orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, bDebugLevelBackup);

        /* Updates flags */
        orxFLAG_SET(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_WATCH_SET, orxRESOURCE_KU32_STATIC_FLAG_NONE);
      }
    }
  }
}

/** Load storage callback
 */
static orxBOOL orxFASTCALL orxResource_LoadStorage(const orxSTRING _zKeyName, const orxSTRING _zSectionName, void *_pContext)
{
  const orxSTRING   zGroup;
  orxSTRINGID       stGroupID;
  orxSTATUS        *peResult;
  orxBOOL           bResult = orxTRUE;

  /* Gets status */
  peResult = (orxSTATUS *)_pContext;

  /* Gets group */
  zGroup = _zKeyName;

  /* Gets group ID */
  stGroupID = orxString_Hash(zGroup);

  /* Is not watch list? */
  if(stGroupID != orxString_Hash(orxRESOURCE_KZ_CONFIG_WATCH_LIST))
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxS32              i, iCount;

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* For all storages in list */
    for(i = 0, iCount = orxConfig_GetListCount(zGroup); i < iCount; i++)
    {
      const orxSTRING zStorage;
      orxBOOL         bAdd = orxTRUE;

      /* Gets storage */
      zStorage = orxConfig_GetListString(zGroup, i);

      /* Did the group exist? */
      if(pstGroup != orxNULL)
      {
        orxRESOURCE_STORAGE  *pstStorage;
        orxSTRINGID           stStorageID;

        /* Gets storage ID */
        stStorageID = orxString_Hash(zStorage);

        /* For all storages in group */
        for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
            pstStorage != orxNULL;
            pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)))
        {
          /* Found? */
          if(pstStorage->stID == stStorageID)
          {
            /* Don't add it */
            bAdd = orxFALSE;

            break;
          }
        }
      }

      /* Should add storage? */
      if(bAdd != orxFALSE)
      {
        /* Adds storage to group */
        if(orxResource_AddStorage(zGroup, zStorage, orxFALSE) == orxSTATUS_FAILURE)
        {
          /* Updates result */
          *peResult = orxSTATUS_FAILURE;
        }
      }
    }
  }

  /* Updates status */
  orxFLAG_SET(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED, orxRESOURCE_KU32_STATIC_FLAG_NONE);

  /* Done! */
  return bResult;
}


/** Command: AddStorage
 */
void orxFASTCALL orxResource_CommandAddStorage(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxResource_AddStorage(_astArgList[0].zValue, _astArgList[1].zValue, ((_u32ArgNumber < 3) || (_astArgList[2].bValue == orxFALSE)) ? orxFALSE : orxTRUE) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: RemoveStorage
 */
void orxFASTCALL orxResource_CommandRemoveStorage(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxResource_RemoveStorage((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxNULL, (_u32ArgNumber > 1) ? _astArgList[1].zValue : orxNULL) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: ReloadStorage
 */
void orxFASTCALL orxResource_CommandReloadStorage(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxResource_ReloadStorage() != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: Locate
 */
void orxFASTCALL orxResource_CommandLocate(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Has given storage? */
  if(_u32ArgNumber > 2)
  {
    /* Updates result */
    _pstResult->zValue = orxResource_LocateInStorage(_astArgList[0].zValue, _astArgList[2].zValue, _astArgList[1].zValue);
  }
  else
  {
    /* Updates result */
    _pstResult->zValue = orxResource_Locate(_astArgList[0].zValue, _astArgList[1].zValue);
  }

  /* Not found? */
  if(_pstResult->zValue == orxNULL)
  {
    /* Updates result */
    _pstResult->zValue = orxSTRING_EMPTY;
  }

  /* Done! */
  return;
}

/** Command: GetPath
 */
void orxFASTCALL orxResource_CommandGetPath(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->zValue = orxResource_GetPath(_astArgList[0].zValue);

  /* Done! */
  return;
}

/** Command: GetTotalPendingOpCount
 */
void orxFASTCALL orxResource_CommandGetTotalPendingOpCount(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->u32Value = orxResource_GetTotalPendingOpCount();

  /* Done! */
  return;
}

/** Command: Sync
 */
void orxFASTCALL orxResource_CommandSync(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxResource_Sync((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxNULL) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Command: ClearCache
 */
void orxFASTCALL orxResource_CommandClearCache(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  /* Updates result */
  _pstResult->bValue = (orxResource_ClearCache((_u32ArgNumber > 0) ? _astArgList[0].zValue : orxNULL) != orxSTATUS_FAILURE) ? orxTRUE : orxFALSE;

  /* Done! */
  return;
}

/** Registers all the resource commands
 */
static orxINLINE void orxResource_RegisterCommands()
{
  /* Command: AddStorage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, AddStorage, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 2, 1, {"Group", orxCOMMAND_VAR_TYPE_STRING}, {"Storage", orxCOMMAND_VAR_TYPE_STRING}, {"First = false", orxCOMMAND_VAR_TYPE_BOOL});
  /* Command: RemoveStorage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, RemoveStorage, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 2, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING}, {"Storage = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: ReloadStorage */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, ReloadStorage, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 0);

  /* Command: Locate */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, Locate, "Location", orxCOMMAND_VAR_TYPE_STRING, 2, 1, {"Group", orxCOMMAND_VAR_TYPE_STRING}, {"Name", orxCOMMAND_VAR_TYPE_STRING}, {"Storage = <all>", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetPath */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, GetPath, "Path", orxCOMMAND_VAR_TYPE_STRING, 1, 0, {"Location", orxCOMMAND_VAR_TYPE_STRING});

  /* Command: GetTotalPendingOpCount */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, GetTotalPendingOpCount, "Count", orxCOMMAND_VAR_TYPE_U32, 0, 0);

  /* Command: Sync */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, Sync, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 1, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING});
  /* Command: ClearCache */
  orxCOMMAND_REGISTER_CORE_COMMAND(Resource, ClearCache, "Success?", orxCOMMAND_VAR_TYPE_BOOL, 0, 1, {"Group = <void>", orxCOMMAND_VAR_TYPE_STRING});
}

/** Unregisters all the resource commands
 */
static orxINLINE void orxResource_UnregisterCommands()
{
  /* Command: AddStorage */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, AddStorage);
  /* Command: RemoveStorage */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, RemoveStorage);
  /* Command: ReloadStorage */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, ReloadStorage);

  /* Command: Locate */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, Locate);

  /* Command: GetPath */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, GetPath);

  /* Command: GetTotalPendingOpCount */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, GetTotalPendingOpCount);

  /* Command: Sync */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, Sync);
  /* Command: ClearCache */
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Resource, ClearCache);
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Resource module setup
 */
void orxFASTCALL orxResource_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_COMMAND);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_FILE);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_PROFILER);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_SYSTEM);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_THREAD);

  /* Done! */
  return;
}

/** Inits resource module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxMath_IsPowerOfTwo(orxRESOURCE_KU32_REQUEST_LIST_SIZE) != orxFALSE);

  /* Not already Initialized? */
  if(!(sstResource.u32Flags & orxRESOURCE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstResource, sizeof(orxRESOURCE_STATIC));

    /* Creates semaphores */
    sstResource.pstRequestSemaphore = orxThread_CreateSemaphore(1);
    sstResource.pstWorkerSemaphore  = orxThread_CreateSemaphore(1);

    /* Valid? */
    if((sstResource.pstRequestSemaphore != orxNULL) && (sstResource.pstWorkerSemaphore != orxNULL))
    {
      /* Inits request thread ID */
      sstResource.u32RequestThreadID = orxU32_UNDEFINED;

      /* Creates resource info bank */
      sstResource.pstResourceInfoBank = orxBank_Create(orxRESOURCE_KU32_RESOURCE_INFO_BANK_SIZE, sizeof(orxRESOURCE_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Creates open resource info bank */
      sstResource.pstOpenInfoBank     = orxBank_Create(orxRESOURCE_KU32_OPEN_INFO_BANK_SIZE, sizeof(orxRESOURCE_OPEN_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Creates group bank */
      sstResource.pstGroupBank        = orxBank_Create(orxRESOURCE_KU32_GROUP_BANK_SIZE, sizeof(orxRESOURCE_GROUP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Creates type info bank */
      sstResource.pstTypeBank         = orxBank_Create(orxRESOURCE_KU32_TYPE_BANK_SIZE, sizeof(orxRESOURCE_TYPE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Creates memory table & banks */
      sstResource.pstMemoryDataTable    = orxHashTable_Create(orxRESOURCE_KU32_MEMORY_BANK_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstResource.pstMemoryDataBank     = orxBank_Create(orxRESOURCE_KU32_MEMORY_BANK_SIZE, sizeof(orxRESOURCE_MEMORY_DATA), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      sstResource.pstMemoryResourceBank = orxBank_Create(orxRESOURCE_KU32_MEMORY_BANK_SIZE, sizeof(orxRESOURCE_MEMORY_RESOURCE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      /* Success? */
      if((sstResource.pstResourceInfoBank != orxNULL) && (sstResource.pstOpenInfoBank != orxNULL) && (sstResource.pstGroupBank != orxNULL) && (sstResource.pstTypeBank != orxNULL)
      && (sstResource.pstMemoryDataTable != orxNULL) && (sstResource.pstMemoryDataBank != orxNULL) && (sstResource.pstMemoryResourceBank != orxNULL))
      {
        orxRESOURCE_TYPE_INFO stTypeInfo;

        /* Inits flags */
        sstResource.u32Flags = orxRESOURCE_KU32_STATIC_FLAG_READY;

        /* Inits file type */
        orxMemory_Zero(&stTypeInfo, sizeof(orxRESOURCE_TYPE_INFO));
        stTypeInfo.zTag       = orxRESOURCE_KZ_TYPE_TAG_FILE;
        stTypeInfo.pfnLocate  = orxResource_File_Locate;
        stTypeInfo.pfnGetTime = orxResource_File_GetTime;
        stTypeInfo.pfnOpen    = orxResource_File_Open;
        stTypeInfo.pfnClose   = orxResource_File_Close;
        stTypeInfo.pfnGetSize = orxResource_File_GetSize;
        stTypeInfo.pfnSeek    = orxResource_File_Seek;
        stTypeInfo.pfnTell    = orxResource_File_Tell;
        stTypeInfo.pfnRead    = orxResource_File_Read;
        stTypeInfo.pfnWrite   = orxResource_File_Write;
        stTypeInfo.pfnDelete  = orxResource_File_Delete;

        /* Registers it */
        eResult = orxResource_RegisterType(&stTypeInfo);

        /* Success? */
        if(eResult != orxSTATUS_FAILURE)
        {
          /* Inits memory type */
          orxMemory_Zero(&stTypeInfo, sizeof(orxRESOURCE_TYPE_INFO));
          stTypeInfo.zTag       = orxRESOURCE_KZ_TYPE_TAG_MEMORY;
          stTypeInfo.pfnLocate  = orxResource_Memory_Locate;
          stTypeInfo.pfnGetTime = orxResource_Memory_GetTime;
          stTypeInfo.pfnOpen    = orxResource_Memory_Open;
          stTypeInfo.pfnClose   = orxResource_Memory_Close;
          stTypeInfo.pfnGetSize = orxResource_Memory_GetSize;
          stTypeInfo.pfnSeek    = orxResource_Memory_Seek;
          stTypeInfo.pfnTell    = orxResource_Memory_Tell;
          stTypeInfo.pfnRead    = orxResource_Memory_Read;
          stTypeInfo.pfnWrite   = orxNULL;
          stTypeInfo.pfnDelete  = orxNULL;

          /* Registers it */
          eResult = orxResource_RegisterType(&stTypeInfo);

          /* Success? */
          if(eResult != orxSTATUS_FAILURE)
          {
            /* Inits thread result */
            sstResource.eThreadResult = orxSTATUS_SUCCESS;

            /* Waits for worker semaphore */
            orxThread_WaitSemaphore(sstResource.pstWorkerSemaphore);

            /* Starts request processing thread */
            sstResource.u32RequestThreadID = orxThread_Start(&orxResource_ProcessRequests, orxRESOURCE_KZ_THREAD_NAME, orxNULL);

            /* Success? */
            if(sstResource.u32RequestThreadID != orxU32_UNDEFINED)
            {
              /* Registers commands */
              orxResource_RegisterCommands();

              /* Inits vars */
              sstResource.stLastWatchedGroupID = orxSTRINGID_UNDEFINED;

#ifdef __orxANDROID__

              /* Registers APK type */
              eResult = orxAndroid_RegisterAPKResource();

#endif /* __orxANDROID__ */
            }
          }
        }
      }
    }

    /* Failed? */
    if(eResult != orxSTATUS_SUCCESS)
    {
      /* Unregisters internal types */
      orxResource_UnregisterType(orxRESOURCE_KZ_TYPE_TAG_FILE);
      orxResource_UnregisterType(orxRESOURCE_KZ_TYPE_TAG_MEMORY);

      /* Removes Flags */
      sstResource.u32Flags &= ~orxRESOURCE_KU32_STATIC_FLAG_READY;

      /* Deletes semaphores */
      if(sstResource.pstRequestSemaphore != orxNULL)
      {
        orxThread_DeleteSemaphore(sstResource.pstRequestSemaphore);
      }
      if(sstResource.pstWorkerSemaphore != orxNULL)
      {
        orxThread_DeleteSemaphore(sstResource.pstWorkerSemaphore);
      }

      /* Deletes info bank */
      if(sstResource.pstResourceInfoBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstResourceInfoBank);
      }

      /* Deletes open info bank */
      if(sstResource.pstOpenInfoBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstOpenInfoBank);
      }

      /* Deletes group bank */
      if(sstResource.pstGroupBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstGroupBank);
      }

      /* Deletes type bank */
      if(sstResource.pstTypeBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstTypeBank);
      }

      /* Deletes memory table & banks */
      if(sstResource.pstMemoryDataTable != orxNULL)
      {
        orxHashTable_Delete(sstResource.pstMemoryDataTable);
      }
      if(sstResource.pstMemoryDataBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstMemoryDataBank);
      }
      if(sstResource.pstMemoryResourceBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstMemoryResourceBank);
      }

      /* Has request thread? */
      if(sstResource.u32RequestThreadID != orxU32_UNDEFINED)
      {
        /* Joins it */
        orxThread_Join(sstResource.u32RequestThreadID);
        sstResource.u32RequestThreadID = orxU32_UNDEFINED;
      }

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Couldn't init resource module: can't allocate internal banks.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize resource module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from resource module
 */
void orxFASTCALL orxResource_Exit()
{
  /* Initialized? */
  if(sstResource.u32Flags & orxRESOURCE_KU32_STATIC_FLAG_READY)
  {
    orxRESOURCE_GROUP      *pstGroup;
    orxRESOURCE_OPEN_INFO  *pstOpenInfo;

    /* Unregisters commands */
    orxResource_UnregisterCommands();

    /* Updates status */
    orxFLAG_SET(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_EXIT, orxRESOURCE_KU32_STATIC_FLAG_NONE);

    /* Makes sure resource thread is enabled */
    orxThread_Enable(orxTHREAD_GET_FLAG_FROM_ID(sstResource.u32RequestThreadID), orxTHREAD_KU32_FLAG_NONE);

    /* Waits for all pending operations to complete */
    while(sstResource.u32RequestProcessIndex != sstResource.u32RequestInIndex);

    /* Updates worker result */
    sstResource.eThreadResult = orxSTATUS_FAILURE;
    orxMEMORY_BARRIER();

    /* Signals worker semaphore */
    orxThread_SignalSemaphore(sstResource.pstWorkerSemaphore);

    /* Joins request thread */
    orxThread_Join(sstResource.u32RequestThreadID);
    sstResource.u32RequestThreadID = orxU32_UNDEFINED;

    /* Delete semaphores */
    orxThread_DeleteSemaphore(sstResource.pstRequestSemaphore);
    orxThread_DeleteSemaphore(sstResource.pstWorkerSemaphore);

    /* Is the clock module still present? */
    if(orxModule_IsInitialized(orxMODULE_ID_CLOCK) != orxFALSE)
    {
      /* Unregisters request notification callback */
      orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxResource_NotifyRequest);

      /* Has watch callback? */
      if(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_WATCH_REGISTERED))
      {
        /* Registers watch callbacks */
        orxClock_Unregister(orxClock_Get(orxCLOCK_KZ_CORE), orxResource_Watch);
      }
    }

    /* Has uncached location? */
    if(sstResource.zLastUncachedLocation != orxNULL)
    {
      /* Deletes it */
      orxMemory_Free(sstResource.zLastUncachedLocation);
    }

    /* For all groups */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        pstGroup != orxNULL;
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL))
    {
      /* Deletes it */
      orxResource_DeleteGroup(pstGroup);
    }

    /* Deletes group bank */
    orxBank_Delete(sstResource.pstGroupBank);

    /* Deletes type bank */
    orxBank_Delete(sstResource.pstTypeBank);

    /* For all open resources */
    while((pstOpenInfo = (orxRESOURCE_OPEN_INFO *)orxBank_GetNext(sstResource.pstOpenInfoBank, orxNULL)) != orxNULL)
    {
      /* Closes it */
      orxResource_Close((orxHANDLE)pstOpenInfo);
    }

    /* Deletes open info bank */
    orxBank_Delete(sstResource.pstOpenInfoBank);

    /* Checks */
    orxASSERT(orxBank_GetCount(sstResource.pstResourceInfoBank) == 0);

    /* Deletes info bank */
    orxBank_Delete(sstResource.pstResourceInfoBank);

    /* Deletes memory table & banks */
    orxHashTable_Delete(sstResource.pstMemoryDataTable);
    orxBank_Delete(sstResource.pstMemoryDataBank);
    orxBank_Delete(sstResource.pstMemoryResourceBank);

    /* Updates flags */
    sstResource.u32Flags &= ~orxRESOURCE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return;
}

/** Gets number of resource groups
 * @return Number of resource groups
 */
orxU32 orxFASTCALL orxResource_GetGroupCount()
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Updates result */
  u32Result = orxBank_GetCount(sstResource.pstGroupBank);

  /* Done! */
  return u32Result;
}

/** Gets resource group at given index
 * @param[in] _u32Index         Index of resource group
 * @return Resource group if index is valid, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxResource_GetGroup(orxU32 _u32Index)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Valid index? */
  if(_u32Index < orxBank_GetCount(sstResource.pstGroupBank))
  {
    orxRESOURCE_GROUP *pstGroup;

    /* Finds requested group */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        _u32Index > 0;
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup), _u32Index--)
    ;

    /* Checks */
    orxASSERT(pstGroup != orxNULL);

    /* Updates result */
    zResult = orxString_GetFromID(pstGroup->stID);
  }

  /* Done! */
  return zResult;
}

/** Adds a storage for a given resource group, this storage will be used when looking for resources prior to any previously added storage
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zStorage         Description of the storage, as understood by one of the resource type
 * @param[in] _bAddFirst        If true this storage will be used *before* any already added ones, otherwise it'll be used *after* all those
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_AddStorage(const orxSTRING _zGroup, const orxSTRING _zStorage, orxBOOL _bAddFirst)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);
  orxASSERT(_zStorage != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_GetID(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Not found? */
    if(pstGroup == orxNULL)
    {
      /* Creates it */
      pstGroup = orxResource_CreateGroup(stGroupID);
    }

    /* Success? */
    if(pstGroup != orxNULL)
    {
      orxRESOURCE_STORAGE *pstStorage;

      /* Creates storage */
      pstStorage = (orxRESOURCE_STORAGE *)orxBank_Allocate(pstGroup->pstStorageBank);

      /* Success? */
      if(pstStorage != orxNULL)
      {
        /* Stores its content */
        pstStorage->stID = orxString_GetID(_zStorage);

        /* Clears its node */
        orxMemory_Zero(&(pstStorage->stNode), sizeof(orxLINKLIST_NODE));

        /* Should be added first? */
        if(_bAddFirst != orxFALSE)
        {
          /* Adds it first */
          orxLinkList_AddStart(&(pstGroup->stStorageList), &(pstStorage->stNode));
        }
        else
        {
          orxRESOURCE_STORAGE *pstLastStorage;

          /* Gets last storage */
          pstLastStorage = (orxLinkList_GetCount(&(pstGroup->stStorageList)) != 0) ? (orxRESOURCE_STORAGE *)orxLinkList_GetLast(&(pstGroup->stStorageList)) : orxNULL;

          /* Found and is default storage? */
          if((pstLastStorage != orxNULL) && (pstLastStorage->stID == orxString_Hash(orxRESOURCE_KZ_DEFAULT_STORAGE)))
          {
            /* Adds it before it */
            orxLinkList_AddBefore(&(pstLastStorage->stNode), &(pstStorage->stNode));
          }
          else
          {
          /* Adds it last */
          orxLinkList_AddEnd(&(pstGroup->stStorageList), &(pstStorage->stNode));
        }
        }

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't add storage <%s> to resource group <%s>: unable to allocate memory for storage.", _zStorage, _zGroup);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't add storage <%s> to resource group <%s>: unable to allocate memory for group.", _zStorage, _zGroup);
    }
  }

  /* Done! */
  return eResult;
}

/** Removes storage(s) for specific resource group(s)
 * @param[in] _zGroup           Concerned resource group, orxNULL for all groups
 * @param[in] _zStorage         Concerned storage, orxNULL for all storages (except default one)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_RemoveStorage(const orxSTRING _zGroup, const orxSTRING _zStorage)
{
  orxRESOURCE_GROUP  *pstGroup;
  orxSTRINGID         stGroupID, stStorageID;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Gets group & storage IDs */
  stGroupID   = (_zGroup != orxNULL) ? orxString_Hash(_zGroup) : orxSTRINGID_UNDEFINED;
  stStorageID = (_zStorage != orxNULL) ? orxString_Hash(_zStorage) : orxSTRINGID_UNDEFINED;

  /* For all groups */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
      pstGroup != orxNULL;
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup))
    {
    /* Matches? */
    if((_zGroup == orxNULL)
    || (pstGroup->stID == stGroupID))
    {
      orxRESOURCE_STORAGE *pstStorage, *pstNextStorage;

      /* For all storages in group */
      for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
          pstStorage != orxNULL;
          pstStorage = pstNextStorage)
      {
        /* Gets next storage */
        pstNextStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode));

        /* Matches? */
        if((_zStorage == orxNULL)
        || (pstStorage->stID == stStorageID))
        {
          /* Removes it from list */
          orxLinkList_Remove(&(pstStorage->stNode));

          /* Frees it */
          orxBank_Free(pstGroup->pstStorageBank, pstStorage);

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Gets number of storages for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @return Number of storages for this resource group
 */
orxU32 orxFASTCALL orxResource_GetStorageCount(const orxSTRING _zGroup)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_Hash(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Success? */
    if(pstGroup != orxNULL)
    {
      /* Updates result */
      u32Result = orxLinkList_GetCount(&(pstGroup->stStorageList));
    }
  }

  /* Done! */
  return u32Result;
}

/** Gets storage at given index for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _u32Index         Index of storage
 * @return Storage if index is valid, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxResource_GetStorage(const orxSTRING _zGroup, orxU32 _u32Index)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_Hash(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Success? */
    if(pstGroup != orxNULL)
    {
      /* Valid index? */
      if(_u32Index < orxBank_GetCount(pstGroup->pstStorageBank))
      {
        orxRESOURCE_STORAGE *pstStorage;

        /* Finds requested storage */
        for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
            _u32Index > 0;
            pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)), _u32Index--)
        ;

        /* Checks */
        orxASSERT(pstStorage != orxNULL);

        /* Updates result */
        zResult = orxString_GetFromID(pstStorage->stID);
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Reloads storage from config
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_ReloadStorage()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Pushes resource config section */
  orxConfig_PushSection(orxRESOURCE_KZ_CONFIG_SECTION);

  /* Loads all storages */
  orxConfig_ForAllKeys(orxResource_LoadStorage, orxTRUE, &eResult);

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return eResult;
}

/** Gets the location of an *existing* resource for a given group, location gets cached if found
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zName            Name of the resource to locate
 * @return Location string if found, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxResource_Locate(const orxSTRING _zGroup, const orxSTRING _zName)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);
  orxASSERT(_zName != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Locate");

  /* Updates post-init */
  orxResource_UpdatePostInit();

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_GetID(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Not found? */
    if(pstGroup == orxNULL)
    {
      /* Creates it */
      pstGroup = orxResource_CreateGroup(stGroupID);
    }

    /* Success? */
    if(pstGroup != orxNULL)
    {
      orxSTRINGID       stKey;
      orxRESOURCE_INFO *pstResourceInfo;

      /* Gets resource info key */
      stKey = orxString_GetID(_zName);

      /* Gets resource info from cache */
      pstResourceInfo = (orxRESOURCE_INFO *)orxHashTable_Get(pstGroup->pstCacheTable, stKey);

      /* Found? */
      if(pstResourceInfo != orxNULL)
      {
        /* Updates result */
        zResult = pstResourceInfo->zLocation;
      }
      else
      {
        orxRESOURCE_STORAGE *pstStorage;

        /* For all storages in group */
        for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
            (zResult == orxNULL) && (pstStorage != orxNULL);
            pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)))
        {
          orxRESOURCE_TYPE *pstType;

          /* For all registered types */
          for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
              pstType != orxNULL;
              pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
          {
            const orxSTRING zLocation;

            /* Locates resource */
            zLocation = pstType->stInfo.pfnLocate(_zGroup, orxString_GetFromID(pstStorage->stID), _zName, orxTRUE);

            /* Success? */
            if(zLocation != orxNULL)
            {
              orxRESOURCE_INFO *pstResourceInfo;
              orxS32            s32Size;

              /* Allocates resource info */
              pstResourceInfo = (orxRESOURCE_INFO *)orxBank_Allocate(sstResource.pstResourceInfoBank);

              /* Checks */
              orxASSERT(pstResourceInfo != orxNULL);

              /* Inits it */
              pstResourceInfo->pstTypeInfo  = &(pstType->stInfo);
              pstResourceInfo->s64Time      = orxRESOURCE_KU32_WATCH_TIME_UNINITIALIZED;
              s32Size                       = orxString_GetLength(pstType->stInfo.zTag) + orxString_GetLength(zLocation) + 2;
              pstResourceInfo->zLocation    = (orxSTRING)orxMemory_Allocate(s32Size, orxMEMORY_TYPE_TEXT);
              orxASSERT(pstResourceInfo->zLocation != orxNULL);
              orxString_NPrint(pstResourceInfo->zLocation, s32Size, "%s%c%s", pstType->stInfo.zTag, orxRESOURCE_KC_LOCATION_SEPARATOR, zLocation);
              pstResourceInfo->stGroupID    = stGroupID;
              pstResourceInfo->stStorageID  = pstStorage->stID;
              pstResourceInfo->stNameID     = stKey;
              orxMEMORY_BARRIER();

              /* Adds it to cache */
              orxHashTable_Add(pstGroup->pstCacheTable, stKey, pstResourceInfo);

              /* Updates result */
              zResult = pstResourceInfo->zLocation;

              break;
            }
          }
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return zResult;
}

/** Gets the location for a resource (existing or not) in a *specific storage*, for a given group. The location doesn't get cached and thus needs to be copied by the caller before the next call
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zStorage         Concerned storage, if orxNULL then the highest priority storage will be used
 * @param[in] _zName            Name of the resource
 * @return Location string if found, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxResource_LocateInStorage(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);
  orxASSERT(_zName != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_LocateInStorage");

  /* Updates post-init */
  orxResource_UpdatePostInit();

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_GetID(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Not found? */
    if(pstGroup == orxNULL)
    {
      /* Creates it */
      pstGroup = orxResource_CreateGroup(stGroupID);
    }

    /* Success? */
    if(pstGroup != orxNULL)
    {
      orxRESOURCE_STORAGE  *pstStorage;
      orxSTRINGID           stStorageID;

      /* Gets storage ID */
      stStorageID = (_zStorage != orxNULL) ? orxString_Hash(_zStorage) : orxSTRINGID_UNDEFINED;

      /* For all storages in group */
      for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
          (zResult == orxNULL) && (pstStorage != orxNULL);
          pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)))
      {
        /* Is the requested storage? */
        if((_zStorage == orxNULL)
        || (pstStorage->stID == stStorageID))
        {
          orxRESOURCE_TYPE *pstType;
          const orxSTRING   zStorage = orxString_GetFromID(pstStorage->stID);

          /* For all registered types */
          for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
              pstType != orxNULL;
              pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
          {
            const orxSTRING zLocation;

            /* Locates resource */
            zLocation = pstType->stInfo.pfnLocate(_zGroup, zStorage, _zName, orxFALSE);

            /* Success? */
            if(zLocation != orxNULL)
            {
              orxS32 s32Size;

              /* Has previous uncached location? */
              if(sstResource.zLastUncachedLocation != orxNULL)
              {
                /* Deletes it */
                orxMemory_Free(sstResource.zLastUncachedLocation);
              }

              /* Creates new location */
              s32Size = orxString_GetLength(pstType->stInfo.zTag) + orxString_GetLength(zLocation) + 2;
              sstResource.zLastUncachedLocation = (orxSTRING)orxMemory_Allocate(s32Size, orxMEMORY_TYPE_TEXT);
              orxASSERT(sstResource.zLastUncachedLocation != orxNULL);
              orxString_NPrint(sstResource.zLastUncachedLocation, s32Size, "%s%c%s", pstType->stInfo.zTag, orxRESOURCE_KC_LOCATION_SEPARATOR, zLocation);

              /* Updates result */
              zResult = sstResource.zLastUncachedLocation;

              break;
            }
          }

          break;
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return zResult;
}

/** Gets the resource path from a location
 * @param[in] _zLocation        Location of the concerned resource
 * @return Path string if valid, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxResource_GetPath(const orxSTRING _zLocation)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE *pstType;

    /* For all registered types */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        pstType != orxNULL;
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    {
      orxU32 u32TagLength;

      /* Gets tag length */
      u32TagLength = orxString_GetLength(pstType->stInfo.zTag);

      /* Match tag? */
      if(orxString_NICompare(_zLocation, pstType->stInfo.zTag, u32TagLength) == 0)
      {
        /* Valid? */
        if(*(_zLocation + u32TagLength) == orxRESOURCE_KC_LOCATION_SEPARATOR)
        {
          /* Updates result */
          zResult = _zLocation + u32TagLength + 1;
        }

        break;
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Gets the resource type from a location
 * @param[in] _zLocation        Location of the concerned resource
 * @return orxRESOURCE_TYPE_INFO if valid, orxNULL otherwise
 */
const orxRESOURCE_TYPE_INFO *orxFASTCALL orxResource_GetType(const orxSTRING _zLocation)
{
  const orxRESOURCE_TYPE_INFO *pstResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE *pstType;

    /* For all registered types */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        pstType != orxNULL;
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    {
      orxU32 u32TagLength;

      /* Gets tag length */
      u32TagLength = orxString_GetLength(pstType->stInfo.zTag);

      /* Match tag? */
      if(orxString_NICompare(_zLocation, pstType->stInfo.zTag, u32TagLength) == 0)
      {
        /* Valid? */
        if(*(_zLocation + u32TagLength) == orxRESOURCE_KC_LOCATION_SEPARATOR)
        {
          /* Updates result */
          pstResult = &(pstType->stInfo);
        }

        break;
      }
    }
  }

  /* Done! */
  return pstResult;
}

/** Gets the time of last modification of a resource
 * @param[in] _zLocation        Location of the concerned resource
 * @return Time of last modification, in seconds since epoch, if found, 0 otherwise
 */
orxS64 orxFASTCALL orxResource_GetTime(const orxSTRING _zLocation)
{
  orxS64 s64Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_GetTime");

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE *pstType;
    orxU32            u32TagLength;

    /* For all registered types */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        pstType != orxNULL;
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    {
      /* Gets tag length */
      u32TagLength = orxString_GetLength(pstType->stInfo.zTag);

      /* Match tag? */
      if(orxString_NICompare(_zLocation, pstType->stInfo.zTag, u32TagLength) == 0)
      {
        /* Selects it */
        break;
      }
    }

    /* Found? */
    if(pstType != orxNULL)
    {
      /* Supports time? */
      if(pstType->stInfo.pfnGetTime != orxNULL)
      {
        /* Updates result */
        s64Result = pstType->stInfo.pfnGetTime(_zLocation + u32TagLength + 1);
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return s64Result;
}

/** Opens the resource at the given location
 * @param[in] _zLocation        Location of the resource to open
 * @param[in] _bEraseMode       If true, the resource will be erased if existing or created otherwise, if false, no content will get destroyed when opening
 * @return Handle to the open location, orxHANDLE_UNDEFINED otherwise
 */
orxHANDLE orxFASTCALL orxResource_Open(const orxSTRING _zLocation, orxBOOL _bEraseMode)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Open");

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE *pstType;
    orxU32            u32TagLength;

    /* For all registered types */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        pstType != orxNULL;
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    {
      /* Gets tag length */
      u32TagLength = orxString_GetLength(pstType->stInfo.zTag);

      /* Match tag? */
      if((orxString_NICompare(_zLocation, pstType->stInfo.zTag, u32TagLength) == 0)
      && (*(_zLocation + u32TagLength) == orxRESOURCE_KC_LOCATION_SEPARATOR))
      {
        /* Selects it */
        break;
      }
    }

    /* Found? */
    if(pstType != orxNULL)
    {
      orxRESOURCE_OPEN_INFO *pstOpenInfo;

      /* Allocates open info */
      pstOpenInfo = (orxRESOURCE_OPEN_INFO *)orxBank_Allocate(sstResource.pstOpenInfoBank);

      /* Checks */
      orxASSERT(pstOpenInfo != orxNULL);

      /* Inits it */
      pstOpenInfo->pstTypeInfo  = &(pstType->stInfo);
      pstOpenInfo->u32OpCount   = 0;

      /* Opens it */
      pstOpenInfo->hResource = pstType->stInfo.pfnOpen(_zLocation + u32TagLength + 1, _bEraseMode);

      /* Valid? */
      if((pstOpenInfo->hResource != orxHANDLE_UNDEFINED) && (pstOpenInfo->hResource != orxNULL))
      {
        /* Updates result */
        hResult = (orxHANDLE)pstOpenInfo;

        /* Stores location */
        pstOpenInfo->zLocation = orxString_Duplicate(_zLocation);
      }
      else
      {
        /* Frees open info */
        orxBank_Free(sstResource.pstOpenInfoBank, pstOpenInfo);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't open resource <%s> of type <%s>: unable to open the location.", _zLocation, pstType->stInfo.zTag);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't open resource <%s>: unknown resource type.", _zLocation);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return hResult;
}

/** Closes a resource
 * @param[in] _hResource        Concerned resource
 */
void orxFASTCALL orxResource_Close(orxHANDLE _hResource)
{
  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Close");

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Has pending operations (and thread hasn't been terminated)? */
    if((pstOpenInfo->u32OpCount != 0) && (sstResource.u32RequestThreadID != orxU32_UNDEFINED))
    {
      /* Adds request */
      orxResource_AddRequest(orxRESOURCE_REQUEST_TYPE_CLOSE, 0, orxNULL, orxNULL, orxNULL, pstOpenInfo);
    }
    else
    {
      /* Closes resource */
      pstOpenInfo->pstTypeInfo->pfnClose(pstOpenInfo->hResource);

      /* Deletes location */
      orxString_Delete(pstOpenInfo->zLocation);

      /* Frees open info */
      orxBank_Free(sstResource.pstOpenInfoBank, pstOpenInfo);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return;
}

/** Gets the literal location of a resource
 * @param[in] _hResource        Concerned resource
 * @return Literal location string
 */
const orxSTRING orxFASTCALL orxResource_GetLocation(orxHANDLE _hResource)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Updates result */
    zResult = pstOpenInfo->zLocation;
  }

  /* Done! */
  return zResult;
}

/** Gets the size, in bytes, of a resource
 * @param[in] _hResource        Concerned resource
 * @return Size of the resource, in bytes
 */
orxS64 orxFASTCALL orxResource_GetSize(orxHANDLE _hResource)
{
  orxS64 s64Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_GetSize");

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Checks */
    orxASSERT(pstOpenInfo->u32OpCount == 0);

    /* Updates result */
    s64Result = pstOpenInfo->pstTypeInfo->pfnGetSize(pstOpenInfo->hResource);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return s64Result;
}

/** Seeks a position in a given resource (moves cursor)
 * @param[in] _hResource        Concerned resource
 * @param[in] _s64Offset        Number of bytes to offset from 'origin'
 * @param[in] _eWhence          Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor position if successful, -1 otherwise
*/
orxS64 orxFASTCALL orxResource_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxS64 s64Result = -1;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_eWhence < orxSEEK_OFFSET_WHENCE_NUMBER);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Seek");

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Checks */
    orxASSERT(pstOpenInfo->u32OpCount == 0);

    /* Updates result */
    s64Result = pstOpenInfo->pstTypeInfo->pfnSeek(pstOpenInfo->hResource, _s64Offset, _eWhence);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return s64Result;
}

/** Tells the position of the cursor in a given resource
 * @param[in] _hResource        Concerned resource
 * @return Position (offset), in bytes
 */
orxS64 orxFASTCALL orxResource_Tell(orxHANDLE _hResource)
{
  orxS64 s64Result = -1;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Tell");

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Checks */
    orxASSERT(pstOpenInfo->u32OpCount == 0);

    /* Updates result */
    s64Result = pstOpenInfo->pstTypeInfo->pfnTell(pstOpenInfo->hResource);
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return s64Result;
}

/** Reads data from a resource
 * @param[in] _hResource        Concerned resource
 * @param[in] _s64Size          Size to read (in bytes)
 * @param[out] _pBuffer         Buffer that will be filled with the read data
 * @param[in] _pfnCallback      Callback that will get called after asynchronous operation; if orxNULL, operation will be synchronous
 * @param[in] _pContext         Context that will be transmitted to the callback when called
 * @return Size of the read data, in bytes or -1 for successful asynchronous call
 */
orxS64 orxFASTCALL orxResource_Read(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, orxRESOURCE_OP_FUNCTION _pfnCallback, void *_pContext)
{
  orxS64 s64Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_pBuffer != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Read");

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Has a callback (asynchronous call) */
    if(_pfnCallback != orxNULL)
    {
      /* Adds request */
      orxResource_AddRequest(orxRESOURCE_REQUEST_TYPE_READ, _s64Size, _pBuffer, _pfnCallback, _pContext, pstOpenInfo);

      /* Updates result */
      s64Result = -1;
    }
    else
    {
      /* Updates result */
      s64Result = pstOpenInfo->pstTypeInfo->pfnRead(pstOpenInfo->hResource, _s64Size, _pBuffer);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return s64Result;
}

/** Writes data to a resource
 * @param[in] _hResource        Concerned resource
 * @param[in] _s64Size          Size to write (in bytes)
 * @param[out] _pBuffer         Buffer that will be written
 * @param[in] _pfnCallback      Callback that will get called after asynchronous operation; if orxNULL, operation will be synchronous
 * @param[in] _pContext         Context that will be transmitted to the callback when called
 * @return Size of the written data, in bytes, 0 if nothing could be written/no write support for this resource type or -1 for successful asynchronous call
 */
orxS64 orxFASTCALL orxResource_Write(orxHANDLE _hResource, orxS64 _s64Size, const void *_pBuffer, orxRESOURCE_OP_FUNCTION _pfnCallback, void *_pContext)
{
  orxS64 s64Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_pBuffer != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Write");

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Supports writing? */
    if(pstOpenInfo->pstTypeInfo->pfnWrite != orxNULL)
    {
      /* Has a callback (asynchronous call) */
      if(_pfnCallback != orxNULL)
      {
        /* Adds request */
        orxResource_AddRequest(orxRESOURCE_REQUEST_TYPE_WRITE, _s64Size, (void *)_pBuffer, _pfnCallback, _pContext, pstOpenInfo);

        /* Updates result */
        s64Result = -1;
      }
      else
      {
        /* Updates result */
        s64Result = pstOpenInfo->pstTypeInfo->pfnWrite(pstOpenInfo->hResource, _s64Size, _pBuffer);
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return s64Result;
}

/** Deletes a resource, given its location
 * @param[in] _zLocation        Location of the resource to delete
 * @return orxSTATUS_SUCCESS upon success, orxSTATUS_FAILURE otherwise
 */
orxSTATUS orxFASTCALL orxResource_Delete(const orxSTRING _zLocation)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Delete");

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE *pstType;
    orxU32            u32TagLength;

    /* For all registered types */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        pstType != orxNULL;
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    {
      /* Gets tag length */
      u32TagLength = orxString_GetLength(pstType->stInfo.zTag);

      /* Match tag? */
      if((orxString_NICompare(_zLocation, pstType->stInfo.zTag, u32TagLength) == 0)
      && (*(_zLocation + u32TagLength) == orxRESOURCE_KC_LOCATION_SEPARATOR))
      {
        /* Selects it */
        break;
      }
    }

    /* Found? */
    if(pstType != orxNULL)
    {
      /* Supports deletion? */
      if(pstType->stInfo.pfnDelete != orxNULL)
      {
        /* Deletes it */
        eResult = pstType->stInfo.pfnDelete(_zLocation + u32TagLength + 1);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't delete resource <%s>: unknown resource type.", _zLocation);
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Gets pending operation count for a given resource
 * @param[in] _hResource        Concerned resource
 * @return Number of pending asynchronous operations for that resource
 */
orxU32 orxFASTCALL orxResource_GetPendingOpCount(const orxHANDLE _hResource)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Updates result */
    u32Result = pstOpenInfo->u32OpCount;
  }

  /* Done! */
  return u32Result;
}

/** Gets total pending operation count
 * @return Number of total pending asynchronous operations
 */
orxU32 orxFASTCALL orxResource_GetTotalPendingOpCount()
{
  orxU32 u32InIndex, u32OutIndex;
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Gets indices */
  u32InIndex  = sstResource.u32RequestInIndex;
  u32OutIndex = sstResource.u32RequestOutIndex;

  /* Update result */
  u32Result = (u32InIndex >= u32OutIndex) ? u32InIndex - u32OutIndex : u32InIndex + (orxRESOURCE_KU32_REQUEST_LIST_SIZE - u32OutIndex);

  /* Has pending operations? */
  if(u32Result != 0)
  {
    /* Main thread? */
    if(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID)
    {
      /* Pumps some request notifications in case caller is waiting in a closed loop */
      orxResource_NotifyRequest(orxNULL, orxNULL);
    }
  }

  /* Done! */
  return u32Result;
}

/** Registers a new resource type
 * @param[in] _pstInfo          Info describing the new resource type and how to handle it
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_RegisterType(const orxRESOURCE_TYPE_INFO *_pstInfo)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_pstInfo != orxNULL);

  /* Valid? */
  if((_pstInfo->zTag != orxNULL)
  && (*(_pstInfo->zTag) != orxCHAR_NULL)
  && (_pstInfo->pfnLocate != orxNULL)
  && (_pstInfo->pfnOpen != orxNULL)
  && (_pstInfo->pfnClose != orxNULL)
  && (_pstInfo->pfnGetSize != orxNULL)
  && (_pstInfo->pfnSeek != orxNULL)
  && (_pstInfo->pfnTell != orxNULL)
  && (_pstInfo->pfnRead != orxNULL))
  {
    orxRESOURCE_TYPE *pstType;

    /* For all registered types */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        (pstType != orxNULL) && (orxString_ICompare(pstType->stInfo.zTag, _pstInfo->zTag) != 0);
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    ;

    /* Not already registered? */
    if(pstType == orxNULL)
    {
      /* Allocates type */
      pstType = (orxRESOURCE_TYPE *)orxBank_Allocate(sstResource.pstTypeBank);

      /* Checks */
      orxASSERT(pstType != orxNULL);

      /* Inits it */
      orxMemory_Zero(&(pstType->stNode), sizeof(orxLINKLIST_NODE));
      orxMemory_Copy(&(pstType->stInfo), _pstInfo, sizeof(orxRESOURCE_TYPE_INFO));
      pstType->stInfo.zTag = orxString_Store(_pstInfo->zTag);

      /* Checks */
      orxASSERT(pstType->stInfo.zTag != orxNULL);

      /* Adds it first */
      orxLinkList_AddStart(&(sstResource.stTypeList), &(pstType->stNode));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't register resource type with tag <%s>: tag is already used by a registered type.", _pstInfo->zTag);
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't register resource type with tag <%s>: incomplete info.", _pstInfo->zTag);
  }

  /* Done! */
  return eResult;
}

/** Unregisters a resource type
 * @param[in] _zTypeTag         Tag of the resource type to unregister
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_UnregisterType(const orxSTRING _zTypeTag)
{
  orxRESOURCE_TYPE *pstType;
  orxSTATUS         eResult;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zTypeTag != orxNULL);

  /* For all registered types */
  for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
      (pstType != orxNULL) && (orxString_ICompare(pstType->stInfo.zTag, _zTypeTag) != 0);
      pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
    ;

  /* Found? */
  if(pstType != orxNULL)
  {
    /* Removes it */
    orxLinkList_Remove(&(pstType->stNode));

    /* Frees it */
    orxBank_Free(sstResource.pstTypeBank, pstType);

    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Gets number of registered resource types
 * @return Number of registered resource types
 */
orxU32 orxFASTCALL orxResource_GetTypeCount()
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Updates result */
  u32Result = orxLinkList_GetCount(&(sstResource.stTypeList));

  /* Done! */
  return u32Result;
}

/** Gets registered type tag at given index
 * @param[in] _u32Index         Index of storage
 * @return Type tag string if index is valid, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxResource_GetTypeTag(orxU32 _u32Index)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Valid index? */
  if(_u32Index < orxLinkList_GetCount(&(sstResource.stTypeList)))
  {
    orxRESOURCE_TYPE *pstType;

    /* Finds requested group */
    for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
        _u32Index > 0;
        pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)), _u32Index--)
    ;

    /* Checks */
    orxASSERT(pstType != orxNULL);

    /* Updates result */
    zResult = pstType->stInfo.zTag;
  }

  /* Done! */
  return zResult;
}

/** Syncs all cached resources for specific resource group(s): update, add or remove events will be sent for all resources that are not located in their original storage anymore
 * @param[in] _zGroup           Concerned resource group, orxNULL for all groups
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_Sync(const orxSTRING _zGroup)
{
  orxSTRINGID         stGroupID;
  orxRESOURCE_GROUP  *pstGroup;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_Sync");

  /* Gets group ID */
  stGroupID = (_zGroup != orxNULL) ? orxString_Hash(_zGroup) : orxSTRINGID_UNDEFINED;

  /* For all groups */
  for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
      pstGroup != orxNULL;
      pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup))
  {
    /* Matches? */
    if((_zGroup == orxNULL)
    || (pstGroup->stID == stGroupID))
    {
      orxHANDLE         hIterator;
      orxRESOURCE_INFO *pstResourceInfo;
      const orxSTRING   zGroup;

      /* Gets group */
      zGroup = orxString_GetFromID(pstGroup->stID);

      /* For all cached resources */
      for(hIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstResourceInfo);
          hIterator != orxHANDLE_UNDEFINED;
          hIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, hIterator, orxNULL, (void **)&pstResourceInfo))
      {
        orxRESOURCE_STORAGE  *pstStorage;
        const orxSTRING       zName;

        /* Gets its name */
        zName = orxString_GetFromID(pstResourceInfo->stNameID);

        /* For all storages in group */
        for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
            pstStorage != orxNULL;
            pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)))
        {
          orxRESOURCE_TYPE *pstType;

          /* For all registered types */
          for(pstType = (orxRESOURCE_TYPE *)orxLinkList_GetFirst(&(sstResource.stTypeList));
              pstType != orxNULL;
              pstType = (orxRESOURCE_TYPE *)orxLinkList_GetNext(&(pstType->stNode)))
          {
            const orxSTRING zNewLocation;

            /* Locates resource */
            zNewLocation = pstType->stInfo.pfnLocate(zGroup, orxString_GetFromID(pstStorage->stID), zName, orxTRUE);

            /* Success? */
            if(zNewLocation != orxNULL)
            {
              /* New location? */
              if((&(pstType->stInfo) != pstResourceInfo->pstTypeInfo)
              || (pstResourceInfo->s64Time == 0)
              || (orxString_ICompare(pstResourceInfo->zLocation + orxString_GetLength(pstType->stInfo.zTag) + 1, zNewLocation) != 0))
              {
                orxRESOURCE_EVENT_PAYLOAD stPayload;
                orxS32                    s32Size;
                orxBOOL                   bAdd;

                /* Updates status */
                bAdd = (pstResourceInfo->s64Time == 0) ? orxTRUE : orxFALSE;

                /* Deletes its previous location */
                orxMemory_Free(pstResourceInfo->zLocation);

                /* Updates its resource info */
                pstResourceInfo->pstTypeInfo  = &(pstType->stInfo);
                pstResourceInfo->s64Time      = orxRESOURCE_KU32_WATCH_TIME_UNINITIALIZED;
                s32Size                       = orxString_GetLength(pstType->stInfo.zTag) + orxString_GetLength(zNewLocation) + 2;
                pstResourceInfo->zLocation    = (orxSTRING)orxMemory_Allocate(s32Size, orxMEMORY_TYPE_TEXT);
                orxASSERT(pstResourceInfo->zLocation != orxNULL);
                orxString_NPrint(pstResourceInfo->zLocation, s32Size, "%s%c%s", pstType->stInfo.zTag, orxRESOURCE_KC_LOCATION_SEPARATOR, zNewLocation);
                orxMEMORY_BARRIER();

                /* Clears payload */
                orxMemory_Zero(&stPayload, sizeof(orxRESOURCE_EVENT_PAYLOAD));

                /* Inits payload */
                stPayload.s64Time     = pstResourceInfo->s64Time;
                stPayload.zLocation   = pstResourceInfo->zLocation;
                stPayload.pstTypeInfo = pstResourceInfo->pstTypeInfo;
                stPayload.stGroupID   = pstResourceInfo->stGroupID;
                stPayload.stStorageID = pstResourceInfo->stStorageID;
                stPayload.stNameID    = pstResourceInfo->stNameID;

                /* Sends event */
                orxEVENT_SEND(orxEVENT_TYPE_RESOURCE, (bAdd != orxFALSE) ? orxRESOURCE_EVENT_ADD : orxRESOURCE_EVENT_UPDATE, orxNULL, orxNULL, &stPayload);
              }

              break;
            }
          }

          /* Was found? */
          if(pstType != orxNULL)
          {
            /* Stops */
            break;
          }
        }

        /* Was not found and not already removed? */
        if((pstStorage == orxNULL) && (pstResourceInfo->s64Time != 0))
        {
          orxRESOURCE_EVENT_PAYLOAD stPayload;

          /* Clears payload */
          orxMemory_Zero(&stPayload, sizeof(orxRESOURCE_EVENT_PAYLOAD));

          /* Updates its resource info */
          pstResourceInfo->s64Time = 0;

          /* Inits payload */
          stPayload.s64Time     = pstResourceInfo->s64Time;
          stPayload.zLocation   = pstResourceInfo->zLocation;
          stPayload.pstTypeInfo = pstResourceInfo->pstTypeInfo;
          stPayload.stGroupID   = pstResourceInfo->stGroupID;
          stPayload.stStorageID = pstResourceInfo->stStorageID;
          stPayload.stNameID    = pstResourceInfo->stNameID;

          /* Sends event */
          orxEVENT_SEND(orxEVENT_TYPE_RESOURCE, orxRESOURCE_EVENT_REMOVE, orxNULL, orxNULL, &stPayload);
        }
      }
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Clears cache for specific resource group(s)
 * @param[in] _zGroup           Concerned resource group, orxNULL for all groups
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_ClearCache(const orxSTRING _zGroup)
{
  orxSTRINGID         stGroupID;
  orxRESOURCE_GROUP  *pstGroup;
  orxSTATUS           eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxResource_ClearCache");

  /* Waits for all pending operations to complete */
  while(sstResource.u32RequestProcessIndex != sstResource.u32RequestInIndex);

  /* Gets group ID */
  stGroupID = (_zGroup != orxNULL) ? orxString_Hash(_zGroup) : orxSTRINGID_UNDEFINED;

    /* For all groups */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        pstGroup != orxNULL;
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup))
    {
    /* Matches? */
    if((_zGroup == orxNULL)
    || (pstGroup->stID == stGroupID))
    {
      orxHANDLE         hIterator;
      orxRESOURCE_INFO *pstResourceInfo;

      /* For all cached resources */
      for(hIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstResourceInfo);
          hIterator != orxHANDLE_UNDEFINED;
          hIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, hIterator, orxNULL, (void **)&pstResourceInfo))
      {
        /* Deletes its location */
        orxMemory_Free(pstResourceInfo->zLocation);

        /* Frees it */
        orxBank_Free(sstResource.pstResourceInfoBank, pstResourceInfo);
      }

      /* Clears cache table */
      orxHashTable_Clear(pstGroup->pstCacheTable);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
  }

/** Gets the cached location count for a given group
 * @param[in] _zGroup           Concerned resource group
 * @return    Item number
 */
orxU32 orxFASTCALL orxResource_GetCacheCount(const orxSTRING _zGroup)
{
  orxU32 u32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_Hash(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Success? */
    if(pstGroup != orxNULL)
    {
      /* Updates result */
      u32Result = orxHashTable_GetCount(pstGroup->pstCacheTable);
    }
  }

  /* Done! */
  return u32Result;
}

/** Gets the next cached location for the given group and returns an iterator for next search
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _hIterator        Iterator from previous search or orxHANDLE_UNDEFINED/orxNULL for a new search
 * @param[out] _pzLocation      Current resource's location, orxNULL to ignore
 * @param[out] _pzStorage       Current resource's storage, orxNULL to ignore
 * @param[out] _pzName          Current resource's name, orxNULL to ignore
 * @return Iterator for next element if an element has been found, orxHANDLE_UNDEFINED otherwise
 */
orxHANDLE orxFASTCALL orxResource_GetNextCachedLocation(const orxSTRING _zGroup, orxHANDLE _hIterator, const orxSTRING *_pzLocation, const orxSTRING *_pzStorage, const orxSTRING *_pzName)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP  *pstGroup;
    orxSTRINGID         stGroupID;

    /* Gets group ID */
    stGroupID = orxString_Hash(_zGroup);

    /* Gets group */
    pstGroup = orxResource_FindGroup(stGroupID);

    /* Success? */
    if(pstGroup != orxNULL)
    {
      orxRESOURCE_INFO *pstResourceInfo;

      /* Gets next cached resource */
      hResult = orxHashTable_GetNext(pstGroup->pstCacheTable, _hIterator, orxNULL, (void **)&pstResourceInfo);

      /* Success? */
      if(hResult != orxHANDLE_UNDEFINED)
      {
        /* Updates location */
        if(_pzLocation != orxNULL)
        {
          *_pzLocation = pstResourceInfo->zLocation;
        }

        /* Updates storage */
        if(_pzStorage != orxNULL)
        {
          *_pzStorage = orxString_GetFromID(pstResourceInfo->stStorageID);
        }

        /* Updates name */
        if(_pzName != orxNULL)
        {
          *_pzName = orxString_GetFromID(pstResourceInfo->stNameID);
        }
      }
    }
  }

  /* Done! */
  return hResult;
}

/** Sets an internal memory resource
 * !IMPORTANT! The content of _pBuffer is *required* to remain valid until this resource has been successfully unset (by passing _s64Size=0 or _pBuffer=orxNULL), no internal copies will be made!
 * @param[in] _zGroup           Group of the resource to set/unset, orxNULL to be available for all groups
 * @param[in] _zStorage         Storage of the resource to set/unset, orxNULL for the default storage
 * @param[in] _zName            Name of the resource to set/unset
 * @param[in] _s64Size          Size of the resource's data (0 to unset)
 * @param[in] _pBuffer          Data of the resource (orxNULL to unset)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_SetMemoryResource(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName, orxS64 _s64Size, const void *_pBuffer)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zName != orxNULL);
  orxASSERT(_s64Size >= 0);

  /* Valid name? */
  if(*_zName != orxCHAR_NULL)
  {
    orxCHAR                   acBuffer[orxRESOURCE_KU32_BUFFER_SIZE];
    orxRESOURCE_MEMORY_DATA  *pstData;
    orxSTRINGID               stNameID;

    /* Composes full name */
    orxString_NPrint(acBuffer, sizeof(acBuffer), "%s%c%s%c%s", (_zGroup != orxNULL) ? _zGroup : orxSTRING_EMPTY, orxRESOURCE_KC_LOCATION_SEPARATOR, ((_zStorage != orxNULL) && (orxString_Compare(_zStorage, orxRESOURCE_KZ_DEFAULT_STORAGE) != 0)) ? _zStorage : orxSTRING_EMPTY, orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zName);

    /* Gets its ID */
    stNameID = orxString_Hash(acBuffer);

    /* Gets its data */
    pstData = (orxRESOURCE_MEMORY_DATA *)orxHashTable_Get(sstResource.pstMemoryDataTable, stNameID);

    /* Found? */
    if(pstData != orxNULL)
    {
      /* Has ref count? */
      if(pstData->u32RefCount != 0)
      {
        /* Updates result */
        eResult = orxSTATUS_FAILURE;
      }
    }

    /* Should continue? */
    if(eResult != orxSTATUS_FAILURE)
    {
      /* Set? */
      if((_s64Size > 0) && (_pBuffer != orxNULL))
      {
        /* No data? */
        if(pstData == orxNULL)
        {
          /* Allocates its internal data */
          pstData = (orxRESOURCE_MEMORY_DATA *)orxBank_Allocate(sstResource.pstMemoryDataBank);
        }

        /* Success? */
        if(pstData != orxNULL)
        {
          /* Inits it */
          orxMemory_Zero(pstData, sizeof(orxRESOURCE_MEMORY_DATA));
          pstData->stNameID   = stNameID;
          pstData->s64Size    = _s64Size;
          pstData->s64Time    = (orxS64)(1000000 * orxSystem_GetSystemTime());
          pstData->pu8Buffer  = (orxU8 *)_pBuffer;

          /* Stores it */
          eResult = orxHashTable_Set(sstResource.pstMemoryDataTable, stNameID, pstData);
        }
        else
        {
          /* Updates result */
          eResult = orxSTATUS_FAILURE;
        }
      }
      /* Unset */
      else
      {
        /* Has data? */
        if(pstData != orxNULL)
        {
          /* Checks */
          orxASSERT(pstData->u32RefCount == 0);

          /* Removes it */
          orxHashTable_Remove(sstResource.pstMemoryDataTable, stNameID);

          /* Frees it */
          orxBank_Free(sstResource.pstMemoryDataBank, pstData);
        }
      }
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

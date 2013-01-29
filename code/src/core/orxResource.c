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
 * @file orxResource.c
 * @date 20/01/2013
 * @author iarwain@orx-project.org
 *
 */


#include "core/orxResource.h"

#include "debug/orxDebug.h"
#include "core/orxConfig.h"
#include "io/orxFile.h"
#include "memory/orxBank.h"
#include "memory/orxMemory.h"
#include "utils/orxHashTable.h"
#include "utils/orxLinkList.h"
#include "utils/orxString.h"


/** Module flags
 */
#define orxRESOURCE_KU32_STATIC_FLAG_NONE             0x00000000                      /**< No flags */

#define orxRESOURCE_KU32_STATIC_FLAG_READY            0x00000001                      /**< Ready flag */
#define orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED    0x00000002                      /**< Config loaded flag */

#define orxRESOURCE_KU32_STATIC_MASK_ALL              0xFFFFFFFF                      /**< All mask */


/** Misc
 */
#define orxRESOURCE_KU32_CACHE_TABLE_SIZE             32                              /**< Resource info cache table size */

#define orxRESOURCE_KU32_RESOURCE_INFO_BANK_SIZE      128                             /**< Resource info bank size */

#define orxRESOURCE_KU32_STORAGE_BANK_SIZE            16                              /**< Storage bank size */
#define orxRESOURCE_KU32_GROUP_BANK_SIZE              8                               /**< Group bank size */
#define orxRESOURCE_KU32_TYPE_INFO_BANK_SIZE          8                               /**< Type info bank size */

#define orxRESOURCE_KU32_OPEN_INFO_BANK_SIZE          8                               /**< Open resource info bank size */

#define orxRESOURCE_KZ_DEFAULT_STORAGE                "."                             /**< Default storage */

#define orxRESOURCE_KZ_TYPE_TAG_FILE                  "file"                          /**< Resource type file tag */

#define orxRESOURCE_KU32_BUFFER_SIZE                  256                             /**< Buffer size */

#define orxRESOURCE_KZ_CONFIG_SECTION                 "Resource"                      /**< Config section name */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Resource group
 */
typedef struct __orxRESOURCE_GROUP_t
{
  orxSTRING                 zName;                                                    /**< Group name */
  orxLINKLIST               stStorageList;                                            /**< Group storage list */
  orxBANK                  *pstStorageBank;                                           /**< Group storage bank */
  orxHASHTABLE             *pstCacheTable;                                            /**< Group cache table */

} orxRESOURCE_GROUP;

/** Storage info
 */
typedef struct __orxRESOURCE_STORAGE_t
{
  orxLINKLIST_NODE          stNode;                                                   /**< Linklist node */
  orxSTRING                 zStorage;                                                 /**< Literal storage */

} orxRESOURCE_STORAGE;

/** Resource info
 */
typedef struct __orxRESOURCE_LOCATION_t
{
  orxSTRING                 zLocation;                                                /**< Resource literal location */
  orxRESOURCE_TYPE_INFO    *pstTypeInfo;                                              /**< Resource type info */

} orxRESOURCE_LOCATION;

/** Open resource info
 */
typedef struct __orxRESOURCE_OPEN_INFO_t
{
  orxRESOURCE_TYPE_INFO    *pstTypeInfo;
  orxHANDLE                 hResource;

} orxRESOURCE_OPEN_INFO;

/** Static structure
 */
typedef struct __orxRESOURCE_STATIC_t
{
  orxBANK                  *pstGroupBank;                                             /**< Group bank */
  orxBANK                  *pstTypeInfoBank;                                          /**< Type info bank */
  orxBANK                  *pstResourceInfoBank;                                      /**< Resource info bank */
  orxBANK                  *pstOpenInfoBank;                                          /**< Open resource table size */
  orxRESOURCE_TYPE_INFO     stNativeFileSystemTypeInfo;                               /**< Native file system type info */
  orxCHAR                   acFileLocationBuffer[orxRESOURCE_KU32_BUFFER_SIZE];       /**< File location buffer size */
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

static const orxSTRING orxFASTCALL orxResource_File_Locate(const orxSTRING _zStorage, const orxSTRING _zName)
{
  const orxSTRING zResult = orxNULL;

  /* Default storage? */
  if(orxString_Compare(_zStorage, orxRESOURCE_KZ_DEFAULT_STORAGE) == 0)
  {
    /* Uses name as path */
    orxString_NPrint(sstResource.acFileLocationBuffer, orxRESOURCE_KU32_BUFFER_SIZE - 1, "%s", _zName);
  }
  else
  {
    /* Composes full name */
    orxString_NPrint(sstResource.acFileLocationBuffer, orxRESOURCE_KU32_BUFFER_SIZE - 1, "%s%c%s", _zStorage, orxCHAR_DIRECTORY_SEPARATOR_LINUX, _zName);
  }

  /* Exist? */
  if(orxFile_Exists(sstResource.acFileLocationBuffer) != orxFALSE)
  {
    /* Updates result */
    zResult = sstResource.acFileLocationBuffer;
  }

  /* Done! */
  return zResult;
}

static orxHANDLE orxFASTCALL orxResource_File_Open(const orxSTRING _zLocation)
{
  orxFILE  *pstFile;
  orxHANDLE hResult;

  /* Opens file */
  pstFile = orxFile_Open(_zLocation, orxFILE_KU32_FLAG_OPEN_READ |  orxFILE_KU32_FLAG_OPEN_BINARY);

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
}

static orxS32 orxFASTCALL orxResource_File_GetSize(orxHANDLE _hResource)
{
  orxFILE  *pstFile;
  orxS32    s32Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s32Result = orxFile_GetSize(pstFile);

  /* Done! */
  return s32Result;
}

static orxS32 orxFASTCALL orxResource_File_Seek(orxHANDLE _hResource, orxS32 _s32Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxFILE  *pstFile;
  orxS32    s32Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s32Result = orxFile_Seek(pstFile, _s32Offset, _eWhence);

  /* Done! */
  return s32Result;
}

static orxS32 orxFASTCALL orxResource_File_Tell(orxHANDLE _hResource)
{
  orxFILE  *pstFile;
  orxS32    s32Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s32Result = orxFile_Tell(pstFile);

  /* Done! */
  return s32Result;
}

static orxS32 orxFASTCALL orxResource_File_Read(orxHANDLE _hResource, orxS32 _s32Size, void *_pBuffer)
{
  orxFILE  *pstFile;
  orxS32    s32Result;

  /* Gets file */
  pstFile = (orxFILE *)_hResource;

  /* Updates result */
  s32Result = orxFile_Read(_pBuffer, sizeof(orxCHAR), _s32Size, pstFile);

  /* Done! */
  return s32Result;
}

static orxINLINE void orxResource_DeleteGroup(orxRESOURCE_GROUP *_pstGroup)
{
  orxRESOURCE_STORAGE  *pstStorage;
  orxRESOURCE_LOCATION *pstResourceInfo;
  orxU32                u32Key;
  orxHANDLE             hIterator;

  /* For all of its storages */
  for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(_pstGroup->stStorageList));
      pstStorage != orxNULL;
      pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)))
  {
    /* Deletes its content */
    orxString_Delete(pstStorage->zStorage);
  }

  /* For all cached resources */
  for(hIterator = orxHashTable_GetNext(_pstGroup->pstCacheTable, orxHANDLE_UNDEFINED, &u32Key, (void **)&pstResourceInfo);
      hIterator != orxHANDLE_UNDEFINED;
      hIterator = orxHashTable_GetNext(_pstGroup->pstCacheTable, hIterator, &u32Key, (void **)&pstResourceInfo))
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

  /* Deletes its name */
  orxString_Delete(_pstGroup->zName);
}

static orxINLINE orxRESOURCE_GROUP *orxResource_CreateGroup(const orxSTRING _zGroup)
{
  orxRESOURCE_GROUP *pstResult;

  /* Creates group */
  pstResult = (orxRESOURCE_GROUP *)orxBank_Allocate(sstResource.pstGroupBank);

  /* Success? */
  if(pstResult != orxNULL)
  {
    orxRESOURCE_STORAGE *pstStorage;

    /* Inits it */
    pstResult->zName          = orxString_Duplicate(_zGroup);
    pstResult->pstStorageBank = orxBank_Create(orxRESOURCE_KU32_STORAGE_BANK_SIZE, sizeof(orxRESOURCE_STORAGE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    pstResult->pstCacheTable  = orxHashTable_Create(orxRESOURCE_KU32_CACHE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    orxMemory_Zero(&(pstResult->stStorageList), sizeof(orxLINKLIST));

    /* Creates storage */
    pstStorage = (orxRESOURCE_STORAGE *)orxBank_Allocate(pstResult->pstStorageBank);

    /* Success? */
    if(pstStorage != orxNULL)
    {
      /* Stores its content */
      pstStorage->zStorage = orxString_Duplicate(orxRESOURCE_KZ_DEFAULT_STORAGE);

      /* Clears node */
      orxMemory_Zero(&(pstStorage->stNode), sizeof(orxLINKLIST_NODE));

      /* Adds it last */
      orxLinkList_AddEnd(&(pstResult->stStorageList), &(pstStorage->stNode));
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


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Resource module setup
 */
void orxFASTCALL orxResource_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_BANK);
  orxModule_AddDependency(orxMODULE_ID_RESOURCE, orxMODULE_ID_FILE);

  /* Done! */
  return;
}

/** Inits resource module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstResource.u32Flags & orxRESOURCE_KU32_STATIC_FLAG_READY))
  {
    /* Cleans control structure */
    orxMemory_Zero(&sstResource, sizeof(orxRESOURCE_STATIC));

    /* Creates resource info bank */
    sstResource.pstResourceInfoBank = orxBank_Create(orxRESOURCE_KU32_RESOURCE_INFO_BANK_SIZE, sizeof(orxRESOURCE_LOCATION), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Creates open resource info bank */
    sstResource.pstOpenInfoBank     = orxBank_Create(orxRESOURCE_KU32_OPEN_INFO_BANK_SIZE, sizeof(orxRESOURCE_OPEN_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Creates group bank */
    sstResource.pstGroupBank        = orxBank_Create(orxRESOURCE_KU32_GROUP_BANK_SIZE, sizeof(orxRESOURCE_GROUP), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Creates type info bank */
    sstResource.pstTypeInfoBank     = orxBank_Create(orxRESOURCE_KU32_TYPE_INFO_BANK_SIZE, sizeof(orxRESOURCE_TYPE_INFO), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Success? */
    if((sstResource.pstResourceInfoBank != orxNULL) && (sstResource.pstOpenInfoBank != orxNULL) && (sstResource.pstGroupBank != orxNULL) && (sstResource.pstTypeInfoBank != orxNULL))
    {
      orxRESOURCE_TYPE_INFO stTypeInfo;

      /* Inits Flags */
      sstResource.u32Flags = orxRESOURCE_KU32_STATIC_FLAG_READY;

      /* Inits file type */
      stTypeInfo.zTag       = orxRESOURCE_KZ_TYPE_TAG_FILE;
      stTypeInfo.pfnLocate  = orxResource_File_Locate;
      stTypeInfo.pfnOpen    = orxResource_File_Open;
      stTypeInfo.pfnClose   = orxResource_File_Close;
      stTypeInfo.pfnGetSize = orxResource_File_GetSize;
      stTypeInfo.pfnSeek    = orxResource_File_Seek;
      stTypeInfo.pfnTell    = orxResource_File_Tell;
      stTypeInfo.pfnRead    = orxResource_File_Read;

      /* Registers it */
      eResult = orxResource_RegisterType(&stTypeInfo);
    }

    /* Failed? */
    if(eResult != orxSTATUS_SUCCESS)
    {
      /* Removes Flags */
      sstResource.u32Flags &= ~orxRESOURCE_KU32_STATIC_FLAG_READY;

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

      /* Deletes type info bank */
      if(sstResource.pstTypeInfoBank != orxNULL)
      {
        orxBank_Delete(sstResource.pstTypeInfoBank);
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
    orxRESOURCE_TYPE_INFO  *pstTypeInfo;
    orxRESOURCE_OPEN_INFO  *pstOpenInfo;

    /* For all groups */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        pstGroup != orxNULL;
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup))
    {
      /* Deletes it */
      orxResource_DeleteGroup(pstGroup);
    }

    /* Deletes group bank */
    orxBank_Delete(sstResource.pstGroupBank);

    /* For all type Info */
    for(pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, orxNULL);
        pstTypeInfo != orxNULL;
        pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, pstTypeInfo))
    {
      /* Deletes its tag */
      orxString_Delete(pstTypeInfo->zTag);
    }

    /* Deletes type info bank */
    orxBank_Delete(sstResource.pstTypeInfoBank);

    /* For all open resources */
    while((pstOpenInfo = (orxRESOURCE_OPEN_INFO *)orxBank_GetNext(sstResource.pstOpenInfoBank, orxNULL)) != orxNULL)
    {
      /* Closes it */
      orxResource_Close((orxHANDLE)pstOpenInfo);
    }

    /* Deletes open info bank */
    orxBank_Delete(sstResource.pstOpenInfoBank);

    /* Checks */
    orxASSERT(orxBank_GetCounter(sstResource.pstResourceInfoBank) == 0);

    /* Deletes info bank */
    orxBank_Delete(sstResource.pstResourceInfoBank);

    /* Updates flags */
    sstResource.u32Flags &= ~orxRESOURCE_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return;
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
    orxRESOURCE_GROUP *pstGroup = orxNULL;

    /* Gets group */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        (pstGroup != orxNULL) && (orxString_Compare(pstGroup->zName, _zGroup) != 0);
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup));

    /* Not found? */
    if(pstGroup == orxNULL)
    {
      /* Creates it */
      pstGroup = orxResource_CreateGroup(_zGroup);
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
        pstStorage->zStorage = orxString_Duplicate(_zStorage);

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
          /* Adds it before default */
          orxLinkList_AddBefore(orxLinkList_GetLast(&(pstGroup->stStorageList)), &(pstStorage->stNode));
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

/** Removes a storage for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zStorage         Concerned storage
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_RemoveStorage(const orxSTRING _zGroup, const orxSTRING _zStorage)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);
  orxASSERT(_zStorage != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP *pstGroup = orxNULL;

    /* Gets group */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        (pstGroup != orxNULL) && (orxString_Compare(pstGroup->zName, _zGroup) != 0);
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup));

    /* Success? */
    if(pstGroup != orxNULL)
    {
      orxRESOURCE_STORAGE *pstStorage;

      /* For all storages in group */
      for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
          pstStorage != orxNULL;
          pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)))
      {
        /* Matches? */
        if(orxString_Compare(_zStorage, pstStorage->zStorage) == 0)
        {
          /* Removes it from list */
          orxLinkList_Remove(&(pstStorage->stNode));

          /* Deletes its content */
          orxString_Delete(pstStorage->zStorage);

          /* Frees it */
          orxBank_Free(pstGroup->pstStorageBank, pstStorage);

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;

          break;
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
orxS32 orxFASTCALL orxResource_GetStorageCounter(const orxSTRING _zGroup)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP *pstGroup = orxNULL;

    /* Gets group */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        (pstGroup != orxNULL) && (orxString_Compare(pstGroup->zName, _zGroup) != 0);
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup));

    /* Success? */
    if(pstGroup != orxNULL)
    {
      /* Updates result */
      s32Result = orxLinkList_GetCounter(&(pstGroup->stStorageList));
    }
  }

  /* Done! */
  return s32Result;
}

/** Gets storage at given index for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _s32Index         Index of storage
 * @return Storage if index is valid, orxNULL otherwise
 */
const orxSTRING orxFASTCALL orxResource_GetStorage(const orxSTRING _zGroup, orxS32 _s32Index)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP *pstGroup = orxNULL;

    /* Gets group */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        (pstGroup != orxNULL) && (orxString_Compare(pstGroup->zName, _zGroup) != 0);
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup));

    /* Success? */
    if(pstGroup != orxNULL)
    {
      /* Valid index? */
      if(_s32Index < (orxS32)orxBank_GetCounter(pstGroup->pstStorageBank))
      {
        orxRESOURCE_STORAGE *pstStorage;

        /* Finds requested storage */
        for(pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetFirst(&(pstGroup->stStorageList));
            _s32Index > 0;
            pstStorage = (orxRESOURCE_STORAGE *)orxLinkList_GetNext(&(pstStorage->stNode)), _s32Index--);

        /* Checks */
        orxASSERT(pstStorage != orxNULL);

        /* Updates result */
        zResult = pstStorage->zStorage;
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Gets the location of a resource for a given group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zName            Name of the resource to locate
 * @return Location string if found, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxResource_Locate(const orxSTRING _zGroup, const orxSTRING _zName)
{
  const orxSTRING zResult = orxNULL;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zGroup != orxNULL);
  orxASSERT(_zName != orxNULL);

  /* Isn't config already loaded? */
  if(!orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED))
  {
    orxS32 i, s32SectionCounter;

    /* Pushes resource config section */
    orxConfig_PushSection(orxRESOURCE_KZ_CONFIG_SECTION);

    /* For all keys */
    for(i = 0, s32SectionCounter = orxConfig_GetKeyCounter(); i < s32SectionCounter; i++)
    {
      const orxSTRING zGroup;
      orxS32          j;

      /* Gets group */
      zGroup = orxConfig_GetKey(i);

      /* For all storages in list */
      for(j = orxConfig_GetListCounter(zGroup) - 1; j >= 0; j--)
      {
        /* Adds it to group */
        orxResource_AddStorage(zGroup, orxConfig_GetListString(zGroup, j), orxTRUE);
      }

      /* Updates status */
      orxFLAG_SET(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_CONFIG_LOADED, orxRESOURCE_KU32_STATIC_FLAG_NONE);
    }

    /* Pops config section */
    orxConfig_PopSection();
  }

  /* Valid? */
  if(*_zGroup != orxCHAR_NULL)
  {
    orxRESOURCE_GROUP *pstGroup = orxNULL;

    /* Gets group */
    for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
        (pstGroup != orxNULL) && (orxString_Compare(pstGroup->zName, _zGroup) != 0);
        pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup));

    /* Not found? */
    if(pstGroup == orxNULL)
    {
      /* Creates it */
      pstGroup = orxResource_CreateGroup(_zGroup);
    }

    /* Success? */
    if(pstGroup != orxNULL)
    {
      orxU32            u32Key;
      orxRESOURCE_LOCATION *pstResourceInfo;

      /* Gets resource info key */
      u32Key = orxString_ToCRC(_zName);

      /* Gets resource info from cache */
      pstResourceInfo = (orxRESOURCE_LOCATION *)orxHashTable_Get(pstGroup->pstCacheTable, u32Key);

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
          orxRESOURCE_TYPE_INFO *pstTypeInfo;

          /* For all registered types */
          for(pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, orxNULL);
              pstTypeInfo != orxNULL;
              pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, pstTypeInfo))
          {
            const orxSTRING zLocation;

            /* Locates resource */
            zLocation = pstTypeInfo->pfnLocate(pstStorage->zStorage, _zName);

            /* Success? */
            if(zLocation != orxNULL)
            {
              orxRESOURCE_LOCATION *pstResourceInfo;

              /* Allocates resource info */
              pstResourceInfo = (orxRESOURCE_LOCATION *)orxBank_Allocate(sstResource.pstResourceInfoBank);

              /* Checks */
              orxASSERT(pstResourceInfo != orxNULL);

              /* Inits it */
              pstResourceInfo->pstTypeInfo  = pstTypeInfo;
              pstResourceInfo->zLocation    = (orxSTRING)orxMemory_Allocate(orxString_GetLength(pstTypeInfo->zTag) + orxString_GetLength(zLocation) + 2, orxMEMORY_TYPE_MAIN);
              orxASSERT(pstResourceInfo->zLocation != orxNULL);
              orxString_Print(pstResourceInfo->zLocation, "%s%c%s", pstTypeInfo->zTag, orxRESOURCE_KC_LOCATION_SEPARATOR, zLocation);

              /* Adds it to cache */
              orxHashTable_Add(pstGroup->pstCacheTable, u32Key, pstResourceInfo);

              /* Updates result */
              zResult = pstResourceInfo->zLocation;

              break;
            }
          }
        }
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Gets the resource name from a location
 * @param[in] _zLocation        Location of the concerned resource
 * @return Name string if valid, orxSTRING_EMPTY otherwise
 */
const orxSTRING orxFASTCALL orxResource_GetName(const orxSTRING _zLocation)
{
  const orxSTRING zResult = orxSTRING_EMPTY;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE_INFO *pstTypeInfo;

    /* For all registered types */
    for(pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, orxNULL);
        pstTypeInfo != orxNULL;
        pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, pstTypeInfo))
    {
      orxS32 s32TagLength;

      /* Gets tag length */
      s32TagLength = orxString_GetLength(pstTypeInfo->zTag);

      /* Match tag? */
      if(orxString_NICompare(_zLocation, pstTypeInfo->zTag, s32TagLength) == 0)
      {
        /* Valid? */
        if(*(_zLocation + s32TagLength) == orxRESOURCE_KC_LOCATION_SEPARATOR)
        {
          /* Updates result */
          zResult = _zLocation + s32TagLength + 1;
        }

        break;
      }
    }
  }

  /* Done! */
  return zResult;
}

/** Opens the resource at the given location
 * @param[in] _zLocation        Location of the resource to open
 * @return Handle to the open location, orxHANDLE_UNDEFINED otherwise
 */
orxHANDLE orxFASTCALL orxResource_Open(const orxSTRING _zLocation)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_zLocation != orxNULL);

  /* Valid? */
  if(*_zLocation != orxCHAR_NULL)
  {
    orxRESOURCE_TYPE_INFO  *pstTypeInfo;
    orxS32                  s32TagLength;

    /* For all registered types */
    for(pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, orxNULL);
        pstTypeInfo != orxNULL;
        pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, pstTypeInfo))
    {
      /* Gets tag length */
      s32TagLength = orxString_GetLength(pstTypeInfo->zTag);

      /* Match tag? */
      if(orxString_NICompare(_zLocation, pstTypeInfo->zTag, s32TagLength) == 0)
      {
        /* Selects it */
        break;
      }
    }

    /* Found? */
    if(pstTypeInfo != orxNULL)
    {
      orxRESOURCE_OPEN_INFO *pstOpenInfo;

      /* Allocates open info */
      pstOpenInfo = (orxRESOURCE_OPEN_INFO *)orxBank_Allocate(sstResource.pstOpenInfoBank);

      /* Checks */
      orxASSERT(pstOpenInfo != orxNULL);

      /* Inits it */
      pstOpenInfo->pstTypeInfo = pstTypeInfo;

      /* Opens it */
      pstOpenInfo->hResource = pstTypeInfo->pfnOpen(_zLocation + s32TagLength + 1);

      /* Valid? */
      if((pstOpenInfo->hResource != orxHANDLE_UNDEFINED) && (pstOpenInfo->hResource != orxNULL))
      {
        /* Updates result */
        hResult = (orxHANDLE)pstOpenInfo;
      }
      else
      {
        /* Frees open info */
        orxBank_Free(sstResource.pstOpenInfoBank, pstOpenInfo);

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't open resource <%s> of type <%s>: unable to open the location.", _zLocation);
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Can't open resource <%s>: unknown resource type.", _zLocation);
    }
  }

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

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Closes resource */
    pstOpenInfo->pstTypeInfo->pfnClose(pstOpenInfo->hResource);

    /* Frees open info */
    orxBank_Free(sstResource.pstOpenInfoBank, pstOpenInfo);
  }

  /* Done! */
  return;
}

/** Gets the size, in bytes, of a resource
 * @param[in] _hResource        Concerned resource
 * @return Size of the resource, in bytes
 */
orxS32 orxFASTCALL orxResource_GetSize(orxHANDLE _hResource)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Updates result */
    s32Result = pstOpenInfo->pstTypeInfo->pfnGetSize(pstOpenInfo->hResource);
  }

  /* Done! */
  return s32Result;
}

/** Seeks a position in a given resource (moves cursor)
 * @param[in] _hResource        Concerned resource
 * @param[in] _s32Offset        Number of bytes to offset from 'origin'
 * @param[in] _eWhence          Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor position
*/
orxS32 orxFASTCALL orxResource_Seek(orxHANDLE _hResource, orxS32 _s32Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_eWhence < orxSEEK_OFFSET_WHENCE_NUMBER);

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Updates result */
    s32Result = pstOpenInfo->pstTypeInfo->pfnSeek(pstOpenInfo->hResource, _s32Offset, _eWhence);
  }

  /* Done! */
  return s32Result;
}

/** Tells the position of the cursor in a given resource
 * @param[in] _hResource        Concerned resource
 * @return Position (offset), in bytes
 */
orxS32 orxFASTCALL orxResource_Tell(orxHANDLE _hResource)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Updates result */
    s32Result = pstOpenInfo->pstTypeInfo->pfnTell(pstOpenInfo->hResource);
  }

  /* Done! */
  return s32Result;
}

/** Reads data from a resource
 * @param[in] _hResource        Concerned resource
 * @param[in] _s32Size          Size to read (in bytes)
 * @param[out] _pBuffer         Buffer that will be filled by the read data
 * @return Size of the read data, in bytes
 */
orxS32 orxFASTCALL orxResource_Read(orxHANDLE _hResource, orxS32 _s32Size, void *_pBuffer)
{
  orxS32 s32Result = 0;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));
  orxASSERT(_s32Size >= 0);
  orxASSERT(_pBuffer != orxNULL);

  /* Valid? */
  if((_hResource != orxHANDLE_UNDEFINED) && (_hResource != orxNULL))
  {
    orxRESOURCE_OPEN_INFO *pstOpenInfo;

    /* Gets open info */
    pstOpenInfo = (orxRESOURCE_OPEN_INFO *)_hResource;

    /* Updates result */
    s32Result = pstOpenInfo->pstTypeInfo->pfnRead(pstOpenInfo->hResource, _s32Size, _pBuffer);
  }

  /* Done! */
  return s32Result;
}

/** Registers a new resource type
 * @param[in] _pstInfo          Info describing the new resource type and how to handle it
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
const orxSTATUS orxFASTCALL orxResource_RegisterType(const orxRESOURCE_TYPE_INFO *_pstInfo)
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
    orxRESOURCE_TYPE_INFO *pstTypeInfo;

    /* For all registered types */
    for(pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, orxNULL);
        (pstTypeInfo != orxNULL) && (orxString_ICompare(pstTypeInfo->zTag, _pstInfo->zTag) != 0);
        pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_GetNext(sstResource.pstTypeInfoBank, pstTypeInfo));

    /* Not already registered? */
    if(pstTypeInfo == orxNULL)
    {
      /* Allocates info */
      pstTypeInfo = (orxRESOURCE_TYPE_INFO *)orxBank_Allocate(sstResource.pstTypeInfoBank);

      /* Checks */
      orxASSERT(pstTypeInfo != orxNULL);

      /* Inits it */
      orxMemory_Copy(pstTypeInfo, _pstInfo, sizeof(orxRESOURCE_TYPE_INFO));
      pstTypeInfo->zTag = orxString_Duplicate(_pstInfo->zTag);

      /* Checks */
      orxASSERT(pstTypeInfo->zTag != orxNULL);

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

/** Clears cache
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxResource_ClearCache()
{
  orxRESOURCE_GROUP  *pstGroup;
  orxSTATUS           eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(orxFLAG_TEST(sstResource.u32Flags, orxRESOURCE_KU32_STATIC_FLAG_READY));

  /* For all groups */
  for(pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, orxNULL);
      pstGroup != orxNULL;
      pstGroup = (orxRESOURCE_GROUP *)orxBank_GetNext(sstResource.pstGroupBank, pstGroup))
  {
    orxHANDLE         hIterator;
    orxU32            u32Key;
    orxRESOURCE_LOCATION *pstResourceInfo;

    /* For all cached resources */
    for(hIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, orxHANDLE_UNDEFINED, &u32Key, (void **)&pstResourceInfo);
        hIterator != orxHANDLE_UNDEFINED;
        hIterator = orxHashTable_GetNext(pstGroup->pstCacheTable, hIterator, &u32Key, (void **)&pstResourceInfo))
    {
      /* Deletes its location */
      orxMemory_Free(pstResourceInfo->zLocation);
    }

    /* Clears cache table */
    orxHashTable_Clear(pstGroup->pstCacheTable);
  }

  /* Clears info bank */
  orxBank_Clear(sstResource.pstResourceInfoBank);

  /* Done! */
  return eResult;
}

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
 * @file orxResource.h
 * @date 20/01/2013
 * @author iarwain@orx-project.org
 *
 */

/**
 * @addtogroup orxResource
 *
 * Resource module
 * Module that handles resource mapping
 * @{
 */

#ifndef _orxRESOURCE_H_
#define _orxRESOURCE_H_


#include "orxInclude.h"


#define orxRESOURCE_KC_LOCATION_SEPARATOR                 ':'


/** Resource asynchronous operation callback function
 */
typedef void (orxFASTCALL *orxRESOURCE_OP_FUNCTION)(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, void *_pContext);


/** Resource handlers
 */
typedef const orxSTRING (orxFASTCALL *orxRESOURCE_FUNCTION_LOCATE)(const orxSTRING _zStorage, const orxSTRING _zName, orxBOOL _bRequireExistence);
typedef orxS64          (orxFASTCALL *orxRESOURCE_FUNCTION_GET_TIME)(const orxSTRING _zPath);
typedef orxHANDLE       (orxFASTCALL *orxRESOURCE_FUNCTION_OPEN)(const orxSTRING _zPath, orxBOOL _bEraseMode);
typedef void            (orxFASTCALL *orxRESOURCE_FUNCTION_CLOSE)(orxHANDLE _hResource);
typedef orxS64          (orxFASTCALL *orxRESOURCE_FUNCTION_GET_SIZE)(orxHANDLE _hResource);
typedef orxS64          (orxFASTCALL *orxRESOURCE_FUNCTION_SEEK)(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence);
typedef orxS64          (orxFASTCALL *orxRESOURCE_FUNCTION_TELL)(orxHANDLE _hResource);
typedef orxS64          (orxFASTCALL *orxRESOURCE_FUNCTION_READ)(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer);
typedef orxS64          (orxFASTCALL *orxRESOURCE_FUNCTION_WRITE)(orxHANDLE _hResource, orxS64 _s64Size, const void *_pBuffer);

/** Resource type info
 */
typedef struct __orxRESOURCE_TYPE_INFO_t
{
  const orxSTRING               zTag;                     /**< Unique tag, mandatory */
  orxRESOURCE_FUNCTION_LOCATE   pfnLocate;                /**< Locate function, mandatory */
  orxRESOURCE_FUNCTION_GET_TIME pfnGetTime;               /**< GetTime function, optional, for hotload support */
  orxRESOURCE_FUNCTION_OPEN     pfnOpen;                  /**< Open function, mandatory */
  orxRESOURCE_FUNCTION_CLOSE    pfnClose;                 /**< Close function, mandatory */
  orxRESOURCE_FUNCTION_GET_SIZE pfnGetSize;               /**< GetSize function, mandatory */
  orxRESOURCE_FUNCTION_SEEK     pfnSeek;                  /**< Seek function, mandatory */
  orxRESOURCE_FUNCTION_TELL     pfnTell;                  /**< Tell function, mandatory */
  orxRESOURCE_FUNCTION_READ     pfnRead;                  /**< Read function, mandatory */
  orxRESOURCE_FUNCTION_WRITE    pfnWrite;                 /**< Write function, optional, for write support */

} orxRESOURCE_TYPE_INFO;

/** Event enum
 */
typedef enum __orxRESOURCE_EVENT_t
{
  orxRESOURCE_EVENT_UPDATE = 0,
  orxRESOURCE_EVENT_ADD,
  orxRESOURCE_EVENT_REMOVE,

  orxRESOURCE_EVENT_NUMBER,

  orxRESOURCE_EVENT_NONE = orxENUM_NONE

} orxRESOURCE_EVENT;

/** Event payload
 */
typedef struct __orxRESOURCE_EVENT_PAYLOAD_t
{
  orxS64                        s64Time;                  /**< New resource time : 8 */
  const orxSTRING               zLocation;                /**< Resource location : 12 / 16 */
  const orxRESOURCE_TYPE_INFO  *pstTypeInfo;              /**< Type info : 16 / 24 */
  orxU32                        u32GroupID;               /**< Group ID : 20 / 28 */
  orxU32                        u32NameID;                /**< Name ID : 24 / 32 */

} orxRESOURCE_EVENT_PAYLOAD;


/** Resource module setup
 */
extern orxDLLAPI void orxFASTCALL                         orxResource_Setup();

/** Inits the resource module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_Init();

/** Exits from the resource module
 */
extern orxDLLAPI void orxFASTCALL                         orxResource_Exit();


/** Gets number of resource groups
 * @return Number of resource groups
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxResource_GetGroupCounter();

/** Gets resource group at given index
 * @param[in] _u32Index         Index of resource group
 * @return Resource group if index is valid, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetGroup(orxU32 _u32Index);


/** Adds a storage for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zStorage         Description of the storage, as understood by one of the resource type
 * @param[in] _bAddFirst        If true this storage will be used *before* any already added ones, otherwise it'll be used *after* all those
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_AddStorage(const orxSTRING _zGroup, const orxSTRING _zStorage, orxBOOL _bAddFirst);

/** Removes a storage for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zStorage         Concerned storage
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_RemoveStorage(const orxSTRING _zGroup, const orxSTRING _zStorage);

/** Gets number of storages for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @return Number of storages for this resource group
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxResource_GetStorageCounter(const orxSTRING _zGroup);

/** Gets storage at given index for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _u32Index         Index of storage
 * @return Storage if index is valid, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetStorage(const orxSTRING _zGroup, orxU32 _u32Index);

/** Reloads storage from config
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_ReloadStorage();


/** Gets the location of an *existing* resource for a given group, location gets cached if found
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zName            Name of the resource to locate
 * @return Location string if found, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_Locate(const orxSTRING _zGroup, const orxSTRING _zName);

/** Gets the location for a resource (existing or not) in a *specific storage*, for a given group. The location doesn't get cached and thus needs to be copied by the caller before the next call
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zStorage         Concerned storage, if orxNULL then the highest priority storage will be used
 * @param[in] _zName            Name of the resource
 * @return Location string if found, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_LocateInStorage(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName);

/** Gets the resource path from a location
 * @param[in] _zLocation        Location of the concerned resource
 * @return Path string if valid, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetPath(const orxSTRING _zLocation);

/** Gets the resource type from a location
 * @param[in] _zLocation        Location of the concerned resource
 * @return orxRESOURCE_TYPE_INFO if valid, orxNULL otherwise
 */
extern orxDLLAPI const orxRESOURCE_TYPE_INFO *orxFASTCALL orxResource_GetType(const orxSTRING _zLocation);


/** Gets the time of last modification of a resource
 * @param[in] _zLocation        Location of the concerned resource
 * @return Time of last modification, in seconds since epoch, if found, 0 otherwise
 */
extern orxDLLAPI orxS64 orxFASTCALL                       orxResource_GetTime(const orxSTRING _zLocation);

/** Opens the resource at the given location
 * @param[in] _zLocation        Location of the resource to open
 * @param[in] _bEraseMode       If true, the file will be erased if existing or created otherwise, if false, no content will get destroyed when opening
 * @return Handle to the open location, orxHANDLE_UNDEFINED otherwise
 */
extern orxDLLAPI orxHANDLE orxFASTCALL                    orxResource_Open(const orxSTRING _zLocation, orxBOOL _bEraseMode);

/** Closes a resource
 * @param[in] _hResource        Concerned resource
 */
extern orxDLLAPI void orxFASTCALL                         orxResource_Close(orxHANDLE _hResource);

/** Gets the literal location of a resource
 * @param[in] _hResource        Concerned resource
 * @return Literal location string
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetLocation(orxHANDLE _hResource);

/** Gets the size, in bytes, of a resource
 * @param[in] _hResource        Concerned resource
 * @return Size of the resource, in bytes
 */
extern orxDLLAPI orxS64 orxFASTCALL                       orxResource_GetSize(orxHANDLE _hResource);

/** Seeks a position in a given resource (moves cursor)
 * @param[in] _hResource        Concerned resource
 * @param[in] _s64Offset        Number of bytes to offset from 'origin'
 * @param[in] _eWhence          Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor position
 */
extern orxDLLAPI orxS64 orxFASTCALL                       orxResource_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence);

/** Tells the position of the cursor in a given resource
 * @param[in] _hResource        Concerned resource
 * @return Position (offset), in bytes
 */
extern orxDLLAPI orxS64 orxFASTCALL                       orxResource_Tell(orxHANDLE _hResource);

/** Reads data from a resource
 * @param[in] _hResource        Concerned resource
 * @param[in] _s64Size          Size to read (in bytes)
 * @param[out] _pBuffer         Buffer that will be filled by the read data
 * @param[in] _pfnCallback      Callback that will get called after asynchronous operation; if orxNULL, operation will be synchronous
 * @param[in] _pContext         Context that will be transmitted to the callback when called
 * @return Size of the read data, in bytes or -1 for successful asynchronous call
 */
extern orxDLLAPI orxS64 orxFASTCALL                       orxResource_Read(orxHANDLE _hResource, orxS64 _s64Size, void *_pBuffer, orxRESOURCE_OP_FUNCTION _pfnCallback, void *_pContext);

/** Writes data to a resource
 * @param[in] _hResource        Concerned resource
 * @param[in] _s64Size          Size to write (in bytes)
 * @param[out] _pBuffer         Buffer that will be written
 * @param[in] _pfnCallback      Callback that will get called after asynchronous operation; if orxNULL, operation will be synchronous
 * @param[in] _pContext         Context that will be transmitted to the callback when called
 * @return Size of the written data, in bytes, 0 if nothing could be written/no write support for this resource type or -1 for successful asynchronous call
 */
extern orxDLLAPI orxS64 orxFASTCALL                       orxResource_Write(orxHANDLE _hResource, orxS64 _s64Size, const void *_pBuffer, orxRESOURCE_OP_FUNCTION _pfnCallback, void *_pContext);


/** Gets pending operation counter for a given resource
 * @param[in] _hResource        Concerned resource
 * @return Number of pending asynchronous operations for that resource
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxResource_GetPendingOpCounter(const orxHANDLE _hResource);

/** Gets total pending operation counter
 * @return Number of total pending asynchronous operations
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxResource_GetTotalPendingOpCounter();


/** Registers a new resource type
 * @param[in] _pstInfo          Info describing the new resource type and how to handle it
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_RegisterType(const orxRESOURCE_TYPE_INFO *_pstInfo);

/** Gets number of registered resource types
 * @return Number of registered resource types
 */
extern orxDLLAPI orxU32 orxFASTCALL                       orxResource_GetTypeCounter(const orxSTRING _zGroup);

/** Gets registered type info at given index
 * @param[in] _u32Index         Index of storage
 * @return Type tag string if index is valid, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetTypeTag(orxU32 _u32Index);


/** Clears cache
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_ClearCache();


#endif /* _orxRESOURCE_H_ */

/** @} */

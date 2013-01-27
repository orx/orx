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


/** Resource handlers
 */
typedef const orxSTRING (orxFASTCALL *orxRESOURCE_FUNCTION_LOCATE)(const orxSTRING _zStorage, const orxSTRING _zName);
typedef orxHANDLE       (orxFASTCALL *orxRESOURCE_FUNCTION_OPEN)(const orxSTRING _zLocation);
typedef void            (orxFASTCALL *orxRESOURCE_FUNCTION_CLOSE)(orxHANDLE _hResource);
typedef orxS32          (orxFASTCALL *orxRESOURCE_FUNCTION_GET_SIZE)(orxHANDLE _hResource);
typedef orxS32          (orxFASTCALL *orxRESOURCE_FUNCTION_SEEK)(orxHANDLE _hResource, orxS32 _s32Offset, orxSEEK_OFFSET_WHENCE _eWhence);
typedef orxS32          (orxFASTCALL *orxRESOURCE_FUNCTION_TELL)(orxHANDLE _hResource);
typedef orxS32          (orxFASTCALL *orxRESOURCE_FUNCTION_READ)(orxHANDLE _hResource, orxS32 _s32Size, void *_pBuffer);

/** Resource type info
 */
typedef struct __orxRESOURCE_TYPE_INFO_t
{
  orxSTRING                     zTag;
  orxRESOURCE_FUNCTION_LOCATE   pfnLocate;
  orxRESOURCE_FUNCTION_OPEN     pfnOpen;
  orxRESOURCE_FUNCTION_CLOSE    pfnClose;
  orxRESOURCE_FUNCTION_GET_SIZE pfnGetSize;
  orxRESOURCE_FUNCTION_SEEK     pfnSeek;
  orxRESOURCE_FUNCTION_TELL     pfnTell;
  orxRESOURCE_FUNCTION_READ     pfnRead;

} orxRESOURCE_TYPE_INFO;


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
extern orxDLLAPI orxS32 orxFASTCALL                       orxResource_GetStorageCounter(const orxSTRING _zGroup);

/** Gets storage at given index for a given resource group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _s32Index         Index of storage
 * @return Storage if index is valid, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetStorage(const orxSTRING _zGroup, orxS32 _s32Index);


/** Gets the location of a resource for a given group
 * @param[in] _zGroup           Concerned resource group
 * @param[in] _zName            Name of the resource to locate
 * @return Location string if found, orxNULL otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_Locate(const orxSTRING _zGroup, const orxSTRING _zName);

/** Gets the resource name from a location
 * @param[in] _zLocation        Location of the concerned resource
 * @return Name string if valid, orxSTRING_EMPTY otherwise
 */
extern orxDLLAPI const orxSTRING orxFASTCALL              orxResource_GetName(const orxSTRING _zLocation);


/** Opens the resource at the given location
 * @param[in] _zLocation        Location of the resource to open
 * @return Handle to the open location, orxHANDLE_UNDEFINED otherwise
 */
extern orxDLLAPI orxHANDLE orxFASTCALL                    orxResource_Open(const orxSTRING _zLocation);

/** Closes a resource
 * @param[in] _hResource        Concerned resource
 */
extern orxDLLAPI void orxFASTCALL                         orxResource_Close(orxHANDLE _hResource);

/** Gets the size, in bytes, of a resource
 * @param[in] _hResource        Concerned resource
 * @return Size of the resource, in bytes
 */
extern orxDLLAPI orxS32 orxFASTCALL                       orxResource_GetSize(orxHANDLE _hResource);

/** Seeks a position in a given resource (moves cursor)
 * @param[in] _hResource        Concerned resource
 * @param[in] _s32Offset        Number of bytes to offset from 'origin'
 * @param[in] _eWhence          Starting point for the offset computation (start, current position or end)
 * @return Absolute cursor position
 */
extern orxDLLAPI orxS32 orxFASTCALL                       orxResource_Seek(orxHANDLE _hResource, orxS32 _s32Offset, orxSEEK_OFFSET_WHENCE _eWhence);

/** Tells the position of the cursor in a given resource
 * @param[in] _hResource        Concerned resource
 * @return Position (offset), in bytes
 */
extern orxDLLAPI orxS32 orxFASTCALL                       orxResource_Tell(orxHANDLE _hResource);

/** Reads data from a resource
 * @param[in] _hResource        Concerned resource
 * @param[in] _s32Size          Size to read (in bytes)
 * @param[out] _pBuffer         Buffer that will be filled by the read data
 * @return Size of the read data, in bytes
 */
extern orxDLLAPI orxS32 orxFASTCALL                       orxResource_Read(orxHANDLE _hResource, orxS32 _s32Size, void *_pBuffer);


/** Registers a new resource type
 * @param[in] _pstInfo          Info describing the new resource type and how to handle it
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI const orxSTATUS orxFASTCALL              orxResource_RegisterType(const orxRESOURCE_TYPE_INFO *_pstInfo);


/** Clears cache
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                    orxResource_ClearCache();


#endif /* _orxRESOURCE_H_ */

/** @} */

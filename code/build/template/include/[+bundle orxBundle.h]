//! Includes

#ifndef _orxBUNDLE_H_
#define _orxBUNDLE_H_

#include "orx.h"


//! Prototypes

orxSTATUS orxFASTCALL                   orxBundle_Init();
void orxFASTCALL                        orxBundle_Exit();
orxBOOL orxFASTCALL                     orxBundle_IsProcessing();


//! Defines

#define orxBUNDLE_KZ_INCLUDE_PATH       "../include/"
#define orxBUNDLE_KZ_INCLUDE_FILENAME   "orxBundleData.inc"

#define orxBUNDLE_KZ_RESOURCE_STORAGE   "bundle:"
#define orxBUNDLE_KZ_RESOURCE_TAG       "bundle"

#define orxBUNDLE_KZ_CONFIG_SECTION     "Bundle"
#define orxBUNDLE_KZ_LOG_TAG            orxANSI_KZ_COLOR_FG_YELLOW "[BUNDLE] " orxANSI_KZ_COLOR_RESET
#define orxBUNDLE_KZ_RESOURCE_FORMAT    orxANSI_KZ_COLOR_FG_MAGENTA "[%s|%s]" orxANSI_KZ_COLOR_RESET
#define orxBUNDLE_KU32_BUFFER_SIZE      16384
#define orxBUNDLE_KU32_LINE_LENGTH      16
#define orxBUNDLE_KU32_TABLE_SIZE       256


#ifdef orxBUNDLE_IMPL


//! Variables / Structures

typedef struct BundleData
{
  const orxSTRING   zName;
  const orxU8      *pu8Buffer;
  const orxS64      s64Size;
} BundleData;

typedef struct BundleResource
{
  orxS64            s64Cursor;
  const BundleData *pstData;
} BundleResource;

#if __has_include(orxBUNDLE_KZ_INCLUDE_FILENAME)
  #include orxBUNDLE_KZ_INCLUDE_FILENAME
#endif // __has_include(orxBUNDLE_KZ_INCLUDE_FILENAME)

#ifdef orxBUNDLE_HAS_DATA
  static const orxU32       su32DataCount = orxARRAY_GET_ITEM_COUNT(sastDataList);
#else // orxBUNDLE_HAS_DATA
  static const BundleData  *sastDataList  = orxNULL;
  static const orxU32       su32DataCount = 0;
#endif // !orxBUNDLE_HAS_DATA

static orxHASHTABLE  *spstDataTable = orxNULL;
static orxBOOL        sbProcess     = orxFALSE;


//! Code

static orxSTATUS orxFASTCALL ProcessBundleParams(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  orxSTATUS eResult;

  // Valid?
  if(_u32ParamCount == 1)
  {
    // Updates status
    sbProcess = orxTRUE;

    // Updates result
    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    // Updates result
    eResult = orxSTATUS_FAILURE;
  }

  // Done!
  return eResult;
}

static orxINLINE const orxSTRING orxBundle_GetHumanReadableSize(orxS64 _s64Size, orxU32 _u32Decimals)
{
  static const orxSTRING  sazUnitList[] = {"b", "kb", "mb", "gb", "tb"};
  static orxCHAR          sacBuffer[16];
  orxU32                  u32UnitIndex;
  orxFLOAT                fSize;

  /* Finds best unit */
  for(u32UnitIndex = 0, fSize = orxS2F(_s64Size);
      (u32UnitIndex < orxARRAY_GET_ITEM_COUNT(sazUnitList) - 1) && (fSize > orx2F(1024.0f));
      u32UnitIndex++, fSize *= orx2F(1.0f/1024.0f))
    ;

  // Prints it
  sacBuffer[orxString_NPrint(sacBuffer, sizeof(sacBuffer) - 1, "%.*g %s", orxF2U(log10f(fSize) + _u32Decimals + 1), fSize, sazUnitList[u32UnitIndex])] = orxCHAR_NULL;

  // Done!
  return sacBuffer;
}

static orxINLINE orxSTATUS orxBundle_Process()
{
  orxFILE  *pstIncludeFile;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Opens include file
  pstIncludeFile = orxFile_Open(orxBUNDLE_KZ_INCLUDE_PATH orxBUNDLE_KZ_INCLUDE_FILENAME, orxFILE_KU32_FLAG_OPEN_WRITE | orxFILE_KU32_FLAG_OPEN_BINARY);

  // Success?
  if(pstIncludeFile != orxNULL)
  {
    typedef struct orxBUNDLE_RESOURCE_REF
    {
      orxSTRINGID stNameID;
      orxS64      s64Size;
    } orxBUNDLE_RESOURCE_REF;

    orxBANK *pstResourceBank;

    // Creates resource bank
    pstResourceBank = orxBank_Create(orxBUNDLE_KU32_TABLE_SIZE, sizeof(orxBUNDLE_RESOURCE_REF), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);

    // Success?
    if(pstResourceBank != orxNULL)
    {
      orxU32 i, iCount, u32GroupCount, u32ResourceIndex, u32ConfigHistoryExtensionLength;

      // Pushes current section
      orxConfig_PushSection(orxBUNDLE_KZ_CONFIG_SECTION);

      // Unregisters bundle type
      orxResource_UnregisterType(orxBUNDLE_KZ_RESOURCE_TAG);

      // Syncs all groups
      orxResource_Sync(orxNULL);

      // Reloads config history
      orxConfig_ReloadHistory();

      // Gets group count
      u32GroupCount = orxResource_GetGroupCount();

      // For all config sections
      for(i = 0, iCount = orxConfig_GetSectionCount(); i < iCount; i++)
      {
        orxU32 j, jCount;

        // Selects it
        orxConfig_SelectSection(orxConfig_GetSection(i));

        // For all keys
        for(j = 0, jCount = orxConfig_GetKeyCount(); j < jCount; j++)
        {
          const orxSTRING zKey;
          orxU32          k, kCount;

          // Gets it
          zKey = orxConfig_GetKey(j);

          // For all its values
          for(k = 0, kCount = orxConfig_GetListCount(zKey); k < kCount; k++)
          {
            const orxSTRING zValue;
            orxU32          u32GroupIndex;

            // Gets it
            zValue = orxConfig_GetListString(zKey, k);

            // For all resource groups
            for(u32GroupIndex = 0; u32GroupIndex < u32GroupCount; u32GroupIndex++)
            {
              const orxSTRING zGroup;

              // Gets group
              zGroup = orxResource_GetGroup(u32GroupIndex);

              // Locates it
              if(orxResource_Locate(zGroup, zValue) != orxNULL)
              {
                // Logs message
                orxLOG(orxBUNDLE_KZ_LOG_TAG "Discovered " orxBUNDLE_KZ_RESOURCE_FORMAT, zGroup, zValue);

                // Stops
                break;
              }
            }
          }
        }
      }

      // Gets config history file extension length
      u32ConfigHistoryExtensionLength = orxString_GetLength(orxCONSOLE_KZ_CONFIG_HISTORY_FILE_EXTENSION);

      // For all groups
      for(i = 0, iCount = orxResource_GetGroupCount(), u32ResourceIndex = 0; i < iCount; i++)
      {
        const orxSTRING zGroup;
        const orxSTRING zLocation;
        const orxSTRING zName;
        orxHANDLE       hIterator;

        // Gets group
        zGroup = orxResource_GetGroup(i);

        // For all cached resources
        for(hIterator = orxResource_GetNextCachedLocation(zGroup, orxHANDLE_UNDEFINED, &zLocation, &zName);
            hIterator != orxHANDLE_UNDEFINED;
            hIterator = orxResource_GetNextCachedLocation(zGroup, hIterator, &zLocation, &zName))
        {
          // Not a bundle?
          if(orxString_Compare(orxResource_GetType(zLocation)->zTag, orxBUNDLE_KZ_RESOURCE_TAG) != 0)
          {
            const orxSTRING zMatch;

            // Not a config history file?
            if(((zMatch = orxString_SearchString(zName, orxCONSOLE_KZ_CONFIG_HISTORY_FILE_EXTENSION)) == orxNULL)
            || (*(zMatch + u32ConfigHistoryExtensionLength) != orxCHAR_NULL))
            {
              orxHANDLE hResource;
              orxS64    s64Size;

              // Opens resource
              orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_SYSTEM, orxFALSE);
              hResource = orxResource_Open(zLocation, orxFALSE);
              orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_SYSTEM, orxTRUE);

              // Success?
              if((hResource != orxHANDLE_UNDEFINED) && ((s64Size = orxResource_GetSize(hResource)) > 0))
              {
                orxU8                   au8Buffer[orxBUNDLE_KU32_BUFFER_SIZE];
                orxS64                  s64ByteIndex;
                orxBUNDLE_RESOURCE_REF *pstResourceRef;

                // Adds ref
                pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_Allocate(pstResourceBank);
                orxASSERT(pstResourceRef != orxNULL);
                pstResourceRef->stNameID  = orxString_GetID(zName);
                pstResourceRef->s64Size   = s64Size;

                // Logs message
                orxLOG(orxBUNDLE_KZ_LOG_TAG "Bundling " orxBUNDLE_KZ_RESOURCE_FORMAT ", " orxANSI_KZ_COLOR_FG_GREEN "(%s)" orxANSI_KZ_COLOR_RESET, zGroup, zLocation, orxBundle_GetHumanReadableSize(s64Size, 2));

                // Outputs structure header
                orxFile_Print(pstIncludeFile, "static const orxU8 BundleData%u[] =\r\n{", u32ResourceIndex++);

                // While not empty
                for(s64Size = orxResource_Read(hResource, orxBUNDLE_KU32_BUFFER_SIZE, au8Buffer, orxNULL, orxNULL), s64ByteIndex = 0;
                    s64Size > 0;
                    s64Size = orxResource_Read(hResource, orxBUNDLE_KU32_BUFFER_SIZE, au8Buffer, orxNULL, orxNULL))
                {
                  orxS64 j;

                  // For all bytes
                  for(j = 0; j < s64Size; j++, s64ByteIndex++)
                  {
                    static const orxCHAR  acDigits[]      = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
                    static orxCHAR        acByteBuffer[]  = {' ', '0', 'x', '0', '0', ','};
                    orxU8                 u8Byte;

                    // New line?
                    if((s64ByteIndex % orxBUNDLE_KU32_LINE_LENGTH) == 0)
                    {
                      // Output it
                      orxFile_Write("\r\n ", 1, 3, pstIncludeFile);
                    }

                    // Outputs byte
                    u8Byte          = au8Buffer[j];
                    acByteBuffer[3] = acDigits[u8Byte >> 4];
                    acByteBuffer[4] = acDigits[u8Byte & 0x0F];
                    orxFile_Write(acByteBuffer, 1, 6, pstIncludeFile);
                  }
                }

                // Outputs structure footer
                if(s64Size > 0)
                {
                  orxFile_Seek(pstIncludeFile, -1, orxSEEK_OFFSET_WHENCE_CURRENT);
                }
                orxFile_Print(pstIncludeFile, "\r\n};\r\n\r\n");

                // Closes it
                orxResource_Close(hResource);
              }
            }
          }
        }
      }

      // Has data?
      if(orxBank_GetCount(pstResourceBank) != 0)
      {
        orxS64                  s64Size;
        orxBUNDLE_RESOURCE_REF *pstResourceRef;

        // Outputs table header
        orxFile_Print(pstIncludeFile, "static const BundleData sastDataList[] =\r\n{");

        // For all refs
        for(pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, orxNULL), u32ResourceIndex = 0, s64Size = 0;
            pstResourceRef != orxNULL;
            pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, pstResourceRef), u32ResourceIndex++)
        {
          const orxSTRING zName;

          // Gets its name
          zName = orxString_GetFromID(pstResourceRef->stNameID);

          // Outputs ref
          orxFile_Print(pstIncludeFile, "\r\n  {\"");
          for(i = 0, iCount = orxString_GetLength(zName); i < iCount; i++)
          {
            orxFile_Write(zName + i, 1, 1, pstIncludeFile);
            if(zName[i] == '\\')
            {
              orxFile_Write(zName + i, 1, 1, pstIncludeFile);
            }
          }
          orxFile_Print(pstIncludeFile, "\", (const orxU8 *)BundleData%u, %lld},", u32ResourceIndex, pstResourceRef->s64Size);

          // Updates size
          s64Size += pstResourceRef->s64Size;
        }

        // Outputs table footer
        orxFile_Seek(pstIncludeFile, -1, orxSEEK_OFFSET_WHENCE_CURRENT);
        orxFile_Print(pstIncludeFile, "\r\n};\r\n\r\n#define orxBUNDLE_HAS_DATA\r\n");

        // Logs message
        orxLOG(orxBUNDLE_KZ_LOG_TAG "Bundled " orxANSI_KZ_COLOR_FG_MAGENTA "[%u]" orxANSI_KZ_COLOR_RESET " resources to " orxANSI_KZ_COLOR_FG_MAGENTA orxBUNDLE_KZ_INCLUDE_PATH orxBUNDLE_KZ_INCLUDE_FILENAME orxANSI_KZ_COLOR_RESET ", " orxANSI_KZ_COLOR_FG_GREEN "(%s)" orxANSI_KZ_COLOR_RESET, orxBank_GetCount(pstResourceBank), orxBundle_GetHumanReadableSize(s64Size, 2));
      }

      // Pops resource config section
      orxConfig_PopSection();

      // Deletes resource bank
      orxBank_Delete(pstResourceBank);
    }
    else
    {
      // Logs message
      orxLOG(orxBUNDLE_KZ_LOG_TAG orxANSI_KZ_COLOR_FG_RED "Can't allocate internal structure, aborting!" orxANSI_KZ_COLOR_RESET);
    }

    // Closes include file
    orxFile_Close(pstIncludeFile);
  }
  else
  {
    // Logs message
    orxLOG(orxBUNDLE_KZ_LOG_TAG orxANSI_KZ_COLOR_FG_RED "Can't open include file " orxANSI_KZ_COLOR_RESET "[%s]" orxANSI_KZ_COLOR_FG_RED ", aborting!" orxANSI_KZ_COLOR_RESET, orxBUNDLE_KZ_INCLUDE_PATH orxBUNDLE_KZ_INCLUDE_FILENAME);
  }

  // Sends close event
  orxEvent_SendShort(orxEVENT_TYPE_SYSTEM, orxSYSTEM_EVENT_CLOSE);

  // Done!
  return eResult;
}

static orxSTATUS orxFASTCALL orxBundle_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Resource?
  if(_pstEvent->eType == orxEVENT_TYPE_RESOURCE)
  {
    // Inhibits it
    eResult = orxSTATUS_FAILURE;
  }
  // System
  else
  {
    // Param ready?
    if(_pstEvent->eID == orxSYSTEM_EVENT_PARAM_READY)
    {
      #define orxBUNDLE_DECLARE_PARAM(SN, LN, SD, LD, FN) {orxPARAM_KU32_FLAG_STOP_ON_ERROR, SN, LN, SD, LD, &FN}

      orxU32    i, iCount;
      orxPARAM  astParamList[] =
      {
        orxBUNDLE_DECLARE_PARAM("b", "bundle", "Create a resource bundle", "Create a bundle of all resources that are either defined in config or loaded at init time", ProcessBundleParams),
      };

      // For all params
      for(i = 0, iCount = orxARRAY_GET_ITEM_COUNT(astParamList); (i < iCount) && (eResult != orxSTATUS_FAILURE); i++)
      {
        // Registers it
        orxParam_Register(&astParamList[i]);
      }

      #undef orxBUNDLE_DECLARE_PARAM
    }
    else
    {
      // Should process?
      if(sbProcess != orxFALSE)
      {
        // Processes
        orxBundle_Process();

        // Updates result
        eResult = orxSTATUS_FAILURE;
      }
      else
      {
        // Updates result
        eResult = orxSTATUS_SUCCESS;
      }

      // Removes resource event handler
      orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxBundle_EventHandler);
    }
  }

  // Done!
  return eResult;
}

// Locate function, returns NULL if it can't handle the storage or if the resource can't be found in this storage
const orxSTRING orxFASTCALL orxBundle_Locate(const orxSTRING _zStorage, const orxSTRING _zResource, orxBOOL _bRequireExistence)
{
  static orxCHAR  sacBuffer[32];
  orxSTRING       zResult = orxNULL;

  // Has data?
  if(spstDataTable != orxNULL)
  {
    static orxU32 su32StorageLength = 0;
    if(su32StorageLength == 0)
    {
      su32StorageLength = orxString_GetLength(orxBUNDLE_KZ_RESOURCE_STORAGE);
    }

    // Bundle storage?
    if(orxString_NCompare(_zStorage, orxBUNDLE_KZ_RESOURCE_STORAGE, su32StorageLength) == 0)
    {
      orxSTRINGID   stResourceID;
      BundleData   *pstData;

      // Gets resource ID
      stResourceID = orxString_Hash(_zResource);

      // Gets resource
      pstData = (BundleData *)orxHashTable_Get(spstDataTable, stResourceID);

      // Found?
      if(pstData != orxNULL)
      {
        // Creates location string: index of file
        sacBuffer[orxString_NPrint(sacBuffer, sizeof(sacBuffer) - 1, "0x%x", (orxU32)(pstData - sastDataList))] = orxCHAR_NULL;

        // Updates result
        zResult = sacBuffer;
      }
    }
  }

  // Done!
  return zResult;
}

// Open function: returns an opaque handle for subsequent function calls (GetSize, Seek, Tell, Read and Close) upon success, orxHANDLE_UNDEFINED otherwise
orxHANDLE orxFASTCALL orxBundle_Open(const orxSTRING _zLocation, orxBOOL _bEraseMode)
{
  orxHANDLE hResult = orxHANDLE_UNDEFINED;

  // Not in erase mode?
  if(_bEraseMode == orxFALSE)
  {
    orxU32 u32Index;

    // Retrieves resource index
    if((orxString_ToU32(_zLocation, &u32Index, orxNULL) != orxSTATUS_FAILURE)
    && (u32Index < su32DataCount))
    {
      BundleResource *pstResource;

      // Allocates memory for our bundle resource
      pstResource = (BundleResource *)orxMemory_Allocate(sizeof(BundleResource), orxMEMORY_TYPE_MAIN);

      // Success?
      if(pstResource != orxNULL)
      {
        // Clears memory
        orxMemory_Zero(pstResource, sizeof(BundleResource));

        // Stores its data
        pstResource->pstData = &sastDataList[u32Index];

        // Inits read cursor
        pstResource->s64Cursor = 0;

        // Updates result
        hResult = (orxHANDLE)pstResource;
      }
    }
  }

  // Done!
  return hResult;
}

// Close function: releases all that has been allocated in Open
void orxFASTCALL orxBundle_Close(orxHANDLE _hResource)
{
  // Frees it
  orxMemory_Free((BundleResource *)_hResource);
}

// GetSize function: simply returns the size of the extracted resource, in bytes
orxS64 orxFASTCALL orxBundle_GetSize(orxHANDLE _hResource)
{
  orxS64 s64Result;

  // Updates result
  s64Result = ((BundleResource *)_hResource)->pstData->s64Size;

  // Done!
  return s64Result;
}

// Seek function: position the read cursor inside the data and returns the offset from start upon success or -1 upon failure
orxS64 orxFASTCALL orxBundle_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  BundleResource *pstResource;
  orxS64          s64Cursor;

  // Gets archive wrapper
  pstResource = (BundleResource *)_hResource;

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
    // Updates archive's cursor
    pstResource->s64Cursor = s64Cursor;
  }
  else
  {
    // Clears value
    s64Cursor = -1;
  }

  // Done!
  return s64Cursor;
}

// Tell function: returns current read cursor
orxS64 orxFASTCALL orxBundle_Tell(orxHANDLE _hResource)
{
  orxS64 s64Result;

  // Updates result
  s64Result = ((BundleResource *)_hResource)->s64Cursor;

  // Done!
  return s64Result;
}

// Read function: copies the requested amount of data, in bytes, to the given buffer and returns the amount of bytes copied
orxS64 orxFASTCALL orxBundle_Read(orxHANDLE _hResource, orxS64 _s64Size, void *_pu8Buffer)
{
  BundleResource *pstResource;
  orxS64          s64CopySize;

  // Gets resource
  pstResource = (BundleResource *)_hResource;

  // Gets actual copy size to prevent any out-of-bound access
  s64CopySize = orxMIN(_s64Size, pstResource->pstData->s64Size - pstResource->s64Cursor);

  // Should copy content?
  if(s64CopySize != 0)
  {
    // Copies content
    orxMemory_Copy(_pu8Buffer, pstResource->pstData->pu8Buffer + pstResource->s64Cursor, (orxS32)s64CopySize);
  }

  // Updates cursor
  pstResource->s64Cursor += s64CopySize;

  // Done!
  return s64CopySize;
}

orxSTATUS orxFASTCALL orxBundle_Init()
{
  orxRESOURCE_TYPE_INFO stInfo;
  orxSTATUS             eResult;

  // Inits variables
  sbProcess = orxFALSE;

  // Inits our zip resource wrapper
  orxMemory_Zero(&stInfo, sizeof(orxRESOURCE_TYPE_INFO));
  stInfo.zTag       = orxBUNDLE_KZ_RESOURCE_TAG;
  stInfo.pfnLocate  = &orxBundle_Locate;
  stInfo.pfnGetTime = orxNULL;                // No hotload support
  stInfo.pfnOpen    = &orxBundle_Open;
  stInfo.pfnClose   = &orxBundle_Close;
  stInfo.pfnGetSize = &orxBundle_GetSize;
  stInfo.pfnSeek    = &orxBundle_Seek;
  stInfo.pfnTell    = &orxBundle_Tell;
  stInfo.pfnRead    = &orxBundle_Read;
  stInfo.pfnWrite   = orxNULL;                // No write support
  stInfo.pfnDelete  = orxNULL;                // No delete support

  // Registers it
  eResult = orxResource_RegisterType(&stInfo);

  // Success?
  if(eResult != orxSTATUS_FAILURE)
  {
    // Has data?
    if(su32DataCount > 0)
    {
      // Creates data table
      spstDataTable = orxHashTable_Create(orxBUNDLE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

      // Success?
      if(spstDataTable != orxNULL)
      {
        orxU32 i;

        // For all bundled data
        for(i = 0; i < su32DataCount; i++)
        {
          orxSTATUS eResult;

          // Stores it
          eResult = orxHashTable_Add(spstDataTable, orxString_Hash(sastDataList[i].zName), (void *)&sastDataList[i]);

          // Checks
          orxASSERT(eResult != orxSTATUS_FAILURE);
        }
      }
    }
    else
    {
      // Clears data table
      spstDataTable = orxNULL;
    }

    // Registers event handler
    orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxBundle_EventHandler);
    orxEvent_SetHandlerIDFlags(orxBundle_EventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_PARAM_READY) | orxEVENT_GET_FLAG(orxSYSTEM_EVENT_PARAM_DISPLAY), orxEVENT_KU32_MASK_ID_ALL);
    orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxBundle_EventHandler);
    orxEvent_SetHandlerIDFlags(orxBundle_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_REMOVE), orxEVENT_KU32_MASK_ID_ALL);
  }

  // Done!
  return eResult;
}

void orxFASTCALL orxBundle_Exit()
{
  // Unregisters event handler
  orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxBundle_EventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxBundle_EventHandler);

  // Has data table?
  if(spstDataTable != orxNULL)
  {
    // Deletes it
    orxHashTable_Delete(spstDataTable);
    spstDataTable = orxNULL;
  }
}

orxBOOL orxFASTCALL orxBundle_IsProcessing()
{
  // Done!
  return sbProcess;
}

#endif // orxBUNDLE_IMPL

#endif // _orxBUNDLE_H_

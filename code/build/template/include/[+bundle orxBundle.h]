//! Includes

#ifndef _orxBUNDLE_H_
#define _orxBUNDLE_H_

#include "orx.h"

#define LZ4_FREESTANDING                    1

#define LZ4_memmove(dst, src, size)         orxMemory_Move((dst), (src), (orxU32)(size))
#define LZ4_memcpy(dst, src, size)          orxMemory_Copy((dst), (src), (orxU32)(size))
#define LZ4_memset(dst, data, size)         orxMemory_Set((dst), (data), (orxU32)(size))

#include "lz4/lz4.c"
#include "lz4/lz4hc.c"

#undef LZ4_FREESTANDING


//! Prototypes

orxSTATUS orxFASTCALL                       orxBundle_Init();
void orxFASTCALL                            orxBundle_Exit();
orxBOOL orxFASTCALL                         orxBundle_IsProcessing();
const orxSTRING orxFASTCALL                 orxBundle_GetOutputName();


//! Defines

#define orxBUNDLE_KZ_INCLUDE_PATH           "../include/"
#define orxBUNDLE_KZ_INCLUDE_FILENAME       "orxBundleData.inc"

#define orxBUNDLE_KZ_RESOURCE_GROUP         "Bundle"
#define orxBUNDLE_KZ_RESOURCE_STORAGE       "bundle:"
#define orxBUNDLE_KZ_RESOURCE_TAG           "bundle"

#define orxBUNDLE_KZ_CONFIG_SECTION         "Bundle"
#define orxBUNDLE_KZ_CONFIG_INCLUDE_LIST    "IncludeList"
#define orxBUNDLE_KZ_CONFIG_EXCLUDE_LIST    "ExcludeList"
#define orxBUNDLE_KZ_LOG_TAG                orxANSI_KZ_COLOR_FG_YELLOW "[BUNDLE] " orxANSI_KZ_COLOR_RESET
#define orxBUNDLE_KZ_RESOURCE_FORMAT        orxANSI_KZ_COLOR_FG_MAGENTA "[%s|%s]" orxANSI_KZ_COLOR_RESET
#define orxBUNDLE_KU32_BUFFER_SIZE          16384
#define orxBUNDLE_KU32_LINE_LENGTH          16
#define orxBUNDLE_KU32_TABLE_SIZE           256
#define orxBUNDLE_KU32_TOC_SIZE             1024

#define orxBUNDLE_KZ_BINARY_TAG             "OBR1"
#define orxBUNDLE_KU32_HEADER_INTRO_SIZE    (4 + 4)
#define orxBUNDLE_KU32_HEADER_ENTRY_SIZE    (8 + 4 + 4 + 4)


#ifdef orxBUNDLE_IMPL

#if defined(__orxGCC__)

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wtautological-compare"

#elif defined(__orxLLVM__)

  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wtautological-compare"

#endif

//! Variables / Structures

typedef struct BundleData
{
  union
  {
    orxSTRINGID stNameID;
    orxHANDLE   hResource;
  };
  const orxU8  *pu8Buffer;
  orxS64        s64Size;
  orxS64        s64FinalSize;
} BundleData;

typedef struct BundleResource
{
  orxS64        s64Cursor;
  BundleData    stData;
  orxU8        *pu8FinalBuffer;
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

static orxCHAR        sacPrintBuffer[1024];
static orxHASHTABLE  *sapstResourceTableList[orxTHREAD_KU32_MAX_THREAD_NUMBER];
static orxHASHTABLE  *spstToCTable        = orxNULL;
static orxHASHTABLE  *spstDataTable       = orxNULL;
static orxHANDLE      shResource          = orxHANDLE_UNDEFINED;
static orxBOOL        sbProcess           = orxFALSE;


//! Helpers

#if defined(__orxGCC__) || defined(__orxLLVM__)
  #define orxResource_Print(RESOURCE, FORMAT, ...)                                                \
    do                                                                                            \
    {                                                                                             \
      orxS32 s32Count;                                                                            \
      s32Count = orxString_NPrint(sacPrintBuffer, sizeof(sacPrintBuffer), FORMAT, ##__VA_ARGS__); \
      orxResource_Write(RESOURCE, (orxS64)s32Count, sacPrintBuffer, orxNULL, orxNULL);            \
    } while(orxFALSE)
#else // __orxGCC__ || __orxLLVM__
  #define orxResource_Print(RESOURCE, FORMAT, ...)                                                \
    do                                                                                            \
    {                                                                                             \
      orxS32 s32Count;                                                                            \
      s32Count = orxString_NPrint(sacPrintBuffer, sizeof(sacPrintBuffer), FORMAT, __VA_ARGS__);   \
      orxResource_Write(RESOURCE, (orxS64)s32Count, sacPrintBuffer, orxNULL, orxNULL);            \
    } while(orxFALSE)
#endif // __orxGCC__ || __orxLLVM__

static orxINLINE void orxResource_WriteU32(orxHANDLE _hResource, orxU32 _u32Value)
{
  orxU8 u8Value;

  u8Value = _u32Value & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u32Value >> 8) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u32Value >> 16) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u32Value >> 24) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);

  // Done!
  return;
}

static orxINLINE void orxResource_WriteU64(orxHANDLE _hResource, orxU64 _u64Value)
{
  orxU8 u8Value;

  u8Value = _u64Value & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 8) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 16) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 24) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 32) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 40) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 48) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u8Value = (_u64Value >> 56) & 0xFF;
  orxResource_Write(_hResource, 1, &u8Value, orxNULL, orxNULL);

  // Done!
  return;
}

static orxINLINE orxU32 orxResource_ReadU32(orxHANDLE _hResource)
{
  orxU32  u32Result = 0;
  orxU8   u8Value;

  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u32Result |= u8Value;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u32Result |= ((orxU32)u8Value) << 8;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u32Result |= ((orxU32)u8Value) << 16;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u32Result |= ((orxU32)u8Value) << 24;

  // Done!
  return u32Result;
}

static orxINLINE orxU64 orxResource_ReadU64(orxHANDLE _hResource)
{
  orxU64  u64Result = 0;
  orxU8   u8Value;

  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= u8Value;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 8;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 16;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 24;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 32;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 40;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 48;
  orxResource_Read(_hResource, 1, &u8Value, orxNULL, orxNULL);
  u64Result |= ((orxU64)u8Value) << 56;

  // Done!
  return u64Result;
}


//! Code

void orxFASTCALL orxBundle_CommandIsProcessing(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  // Updates result
  _pstResult->bValue = orxBundle_IsProcessing();

  // Done!
  return;
}

void orxFASTCALL orxBundle_CommandGetOutputName(orxU32 _u32ArgNumber, const orxCOMMAND_VAR *_astArgList, orxCOMMAND_VAR *_pstResult)
{
  // Updates result
  _pstResult->zValue = orxBundle_GetOutputName();

  // Done!
  return;
}

static orxINLINE void orxBundle_ClearResourceTable(orxHASHTABLE *_pstResourceTable)
{
  orxHANDLE hIterator, hResource;

  // For all resources
  for(hIterator = orxHashTable_GetNext(_pstResourceTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&hResource);
      hIterator != orxHANDLE_UNDEFINED;
      hIterator = orxHashTable_GetNext(_pstResourceTable, hIterator, orxNULL, (void **)&hResource))
  {
    // Closes it
    orxResource_Close(hResource);
  }

  // Clears table
  orxHashTable_Clear(_pstResourceTable);

  // Done!
  return;
}

static orxINLINE void orxBundle_ClearResourceTables()
{
  orxU32 i;

  // Checks
  orxASSERT(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID);

  // For all resource tables
  for(i = 0; i < orxARRAY_GET_ITEM_COUNT(sapstResourceTableList); i++)
  {
    // Clears it
    orxBundle_ClearResourceTable(sapstResourceTableList[i]);
  }

  // Done!
  return;
}

static orxINLINE void orxBundle_ClearToCTable()
{
  orxHANDLE     hIterator;
  orxHASHTABLE *pstToC;

  // Checks
  orxASSERT(orxThread_GetCurrent() == orxTHREAD_KU32_MAIN_THREAD_ID);

  // For all ToCs
  for(hIterator = orxHashTable_GetNext(spstToCTable, orxHANDLE_UNDEFINED, orxNULL, (void **)&pstToC);
      hIterator != orxHANDLE_UNDEFINED;
      hIterator = orxHashTable_GetNext(spstToCTable, hIterator, orxNULL, (void **)&pstToC))
  {
    // Valid?
    if(pstToC != orxNULL)
    {
      // Deletes it
      orxHashTable_Delete(pstToC);
    }
  }

  // Clears ToC table
  orxHashTable_Clear(spstToCTable);

  // Done!
  return;
}

static orxSTATUS orxFASTCALL orxBundle_BundleParamHandler(orxU32 _u32ParamCount, const orxSTRING _azParams[])
{
  const orxSTRING zLocation;
  orxU32          u32DebugFlags;
  orxSTATUS       eResult;

  // Depending on parameters
  switch(_u32ParamCount)
  {
    default:
    {
      // Updates result
      eResult = orxSTATUS_FAILURE;

      // Locates resource
      zLocation = orxResource_LocateInStorage(orxBUNDLE_KZ_RESOURCE_GROUP, orxRESOURCE_KZ_DEFAULT_STORAGE, _azParams[1]);

      // Success?
      if(zLocation != orxNULL)
      {
        // Opens resource
        shResource = orxResource_Open(zLocation, orxTRUE);

        // Success?
        if(shResource != orxHANDLE_UNDEFINED)
        {
          // Updates result
          eResult = orxSTATUS_SUCCESS;
        }
      }

      // Failure?
      if(eResult == orxSTATUS_FAILURE)
      {
        // Stops
        break;
      }

      // Fall through
    }

    case 1:
    {
      // Updates status
      sbProcess = orxTRUE;

      // Updates debug flags
      u32DebugFlags = orxDEBUG_GET_FLAGS();
      orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_NONE, orxDEBUG_KU32_STATIC_FLAG_TYPE|orxDEBUG_KU32_STATIC_FLAG_TAGGED);

      // Logs message
      orxLOG(orxBUNDLE_KZ_LOG_TAG "Processing | " orxANSI_KZ_COLOR_FG_YELLOW "Please do any resource-related config generation in your init() function!" orxANSI_KZ_COLOR_RESET);

      // Unregisters bundle type
      orxResource_UnregisterType(orxBUNDLE_KZ_RESOURCE_TAG);

      // Clears resource tables
      orxBundle_ClearResourceTables();

      // Clears ToC table
      orxBundle_ClearToCTable();

      // Syncs all groups
      orxResource_Sync(orxNULL);

      // Clears config group
      orxResource_ClearCache(orxCONFIG_KZ_RESOURCE_GROUP);

      // Reloads config history
      orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, orxFALSE);
      orxConfig_ReloadHistory();
      orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_CONFIG, orxTRUE);

      // Reloads storages
      orxResource_ReloadStorage();

      // Updates debug flags
      orxDEBUG_SET_FLAGS(u32DebugFlags, orxDEBUG_KU32_STATIC_FLAG_NONE);

      // Updates result
      eResult = orxSTATUS_SUCCESS;
      break;
    }
  }

  // Done!
  return eResult;
}

static orxINLINE const orxSTRING orxBundle_GetHumanReadableSize(orxS64 _s64Size, orxU32 _u32Decimals)
{
  static const orxSTRING  sazUnitList[] = {"B", "KB", "MB", "GB", "TB"};
  static orxCHAR          sacBuffer[16];
  orxU32                  u32UnitIndex;
  orxFLOAT                fSize;

  /* Finds best unit */
  for(u32UnitIndex = 0, fSize = orxS2F(_s64Size);
      (u32UnitIndex < orxARRAY_GET_ITEM_COUNT(sazUnitList) - 1) && (fSize > orx2F(1024.0f));
      u32UnitIndex++, fSize *= orx2F(1.0f/1024.0f))
    ;

  // Prints it
  orxString_NPrint(sacBuffer, sizeof(sacBuffer), "%.*g%s", orxF2U(log10f(fSize) + _u32Decimals + 1), fSize, sazUnitList[u32UnitIndex]);

  // Done!
  return sacBuffer;
}

static orxINLINE orxSTATUS orxBundle_Process()
{
  orxHANDLE hOutput;
  orxU32    u32DebugFlags;
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  // Waits until all pending operations are over
  while(orxResource_GetTotalPendingOpCount() != 0)
    ;

  // Updates debug flag
  u32DebugFlags = orxDEBUG_GET_FLAGS();
  orxDEBUG_SET_FLAGS(orxDEBUG_KU32_STATIC_FLAG_NONE, orxDEBUG_KU32_STATIC_FLAG_TYPE|orxDEBUG_KU32_STATIC_FLAG_TAGGED);

  // Opens output resource
  hOutput = (shResource != orxHANDLE_UNDEFINED) ? shResource : orxResource_Open(orxResource_LocateInStorage(orxBUNDLE_KZ_RESOURCE_GROUP, orxNULL, orxBUNDLE_KZ_INCLUDE_PATH orxBUNDLE_KZ_INCLUDE_FILENAME), orxTRUE);

  // Success?
  if(hOutput != orxHANDLE_UNDEFINED)
  {
    typedef struct orxBUNDLE_RESOURCE_REF
    {
      orxSTRINGID     stNameID;
      const orxSTRING zLocation;
      const orxSTRING zGroup;
      const orxSTRING zRule;
      orxS64          s64Size;
      orxS64          s64FinalSize;
    } orxBUNDLE_RESOURCE_REF;

    struct
    {
      const orxSTRING zKey;
      const orxSTRING zQualifier;
      void *          pValue;
    } astRuleInfoList[] =
    {
      {orxBUNDLE_KZ_CONFIG_EXCLUDE_LIST, orxANSI_KZ_COLOR_FG_RED "-" orxANSI_KZ_COLOR_RESET,    (void *)orxSTRING_FALSE},
      {orxBUNDLE_KZ_CONFIG_INCLUDE_LIST, orxANSI_KZ_COLOR_FG_GREEN "+" orxANSI_KZ_COLOR_RESET,  (void *)orxSTRING_TRUE}
    };

    orxDOUBLE     dBeginTime, dEndTime;
    orxBANK      *pstResourceBank;
    orxHASHTABLE *pstRuleTable, *pstDiscoveryTable;
    orxU32        i, j, iCount, jCount, u32GroupCount, u32ConfigHistoryExtensionLength;
    orxBOOL       bBinary;

    // Gets begin time
    dBeginTime = orxSystem_GetSystemTime();

    // Creates resource bank
    pstResourceBank = orxBank_Create(orxBUNDLE_KU32_TABLE_SIZE, sizeof(orxBUNDLE_RESOURCE_REF), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);
    orxASSERT(pstResourceBank != orxNULL);

    // Creates rule & discovery tables
    pstRuleTable = orxHashTable_Create(orxBUNDLE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);
    orxASSERT(pstRuleTable != orxNULL);
    pstDiscoveryTable = orxHashTable_Create(orxBUNDLE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_TEMP);
    orxASSERT(pstDiscoveryTable != orxNULL);

    // Updates status
    bBinary = (hOutput == shResource) ? orxTRUE : orxFALSE;

    // Pushes bundle section
    orxConfig_PushSection(orxBUNDLE_KZ_CONFIG_SECTION);

    // For all rule lists
    for(i = 0, iCount = orxARRAY_GET_ITEM_COUNT(astRuleInfoList); i < iCount; i++)
    {
      // For all list entries
      for(j = 0, jCount = (orxU32)orxConfig_GetListCount(astRuleInfoList[i].zKey); j < jCount; j++)
      {
        const orxSTRING zRule;

        // Gets it
        zRule = orxConfig_GetListString(astRuleInfoList[i].zKey, (orxS32)j);

        // Adds it to the rule table
        *orxHashTable_Retrieve(pstRuleTable, orxString_Hash(zRule)) = astRuleInfoList[i].pValue;

        // Logs message
        orxLOG(orxBUNDLE_KZ_LOG_TAG "Applying rule %s" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_RESET, astRuleInfoList[i].zQualifier, zRule);
      }
    }

    // Gets group count
    u32GroupCount = orxResource_GetGroupCount();

    // For all config sections
    for(i = 0, iCount = orxConfig_GetSectionCount(); i < iCount; i++)
    {
      const orxSTRING zSection;

      // Gets it
      zSection = orxConfig_GetSection(i);

      // Not resource nor bundle?
      if((orxString_Compare(zSection, "Resource") != 0)
      && (orxString_Compare(zSection, orxBUNDLE_KZ_CONFIG_SECTION) != 0))
      {
        // Selects it
        orxConfig_SelectSection(zSection);

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
              const orxSTRING zLocation;

              // Gets group
              zGroup = orxResource_GetGroup(u32GroupIndex);

              // Locates it
              zLocation = orxResource_Locate(zGroup, zValue);

              // Valid?
              if(zLocation != orxNULL)
              {
                // Not an internal resource & not already discovered?
                if(((orxString_Compare(orxResource_GetType(zLocation)->zTag, orxRESOURCE_KZ_TYPE_TAG_MEMORY) != 0)
                 || (orxString_SearchString(zValue, "orx:") != zValue))
                && (orxHashTable_Add(pstDiscoveryTable, orxString_Hash(zLocation), (void *)orxTRUE) != orxSTATUS_FAILURE))
                {
                  // Logs message
                  orxLOG(orxBUNDLE_KZ_LOG_TAG "Discovered " orxBUNDLE_KZ_RESOURCE_FORMAT, zGroup, zValue);
                }

                // Stops
                break;
              }
            }
          }
        }
      }
    }

    // Gets config history file extension length
    u32ConfigHistoryExtensionLength = orxString_GetLength(orxCONSOLE_KZ_CONFIG_HISTORY_FILE_EXTENSION);

    // For all groups
    for(i = 0, iCount = orxResource_GetGroupCount(); i < iCount; i++)
    {
      const orxSTRING zGroup;
      const orxSTRING zLocation;
      const orxSTRING zStorage;
      const orxSTRING zName;
      orxHANDLE       hIterator;

      // Gets group
      zGroup = orxResource_GetGroup(i);

      // Not bundle?
      if(orxString_Compare(zGroup, orxBUNDLE_KZ_RESOURCE_GROUP) != 0)
      {
        // For all cached resources
        for(hIterator = orxResource_GetNextCachedLocation(zGroup, orxHANDLE_UNDEFINED, &zLocation, &zStorage, &zName);
            hIterator != orxHANDLE_UNDEFINED;
            hIterator = orxResource_GetNextCachedLocation(zGroup, hIterator, &zLocation, &zStorage, &zName))
        {
          const orxSTRING zTag;

          // Gets resource tag
          zTag = orxResource_GetType(zLocation)->zTag;

          // Not a bundle?
          if(orxString_Compare(zTag, orxBUNDLE_KZ_RESOURCE_TAG) != 0)
          {
            // Not an internal resource?
            if((orxString_Compare(zTag, orxRESOURCE_KZ_TYPE_TAG_MEMORY) != 0)
            || (orxString_SearchString(zName, "orx:") != zName))
            {
              const orxSTRING zMatch;

              // Not a config history file?
              if(((zMatch = orxString_SearchString(zName, orxCONSOLE_KZ_CONFIG_HISTORY_FILE_EXTENSION)) == orxNULL)
              || ((zMatch > zName) && (*(zMatch - 1) != '.'))
              || (*(zMatch + u32ConfigHistoryExtensionLength) != orxCHAR_NULL))
              {
                orxHANDLE hResource;
                orxS64    s64Size;

                // Opens resource
                orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_SYSTEM, orxFALSE);
                hResource = orxResource_Open(zLocation, orxFALSE);
                orxDEBUG_ENABLE_LEVEL(orxDEBUG_LEVEL_SYSTEM, orxTRUE);

                // Success?
                if(hResource != orxHANDLE_UNDEFINED)
                {
                  const orxSTRING azRuleList[] = {zName, zStorage, zGroup};
                  const orxSTRING zRule = orxNULL;
                  orxU32          j, jCount;
                  orxBOOL         bCollect;

                  // For all potential rules
                  for(j = 0, jCount = orxARRAY_GET_ITEM_COUNT(azRuleList), bCollect = orxTRUE; j < jCount; j++)
                  {
                    void *pValue;

                    // Has rule?
                    if((pValue = orxHashTable_Get(pstRuleTable, orxString_Hash(azRuleList[j]))) != orxNULL)
                    {
                      // Stores it
                      zRule = azRuleList[j];

                      // Updates status
                      bCollect = (pValue != (void *)orxSTRING_FALSE) ? orxTRUE : orxFALSE;
                      break;
                    }
                  }

                  // Should collect?
                  if(bCollect != orxFALSE)
                  {
                    // Isn't empty?
                    if((s64Size = orxResource_GetSize(hResource)) > 0)
                    {
                      orxBUNDLE_RESOURCE_REF *pstResourceRef;

                      // Adds ref
                      pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_Allocate(pstResourceBank);
                      orxASSERT(pstResourceRef != orxNULL);
                      pstResourceRef->stNameID        = orxString_GetID(zName);
                      pstResourceRef->zLocation       = zLocation;
                      pstResourceRef->zGroup          = zGroup;
                      pstResourceRef->zRule           = zRule;
                      pstResourceRef->s64Size         = 0;
                      pstResourceRef->s64FinalSize    = s64Size;
                    }
                  }
                  else
                  {
                    // Logs message
                    orxLOG(orxBUNDLE_KZ_LOG_TAG "Skipping " orxBUNDLE_KZ_RESOURCE_FORMAT " (rule " orxANSI_KZ_COLOR_FG_RED "-" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_RESET ")", zGroup, zName, zRule);
                  }

                  // Closes it
                  orxResource_Close(hResource);
                }
              }
            }
          }
        }
      }
    }

    // Has data?
    if(orxBank_GetCount(pstResourceBank) != 0)
    {
      orxBUNDLE_RESOURCE_REF *pstResourceRef, *pstNextResourceRef;
      orxU32                  u32HeaderSize = 0, u32ResourceIndex;

      // Binary output?
      if(bBinary != orxFALSE)
      {
        // Skips header size
        u32HeaderSize = orxBUNDLE_KU32_HEADER_INTRO_SIZE + orxBank_GetCount(pstResourceBank) * orxBUNDLE_KU32_HEADER_ENTRY_SIZE;
        orxResource_Seek(hOutput, u32HeaderSize, orxSEEK_OFFSET_WHENCE_START);
      }

      // For all refs
      for(pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, orxNULL), u32ResourceIndex = 0;
          pstResourceRef != orxNULL;
          pstResourceRef = pstNextResourceRef)
      {
        orxHANDLE hResource;
        orxU8    *pu8Buffer;

        // Gets next resource ref
        pstNextResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, pstResourceRef);

        // Allocates buffer
        pu8Buffer = (orxU8 *)orxMemory_Allocate((orxU32)(pstResourceRef->s64FinalSize), orxMEMORY_TYPE_TEMP);
        orxASSERT(pu8Buffer != orxNULL);

        // Has rule?
        if(pstResourceRef->zRule != orxNULL)
        {
          // Logs message
          orxLOG(orxBUNDLE_KZ_LOG_TAG "Bundling " orxBUNDLE_KZ_RESOURCE_FORMAT orxANSI_KZ_COLOR_FG_YELLOW " @0x%x" orxANSI_KZ_COLOR_RESET " (rule " orxANSI_KZ_COLOR_FG_GREEN "+" orxANSI_KZ_COLOR_FG_CYAN "%s" orxANSI_KZ_COLOR_RESET "), " orxANSI_KZ_COLOR_FG_GREEN "(%s)" orxANSI_KZ_COLOR_RESET,
                 pstResourceRef->zGroup,
                 orxString_GetFromID(pstResourceRef->stNameID),
                 u32ResourceIndex,
                 pstResourceRef->zRule,
                 orxBundle_GetHumanReadableSize(pstResourceRef->s64FinalSize, 2));
        }
        else
        {
          // Logs message
          orxLOG(orxBUNDLE_KZ_LOG_TAG "Bundling " orxBUNDLE_KZ_RESOURCE_FORMAT orxANSI_KZ_COLOR_FG_YELLOW " @0x%x" orxANSI_KZ_COLOR_RESET ", " orxANSI_KZ_COLOR_FG_GREEN "(%s)" orxANSI_KZ_COLOR_RESET,
                 pstResourceRef->zGroup,
                 orxString_GetFromID(pstResourceRef->stNameID),
                 u32ResourceIndex,
                 orxBundle_GetHumanReadableSize(pstResourceRef->s64FinalSize, 2));
        }

        // Gets internal resource
        hResource = orxResource_Open(pstResourceRef->zLocation, orxFALSE);

        // Reads data
        if((hResource != orxHANDLE_UNDEFINED)
        && (orxResource_Read(hResource, pstResourceRef->s64FinalSize, pu8Buffer, orxNULL, orxNULL) == pstResourceRef->s64FinalSize))
        {
          orxS32  s32CompressedSize;
          orxU8  *pu8CompressedBuffer;

          // Gets compressed buffer size
          s32CompressedSize = LZ4_compressBound((orxS32)pstResourceRef->s64FinalSize);

          // Allocates buffer
          pu8CompressedBuffer = (orxU8 *)orxMemory_Allocate(s32CompressedSize, orxMEMORY_TYPE_TEMP);
          orxASSERT(pu8CompressedBuffer != orxNULL);

          // Compresses it
          s32CompressedSize = (orxS32)LZ4_compress_HC((const char *)pu8Buffer, (char *)pu8CompressedBuffer, (int)pstResourceRef->s64FinalSize, (int)s32CompressedSize, LZ4HC_CLEVEL_MAX);

          // Success?
          if(s32CompressedSize > 0)
          {
            const orxSTRING zKey;
            const orxU8    *pu8Key;
            orxU8          *pu8Data;

            // Gets encryption key
            zKey = orxConfig_GetEncryptionKey();

            // Encrypts data
            for(pu8Key = (const orxU8 *)zKey, pu8Data = pu8CompressedBuffer;
                pu8Data < pu8CompressedBuffer + s32CompressedSize;
                pu8Key = (*pu8Key == orxCHAR_NULL) ? (const orxU8 *)zKey : pu8Key + 1, pu8Data++)
            {
              *pu8Data ^= *pu8Key;
            }

            // Binary output?
            if(bBinary != orxFALSE)
            {
              // Outputs resource
              orxResource_Write(hOutput, (orxS64)s32CompressedSize, pu8CompressedBuffer, orxNULL, orxNULL);
            }
            else
            {
              orxS32 s32Index;

              // Outputs structure header
              orxResource_Print(hOutput, "static const orxU8 BundleData0x%x[] =\r\n{", u32ResourceIndex);

              // For all bytes
              for(s32Index = 0; s32Index < s32CompressedSize; s32Index++)
              {
                static const orxCHAR  acDigits[]      = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
                static orxCHAR        acByteBuffer[]  = {' ', '0', 'x', '0', '0', ','};
                orxU8                 u8Byte;

                // New line?
                if((s32Index % orxBUNDLE_KU32_LINE_LENGTH) == 0)
                {
                  // Outputs it
                  orxResource_Print(hOutput, "\r\n ");
                }

                // Outputs byte
                u8Byte          = pu8CompressedBuffer[s32Index];
                acByteBuffer[3] = acDigits[u8Byte >> 4];
                acByteBuffer[4] = acDigits[u8Byte & 0x0F];
                orxResource_Write(hOutput, sizeof(acByteBuffer), acByteBuffer, orxNULL, orxNULL);
              }

              // Outputs structure footer
              if(s32Index > 0)
              {
                orxResource_Seek(hOutput, -1, orxSEEK_OFFSET_WHENCE_CURRENT);
              }
              orxResource_Print(hOutput, "\r\n};\r\n\r\n");
            }

            // Updates resource index
            u32ResourceIndex++;

            // Updates size
            pstResourceRef->s64Size = (orxS64)s32CompressedSize;
          }
          else
          {
            // Logs message
            orxLOG(orxBUNDLE_KZ_LOG_TAG "Failure, skipping!");

            // Removes it
            orxBank_Free(pstResourceBank, pstResourceRef);
          }

          // Frees buffer
          orxMemory_Free(pu8CompressedBuffer);
        }
        else
        {
          // Logs message
          orxLOG(orxBUNDLE_KZ_LOG_TAG "Failure, skipping!");

          // Removes it
          orxBank_Free(pstResourceBank, pstResourceRef);
        }

        // Frees buffer
        orxMemory_Free(pu8Buffer);

        // Closes resource
        if(hResource != orxHANDLE_UNDEFINED)
        {
          orxResource_Close(hResource);
        }
      }

      // Still has data?
      if(orxBank_GetCount(pstResourceBank) != 0)
      {
        orxBUNDLE_RESOURCE_REF *pstResourceRef;
        orxS64                  s64Size = 0, s64FinalSize = 0;
        orxCHAR                 acPrintBuffer[32];

        // Binary output?
        if(bBinary != orxFALSE)
        {
          // Inits header
          orxResource_Seek(hOutput, 0, orxSEEK_OFFSET_WHENCE_START);
          orxResource_Print(hOutput, orxBUNDLE_KZ_BINARY_TAG);
          orxResource_WriteU32(hOutput, orxBank_GetCount(pstResourceBank));

          // For all refs
          for(pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, orxNULL), u32ResourceIndex = 0;
              pstResourceRef != orxNULL;
              pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, pstResourceRef), u32ResourceIndex++)
          {
            // Valid?
            if(pstResourceRef->s64Size > 0)
            {
              //  Writes its header entry
              orxResource_WriteU64(hOutput, (orxU64)pstResourceRef->stNameID);
              orxResource_WriteU32(hOutput, u32HeaderSize + (orxU32)s64Size);
              orxResource_WriteU32(hOutput, (orxU32)pstResourceRef->s64Size);
              orxResource_WriteU32(hOutput, (orxU32)pstResourceRef->s64FinalSize);

              // Updates sizes
              s64Size      += pstResourceRef->s64Size;
              s64FinalSize += pstResourceRef->s64FinalSize;
            }
          }
        }
        else
        {
          // Outputs table header
          orxResource_Print(hOutput, "static const BundleData sastDataList[] =\r\n{");

          // For all refs
          for(pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, orxNULL), u32ResourceIndex = 0;
            pstResourceRef != orxNULL;
            pstResourceRef = (orxBUNDLE_RESOURCE_REF *)orxBank_GetNext(pstResourceBank, pstResourceRef), u32ResourceIndex++)
          {
            // Outputs ref
            orxResource_Print(hOutput, "\r\n  {0x%016llx /* %s */, (const orxU8 *)BundleData0x%x, %lld, %lld},", pstResourceRef->stNameID, orxString_GetFromID(pstResourceRef->stNameID), u32ResourceIndex, pstResourceRef->s64Size, pstResourceRef->s64FinalSize);

            // Updates sizes
            s64Size      += pstResourceRef->s64Size;
            s64FinalSize += pstResourceRef->s64FinalSize;
          }

          // Outputs table footer
          orxResource_Seek(hOutput, -1, orxSEEK_OFFSET_WHENCE_CURRENT);
          orxResource_Print(hOutput, "\r\n};\r\n\r\n#define orxBUNDLE_HAS_DATA\r\n");
        }

        // Gets end time
        dEndTime = orxSystem_GetSystemTime();

        // Logs message
        orxString_NPrint(acPrintBuffer, sizeof(acPrintBuffer), "%s", orxBundle_GetHumanReadableSize(s64FinalSize, 2));
        orxLOG(orxBUNDLE_KZ_LOG_TAG "Bundled " orxANSI_KZ_COLOR_FG_CYAN "%u" orxANSI_KZ_COLOR_RESET " resources into " orxANSI_KZ_COLOR_FG_MAGENTA "%s" orxANSI_KZ_COLOR_RESET " in " orxANSI_KZ_COLOR_FG_CYAN "%.2f" orxANSI_KZ_COLOR_RESET "s, " orxANSI_KZ_COLOR_FG_GREEN "(%s) -> [%s]" orxANSI_KZ_COLOR_FG_CYAN " (%.2f%%)" orxANSI_KZ_COLOR_RESET,
              u32ResourceIndex,
              orxResource_GetPath(orxResource_GetLocation(hOutput)),
              orx2F(dEndTime - dBeginTime),
              acPrintBuffer,
              orxBundle_GetHumanReadableSize(s64Size, 2),
              orx2F(100.0f) * orxS2F(s64Size) / orxS2F(s64FinalSize));
      }
    }

    // Pops config section
    orxConfig_PopSection();

    // Deletes rule & discovery tables
    orxHashTable_Delete(pstRuleTable);
    orxHashTable_Delete(pstDiscoveryTable);

    // Deletes resource bank
    orxBank_Delete(pstResourceBank);

    // Closes output resource
    orxResource_Close(hOutput);
    if(hOutput == shResource)
    {
      shResource = orxHANDLE_UNDEFINED;
    }
  }
  else
  {
    // Logs message
    orxLOG(orxBUNDLE_KZ_LOG_TAG orxANSI_KZ_COLOR_FG_RED "Can't open output file " orxANSI_KZ_COLOR_FG_MAGENTA "%s" orxANSI_KZ_COLOR_FG_RED ", aborting!" orxANSI_KZ_COLOR_RESET, orxBUNDLE_KZ_INCLUDE_PATH orxBUNDLE_KZ_INCLUDE_FILENAME);
  }

  // Updates debug flag
  orxDEBUG_SET_FLAGS(u32DebugFlags, orxDEBUG_KU32_STATIC_FLAG_NONE);

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
        orxBUNDLE_DECLARE_PARAM("b",
                                "bundle",
                                "Creates a resource bundle.",
                                "Creates a bundle of all resources that are either defined in config or loaded at init time.\n\t\t\tIf no output name is provided, code will be generated, ready to be used during the next compilation.",
                                orxBundle_BundleParamHandler),
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
      orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxBundle_EventHandler);
    }
  }

  // Done!
  return eResult;
}

// Locate function, returns NULL if it can't handle the storage or if the resource can't be found in this storage
const orxSTRING orxFASTCALL orxBundle_Locate(const orxSTRING _zGroup, const orxSTRING _zStorage, const orxSTRING _zName, orxBOOL _bRequireExistence)
{
  static orxCHAR  sacBuffer[512];
  static orxU32   su32StorageLength = 0;
  orxSTRING       zResult = orxNULL;

  if(su32StorageLength == 0)
  {
    su32StorageLength = orxString_GetLength(orxBUNDLE_KZ_RESOURCE_STORAGE);
  }

  // Bundle storage?
  if(orxString_NCompare(_zStorage, orxBUNDLE_KZ_RESOURCE_STORAGE, su32StorageLength) == 0)
  {
    const orxSTRING zResource;

    // Gets underlying resource
    zResource = _zStorage + su32StorageLength;

    // Not memory?
    if(*zResource != orxCHAR_NULL)
    {
      static orxBOOL  sbLocating = orxFALSE;
      const orxSTRING zLocation;

      // Not already locating?
      if(sbLocating == orxFALSE)
      {
        // Updates status
        sbLocating = orxTRUE;

        // Locates resource
        zLocation = orxResource_Locate(orxBUNDLE_KZ_RESOURCE_GROUP, zResource);

        // Found?
        if(zLocation != orxNULL)
        {
          orxHASHTABLE  **ppstToC;
          orxSTRINGID     stLocationID;

          // Gets its location ID
          stLocationID = orxString_Hash(zLocation);

          // Gets its ToC
          ppstToC = (orxHASHTABLE **)orxHashTable_Retrieve(spstToCTable, stLocationID);

          // Not found?
          if(*ppstToC == orxNULL)
          {
            orxHANDLE hResource;
            orxU32    u32ThreadID;

            // Gets current thread ID
            u32ThreadID = orxThread_GetCurrent();
            orxASSERT(u32ThreadID != orxU32_UNDEFINED);

            // Gets it from table
            hResource = orxHashTable_Get(sapstResourceTableList[u32ThreadID], (orxU64)stLocationID);

            // Found?
            if(hResource != orxNULL)
            {
              orxS64 s64Offset;

              // Resets it
              s64Offset = orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);
              orxASSERT(s64Offset == 0);
            }
            else
            {
              // Opens it
              hResource = orxResource_Open(zLocation, orxFALSE);

              // Success?
              if(hResource != orxHANDLE_UNDEFINED)
              {
                orxSTATUS eResult;

                // Adds it to table
                eResult = orxHashTable_Add(sapstResourceTableList[u32ThreadID], (orxU64)stLocationID, hResource);
                orxASSERT(eResult != orxSTATUS_FAILURE);
              }
            }

            // Success?
            if(hResource != orxHANDLE_UNDEFINED)
            {
              orxU8 acTag[4];

              // Is a valid bundle?
              if((orxResource_Read(hResource, 4, &acTag, orxNULL, orxNULL) == 4)
              && (orxMemory_Compare(orxBUNDLE_KZ_BINARY_TAG, acTag, 4) == 0))
              {
                orxU32 i, u32Count;

                // Creates ToC
                *ppstToC = orxHashTable_Create(orxBUNDLE_KU32_TOC_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
                orxASSERT(*ppstToC != orxNULL);

                // For all stored resources
                for(i = 0, u32Count = orxResource_ReadU32(hResource); i < u32Count; i++)
                {
                  orxSTRINGID stID;
                  orxSTATUS   eResult;

                  // Gets its ID
                  stID = (orxSTRINGID)orxResource_ReadU64(hResource);

                  // Adds it
                  eResult = orxHashTable_Add(*ppstToC, stID, (void *)(orxUPTR)(i + 1));
                  orxASSERT(eResult != orxSTATUS_FAILURE);

                  // Skips entry
                  orxResource_Seek(hResource, orxBUNDLE_KU32_HEADER_ENTRY_SIZE - 8, orxSEEK_OFFSET_WHENCE_CURRENT);
                }
              }
            }
          }

          // Valid?
          if(*ppstToC != orxNULL)
          {
            orxSTRINGID stResourceID;
            orxU32      u32ResourceIndex;

            // Gets resource ID
            stResourceID = orxString_Hash(_zName);

            // Retrieves its index
            u32ResourceIndex = (orxU32)(orxUPTR)orxHashTable_Get(*ppstToC, stResourceID);

            // Valid?
            if(u32ResourceIndex != 0)
            {
              // Creates location string: location + index
              orxString_NPrint(sacBuffer, sizeof(sacBuffer), "%s%c0x%x", zLocation, orxRESOURCE_KC_LOCATION_SEPARATOR, u32ResourceIndex - 1);

              // Updates result
              zResult = sacBuffer;
            }
          }
        }

        // Updates status
        sbLocating = orxFALSE;
      }
    }
    else
    {
      // Has data?
      if(spstDataTable != orxNULL)
      {
        orxSTRINGID   stResourceID;
        BundleData   *pstData;

        // Gets resource ID
        stResourceID = orxString_Hash(_zName);

        // Gets resource
        pstData = (BundleData *)orxHashTable_Get(spstDataTable, stResourceID);

        // Found?
        if(pstData != orxNULL)
        {
          // Creates location string: index of file
          orxString_NPrint(sacBuffer, sizeof(sacBuffer), "0x%x", (orxU32)(pstData - sastDataList));

          // Updates result
          zResult = sacBuffer;
        }
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
    const orxSTRING zSeparator;
    const orxSTRING zLastSeparator = orxNULL;
    orxU32          u32Index;

    // Finds last separator
    for(zSeparator = orxString_SearchChar(_zLocation, orxRESOURCE_KC_LOCATION_SEPARATOR);
        zSeparator != orxNULL;
        zLastSeparator = zSeparator, zSeparator = orxString_SearchChar(zSeparator + 1, orxRESOURCE_KC_LOCATION_SEPARATOR))
      ;

    // Found?
    if(zLastSeparator != orxNULL)
    {
      static orxCHAR  sacBuffer[512];
      orxHANDLE       hResource;
      orxSTRINGID     stLocationID;
      orxU32          u32ThreadID;

      // Gets current thread ID
      u32ThreadID = orxThread_GetCurrent();
      orxASSERT(u32ThreadID != orxU32_UNDEFINED);

      // Copies its name
      orxString_NPrint(sacBuffer, sizeof(sacBuffer), "%.*s", (orxU32)(zLastSeparator - _zLocation), _zLocation);

      // Gets its location ID
      stLocationID = orxString_Hash(sacBuffer);

      // Gets it from table
      hResource = orxHashTable_Get(sapstResourceTableList[u32ThreadID], (orxU64)stLocationID);

      // Found?
      if(hResource != orxNULL)
      {
        orxS64 s64Offset;

        // Resets it
        s64Offset = orxResource_Seek(hResource, 0, orxSEEK_OFFSET_WHENCE_START);
        orxASSERT(s64Offset == 0);
      }
      else
      {
        // Opens it
        hResource = orxResource_Open(sacBuffer, orxFALSE);

        // Success?
        if(hResource != orxHANDLE_UNDEFINED)
        {
          orxSTATUS eResult;

          // Adds it to table
          eResult = orxHashTable_Add(sapstResourceTableList[u32ThreadID], (orxU64)stLocationID, hResource);
          orxASSERT(eResult != orxSTATUS_FAILURE);
        }
      }

      // Success?
      if(hResource != orxHANDLE_UNDEFINED)
      {
        orxU8 acTag[4];

        // Is a valid bundle?
        if((orxResource_Read(hResource, 4, &acTag, orxNULL, orxNULL) == 4)
        && (orxMemory_Compare(orxBUNDLE_KZ_BINARY_TAG, acTag, 4) == 0))
        {
          orxU32 u32Index;

          // Retrieves resource index
          if((orxString_ToU32(zLastSeparator + 1, &u32Index, orxNULL) != orxSTATUS_FAILURE)
          && (u32Index < orxResource_ReadU32(hResource)))
          {
            BundleResource *pstResource;

            // Allocates memory for our bundle resource
            pstResource = (BundleResource *)orxMemory_Allocate(sizeof(BundleResource), orxMEMORY_TYPE_MAIN);

            // Success?
            if(pstResource != orxNULL)
            {
              // Clears memory
              orxMemory_Zero(pstResource, sizeof(BundleResource));

              // Stores its internal resource
              pstResource->stData.hResource = hResource;

              // Skips to its info
              orxResource_Seek(hResource, orxBUNDLE_KU32_HEADER_INTRO_SIZE + u32Index * orxBUNDLE_KU32_HEADER_ENTRY_SIZE + 8 + 4, orxSEEK_OFFSET_WHENCE_START);

              // Stores it
              pstResource->stData.s64Size       = (orxS64)orxResource_ReadU32(hResource);
              pstResource->stData.s64FinalSize  = (orxS64)orxResource_ReadU32(hResource);

              // Skips to its entry
              orxResource_Seek(hResource, orxBUNDLE_KU32_HEADER_INTRO_SIZE + u32Index * orxBUNDLE_KU32_HEADER_ENTRY_SIZE, orxSEEK_OFFSET_WHENCE_START);

              // Updates result
              hResult = (orxHANDLE)pstResource;
            }
          }
        }
      }
    }
    else
    {
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
          orxMemory_Copy(&(pstResource->stData), &sastDataList[u32Index], sizeof(BundleData));

          // Updates result
          hResult = (orxHANDLE)pstResource;
        }
      }
    }
  }

  // Done!
  return hResult;
}

// Close function: releases all that has been allocated in Open
void orxFASTCALL orxBundle_Close(orxHANDLE _hResource)
{
  BundleResource *pstResource;

  // Gets resource
  pstResource = (BundleResource *)_hResource;

  // Has final buffer?
  if(pstResource->pu8FinalBuffer != orxNULL)
  {
    // Frees it
    orxMemory_Free(pstResource->pu8FinalBuffer);
  }

  // Frees it
  orxMemory_Free(pstResource);

  // Done!
  return;
}

// GetSize function: simply returns the size of the extracted resource, in bytes
orxS64 orxFASTCALL orxBundle_GetSize(orxHANDLE _hResource)
{
  BundleResource *pstResource;
  orxS64          s64Result;

  // Gets resource
  pstResource = (BundleResource *)_hResource;

  // Updates result
  s64Result = pstResource->stData.s64FinalSize;

  // Done!
  return s64Result;
}

// Seek function: position the read cursor inside the data and returns the offset from start upon success or -1 upon failure
orxS64 orxFASTCALL orxBundle_Seek(orxHANDLE _hResource, orxS64 _s64Offset, orxSEEK_OFFSET_WHENCE _eWhence)
{
  BundleResource *pstResource;
  orxS64          s64Cursor;

  // Gets resource
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
      s64Cursor = pstResource->stData.s64FinalSize - _s64Offset;
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
  if((s64Cursor >= 0) && (s64Cursor <= pstResource->stData.s64FinalSize))
  {
    // Updates cursor
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

  // No final buffer?
  if(pstResource->pu8FinalBuffer == orxNULL)
  {
    orxS64          s64Size;
    const orxSTRING zKey;
    orxU8          *pu8Buffer;

    // Allocates intermediate buffer
    pu8Buffer = (orxU8 *)orxMemory_Allocate((orxU32)pstResource->stData.s64Size, orxMEMORY_TYPE_TEMP);
    orxASSERT(pu8Buffer);

    // Gets encryption key
    zKey = orxConfig_GetEncryptionKey();

    // Has external resource?
    if(pstResource->stData.pu8Buffer == orxNULL)
    {
      const orxU8  *pu8Key;
      orxU8        *pu8Data;
      orxHANDLE     hResource;

      // Gets it
      hResource = pstResource->stData.hResource;

      // Store its ID
      pstResource->stData.stNameID = orxResource_ReadU64(hResource);

      // Skips to content
      orxResource_Seek(hResource, (orxS64)orxResource_ReadU32(hResource), orxSEEK_OFFSET_WHENCE_START);

      // Reads its content
      s64Size = orxResource_Read(hResource, pstResource->stData.s64Size, pu8Buffer, orxNULL, orxNULL);
      orxASSERT(s64Size == pstResource->stData.s64Size);

      // Decrypts data
      for(pu8Key = (const orxU8 *)zKey, pu8Data = pu8Buffer;
          pu8Data < pu8Buffer + pstResource->stData.s64Size;
          pu8Key = (*pu8Key == orxCHAR_NULL) ? (const orxU8 *)zKey : pu8Key + 1, pu8Data++)
      {
        *pu8Data ^= *pu8Key;
      }
    }
    else
    {
      const orxU8  *pu8Key, *pu8Src;
      orxU8        *pu8Dst;

      // Decrypts data
      for(pu8Key = (const orxU8 *)zKey, pu8Src = pstResource->stData.pu8Buffer, pu8Dst = pu8Buffer;
          pu8Src < pstResource->stData.pu8Buffer + pstResource->stData.s64Size;
          pu8Key = (*pu8Key == orxCHAR_NULL) ? (const orxU8 *)zKey : pu8Key + 1, pu8Src++, pu8Dst++)
      {
        *pu8Dst = *pu8Src ^ *pu8Key;
      }
    }

    // Allocates final buffer
    pstResource->pu8FinalBuffer = (orxU8 *)orxMemory_Allocate((orxU32)pstResource->stData.s64FinalSize, orxMEMORY_TYPE_MAIN);
    orxASSERT(pstResource->pu8FinalBuffer != orxNULL);

    // Decompresses data
    s64Size = (orxS64)LZ4_decompress_safe((const char *)pu8Buffer, (char *)pstResource->pu8FinalBuffer, (int)pstResource->stData.s64Size, (int)pstResource->stData.s64FinalSize);

    // Failure?
    if(s64Size != pstResource->stData.s64FinalSize)
    {
      // Logs message
      orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, orxANSI_KZ_COLOR_FG_YELLOW "[Bundle]" orxANSI_KZ_COLOR_FG_RED " Can't decompress resource " orxANSI_KZ_COLOR_FG_GREEN "[%s]" orxANSI_KZ_COLOR_FG_RED ": invalid decryption key or corrupted data.", orxString_GetFromID(pstResource->stData.stNameID));

      // Updates its final size
      pstResource->stData.s64FinalSize = 0;
    }

    // Deletes intermediate buffer
    orxMemory_Free(pu8Buffer);
  }

  // Gets actual copy size to prevent any out-of-bound access
  s64CopySize = orxMIN(_s64Size, pstResource->stData.s64FinalSize - pstResource->s64Cursor);

  // Should copy content?
  if(s64CopySize != 0)
  {
    // Copies content
    orxMemory_Copy(_pu8Buffer, pstResource->pu8FinalBuffer + pstResource->s64Cursor, (orxS32)s64CopySize);
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
  orxMemory_Zero(sacPrintBuffer, sizeof(sacPrintBuffer));
  orxMemory_Zero(sapstResourceTableList, sizeof(sapstResourceTableList));
  spstToCTable      = orxNULL;
  spstDataTable     = orxNULL;
  shResource        = orxHANDLE_UNDEFINED;
  sbProcess         = orxFALSE;

  // Inits our bundle resource type
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
    orxU32 i;

    // Has data?
    if(su32DataCount > 0)
    {
      // Creates data table
      spstDataTable = orxHashTable_Create(orxBUNDLE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      orxASSERT(spstDataTable != orxNULL);

      // For all bundled data
      for(i = 0; i < su32DataCount; i++)
      {
        orxSTATUS eResult;

        // Stores it
        eResult = orxHashTable_Add(spstDataTable, sastDataList[i].stNameID, (void *)&sastDataList[i]);

        // Checks
        orxASSERT(eResult != orxSTATUS_FAILURE);
      }
    }

    // Creates ToC table
    spstToCTable = orxHashTable_Create(orxBUNDLE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    orxASSERT(spstToCTable != orxNULL);

    // Creates resource tables
    for(i = 0; i < orxARRAY_GET_ITEM_COUNT(sapstResourceTableList); i++)
    {
      sapstResourceTableList[i] = orxHashTable_Create(orxBUNDLE_KU32_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
      orxASSERT(sapstResourceTableList[i] != orxNULL);
    }

    // Registers commands
    orxCOMMAND_REGISTER_CORE_COMMAND(Bundle, IsProcessing, "Processing?", orxCOMMAND_VAR_TYPE_BOOL, 0, 0);
    orxCOMMAND_REGISTER_CORE_COMMAND(Bundle, GetOutputName, "Name", orxCOMMAND_VAR_TYPE_STRING, 0, 0);

    // Registers event handler
    orxEvent_AddHandler(orxEVENT_TYPE_SYSTEM, orxBundle_EventHandler);
    orxEvent_SetHandlerIDFlags(orxBundle_EventHandler, orxEVENT_TYPE_SYSTEM, orxNULL, orxEVENT_GET_FLAG(orxSYSTEM_EVENT_PARAM_READY) | orxEVENT_GET_FLAG(orxSYSTEM_EVENT_GAME_LOOP_START), orxEVENT_KU32_MASK_ID_ALL);
    orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxBundle_EventHandler);
    orxEvent_SetHandlerIDFlags(orxBundle_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_REMOVE), orxEVENT_KU32_MASK_ID_ALL);
  }

  // Done!
  return eResult;
}

void orxFASTCALL orxBundle_Exit()
{
  orxU32 i;

  // Unregisters event handler
  orxEvent_RemoveHandler(orxEVENT_TYPE_SYSTEM, orxBundle_EventHandler);
  orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxBundle_EventHandler);

  // Unregisters commands
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Bundle, IsProcessing);
  orxCOMMAND_UNREGISTER_CORE_COMMAND(Bundle, GetOutputName);

  // Has pending resource?
  if(shResource != orxHANDLE_UNDEFINED)
  {
    // Closes it
    orxResource_Close(shResource);
    shResource = orxHANDLE_UNDEFINED;
  }

  // Has data table?
  if(spstDataTable != orxNULL)
  {
    // Deletes it
    orxHashTable_Delete(spstDataTable);
    spstDataTable = orxNULL;
  }

  // Clears ToC table
  orxBundle_ClearToCTable();

  // Deletes ToC table
  orxHashTable_Delete(spstToCTable);
  spstToCTable = orxNULL;

  // Clears resource tables
  orxBundle_ClearResourceTables();

  // For all resource tables
  for(i = 0; i < orxARRAY_GET_ITEM_COUNT(sapstResourceTableList); i++)
  {
    // Deletes it
    orxHashTable_Delete(sapstResourceTableList[i]);
    sapstResourceTableList[i] = orxNULL;
  }
}

orxBOOL orxFASTCALL orxBundle_IsProcessing()
{
  // Done!
  return sbProcess;
}

const orxSTRING orxFASTCALL orxBundle_GetOutputName()
{
  // Done!
  return (shResource != orxHANDLE_UNDEFINED) ? orxResource_GetPath(orxResource_GetLocation(shResource)) : orxSTRING_EMPTY;
}

#if defined(__orxGCC__)

  #pragma GCC diagnostic pop

#elif defined(__orxLLVM__)

  #pragma clang diagnostic pop

#endif

#endif // orxBUNDLE_IMPL

#endif // _orxBUNDLE_H_

/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
 * @file orxShader.c
 * @date 11/04/2009
 * @author iarwain@orx-project.org
 *
 */


#include "render/orxShader.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxClock.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "core/orxResource.h"
#include "debug/orxProfiler.h"
#include "display/orxFont.h"
#include "display/orxGraphic.h"
#include "display/orxText.h"
#include "object/orxStructure.h"
#include "render/orxViewport.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"

#ifdef __orxMSVC__

  #include <malloc.h>

#endif /* __orxMSVC__ */

#ifdef __orxGCC__

  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wstrict-aliasing"

#endif /* __orxGCC__ */


/** Module flags
 */
#define orxSHADER_KU32_STATIC_FLAG_NONE       0x00000000

#define orxSHADER_KU32_STATIC_FLAG_READY      0x00000001

#define orxSHADER_KU32_STATIC_MASK_ALL        0xFFFFFFFF


/** Flags
 */
#define orxSHADER_KU32_FLAG_NONE              0x00000000  /**< No flags */

#define orxSHADER_KU32_FLAG_ENABLED           0x10000000  /**< Enabled flag */
#define orxSHADER_KU32_FLAG_COMPILED          0x20000000  /**< Compiled flag */
#define orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM  0x40000000  /**< No custom param flag */
#define orxSHADER_KU32_FLAG_CACHED            0x80000000  /**< Cached flag */

#define orxSHADER_KU32_MASK_ALL               0xFFFFFFFF  /**< All mask */

/** Misc defines
 */
#define orxSHADER_KU32_REFERENCE_TABLE_SIZE   32
#define orxSHADER_KU32_PARAM_BANK_SIZE        16

#define orxSHADER_KU32_BANK_SIZE              32          /**< Bank size */

#define orxSHADER_KZ_CONFIG_CODE              "Code"
#define orxSHADER_KZ_CONFIG_CODE_LIST         "CodeList"
#define orxSHADER_KZ_CONFIG_PARAM_LIST        "ParamList"
#define orxSHADER_KZ_CONFIG_USE_CUSTOM_PARAM  "UseCustomParam"
#define orxSHADER_KZ_CONFIG_KEEP_IN_CACHE     "KeepInCache"

#define orxSHADER_KZ_TIME                     "time"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Shader param-value structure
 */
typedef struct __orxSHADER_PARAM_VALUE_t
{
  orxLINKLIST_NODE    stNode;                             /**< Linklist node : 12 */
  orxSHADER_PARAM    *pstParam;                           /**< Param definition : 16 */
  orxS32              s32ID;                              /**< Param ID : 20 */
  orxS32              s32Index;                           /**< Param index : 24 */

  union
  {
    orxFLOAT          fValue;                             /**< Float value : 28 */
    const orxTEXTURE *pstValue;                           /**< Texture value : 28 */
    orxVECTOR         vValue;                             /**< Vector value : 36 */
  };                                                      /**< Union value : 36 */

} orxSHADER_PARAM_VALUE;

/** Shader structure
 */
struct __orxSHADER_t
{
  orxSTRUCTURE    stStructure;                            /**< Public structure, first structure member : 32 */
  orxLINKLIST     stParamList;                            /**< Parameter list : 28 */
  orxLINKLIST     stParamValueList;                       /**< Parameter value list : 40 */
  const orxSTRING zReference;                             /**< Shader reference : 44 */
  orxHANDLE       hData;                                  /**< Compiled shader data : 48 */
  orxBANK        *pstParamValueBank;                      /**< Parameter value bank : 52 */
  orxBANK        *pstParamBank;                           /**< Parameter bank : 56 */
};

/** Static structure
 */
typedef struct __orxSHADER_STATIC_t
{
  orxU32        u32Flags;                                 /**< Control flags */
  orxHASHTABLE *pstReferenceTable;                        /**< Reference hash table */
  const orxCLOCK_INFO *pstClockInfo;                      /**< Core clock info */

} orxSHADER_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxSHADER_STATIC sstShader;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Processes config data
 */
static orxSTATUS orxFASTCALL orxShader_ProcessConfigData(orxSHADER *_pstShader)
{
  orxS32                  i, s32Number;
  orxSHADER_PARAM_VALUE  *pstParamValue;
  orxSTATUS               eResult = orxSTATUS_FAILURE;

  /* Pushes its section */
  orxConfig_PushSection(_pstShader->zReference);

  /* Use custom param? */
  if(orxConfig_GetBool(orxSHADER_KZ_CONFIG_USE_CUSTOM_PARAM) != orxFALSE)
  {
    /* Updates status */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM, orxSHADER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_NONE, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM);
  }

  /* For all current parameter values */
  for(pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
      pstParamValue != orxNULL;
      pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
  {
    /* Is a texture? */
    if(pstParamValue->pstParam->eType == orxSHADER_PARAM_TYPE_TEXTURE)
    {
      /* Is valid? */
      if(pstParamValue->pstValue != orxNULL)
      {
        /* Deletes it */
        orxTexture_Delete((orxTEXTURE *)pstParamValue->pstValue);
      }
    }
  }

  /* Clears param lists & banks */
  orxMemory_Zero(&(_pstShader->stParamValueList), sizeof(orxLINKLIST));
  orxMemory_Zero(&(_pstShader->stParamList), sizeof(orxLINKLIST));
  orxBank_Clear(_pstShader->pstParamValueBank);
  orxBank_Clear(_pstShader->pstParamBank);

  /* For all parameters */
  for(i = 0, s32Number = orxConfig_GetListCount(orxSHADER_KZ_CONFIG_PARAM_LIST); i < s32Number; i++)
  {
    orxBOOL         bIsList;
    const orxSTRING zParamName;

    /* Gets its name */
    zParamName = orxConfig_GetListString(orxSHADER_KZ_CONFIG_PARAM_LIST, i);

    /* Updates list status */
    bIsList = ((orxConfig_IsList(zParamName) != orxFALSE) && (orxConfig_IsLocallyInheritedValue(zParamName) == orxFALSE));

    /* Valid? */
    if((zParamName != orxNULL) && (zParamName != orxSTRING_EMPTY))
    {
      orxVECTOR astValueBuffer[256];
      orxS32    s32ParamListCount;

      /* Gets param's list count */
      s32ParamListCount = (bIsList != orxFALSE) ? orxConfig_GetListCount(zParamName) : 0;

      /* Is a vector? */
      if(orxConfig_GetVector(zParamName, &(astValueBuffer[0])) != orxNULL)
      {
        /* Is a list? */
        if(bIsList != orxFALSE)
        {
          orxS32 j;

          /* For all defined entries */
          for(j = 0; j < s32ParamListCount; j++)
          {
            /* Stores its vector */
            orxConfig_GetListVector(zParamName, j, &(astValueBuffer[j]));
          }
        }

        /* Adds vector param */
        orxShader_AddVectorParam(_pstShader, zParamName, s32ParamListCount, astValueBuffer);
      }
      else
      {
        const orxSTRING zValue;

        /* Gets its literal value */
        zValue = orxConfig_GetString(zParamName);

        /* Is a float? */
        if(orxString_ToFloat(zValue, (orxFLOAT *)astValueBuffer, orxNULL) != orxSTATUS_FAILURE)
        {
          /* Is a list? */
          if(bIsList != orxFALSE)
          {
            orxS32 j;

            /* For all defined entries */
            for(j = 0; j < s32ParamListCount; j++)
            {
              /* Stores its vector */
              ((orxFLOAT *)astValueBuffer)[j] = orxConfig_GetListFloat(zParamName, j);
            }
          }

          /* Adds float param */
          orxShader_AddFloatParam(_pstShader, zParamName, s32ParamListCount, (orxFLOAT *)astValueBuffer);
        }
        else
        {
          orxBOOL bIsTime = orxFALSE;

          /* Is a list? */
          if(bIsList != orxFALSE)
          {
            orxS32 j;

            /* For all defined entries */
            for(j = 0; j < s32ParamListCount; j++)
            {
              /* Stores its vector */
              zValue = orxConfig_GetListString(zParamName, j);

              /* Valid? */
              if(zValue != orxSTRING_EMPTY)
              {
                /* Is time? */
                if(!orxString_ICompare(zValue, orxSHADER_KZ_TIME))
                {
                  /* Marks as time */
                  bIsTime = orxTRUE;

                  /* Is not using custom param? */
                  if(!orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM))
                  {
                    /* Forces it */
                    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM, orxSHADER_KU32_FLAG_NONE);

                    /* Logs message */
                    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Using time parameter for <%s> -> forcing config property \"%s\" to true.", _pstShader->zReference, _pstShader, zParamName, orxSHADER_KZ_CONFIG_USE_CUSTOM_PARAM);
                  }

                  /* Logs message */
                  orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't use array for time parameter. <%s> will be declared as a regular variable.", _pstShader->zReference, _pstShader, zParamName);

                  break;
                }

                /* Loads texture */
                ((orxTEXTURE **)astValueBuffer)[j] = orxTexture_Load(zValue, orxFALSE);
              }
              else
              {
                /* No texture */
                ((orxTEXTURE **)astValueBuffer)[j] = orxNULL;
              }
            }
          }
          else
          {
            /* Valid? */
            if(zValue != orxSTRING_EMPTY)
            {
              /* Is time? */
              if(!orxString_ICompare(zValue, orxSHADER_KZ_TIME))
              {
                /* Marks as time */
                bIsTime = orxTRUE;

                /* Is not using custom param? */
                if(!orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM))
                {
                  /* Forces it */
                  orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM, orxSHADER_KU32_FLAG_NONE);

                  /* Logs message */
                  orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Using time parameter for <%s> -> forcing config property \"%s\" to true.", _pstShader->zReference, _pstShader, zParamName, orxSHADER_KZ_CONFIG_USE_CUSTOM_PARAM);
                }
              }
              else
              {
                /* Loads texture */
                ((orxTEXTURE **)astValueBuffer)[0] = orxTexture_Load(zValue, orxFALSE);
              }
            }
            else
            {
              /* No texture */
              ((orxTEXTURE **)astValueBuffer)[0] = orxNULL;
            }
          }

          /* Is time? */
          if(bIsTime != orxFALSE)
          {
            /* Adds time param */
            orxShader_AddTimeParam(_pstShader, zParamName);
          }
          else
          {
            /* Adds texture param */
            orxShader_AddTextureParam(_pstShader, zParamName, s32ParamListCount, (const orxTEXTURE **)astValueBuffer);
          }
        }
      }
    }
  }

  /* Has code list? */
  if(orxConfig_HasValue(orxSHADER_KZ_CONFIG_CODE_LIST) != orxFALSE)
  {
    orxS32 s32Count;

    /* Gets its count */
    s32Count = orxConfig_GetListCount(orxSHADER_KZ_CONFIG_CODE_LIST);

    {
      orxS32 i;

#ifdef __orxMSVC__
      const orxSTRING *azCodeList = (const orxSTRING *)alloca(s32Count * sizeof(const orxSTRING));
#else /* __orxMSVC__ */
      const orxSTRING azCodeList[s32Count];
#endif /* __orxMSVC__ */

      /* For all code entries */
      for(i = 0; i < s32Count; i++)
      {
        /* Gets it */
        azCodeList[i] = orxConfig_GetString(orxConfig_GetListString(orxSHADER_KZ_CONFIG_CODE_LIST, i));
      }

      /* Compiles code */
      orxShader_CompileCode(_pstShader, azCodeList, s32Count);
    }
  }
  else
  {
    const orxSTRING zCode;

    /* Gets code */
    zCode = orxConfig_GetString(orxSHADER_KZ_CONFIG_CODE);

    /* Valid? */
    if(zCode != orxSTRING_EMPTY)
    {
      /* Compiles it */
      eResult = orxShader_CompileCode(_pstShader, &zCode, 1);
    }
  }

  /* Pops config section */
  orxConfig_PopSection();

  /* Done! */
  return eResult;
}

/** Event handler
 */
static orxSTATUS orxFASTCALL orxShader_EventHandler(const orxEVENT *_pstEvent)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Add or update? */
  if((_pstEvent->eID == orxRESOURCE_EVENT_ADD) || (_pstEvent->eID == orxRESOURCE_EVENT_UPDATE))
  {
    orxRESOURCE_EVENT_PAYLOAD *pstPayload;

    /* Gets payload */
    pstPayload = (orxRESOURCE_EVENT_PAYLOAD *)_pstEvent->pstPayload;

    /* Is config group? */
    if(pstPayload->stGroupID == orxString_Hash(orxCONFIG_KZ_RESOURCE_GROUP))
    {
      orxSHADER *pstShader;

      /* For all shaders */
      for(pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));
          pstShader != orxNULL;
          pstShader = orxSHADER(orxStructure_GetNext(pstShader)))
      {
        /* Has reference? */
        if((pstShader->zReference != orxNULL) && (pstShader->zReference != orxSTRING_EMPTY))
        {
          /* Match origin? */
          if(orxConfig_GetOriginID(pstShader->zReference) == pstPayload->stNameID)
          {
            /* Re-processes its config data */
            orxShader_ProcessConfigData(pstShader);
          }
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Deletes all the shaders
 */
static orxINLINE void orxShader_DeleteAll()
{
  orxSHADER *pstShader;

  /* Gets first shader */
  pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));

  /* Non empty? */
  while(pstShader != orxNULL)
  {
    /* Deletes it */
    orxShader_Delete(pstShader);

    /* Gets first shader */
    pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));
  }

  return;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Shader module setup
 */
void orxFASTCALL orxShader_Setup()
{
  /* Adds module dependencies */
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_MEMORY);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_STRING);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_CLOCK);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_GRAPHIC);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_TEXTURE);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_PROFILER);

  /* Done! */
  return;
}

/** Inits the shader module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_Init()
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Not already Initialized? */
  if(!(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Zero(&sstShader, sizeof(orxSHADER_STATIC));

    /* Creates reference table */
    sstShader.pstReferenceTable = orxHashTable_Create(orxSHADER_KU32_REFERENCE_TABLE_SIZE, orxHASHTABLE_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(sstShader.pstReferenceTable != orxNULL)
    {
      /* Registers structure type */
      eResult = orxSTRUCTURE_REGISTER(SHADER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxSHADER_KU32_BANK_SIZE, orxNULL);

      /* Success? */
      if(eResult != orxSTATUS_FAILURE)
      {
        orxCLOCK *pstClock;

        /* Gets core clock */
        pstClock = orxClock_Get(orxCLOCK_KZ_CORE);

        /* Valid? */
        if(pstClock != orxNULL)
        {
          /* Stores its info */
          sstShader.pstClockInfo = orxClock_GetInfo(pstClock);

          /* Adds event handler */
          orxEvent_AddHandler(orxEVENT_TYPE_RESOURCE, orxShader_EventHandler);
          orxEvent_SetHandlerIDFlags(orxShader_EventHandler, orxEVENT_TYPE_RESOURCE, orxNULL, orxEVENT_GET_FLAG(orxRESOURCE_EVENT_ADD) | orxEVENT_GET_FLAG(orxRESOURCE_EVENT_UPDATE), orxEVENT_KU32_MASK_ID_ALL);
        }
        else
        {
          /* Updates result */
          eResult = orxSTATUS_FAILURE;

          /* Unregisters structure type */
          orxStructure_Unregister(orxSTRUCTURE_ID_SHADER);

          /* Deletes reference table */
          orxHashTable_Delete(sstShader.pstReferenceTable);
          sstShader.pstReferenceTable = orxNULL;

          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to retrieve core clock.");
        }
      }
      else
      {
        /* Deletes reference table */
        orxHashTable_Delete(sstShader.pstReferenceTable);
        sstShader.pstReferenceTable = orxNULL;

        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to register shader structure.");
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create shader hashtable storage.");
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to initialize the shader module when it was already initialized.");

    /* Already initialized */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Initialized? */
  if(eResult != orxSTATUS_FAILURE)
  {
    /* Inits Flags */
    orxFLAG_SET(sstShader.u32Flags, orxSHADER_KU32_STATIC_FLAG_READY, orxSHADER_KU32_STATIC_FLAG_NONE);
  }

  /* Done! */
  return eResult;
}

/** Exits from the shader module
 */
void orxFASTCALL orxShader_Exit()
{
  /* Initialized? */
  if(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY)
  {
    /* Removes event handler */
    orxEvent_RemoveHandler(orxEVENT_TYPE_RESOURCE, orxShader_EventHandler);

    /* Deletes shader list */
    orxShader_DeleteAll();

    /* Unregisters structure type */
    orxStructure_Unregister(orxSTRUCTURE_ID_SHADER);

    /* Deletes reference table */
    orxHashTable_Delete(sstShader.pstReferenceTable);

    /* Updates flags */
    sstShader.u32Flags &= ~orxSHADER_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Tried to exit from the shader module when it wasn't initialized.");
  }

  return;
}

/** Creates an empty shader
 * @return orxSHADER / orxNULL
 */
orxSHADER *orxFASTCALL orxShader_Create()
{
  orxSHADER *pstResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);

  /* Creates shader */
  pstResult = orxSHADER(orxStructure_Create(orxSTRUCTURE_ID_SHADER));

  /* Created? */
  if(pstResult != orxNULL)
  {
    /* Creates its parameter banks */
    pstResult->pstParamValueBank  = orxBank_Create(orxSHADER_KU32_PARAM_BANK_SIZE, sizeof(orxSHADER_PARAM_VALUE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);
    pstResult->pstParamBank       = orxBank_Create(orxSHADER_KU32_PARAM_BANK_SIZE, sizeof(orxSHADER_PARAM), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if((pstResult->pstParamValueBank != orxNULL)
    && (pstResult->pstParamBank != orxNULL))
    {
      /* Clears its data */
      pstResult->hData      = orxHANDLE_UNDEFINED;
      pstResult->zReference = orxSTRING_EMPTY;

      /* Inits flags */
      orxStructure_SetFlags(pstResult, orxSHADER_KU32_FLAG_ENABLED, orxSHADER_KU32_MASK_ALL);

      /* Increases count */
      orxStructure_IncreaseCount(pstResult);
    }
    else
    {
      /* Deletes banks */
      if(pstResult->pstParamValueBank != orxNULL)
      {
        orxBank_Delete(pstResult->pstParamValueBank);
      }
      if(pstResult->pstParamBank != orxNULL)
      {
        orxBank_Delete(pstResult->pstParamBank);
      }

      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to allocate shader parameter banks.");

      /* Deletes shader */
      orxStructure_Delete(pstResult);

      /* Updates result */
      pstResult = orxNULL;
    }
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to create shader structure.");
  }

  /* Done! */
  return pstResult;
}

/** Creates a shader from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxSHADER / orxNULL
 */
orxSHADER *orxFASTCALL orxShader_CreateFromConfig(const orxSTRING _zConfigID)
{
  orxSTRINGID stID;
  orxSHADER  *pstResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Gets shader ID */
  stID = orxString_Hash(_zConfigID);

  /* Search for reference */
  pstResult = (orxSHADER *)orxHashTable_Get(sstShader.pstReferenceTable, stID);

  /* Found? */
  if(pstResult != orxNULL)
  {
    /* Increases count */
    orxStructure_IncreaseCount(pstResult);
  }
  else
  {
    /* Pushes section */
    if((orxConfig_HasSection(_zConfigID) != orxFALSE)
    && (orxConfig_PushSection(_zConfigID) != orxSTATUS_FAILURE))
    {
      /* Creates shader */
      pstResult = orxShader_Create();

      /* Valid? */
      if(pstResult != orxNULL)
      {
        /* Adds it to reference table */
        if(orxHashTable_Add(sstShader.pstReferenceTable, stID, pstResult) != orxSTATUS_FAILURE)
        {
          /* Stores its reference */
          pstResult->zReference = orxConfig_GetCurrentSection();

          /* Should keep it in cache? */
          if(orxConfig_GetBool(orxSHADER_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference count to keep it in cache table */
            orxStructure_IncreaseCount(pstResult);

            /* Updates its flags */
            orxStructure_SetFlags(pstResult, orxSHADER_KU32_FLAG_CACHED, orxSHADER_KU32_FLAG_NONE);
          }

          /* Processes its data */
          orxShader_ProcessConfigData(pstResult);
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to add hash table.");

          /* Deletes it */
          orxShader_Delete(pstResult);

          /* Updates result */
          pstResult = orxNULL;
        }
      }

      /* Pops previous section */
      orxConfig_PopSection();
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Couldn't create shader because config section (%s) couldn't be found.", _zConfigID);

      /* Updates result */
      pstResult = orxNULL;
    }
  }

  /* Done! */
  return pstResult;
}

/** Deletes a shader
 * @param[in] _pstShader              Concerned Shader
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_Delete(orxSHADER *_pstShader)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Decreases count */
  orxStructure_DecreaseCount(_pstShader);

  /* Not referenced? */
  if(orxStructure_GetRefCount(_pstShader) == 0)
  {
    /* Has an ID? */
    if((_pstShader->zReference != orxNULL)
    && (_pstShader->zReference != orxSTRING_EMPTY))
    {
      orxSHADER_PARAM_VALUE *pstParamValue;

      /* Removes from hashtable */
      orxHashTable_Remove(sstShader.pstReferenceTable, orxString_Hash(_pstShader->zReference));

      /* Has data? */
      if(orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_COMPILED))
      {
        /* Deletes it */
        orxDisplay_DeleteShader(_pstShader->hData);
      }

      /* For all parameter values */
      for(pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
          pstParamValue != orxNULL;
          pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
      {
        /* Is a texture? */
        if(pstParamValue->pstParam->eType == orxSHADER_PARAM_TYPE_TEXTURE)
        {
          /* Is valid? */
          if(pstParamValue->pstValue != orxNULL)
          {
            /* Deletes it */
            orxTexture_Delete((orxTEXTURE *)pstParamValue->pstValue);
          }
        }
      }

      /* Deletes param banks */
      orxBank_Delete(_pstShader->pstParamValueBank);
      orxBank_Delete(_pstShader->pstParamBank);
    }

    /* Deletes structure */
    orxStructure_Delete(_pstShader);
  }
  else
  {
    /* Referenced by others */
    eResult = orxSTATUS_FAILURE;
  }

  /* Done! */
  return eResult;
}

/** Clears cache (if any shader is still in active use, it'll remain in memory until not referenced anymore)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_ClearCache()
{
  orxSHADER  *pstShader, *pstNextShader;
  orxSTATUS   eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);

  /* For all shaders */
  for(pstShader = orxSHADER(orxStructure_GetFirst(orxSTRUCTURE_ID_SHADER));
      pstShader != orxNULL;
      pstShader = pstNextShader)
  {
    /* Gets next shader */
    pstNextShader = orxSHADER(orxStructure_GetNext(pstShader));

    /* Is cached? */
    if(orxStructure_TestFlags(pstShader, orxSHADER_KU32_FLAG_CACHED))
    {
      /* Updates its flags */
      orxStructure_SetFlags(pstShader, orxSHADER_KU32_FLAG_NONE, orxSHADER_KU32_FLAG_CACHED);

      /* Deletes its extra reference */
      orxShader_Delete(pstShader);
    }
  }

  /* Done! */
  return eResult;
}


/** Starts a shader
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _pstOwner               Owner structure (orxOBJECT / orxVIEWPORT / orxNULL)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_Start(const orxSHADER *_pstShader, const orxSTRUCTURE *_pstOwner)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  orxSTRUCTURE_ASSERT(_pstOwner);

  /* Valid & enabled? */
  if((_pstShader->hData != orxHANDLE_UNDEFINED) && (orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED)))
  {
    /* Starts it */
    eResult = orxDisplay_StartShader(_pstShader->hData);

    /* Success? */
    if(eResult != orxSTATUS_FAILURE)
    {
      orxTEXTURE             *pstOwnerTexture = orxNULL;
      orxSHADER_PARAM_VALUE  *pstParamValue;
      orxFLOAT                fTime = orxFLOAT_0;

      /* Depending on its type */
      switch(orxStructure_GetID(_pstOwner))
      {
        case orxSTRUCTURE_ID_OBJECT:
        {
          orxOBJECT *pstOwner;

          /* Gets cast owner */
          pstOwner = orxOBJECT(_pstOwner);

          /* Gets its working texture */
          pstOwnerTexture = orxObject_GetWorkingTexture(pstOwner);

          /* Gets its active time */
          fTime = orxObject_GetActiveTime(pstOwner);

          break;
        }

        case orxSTRUCTURE_ID_VIEWPORT:
        {
          /* Updates owner texture */
          orxViewport_GetTextureList(orxVIEWPORT(_pstOwner), 1, &pstOwnerTexture);

          /* Gets core time */
          fTime = sstShader.pstClockInfo->fTime;

          break;
        }

        default:
        {
          break;
        }
      }

      /* No custom param? */
      if(!orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM))
      {
        /* For all parameter values */
        for(pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
            pstParamValue != orxNULL;
            pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
        {
          /* Depending on parameter type */
          switch(pstParamValue->pstParam->eType)
          {
            case orxSHADER_PARAM_TYPE_FLOAT:
            {
              /* Sets it */
              orxDisplay_SetShaderFloat(_pstShader->hData, pstParamValue->s32ID, pstParamValue->fValue);

              break;
            }

            case orxSHADER_PARAM_TYPE_TEXTURE:
            {
              orxBITMAP *pstBitmap;

              /* Has default texture? */
              if(pstParamValue->pstValue != orxNULL)
              {
                /* Gets its bitmap */
                pstBitmap = orxTexture_GetBitmap(pstParamValue->pstValue);
              }
              /* Has an owner texture? */
              else if(pstOwnerTexture != orxNULL)
              {
                /* Gets its bitmap */
                pstBitmap = orxTexture_GetBitmap(pstOwnerTexture);
              }
              else
              {
                /* No bitmap specified */
                pstBitmap = orxNULL;
              }

              /* Sets it */
              orxDisplay_SetShaderBitmap(_pstShader->hData, pstParamValue->s32ID, pstBitmap);

              break;
            }

            case orxSHADER_PARAM_TYPE_VECTOR:
            {
              /* Sets it */
              orxDisplay_SetShaderVector(_pstShader->hData, pstParamValue->s32ID, &(pstParamValue->vValue));

              break;
            }

            case orxSHADER_PARAM_TYPE_TIME:
            {
              /* Sets it */
              orxDisplay_SetShaderFloat(_pstShader->hData, pstParamValue->s32ID, fTime);

              break;
            }

            default:
            {
              break;
            }
          }
        }
      }
      else
      {
        /* For all parameter values */
        for(pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
            pstParamValue != orxNULL;
            pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
        {
          orxEVENT                stEvent;
          orxSHADER_EVENT_PAYLOAD stPayload;

          /* Inits event */
          orxEVENT_INIT(stEvent, orxEVENT_TYPE_SHADER, orxSHADER_EVENT_SET_PARAM, _pstOwner, _pstOwner, &stPayload);

          /* Inits payload */
          stPayload.pstShader     = _pstShader;
          stPayload.zShaderName   = _pstShader->zReference;
          stPayload.eParamType    = pstParamValue->pstParam->eType;
          stPayload.zParamName    = pstParamValue->pstParam->zName;
          stPayload.s32ParamIndex = pstParamValue->s32Index;

          /* Depending on type */
          switch(stPayload.eParamType)
          {
            case orxSHADER_PARAM_TYPE_FLOAT:
            {
              /* Updates value */
              stPayload.fValue = pstParamValue->fValue;

              /* Sends event */
              if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
              {
                /* Sets it */
                orxDisplay_SetShaderFloat(_pstShader->hData, pstParamValue->s32ID, stPayload.fValue);
              }

              break;
            }

            case orxSHADER_PARAM_TYPE_TEXTURE:
            {
              /* Updates value */
              stPayload.pstValue = (pstParamValue->pstValue != orxNULL) ? pstParamValue->pstValue : pstOwnerTexture;

              /* Sends event */
              if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
              {
                /* Sets it */
                orxDisplay_SetShaderBitmap(_pstShader->hData, pstParamValue->s32ID, (stPayload.pstValue != orxNULL) ? orxTexture_GetBitmap(stPayload.pstValue) : orxNULL);
              }

              break;
            }

            case orxSHADER_PARAM_TYPE_VECTOR:
            {
              /* Updates value */
              orxVector_Copy(&(stPayload.vValue), &(pstParamValue->vValue));

              /* Sends event */
              if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
              {
                /* Sets it */
                orxDisplay_SetShaderVector(_pstShader->hData, pstParamValue->s32ID, &(stPayload.vValue));
              }

              break;
            }

            case orxSHADER_PARAM_TYPE_TIME:
            {
              /* Updates value */
              stPayload.fValue = fTime;

              /* Sends event */
              if(orxEvent_Send(&stEvent) != orxSTATUS_FAILURE)
              {
                /* Sets it */
                orxDisplay_SetShaderFloat(_pstShader->hData, pstParamValue->s32ID, stPayload.fValue);
              }

              break;
            }

            default:
            {
              break;
            }
          }
        }
      }
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Couldn't start Shader [%s/%x].", _pstShader->zReference, _pstShader);

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Stops a shader
 * @param[in] _pstShader              Concerned Shader
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_Stop(const orxSHADER *_pstShader)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Valid & enabled? */
  if((_pstShader->hData != orxHANDLE_UNDEFINED) && (orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED)))
  {
    /* Stops it */
    eResult = orxDisplay_StopShader(_pstShader->hData);
  }
  else
  {
    /* Updates result */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Adds a float parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable
 * @param[in] _afValueList            Parameter's float value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddFloatParam(orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxFLOAT *_afValueList)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM *pstParam;

    /* Allocates param */
    pstParam = (orxSHADER_PARAM *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      orxS32 i;

      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM));

      /* Inits it */
      pstParam->eType         = orxSHADER_PARAM_TYPE_FLOAT;
      pstParam->zName         = orxString_Store(_zName);
      pstParam->u32ArraySize  = _u32ArraySize;

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stNode));

      /* For all array indices */
      for(i = (_u32ArraySize != 0) ? 0 : -1; i < (orxS32)_u32ArraySize; i++)
      {
        orxSHADER_PARAM_VALUE *pstParamValue;

        /* Allocates it */
        pstParamValue = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamValueBank);

        /* Valid? */
        if(pstParamValue != orxNULL)
        {
          /* Clears it */
          orxMemory_Zero(pstParamValue, sizeof(orxSHADER_PARAM_VALUE));

          /* Inits it */
          pstParamValue->pstParam = pstParam;
          pstParamValue->s32Index = i;
          pstParamValue->fValue   = _afValueList[orxMAX(i, 0)];

          /* Adds it to list */
          orxLinkList_AddEnd(&(_pstShader->stParamValueList), &(pstParamValue->stNode));

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for float parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a texture parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable
 * @param[in] _apstValueList          Parameter's texture value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddTextureParam(orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxTEXTURE **_apstValueList)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM *pstParam;

    /* Allocates param */
    pstParam = (orxSHADER_PARAM *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      orxS32 i;

      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM));

      /* Inits it */
      pstParam->eType         = orxSHADER_PARAM_TYPE_TEXTURE;
      pstParam->zName         = orxString_Store(_zName);
      pstParam->u32ArraySize  = _u32ArraySize;

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stNode));

      /* For all array indices */
      for(i = (_u32ArraySize != 0) ? 0 : -1; i < (orxS32)_u32ArraySize; i++)
      {
        orxSHADER_PARAM_VALUE *pstParamValue;

        /* Allocates it */
        pstParamValue = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamValueBank);

        /* Valid? */
        if(pstParamValue != orxNULL)
        {
          /* Clears it */
          orxMemory_Zero(pstParamValue, sizeof(orxSHADER_PARAM_VALUE));

          /* Inits it */
          pstParamValue->pstParam = pstParam;
          pstParamValue->s32Index = i;
          pstParamValue->pstValue = _apstValueList[orxMAX(i, 0)];

          /* Adds it to list */
          orxLinkList_AddEnd(&(_pstShader->stParamValueList), &(pstParamValue->stNode));

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for texture parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a vector parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable
 * @param[in] _avValueList            Parameter's vector value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddVectorParam(orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxVECTOR *_avValueList)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  orxASSERT(_avValueList != orxNULL);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM *pstParam;

    /* Allocates param */
    pstParam = (orxSHADER_PARAM *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      orxS32 i;

      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM));

      /* Inits it */
      pstParam->eType         = orxSHADER_PARAM_TYPE_VECTOR;
      pstParam->zName         = orxString_Store(_zName);
      pstParam->u32ArraySize  = _u32ArraySize;

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stNode));

      /* For all array indices */
      for(i = (_u32ArraySize != 0) ? 0 : -1; i < (orxS32)_u32ArraySize; i++)
      {
        orxSHADER_PARAM_VALUE *pstParamValue;

        /* Allocates it */
        pstParamValue = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamValueBank);

        /* Valid? */
        if(pstParamValue != orxNULL)
        {
          /* Clears it */
          orxMemory_Zero(pstParamValue, sizeof(orxSHADER_PARAM_VALUE));

          /* Inits it */
          pstParamValue->pstParam = pstParam;
          pstParamValue->s32Index = i;
          orxVector_Copy(&(pstParamValue->vValue), &(_avValueList[orxMAX(i, 0)]));

          /* Adds it to list */
          orxLinkList_AddEnd(&(_pstShader->stParamValueList), &(pstParamValue->stNode));

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for vector parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
        }
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a time parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddTimeParam(orxSHADER *_pstShader, const orxSTRING _zName)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM *pstParam;

    /* Allocates param */
    pstParam = (orxSHADER_PARAM *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      orxSHADER_PARAM_VALUE *pstParamValue;

      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM));

      /* Inits it */
      pstParam->eType         = orxSHADER_PARAM_TYPE_TIME;
      pstParam->zName         = orxString_Store(_zName);
      pstParam->u32ArraySize  = 0;

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stNode));

      /* Allocates value */
      pstParamValue = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamValueBank);

      /* Valid? */
      if(pstParamValue != orxNULL)
      {
        /* Clears it */
        orxMemory_Zero(pstParamValue, sizeof(orxSHADER_PARAM_VALUE));

        /* Inits it */
        pstParamValue->pstParam = pstParam;
        pstParamValue->s32Index = -1;
        pstParamValue->fValue   = orxFLOAT_0;

        /* Adds it to list */
        orxLinkList_AddEnd(&(_pstShader->stParamValueList), &(pstParamValue->stNode));

        /* Updates result */
        eResult = orxSTATUS_SUCCESS;
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for float parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
      }
    }
  }

  /* Done! */
  return eResult;
}

/** Sets the default value for a given float parameter in a shader (parameters need to be added beforehand)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable, has to match the size used when declaring the parameter
 * @param[in] _afValueList            Parameter's float value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_SetFloatParam(const orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxFLOAT *_afValueList)
{
  orxSHADER_PARAM  *pstParam;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  orxASSERT(_afValueList != orxNULL);

  /* For all defined parameters? */
  for(pstParam = (orxSHADER_PARAM *)orxLinkList_GetFirst(&(_pstShader->stParamList));
      pstParam != orxNULL;
      pstParam = (orxSHADER_PARAM *)orxLinkList_GetNext(&(pstParam->stNode)))
  {
    /* Found? */
    if(!orxString_Compare(_zName, pstParam->zName))
    {
      /* Correct type? */
      if(pstParam->eType == orxSHADER_PARAM_TYPE_FLOAT)
      {
        /* Correct size? */
        if(pstParam->u32ArraySize == _u32ArraySize)
        {
          orxSHADER_PARAM_VALUE  *pstParamValue;
          orxS32                  i;

          /* For all values */
          for(i = (_u32ArraySize != 0) ? 0 : -1, pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
              (i < (orxS32)_u32ArraySize) && (pstParamValue != orxNULL);
              pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
          {
            /* Match current param? */
            if(pstParamValue->pstParam == pstParam)
            {
              /* Updates it */
              pstParamValue->fValue = _afValueList[orxMAX(i, 0)];

              /* Updates index */
              i++;
            }
          }

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't set value(s) for parameter <%s>: needed %u values, %u were given.", _pstShader->zReference, _pstShader, _zName, orxLinkList_GetCount(&(_pstShader->stParamValueList)), _u32ArraySize);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't set value(s) for parameter <%s>: incorrect type (float) used.", _pstShader->zReference, _pstShader, _zName);
      }

      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Sets the default value for a given float parameter in a shader (parameters need to be added beforehand)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable, has to match the size used when declaring the parameter
 * @param[in] _apstValueList          Parameter's texture value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_SetTextureParam(const orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxTEXTURE **_apstValueList)
{
  orxSHADER_PARAM  *pstParam;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  orxASSERT(_apstValueList != orxNULL);

  /* For all defined parameters? */
  for(pstParam = (orxSHADER_PARAM *)orxLinkList_GetFirst(&(_pstShader->stParamList));
      pstParam != orxNULL;
      pstParam = (orxSHADER_PARAM *)orxLinkList_GetNext(&(pstParam->stNode)))
  {
    /* Found? */
    if(!orxString_Compare(_zName, pstParam->zName))
    {
      /* Correct type? */
      if(pstParam->eType == orxSHADER_PARAM_TYPE_TEXTURE)
      {
        /* Correct size? */
        if(pstParam->u32ArraySize == _u32ArraySize)
        {
          orxSHADER_PARAM_VALUE  *pstParamValue;
          orxS32                  i;

          /* For all values */
          for(i = (_u32ArraySize != 0) ? 0 : -1, pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
              (i < (orxS32)_u32ArraySize) && (pstParamValue != orxNULL);
              pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
          {
            /* Match current param? */
            if(pstParamValue->pstParam == pstParam)
            {
              /* Updates it */
              pstParamValue->pstValue = _apstValueList[orxMAX(i, 0)];

              /* Updates index */
              i++;
            }
          }

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't set value(s) for parameter <%s>: needed %u values, %u were given.", _pstShader->zReference, _pstShader, _zName, orxLinkList_GetCount(&(_pstShader->stParamValueList)), _u32ArraySize);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't set value(s) for parameter <%s>: incorrect type (texture) used.", _pstShader->zReference, _pstShader, _zName);
      }

      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Sets the default value for a given float parameter in a shader (parameters need to be added beforehand)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable, has to match the size used when declaring the parameter
 * @param[in] _avValueList            Parameter's vector value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_SetVectorParam(const orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxVECTOR *_avValueList)
{
  orxSHADER_PARAM  *pstParam;
  orxSTATUS         eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  orxASSERT(_avValueList != orxNULL);

  /* For all defined parameters? */
  for(pstParam = (orxSHADER_PARAM *)orxLinkList_GetFirst(&(_pstShader->stParamList));
      pstParam != orxNULL;
      pstParam = (orxSHADER_PARAM *)orxLinkList_GetNext(&(pstParam->stNode)))
  {
    /* Found? */
    if(!orxString_Compare(_zName, pstParam->zName))
    {
      /* Correct type? */
      if(pstParam->eType == orxSHADER_PARAM_TYPE_VECTOR)
      {
        /* Correct size? */
        if(pstParam->u32ArraySize == _u32ArraySize)
        {
          orxSHADER_PARAM_VALUE  *pstParamValue;
          orxS32                  i;

          /* For all values */
          for(i = (_u32ArraySize != 0) ? 0 : -1, pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
              (i < (orxS32)_u32ArraySize) && (pstParamValue != orxNULL);
              pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
          {
            /* Match current param? */
            if(pstParamValue->pstParam == pstParam)
            {
              /* Updates it */
              orxVector_Copy(&(pstParamValue->vValue), &(_avValueList[orxMAX(i, 0)]));

              /* Updates index */
              i++;
            }
          }

          /* Updates result */
          eResult = orxSTATUS_SUCCESS;
        }
        else
        {
          /* Logs message */
          orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't set value(s) for parameter <%s>: needed %u values, %u were given.", _pstShader->zReference, _pstShader, _zName, pstParam->u32ArraySize, _u32ArraySize);
        }
      }
      else
      {
        /* Logs message */
        orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Can't set value(s) for parameter <%s>: incorrect type (vector) used.", _pstShader->zReference, _pstShader, _zName);
      }

      break;
    }
  }

  /* Done! */
  return eResult;
}

/** Sets shader code & compiles it (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _azCodeList             List of shader codes to compile (parameters need to be set beforehand), will be processed in order
 * @param[in] _u32Size                Size of the shader code list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_CompileCode(orxSHADER *_pstShader, const orxSTRING *_azCodeList, orxU32 _u32Size)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Profiles */
  orxPROFILER_PUSH_MARKER("orxShader_CompileCode");

  /* Has a compiled shader? */
  if(orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_COMPILED))
  {
    /* Deletes its data */
    orxDisplay_DeleteShader(_pstShader->hData);

    /* Updates flags */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_NONE, orxSHADER_KU32_FLAG_COMPILED);
  }

  /* Valid? */
  if((_azCodeList != orxNULL) && (_u32Size > 0))
  {
    /* Creates compiled shader */
    _pstShader->hData = orxDisplay_CreateShader(_azCodeList, _u32Size, &(_pstShader->stParamList), orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM) ? orxTRUE : orxFALSE);

    /* Success? */
    if(_pstShader->hData != orxHANDLE_UNDEFINED)
    {
      orxSHADER_PARAM_VALUE *pstParamValue;

      /* Updates flags */
      orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_COMPILED, orxSHADER_KU32_FLAG_NONE);

      /* For all parameter values */
      for(pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamValueList));
          pstParamValue != orxNULL;
          pstParamValue = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParamValue->stNode)))
      {
        /* Gets its ID */
        pstParamValue->s32ID = orxDisplay_GetParameterID(_pstShader->hData, pstParamValue->pstParam->zName, pstParamValue->s32Index, (pstParamValue->pstParam->eType == orxSHADER_PARAM_TYPE_TEXTURE) ? orxTRUE : orxFALSE);
      }
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Profiles */
  orxPROFILER_POP_MARKER();

  /* Done! */
  return eResult;
}

/** Enables/disables a shader
 * @param[in]   _pstShader            Concerned Shader
 * @param[in]   _bEnable              Enable / disable
 */
void orxFASTCALL    orxShader_Enable(orxSHADER *_pstShader, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED, orxSHADER_KU32_FLAG_NONE);
  }
  else
  {
    /* Updates status flags */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_NONE, orxSHADER_KU32_FLAG_ENABLED);
  }

  return;
}

/** Is shader enabled?
 * @param[in]   _pstShader            Concerned Shader
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
orxBOOL orxFASTCALL orxShader_IsEnabled(const orxSHADER *_pstShader)
{
  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Done! */
  return(orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED));
}

/** Gets shader name
 * @param[in]   _pstShader            Concerned Shader
 * @return      orxSTRING / orxSTRING_EMPTY
 */
const orxSTRING orxFASTCALL orxShader_GetName(const orxSHADER *_pstShader)
{
  const orxSTRING zResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Has reference? */
  if(_pstShader->zReference != orxNULL)
  {
    /* Updates result */
    zResult = _pstShader->zReference;
  }
  else
  {
    /* Updates result */
    zResult = orxSTRING_EMPTY;
  }

  /* Done! */
  return zResult;
}

/** Gets shader (internal) ID
 * @param[in]   _pstShader            Concerned Shader
 * @return      ID
 */
orxU32 orxFASTCALL orxShader_GetID(const orxSHADER *_pstShader)
{
  orxU32 u32Result;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Updates result */
  u32Result = orxDisplay_GetShaderID(_pstShader->hData);

  /* Done! */
  return u32Result;
}

#ifdef __orxGCC__

  #pragma GCC diagnostic pop

#endif /* __orxGCC__ */

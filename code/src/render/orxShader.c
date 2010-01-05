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
 * @file orxShader.c
 * @date 11/04/2009
 * @author iarwain@orx-project.org
 *
 */


#include "render/orxShader.h"

#include "debug/orxDebug.h"
#include "memory/orxMemory.h"
#include "core/orxConfig.h"
#include "core/orxEvent.h"
#include "display/orxGraphic.h"
#include "object/orxStructure.h"
#include "render/orxViewport.h"
#include "utils/orxHashTable.h"
#include "utils/orxString.h"


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

#define orxSHADER_KU32_MASK_ALL               0xFFFFFFFF  /**< All mask */

/** Misc defines
 */
#define orxSHADER_KU32_REFERENCE_TABLE_SIZE   16
#define orxSHADER_KU32_PARAM_BANK_SIZE        4

#define orxSHADER_KZ_CONFIG_CODE              "Code"
#define orxSHADER_KZ_CONFIG_PARAM_LIST        "ParamList"
#define orxSHADER_KZ_CONFIG_USE_CUSTOM_PARAM  "UseCustomParam"
#define orxSHADER_KZ_CONFIG_KEEP_IN_CACHE     "KeepInCache"

#define orxSHADER_KZ_SCREEN                   "screen"


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Shader param-value structure
 */
typedef struct __orxSHADER_PARAM_VALUE_t
{
  orxSHADER_PARAM stParam;                                /**< Param definition : 20 */
  
  union
  {
    orxFLOAT    fValue;                                   /**< Float value : 24 */
    orxTEXTURE *pstValue;                                 /**< Texture value : 24 */
    orxVECTOR   vValue;                                   /**< Vector value : 24 */
  };                                                      /**< Union value : 32 */

} orxSHADER_PARAM_VALUE;

/** Shader structure
 */
struct __orxSHADER_t
{
  orxSTRUCTURE  stStructure;                              /**< Public structure, first structure member : 16 */
  orxLINKLIST   stParamList;                              /**< Parameter list : 28 */
  orxSTRING     zReference;                               /**< Shader reference : 32 */
  orxHANDLE     hData;                                    /**< Compiled shader data : 36 */
  orxBANK      *pstParamBank;                             /**< Parameter bank : 40 */
};

/** Static structure
 */
typedef struct __orxSHADER_STATIC_t
{
  orxU32        u32Flags;                                 /**< Control flags */
  orxHASHTABLE *pstReferenceTable;                        /**< Reference hash table */

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
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_STRUCTURE);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_CONFIG);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_EVENT);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_DISPLAY);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_GRAPHIC);
  orxModule_AddDependency(orxMODULE_ID_SHADER, orxMODULE_ID_TEXTURE);

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
      eResult = orxSTRUCTURE_REGISTER(SHADER, orxSTRUCTURE_STORAGE_TYPE_LINKLIST, orxMEMORY_TYPE_MAIN, orxNULL);
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
  if(eResult == orxSTATUS_SUCCESS)
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
    /* Creates its parameter bank */
    pstResult->pstParamBank = orxBank_Create(orxSHADER_KU32_PARAM_BANK_SIZE, sizeof(orxSHADER_PARAM_VALUE), orxBANK_KU32_FLAG_NONE, orxMEMORY_TYPE_MAIN);

    /* Valid? */
    if(pstResult->pstParamBank != orxNULL)
    {
      /* Clears its data */
      pstResult->hData = orxHANDLE_UNDEFINED;

      /* Inits flags */
      orxStructure_SetFlags(pstResult, orxSHADER_KU32_FLAG_ENABLED, orxSHADER_KU32_MASK_ALL);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Failed to allocate shader parameter bank.");

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
  orxU32      u32ID;
  orxSHADER  *pstResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxASSERT((_zConfigID != orxNULL) && (_zConfigID != orxSTRING_EMPTY));

  /* Gets shader ID */
  u32ID = orxString_ToCRC(_zConfigID);

  /* Search for reference */
  pstResult = (orxSHADER *)orxHashTable_Get(sstShader.pstReferenceTable, u32ID);

  /* Not already created? */
  if(pstResult == orxNULL)
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
        /* Stores its reference */
        pstResult->zReference = orxConfig_GetCurrentSection();

        /* Protects it */
        orxConfig_ProtectSection(pstResult->zReference, orxTRUE);

        /* Adds it to reference table */
        if(orxHashTable_Add(sstShader.pstReferenceTable, u32ID, pstResult) != orxSTATUS_FAILURE)
        {
          orxS32    i, s32Number;
          orxSTRING zCode;

          /* For all parameters */
          for(i = 0, s32Number = orxConfig_GetListCounter(orxSHADER_KZ_CONFIG_PARAM_LIST); i < s32Number; i++)
          {
            orxSTRING zParamName;

            /* Gets its name */
            zParamName = orxConfig_GetListString(orxSHADER_KZ_CONFIG_PARAM_LIST, i);

            /* Valid? */
            if((zParamName != orxNULL) && (zParamName != orxSTRING_EMPTY))
            {
              orxVECTOR vValue;

              /* Is a vector? */
              if(orxConfig_GetVector(zParamName, &vValue) != orxNULL)
              {
                /* Adds vector param */
                orxShader_AddVectorParam(pstResult, zParamName, &vValue);
              }
              else
              {
                orxSTRING zValue;
                orxFLOAT  fValue;

                /* Gets its literal value */
                zValue = orxString_LowerCase(orxConfig_GetString(zParamName));

                /* Is a float? */
                if(orxString_ToFloat(zValue, &fValue, orxNULL) != orxSTATUS_FAILURE)
                {
                  /* Adds float param */
                  orxShader_AddFloatParam(pstResult, zParamName, fValue);
                }
                else
                {
                  orxTEXTURE *pstTexture;

                  /* Valid? */
                  if(zValue != orxSTRING_EMPTY)
                  {
                    /* Is screen? */
                    if(!orxString_Compare(zValue, orxSHADER_KZ_SCREEN))
                    {
                      /* Gets its texture */
                      pstTexture = orxTexture_CreateFromFile(orxTEXTURE_KZ_SCREEN_NAME);
                    }
                    else
                    {
                      /* Creates texture */
                      pstTexture = orxTexture_CreateFromFile(zValue);
                    }
                  }
                  else
                  {
                    /* No texture */
                    pstTexture = orxNULL;
                  }

                  /* Adds texture param */
                  orxShader_AddTextureParam(pstResult, zParamName, pstTexture);
                }
              }
            }
          }

          /* Gets code */
          zCode = orxConfig_GetString(orxSHADER_KZ_CONFIG_CODE);

          /* Valid? */
          if(zCode != orxSTRING_EMPTY)
          {
            /* Sets it */
            orxShader_CompileCode(pstResult, zCode);
          }

          /* Should keep it in cache? */
          if(orxConfig_GetBool(orxSHADER_KZ_CONFIG_KEEP_IN_CACHE) != orxFALSE)
          {
            /* Increases its reference counter to keep it in cache table */
            orxStructure_IncreaseCounter(pstResult);
          }

          /* Use custom param? */
          if(orxConfig_GetBool(orxSHADER_KZ_CONFIG_USE_CUSTOM_PARAM) != orxFALSE)
          {
            /* Updates status */
            orxStructure_SetFlags(pstResult, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM, orxSHADER_KU32_FLAG_NONE);
          }
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
  else
  {
    /* Updates reference counter */
    orxStructure_IncreaseCounter(pstResult);
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

  /* Has an ID? */
  if((_pstShader->zReference != orxNULL)
  && (_pstShader->zReference != orxSTRING_EMPTY))
  {
    /* Not referenced? */
    if(orxStructure_GetRefCounter(_pstShader) == 0)
    {
      orxSHADER_PARAM_VALUE *pstParam;

      /* Removes from hashtable */
      orxHashTable_Remove(sstShader.pstReferenceTable, orxString_ToCRC(_pstShader->zReference));

      /* Unprotects it */
      orxConfig_ProtectSection(_pstShader->zReference, orxFALSE);

      /* Has data? */
      if(orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_COMPILED))
      {
        /* Deletes it */
        orxDisplay_DeleteShader(_pstShader->hData);
      }

      /* For all parameters */
      for(pstParam = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamList));
          pstParam != orxNULL;
          pstParam = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParam->stParam.stNode)))
      {
        /* Deletes its name */
        orxString_Delete(pstParam->stParam.zName);

        /* Is a texture? */
        if(pstParam->stParam.eType == orxSHADER_PARAM_TYPE_TEXTURE)
        {
          /* Is valid? */
          if(pstParam->pstValue != orxNULL)
          {
            /* Deletes it */
            orxTexture_Delete(pstParam->pstValue);
          }
        }
      }

      /* Deletes param bank */
      orxBank_Delete(_pstShader->pstParamBank);

      /* Deletes structure */
      orxStructure_Delete(_pstShader);
    }
    else
    {
      /* Decreases its reference counter */
      orxStructure_DecreaseCounter(_pstShader);
    }
  }
  else
  {
    /* Not referenced? */
    if(orxStructure_GetRefCounter(_pstShader) == 0)
    {
      /* Deletes structure */
      orxStructure_Delete(_pstShader);
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Cannot delete shader while it is still being referenced.");

      /* Referenced by others */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Renders a shader
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _pstOwner               Owner structure (orxOBJECT / orxVIEWPORT / orxNULL)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_Render(const orxSHADER *_pstShader, const orxSTRUCTURE *_pstOwner)
{
  orxSTATUS eResult;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  if(_pstOwner != orxNULL)
  {
    orxSTRUCTURE_ASSERT(_pstOwner);
  }

  /* Valid & enabled? */
  if((_pstShader->hData != orxHANDLE_UNDEFINED) && (orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_ENABLED)))
  {
    orxTEXTURE            *pstOwnerTexture = orxNULL;
    orxSHADER_PARAM_VALUE *pstParam;

    /* Has owner? */
    if(_pstOwner != orxNULL)
    {
      /* Depending on its type */
      switch(orxStructure_GetID(_pstOwner))
      {
        case orxSTRUCTURE_ID_OBJECT:
        {
          orxGRAPHIC *pstGraphic;

          /* Gets its graphic */
          pstGraphic = orxOBJECT_GET_STRUCTURE(orxOBJECT(_pstOwner), GRAPHIC);

          /* Valid? */
          if(pstGraphic != orxNULL)
          {
            /* Updates owner texture */
            pstOwnerTexture = orxTEXTURE(orxGraphic_GetData(pstGraphic));
          }

          break;
        }

        case orxSTRUCTURE_ID_VIEWPORT:
        {
          /* Updates owner texture */
          pstOwnerTexture = orxViewport_GetTexture(orxVIEWPORT(_pstOwner));

          break;
        }

        default:
        {
          break;
        }
      }
    }

    /* For all parameters */
    for(pstParam = (orxSHADER_PARAM_VALUE *)orxLinkList_GetFirst(&(_pstShader->stParamList));
        pstParam != orxNULL;
        pstParam = (orxSHADER_PARAM_VALUE *)orxLinkList_GetNext(&(pstParam->stParam.stNode)))
    {
      /* No custom param? */
      if(!orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_USE_CUSTOM_PARAM))
      {
        /* Depending on parameter type */
        switch(pstParam->stParam.eType)
        {
          case orxSHADER_PARAM_TYPE_FLOAT:
          {
            /* Sets it */
            orxDisplay_SetShaderFloat(_pstShader->hData, pstParam->stParam.zName, pstParam->fValue);

            break;
          }

          case orxSHADER_PARAM_TYPE_TEXTURE:
          {
            orxBITMAP *pstBitmap;

            /* Has default texture? */
            if(pstParam->pstValue != orxNULL)
            {
              /* Gets its bitmap */
              pstBitmap = orxTexture_GetBitmap(pstParam->pstValue);
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
            orxDisplay_SetShaderBitmap(_pstShader->hData, pstParam->stParam.zName, pstBitmap);

            break;
          }

          case orxSHADER_PARAM_TYPE_VECTOR:
          {
            /* Sets it */
            orxDisplay_SetShaderVector(_pstShader->hData, pstParam->stParam.zName, &(pstParam->vValue));

            break;
          }

          default:
          {
            break;
          }
        }
      }
      else
      {
        orxSHADER_EVENT_PARAM_PAYLOAD stPayload;

        /* Inits payload */
        stPayload.pstShader   = _pstShader;
        stPayload.zShaderName = _pstShader->zReference;
        stPayload.zParamName  = pstParam->stParam.zName;
        stPayload.eParamType  = pstParam->stParam.eType;

        /* Depending on type */
        switch(stPayload.eParamType)
        {
          case orxSHADER_PARAM_TYPE_FLOAT:
          {
            /* Updates value */
            stPayload.fValue = pstParam->fValue;

            break;
          }

          case orxSHADER_PARAM_TYPE_TEXTURE:
          {
            /* Updates value */
            stPayload.pstValue = (pstParam->pstValue != orxNULL) ? pstParam->pstValue : pstOwnerTexture;

            break;
          }

          case orxSHADER_PARAM_TYPE_VECTOR:
          {
            /* Updates value */
            orxVector_Copy(&(stPayload.vValue), &(pstParam->vValue));

            break;
          }

          default:
          {
            break;
          }
        }

        /* Sends event */
        orxEVENT_SEND(orxEVENT_TYPE_SHADER, orxSHADER_EVENT_SET_PARAM, _pstOwner, _pstOwner, &stPayload);

        /* Depending on parameter type */
        switch(stPayload.eParamType)
        {
          case orxSHADER_PARAM_TYPE_FLOAT:
          {
            /* Sets it */
            orxDisplay_SetShaderFloat(_pstShader->hData, stPayload.zParamName, stPayload.fValue);

            break;
          }

          case orxSHADER_PARAM_TYPE_TEXTURE:
          {
            /* Sets it */
            orxDisplay_SetShaderBitmap(_pstShader->hData, stPayload.zParamName, (stPayload.pstValue != orxNULL) ? orxTexture_GetBitmap(stPayload.pstValue) : orxNULL);

            break;
          }

          case orxSHADER_PARAM_TYPE_VECTOR:
          {
            /* Sets it */
            orxDisplay_SetShaderVector(_pstShader->hData, stPayload.zParamName, &(stPayload.vValue));

            break;
          }

          default:
          {
            break;
          }
        }
      }
    }

    /* Renders it */
    eResult = orxDisplay_RenderShader(_pstShader->hData);
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
 * @param[in] _fValue                 Parameter's float value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddFloatParam(orxSHADER *_pstShader, const orxSTRING _zName, orxFLOAT _fValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM_VALUE *pstParam;

    /* Allocates it */
    pstParam = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM_VALUE));

      /* Inits it */
      pstParam->stParam.eType = orxSHADER_PARAM_TYPE_FLOAT;
      pstParam->stParam.zName = orxString_Duplicate(_zName);
      pstParam->fValue        = _fValue;

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stParam.stNode));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for float parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a texture parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _pstValue               Parameter's texture value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddTextureParam(orxSHADER *_pstShader, const orxSTRING _zName, orxTEXTURE *_pstValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM_VALUE *pstParam;

    /* Allocates it */
    pstParam = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM_VALUE));

      /* Inits it */
      pstParam->stParam.eType = orxSHADER_PARAM_TYPE_TEXTURE;
      pstParam->stParam.zName = orxString_Duplicate(_zName);
      pstParam->pstValue      = _pstValue;

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stParam.stNode));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for texture parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
    }
  }

  /* Done! */
  return eResult;
}

/** Adds a vector parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _pvValue                Parameter's vector value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_AddVectorParam(orxSHADER *_pstShader, const orxSTRING _zName, const orxVECTOR *_pvValue)
{
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);
  orxASSERT(_pvValue != orxNULL);

  /* Valid? */
  if((_zName != orxNULL) && (_zName != orxSTRING_EMPTY))
  {
    orxSHADER_PARAM_VALUE *pstParam;

    /* Allocates it */
    pstParam = (orxSHADER_PARAM_VALUE *)orxBank_Allocate(_pstShader->pstParamBank);

    /* Valid? */
    if(pstParam != orxNULL)
    {
      /* Clears it */
      orxMemory_Zero(pstParam, sizeof(orxSHADER_PARAM_VALUE));

      /* Inits it */
      pstParam->stParam.eType = orxSHADER_PARAM_TYPE_VECTOR;
      pstParam->stParam.zName = orxString_Duplicate(_zName);
      orxVector_Copy(&(pstParam->vValue), _pvValue);

      /* Adds it to list */
      orxLinkList_AddEnd(&(_pstShader->stParamList), &(pstParam->stParam.stNode));

      /* Updates result */
      eResult = orxSTATUS_SUCCESS;
    }
    else
    {
      /* Logs message */
      orxDEBUG_PRINT(orxDEBUG_LEVEL_RENDER, "Shader [%s/%x]: Couldn't allocate space for vector parameter <%s>.", _pstShader->zReference, _pstShader, _zName);
    }
  }

  /* Done! */
  return eResult;
}

/** Sets shader code (& compiles it)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zCode                  Shader's code (will be compiled immediately)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL orxShader_CompileCode(orxSHADER *_pstShader, const orxSTRING _zCode)
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Has a compiled shader? */
  if(orxStructure_TestFlags(_pstShader, orxSHADER_KU32_FLAG_COMPILED))
  {
    /* Deletes its data */
    orxDisplay_DeleteShader(_pstShader->hData);

    /* Updates flags */
    orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_NONE, orxSHADER_KU32_FLAG_COMPILED);
  }

  /* Valid? */
  if((_zCode != orxNULL) && (_zCode != orxSTRING_EMPTY))
  {
    /* Creates compiled shader */
    _pstShader->hData = orxDisplay_CreateShader(_zCode, &(_pstShader->stParamList));

    /* Success? */
    if(_pstShader->hData != orxHANDLE_UNDEFINED)
    {
      /* Updates flags */
      orxStructure_SetFlags(_pstShader, orxSHADER_KU32_FLAG_COMPILED, orxSHADER_KU32_FLAG_NONE);
    }
    else
    {
      /* Updates result */
      eResult = orxSTATUS_FAILURE;
    }
  }

  /* Done! */
  return eResult;
}

/** Gets shader parameter list
 * @param[in] _pstShader              Concerned Shader
 * @return orxLINKLIST / orxNULL
 */
const orxLINKLIST *orxFASTCALL orxShader_GetParamList(const orxSHADER *_pstShader)
{
  /* Checks */
  orxASSERT(sstShader.u32Flags & orxSHADER_KU32_STATIC_FLAG_READY);
  orxSTRUCTURE_ASSERT(_pstShader);

  /* Done! */
  return(&(_pstShader->stParamList));
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
  orxSTRING zResult;

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

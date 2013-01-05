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
 * @file orxShader.h
 * @date 11/04/2009
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxShader
 *
 * Shader module
 * Allows to store shader information (code + parameters).
 *
 * @{
 */


#ifndef _orxSHADER_H_
#define _orxSHADER_H_


#include "orxInclude.h"
#include "math/orxVector.h"
#include "display/orxTexture.h"
#include "object/orxObject.h"
#include "utils/orxLinkList.h"


/** Shader parameter type
 */
typedef enum __orxSHADER_PARAM_TYPE_t
{
  orxSHADER_PARAM_TYPE_FLOAT = 0,
  orxSHADER_PARAM_TYPE_TEXTURE,
  orxSHADER_PARAM_TYPE_VECTOR,
  orxSHADER_PARAM_TYPE_TIME,

  orxSHADER_PARAM_TYPE_NUMBER,

  orxSHADER_PARAM_TYPE_NONE = orxENUM_NONE

} orxSHADER_PARAM_TYPE;


/** Shader parameter structure
 */
typedef struct __orxSHADER_PARAM_t
{
  orxLINKLIST_NODE      stNode;                 /**< Linklist node : 12 */
  orxSHADER_PARAM_TYPE  eType;                  /**< Parameter type : 16 */
  const orxSTRING       zName;                  /**< Parameter literal name : 20 */
  orxU32                u32ArraySize;           /**< Parameter array size : 24 */

} orxSHADER_PARAM;


/** Internal shader structure
 */
typedef struct __orxSHADER_t                    orxSHADER;


/** Event enum
 */
typedef enum __orxSHADER_EVENT_t
{
  orxSHADER_EVENT_SET_PARAM = 0,                /**< Event sent when setting a parameter */

  orxSHADER_EVENT_NUMBER,

  orxSHADER_EVENT_NONE = orxENUM_NONE

} orxSHADER_EVENT;

/** Shader event payload
 */
typedef struct __orxSHADER_EVENT_PAYLOAD_t
{
  const orxSHADER      *pstShader;              /**< Shader reference : 4 */
  const orxSTRING       zShaderName;            /**< Shader name : 8 */

  orxSHADER_PARAM_TYPE  eParamType;             /**< Parameter type : 12 */
  const orxSTRING       zParamName;             /**< Parameter name : 16 */
  orxS32                s32ParamIndex;          /**< Parameter index : 20 */

  union
  {
    orxFLOAT            fValue;                 /**< Float value : 24 */
    const orxTEXTURE   *pstValue;               /**< Texture value : 24 */
    orxVECTOR           vValue;                 /**< Vector value : 24 */
  };                                            /**< Union value : 32 */

} orxSHADER_EVENT_PAYLOAD;


/** Shader module setup
 */
extern orxDLLAPI void orxFASTCALL               orxShader_Setup();

/** Inits the shader module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_Init();

/** Exits from the shader module
 */
extern orxDLLAPI void orxFASTCALL               orxShader_Exit();

/** Creates an empty shader
 * @return orxSHADER / orxNULL
 */
extern orxDLLAPI orxSHADER *orxFASTCALL         orxShader_Create();

/** Creates a shader from config
 * @param[in]   _zConfigID            Config ID
 * @ return orxSHADER / orxNULL
 */
extern orxDLLAPI orxSHADER *orxFASTCALL         orxShader_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a shader
 * @param[in] _pstShader              Concerned Shader
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_Delete(orxSHADER *_pstShader);

/** Starts a shader
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _pstOwner               Owner structure (orxOBJECT / orxVIEWPORT / orxNULL)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_Start(const orxSHADER *_pstShader, const orxSTRUCTURE *_pstOwner);

/** Stops a shader
 * @param[in] _pstShader              Concerned Shader
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_Stop(const orxSHADER *_pstShader);


/** Adds a float parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable
 * @param[in] _afValueList            Parameter's float value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddFloatParam(orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxFLOAT *_afValueList);

/** Adds a texture parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 simple variable
 * @param[in] _apstValueList          Parameter's texture value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddTextureParam(orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxTEXTURE **_apstValueList);

/** Adds a vector parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _u32ArraySize           Parameter's array size, 0 for simple variable
 * @param[in] _avValueList            Parameter's vector value list
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddVectorParam(orxSHADER *_pstShader, const orxSTRING _zName, orxU32 _u32ArraySize, const orxVECTOR *_avValueList);

/** Adds a time parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddTimeParam(orxSHADER *_pstShader, const orxSTRING _zName);

/** Sets shader code & compiles it (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zCode                  Shader's code to compile (parameters need to be set beforehand)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_CompileCode(orxSHADER *_pstShader, const orxSTRING _zCode);

/** Gets shader parameter list
 * @param[in] _pstShader              Concerned Shader
 * @return orxLINKLIST / orxNULL
 */
extern orxDLLAPI const orxLINKLIST *orxFASTCALL orxShader_GetParamList(const orxSHADER *_pstShader);

/** Enables/disables a shader
 * @param[in]   _pstShader            Concerned Shader
 * @param[in]   _bEnable              Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxShader_Enable(orxSHADER *_pstShader, orxBOOL _bEnable);

/** Is shader enabled?
 * @param[in]   _pstShader            Concerned Shader
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxShader_IsEnabled(const orxSHADER *_pstShader);

/** Gets shader name
 * @param[in]   _pstShader            Concerned Shader
 * @return      orxSTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL    orxShader_GetName(const orxSHADER *_pstShader);

#endif /* _orxSHADER_H_ */

/** @} */

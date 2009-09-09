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

  orxSHADER_PARAM_TYPE_NUMBER,

  orxSHADER_PARAM_TYPE_NONE = orxENUM_NONE

} orxSHADER_PARAM_TYPE;


/** Shader parameter structure
 */
typedef struct __orxSHADER_PARAM_t
{
  orxLINKLIST_NODE      stNode;                 /**< Linklist node : 12 */
  orxSHADER_PARAM_TYPE  eType;                  /**< Parameter type : 16 */
  orxSTRING             zName;                  /**< Parameter literal name : 20 */

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
typedef struct __orxSHADER_EVENT_PARAM_PAYLOAD_t
{
  const orxSHADER      *pstShader;              /**< Shader reference : 4 */
  orxSTRING             zShaderName;            /**< Shader name : 8 */

  orxSHADER_PARAM_TYPE  eParamType;             /**< Parameter type : 12 */
  orxSTRING             zParamName;             /**< Parameter name : 16 */

  union
  {
    orxFLOAT    fValue;                         /**< Float value : 20 */
    orxTEXTURE *pstValue;                       /**< Texture value : 20 */
    orxVECTOR   vValue;                         /**< Vector value : 20 */
  };                                            /**< Union value : 28 */

} orxSHADER_EVENT_PARAM_PAYLOAD;


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

/** Renders a shader
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _pstOwner               Owner structure (orxOBJECT / orxVIEWPORT / orxNULL)
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_Render(const orxSHADER *_pstShader, const orxSTRUCTURE *_pstOwner);


/** Adds a float parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _fValue                 Parameter's float value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddFloatParam(orxSHADER *_pstShader, const orxSTRING _zName, orxFLOAT _fValue);

/** Adds a texture parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _pstValue               Parameter's texture value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddTextureParam(orxSHADER *_pstShader, const orxSTRING _zName, orxTEXTURE *_pstValue);

/** Adds a vector parameter definition to a shader (parameters need to be set before compiling the shader code)
 * @param[in] _pstShader              Concerned Shader
 * @param[in] _zName                  Parameter's literal name
 * @param[in] _pvValue                Parameter's vector value
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShader_AddVectorParam(orxSHADER *_pstShader, const orxSTRING _zName, const orxVECTOR *_pvValue);

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

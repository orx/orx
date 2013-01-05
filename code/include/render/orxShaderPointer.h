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
 * @file orxShaderPointer.h
 * @date 08/04/2009
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxShaderPointer
 *
 * ShaderPointer module
 * Allows to creates shaders (rendering post effects) containers for objects.
 *
 * @{
 */


#ifndef _orxSHADERPOINTER_H_
#define _orxSHADERPOINTER_H_


#include "orxInclude.h"
#include "render/orxShader.h"
#include "object/orxStructure.h"


/** Misc defines
 */
#define orxSHADERPOINTER_KU32_SHADER_NUMBER     4


/** Internal ShaderPointer structure */
typedef struct __orxSHADERPOINTER_t             orxSHADERPOINTER;


/** ShaderPointer module setup
 */
extern orxDLLAPI void orxFASTCALL               orxShaderPointer_Setup();

/** Inits the ShaderPointer module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_Init();

/** Exits from the ShaderPointer module
 */
extern orxDLLAPI void orxFASTCALL               orxShaderPointer_Exit();


/** Creates an empty ShaderPointer
 * @param[in]   _pstOwner       ShaderPointer's owner used for rendering (usually an orxOBJECT)
 * @return orxSHADERPOINTER / orxNULL
 */
extern orxDLLAPI orxSHADERPOINTER *orxFASTCALL  orxShaderPointer_Create(const orxSTRUCTURE *_pstOwner);

/** Deletes an ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_Delete(orxSHADERPOINTER *_pstShaderPointer);

/** Starts a ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_Start(const orxSHADERPOINTER *_pstShaderPointer);

/** Stops a ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_Stop(const orxSHADERPOINTER *_pstShaderPointer);

/** Gets an ShaderPointer owner
 * @param[in]   _pstShaderPointer   Concerned ShaderPointer
 * @return      orxSTRUCTURE / orxNULL
 */
extern orxDLLAPI orxSTRUCTURE *orxFASTCALL      orxShaderPointer_GetOwner(const orxSHADERPOINTER *_pstShaderPointer);

/** Enables/disables an ShaderPointer
 * @param[in]   _pstShaderPointer   Concerned ShaderPointer
 * @param[in]   _bEnable        Enable / disable
 */
extern orxDLLAPI void orxFASTCALL               orxShaderPointer_Enable(orxSHADERPOINTER *_pstShaderPointer, orxBOOL _bEnable);

/** Is ShaderPointer enabled?
 * @param[in]   _pstShaderPointer   Concerned ShaderPointer
 * @return      orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL            orxShaderPointer_IsEnabled(const orxSHADERPOINTER *_pstShaderPointer);


/** Adds a shader
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _pstShader        Shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_AddShader(orxSHADERPOINTER *_pstShaderPointer, orxSHADER *_pstShader);

/** Removes a shader
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _pstShader        Shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_RemoveShader(orxSHADERPOINTER *_pstShaderPointer, orxSHADER *_pstShader);

/** Gets a shader
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _u32Index         Index of shader to get
 * @return      orxSJADER / orxNULL
 */
extern orxDLLAPI const orxSHADER *orxFASTCALL   orxShaderPointer_GetShader(const orxSHADERPOINTER *_pstShaderPointer, orxU32 _u32Index);

/** Adds a shader using its config ID
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_AddShaderFromConfig(orxSHADERPOINTER *_pstShaderPointer, const orxSTRING _zShaderConfigID);

/** Removes a shader using its config ID
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _zShaderConfigID  Config ID of the shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_RemoveShaderFromConfig(orxSHADERPOINTER *_pstShaderPointer, const orxSTRING _zShaderConfigID);

#endif /* _orxSHADERPOINTER_H_ */

/** @} */

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

/** Renders a ShaderPointer
 * @param[in] _pstShaderPointer     Concerned ShaderPointer
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_Render(const orxSHADERPOINTER *_pstShaderPointer);

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

/** Adds a shader using its config ID
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _zShaderConfigID  Config ID of the shader to add
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_AddShaderFromConfig(orxSHADERPOINTER *_pstShaderPointer, const orxSTRING _zShaderConfigID);

/** Removes a shader using using its config ID
 * @param[in]   _pstShaderPointer Concerned ShaderPointer
 * @param[in]   _zShaderConfigID  Config ID of the shader to remove
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL          orxShaderPointer_RemoveShaderFromConfig(orxSHADERPOINTER *_pstShaderPointer, const orxSTRING _zShaderConfigID);

#endif /* _orxSHADERPOINTER_H_ */

/** @} */

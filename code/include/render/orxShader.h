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
#include "object/orxObject.h"


/** Internal shader structure
 */
typedef struct __orxSHADER_t                    orxSHADER;


/** Shader module setup
 */
extern orxDLLAPI void                           orxShader_Setup();

/** Inits the shader module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS                      orxShader_Init();

/** Exits from the shader module
 */
extern orxDLLAPI void                           orxShader_Exit();

/** Creates an empty shader
 * @return orxSHADER / orxNULL
 */
extern orxDLLAPI orxSHADER *                    orxShader_Create();

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

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
 * @file orxFont.h
 * @date 08/03/2010
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxFont
 *
 * Font module
 * Module that handles fonts
 *
 * @{
 */


#ifndef _orxFONT_H_
#define _orxFONT_H_

#include "orxInclude.h"

#include "display/orxTexture.h"
#include "math/orxVector.h"


/** Internal font structure */
typedef struct __orxFONT_t                            orxFONT;


/** Setups the font module
 */
extern orxDLLAPI void orxFASTCALL                     orxFont_Setup();

/** Inits the font module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_Init();

/** Exits from the font module
 */
extern orxDLLAPI void orxFASTCALL                     orxFont_Exit();


/** Creates an empty font
 * @return      orxFONT / orxNULL
 */
extern orxDLLAPI orxFONT *orxFASTCALL                 orxFont_Create();

/** Creates a font from config
 * @param[in]   _zConfigID    Config ID
 * @return      orxFONT / orxNULL
 */
extern orxDLLAPI orxFONT *orxFASTCALL                 orxFont_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a font
 * @param[in]   _pstFont      Concerned font
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_Delete(orxFONT *_pstFont);


/** Gets default font
 * @return      Default font / orxNULL
 */
extern orxDLLAPI const orxFONT *orxFASTCALL           orxFont_GetDefaultFont();


/** Sets font's texture
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pstTexture   Texture to set
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_SetTexture(orxFONT *_pstFont, orxTEXTURE *_pstTexture);

/** Sets font's character list
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _zList        Character list
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_SetCharacterList(orxFONT *_pstFont, const orxSTRING _zList);

/** Sets font's character size
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvSize       Character's size
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_SetCharacterSize(orxFONT *_pstFont, const orxVECTOR *_pvSize);

/** Sets font's origin
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvOrigin     Font's origin
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_SetOrigin(orxFONT *_pstFont, const orxVECTOR *_pvOrigin);

/** Sets font's size
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvSize       Font's size
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_SetSize(orxFONT *_pstFont, const orxVECTOR *_pvSize);


/** Gets font's texture
 * @param[in]   _pstFont      Concerned font
 * @return      Font texture / orxNULL
 */
extern orxDLLAPI orxTEXTURE *orxFASTCALL              orxFont_GetTexture(const orxFONT *_pstFont);

/** Gets font's character list
 * @param[in]   _pstFont      Concerned font
 * @return      Font's character list / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxFont_GetCharacterList(const orxFONT *_pstFont);

/** Gets font's character size
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvSize       Character's size
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxFont_GetCharacterSize(const orxFONT *_pstFont, orxVECTOR *_pvSize);

/** Gets font's origin
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvOrigin     Font's origin
 * @return      orxDISPLAY_FONT / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxFont_GetOrigin(const orxFONT *_pstFont, orxVECTOR *_pvOrigin);

/** Gets font's size
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvSize       Font's size
 * @return      orxDISPLAY_FONT / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxFont_GetSize(const orxFONT *_pstFont, orxVECTOR *_pvSize);


/** Gets font's map
 * @param[in]   _pstFont      Concerned font
 * @return      orxCHARACTER_MAP / orxNULL
 */
extern orxDLLAPI const orxCHARACTER_MAP *orxFASTCALL  orxFont_GetMap(const orxFONT *_pstFont);


/** Gets font name
 * @param[in]   _pstFont      Concerned font
 * @return      Font name / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxFont_GetName(const orxFONT *_pstFont);

#endif /* _orxFONT_H_ */

/** @} */

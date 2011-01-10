/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2011 Orx-Project
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

/** Sets font's character spacing
 * @param[in]   _pstFont      Concerned font
 * @param[in]   _pvSpacing    Character's spacing
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxFont_SetCharacterSpacing(orxFONT *_pstFont, const orxVECTOR *_pvSpacing);

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
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxFont_GetCharacterSize(const orxFONT *_pstFont, orxVECTOR *_pvSize);

/** Gets font's character spacing
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvSpacing    Character's spacing
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxFont_GetCharacterSpacing(const orxFONT *_pstFont, orxVECTOR *_pvSpacing);

/** Gets font's origin
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvOrigin     Font's origin
 * @return      orxVECTOR / orxNULL
 */
extern orxDLLAPI orxVECTOR *orxFASTCALL               orxFont_GetOrigin(const orxFONT *_pstFont, orxVECTOR *_pvOrigin);

/** Gets font's size
 * @param[in]   _pstFont      Concerned font
 * @param[out]  _pvSize       Font's size
 * @return      orxVECTOR / orxNULL
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

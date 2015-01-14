/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxText.h
 * @date 02/12/2008
 * @author iarwain@orx-project.org
 *
 * @todo
 */

/**
 * @addtogroup orxText
 *
 * Text module
 * Module that handles texts
 *
 * @{
 */


#ifndef _orxTEXT_H_
#define _orxTEXT_H_

#include "orxInclude.h"

#include "display/orxFont.h"


/** Internal text structure */
typedef struct __orxTEXT_t                orxTEXT;


/** Setups the text module
 */
extern orxDLLAPI void orxFASTCALL         orxText_Setup();

/** Inits the text module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_Init();

/** Exits from the text module
 */
extern orxDLLAPI void orxFASTCALL         orxText_Exit();


/** Creates an empty text
 * @return      orxTEXT / orxNULL
 */
extern orxDLLAPI orxTEXT *orxFASTCALL     orxText_Create();

/** Creates a text from config
 * @param[in]   _zConfigID    Config ID
 * @return      orxTEXT / orxNULL
 */
extern orxDLLAPI orxTEXT *orxFASTCALL     orxText_CreateFromConfig(const orxSTRING _zConfigID);

/** Deletes a text
 * @param[in]   _pstText      Concerned text
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_Delete(orxTEXT *_pstText);


/** Gets text size
 * @param[in]   _pstText      Concerned text
 * @param[out]  _pfWidth      Text's width
 * @param[out]  _pfHeight     Text's height
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_GetSize(const orxTEXT *_pstText, orxFLOAT *_pfWidth, orxFLOAT *_pfHeight);

/** Gets text name
 * @param[in]   _pstText      Concerned text
 * @return      Text name / orxNULL
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxText_GetName(const orxTEXT *_pstText);


/** Gets text string
 * @param[in]   _pstText      Concerned text
 * @return      Text string / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL orxText_GetString(const orxTEXT *_pstText);

/** Gets text font
 * @param[in]   _pstText      Concerned text
 * @return      Text font / orxNULL
 */
extern orxDLLAPI orxFONT *orxFASTCALL     orxText_GetFont(const orxTEXT *_pstText);


/** Sets text string
 * @param[in]   _pstText      Concerned text
 * @param[in]   _zString      String to contain
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_SetString(orxTEXT *_pstText, const orxSTRING _zString);

/** Sets text font
 * @param[in]   _pstText      Concerned text
 * @param[in]   _pstFont      Font / orxNULL to use default
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_SetFont(orxTEXT *_pstText, orxFONT *_pstFont);

#endif /* _orxTEXT_H_ */

/** @} */

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

#include "display/orxDisplay.h"


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
extern orxDLLAPI const orxSTRING orxFASTCALL orxText_GetFont(const orxTEXT *_pstText);

/** Gets text data
 * @param[in]   _pstText      Concerned text
 * @return      orxDISPLAY_TEXT / orxNULL
 */
extern orxDLLAPI orxDISPLAY_TEXT *orxFASTCALL orxText_GetData(const orxTEXT *_pstText);

/** Sets text string 
 * @param[in]   _pstText      Concerned text
 * @param[in]   _zString      String to contain
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_SetString(orxTEXT *_pstText, const orxSTRING _zString);

/** Sets text font
 * @param[in]   _pstText      Concerned text
 * @param[in]   _zFont        Font name / orxNULL to use default
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL    orxText_SetFont(orxTEXT *_pstText, const orxSTRING _zFont);

#endif /* _orxTEXT_H_ */

/** @} */

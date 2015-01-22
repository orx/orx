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
 * @file orxConsole.h
 * @date 13/08/2012
 * @author iarwain@orx-project.org
 *
 */

/**
 * @addtogroup orxConsole
 *
 * Console module
 * Module that can execute consoles and log info at runtime
 * @{
 */

#ifndef _orxCONSOLE_H_
#define _orxCONSOLE_H_


#include "orxInclude.h"
#include "display/orxFont.h"
#include "io/orxInput.h"


/** Inputs
 */

#define orxCONSOLE_KZ_INPUT_SET                       "-=ConsoleSet=-"                /**< Console input set */

#define orxCONSOLE_KZ_INPUT_TOGGLE                    "Toggle"                        /**< Toggle input */
#define orxCONSOLE_KZ_INPUT_AUTOCOMPLETE              "AutoComplete"                  /**< Autocomplete input */
#define orxCONSOLE_KZ_INPUT_DELETE                    "Delete"                        /**< Delete input */
#define orxCONSOLE_KZ_INPUT_DELETE_AFTER              "DeleteAfter"                   /**< Delete after input */
#define orxCONSOLE_KZ_INPUT_TOGGLE_MODE               "ToggleMode"                    /**< Toggle mode input */
#define orxCONSOLE_KZ_INPUT_ENTER                     "Enter"                         /**< Enter input */
#define orxCONSOLE_KZ_INPUT_PREVIOUS                  "Previous"                      /**< Previous input */
#define orxCONSOLE_KZ_INPUT_NEXT                      "Next"                          /**< Next input */
#define orxCONSOLE_KZ_INPUT_LEFT                      "Left"                          /**< Cursor move left */
#define orxCONSOLE_KZ_INPUT_RIGHT                     "Right"                         /**< Cursor move right */
#define orxCONSOLE_KZ_INPUT_START                     "Start"                         /**< Cursor move start */
#define orxCONSOLE_KZ_INPUT_END                       "End"                           /**< Cursor move end */


/** Console module setup
 */
extern orxDLLAPI void orxFASTCALL                     orxConsole_Setup();

/** Inits the console module
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxConsole_Init();

/** Exits from the console module
 */
extern orxDLLAPI void orxFASTCALL                     orxConsole_Exit();


/** Enables/disables the console
 * @param[in]   _bEnable      Enable / disable
 */
extern orxDLLAPI void orxFASTCALL                     orxConsole_Enable(orxBOOL _bEnable);

/** Is the console enabled?
 * @return orxTRUE if enabled, orxFALSE otherwise
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxConsole_IsEnabled();


/** Is the console input in insert mode?
 * @return orxTRUE if insert mode, orxFALSE otherwise (overwrite mode)
 */
extern orxDLLAPI orxBOOL orxFASTCALL                  orxConsole_IsInsertMode();


/** Sets the console toggle
* @param[in] _eInputType      Type of input peripheral
* @param[in] _eInputID        ID of button/key/axis
* @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxConsole_SetToggle(orxINPUT_TYPE _eInputType, orxENUM _eInputID);


/** Logs to the console
 * @param[in]   _zText        Text to log
 * @return orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxConsole_Log(const orxSTRING _zText);


/** Sets the console font
 * @param[in]   _pstFont      Font to use
 * @return orxSTATUS_SUCCESS/ orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxConsole_SetFont(const orxFONT *_pstFont);

/** Gets the console font
 * @return Current in-use font, orxNULL
 */
extern orxDLLAPI const orxFONT *orxFASTCALL           orxConsole_GetFont();


/** Sets the console log line length
 * @param[in]   _u32LineLength Line length to use
 * @return orxSTATUS_SUCCESS/ orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL                orxConsole_SetLogLineLength(orxU32 _u32LineLength);

/** Gets the console log line length
 * @return Console log line length
 */
extern orxDLLAPI orxU32 orxFASTCALL                   orxConsole_GetLogLineLength();


/** Gets log line from the end (trail)
 * @param[in]   _u32TrailLineIndex Index of the line starting from end
 * @return orxTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxConsole_GetTrailLogLine(orxU32 _u32TrailLineIndex);

/** Gets input text
 * @param[out]  _pu32CursorIndex Index (ie. character position) of the cursor (any character past it has not been validated)
 * @return orxTRING / orxSTRING_EMPTY
 */
extern orxDLLAPI const orxSTRING orxFASTCALL          orxConsole_GetInput(orxU32 *_pu32CursorIndex);


#endif /* _orxCONSOLE_H_ */

/** @} */

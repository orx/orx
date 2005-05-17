/**
 * @file orxTextInput.h
 * 
 * Text input - Offers functions to simply retrieve informations from user
 * 
 */
 
 /***************************************************************************
 orxTextInput.h
 Text input management
 
 begin                : 17/05/2005
 author               : (C) Arcallians
 email                : bestel@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _orxTEXTINPUT_H_
#define _orxTEXTINPUT_H_

#include "orxInclude.h"

/** Initialize the string module
 */
extern orxDLLAPI orxSTATUS orxTextInput_Init();

/** Uninitialize the string module
 */
extern orxDLLAPI orxVOID orxFASTCALL orxTextInput_Exit();

/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
extern orxDLLAPI orxSTATUS orxFASTCALL orxTextInput_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxSTRING _zMessage);

/** Read a S32 Value from STDIN
 * @param _ps32OutValue   (OUT) place where the read value will be stored
 * @param _u32Base        (IN)  Base of the read value
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL orxTextInput_ReadS32(orxS32 *_ps32OutValue, orxU32 _u32Base, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop);

/** Read a S32 Value that must be in a valid range from STDIN
 * @param _ps32OutValue   (OUT) place where the read value will be stored
 * @param _u32Base        (IN)  Base of the read value
 * @param _s32Min         (IN)  Min value that the output value must have
 * @param _s32Max         (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL orxTextInput_ReadS32InRange(orxS32 *_ps32OutValue, orxU32 _u32Base, orxS32 _s32Min, orxS32 _s32Max, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop);

/** Read a FLOAT Value from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL orxTextInput_ReadFloat(orxS32 *_pfOutValue, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop);

/** Read a FLOAT Value that must be in a valid range from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _fMin           (IN)  Min value that the output value must have
 * @param _fMax           (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL orxTextInput_ReadFloatInRange(orxS32 *_pfOutValue, orxFLOAT _fMin, orxFLOAT _fMax, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop);

#endif /* _orxTEXTINPUT_H_ */

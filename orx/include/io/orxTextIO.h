/**
 * @file orxTextIO.h
 * 
 * Text input/output - Offers functions to simply retrieve/send informations from/to tty
 * 
 */
 
 /***************************************************************************
 orxTextIO.h
 Text input/output management
 
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

#ifndef _orxTEXTIO_H_
#define _orxTEXTIO_H_


#include "orxInclude.h"
#include "debug/orxDebug.h"
#include <stdio.h>
#include <stdarg.h>


/** Initialize the text input output module
 */
extern orxDLLAPI orxSTATUS              orxTextIO_Init();

/** Uninitialize the string module
 */
extern orxDLLAPI orxVOID                orxTextIO_Exit();


/***************************************************************************
 * INPUT                                                                   *
 ***************************************************************************/

/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxTextIO_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxCONST orxSTRING _zMessage);

/** Read a S32 Value from STDIN
 * @param _ps32OutValue   (OUT) place where the read value will be stored
 * @param _u32Base        (IN)  Base of the read value
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxTextIO_ReadS32(orxS32 *_ps32OutValue, orxU32 _u32Base, orxCONST orxSTRING _zMessage, orxBOOL _bLoop);

/** Read a S32 Value that must be in a valid range from STDIN
 * @param _ps32OutValue   (OUT) place where the read value will be stored
 * @param _u32Base        (IN)  Base of the read value
 * @param _s32Min         (IN)  Min value that the output value must have
 * @param _s32Max         (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxTextIO_ReadS32InRange(orxS32 *_ps32OutValue, orxU32 _u32Base, orxS32 _s32Min, orxS32 _s32Max, orxCONST orxSTRING _zMessage, orxBOOL _bLoop);

/** Read a FLOAT Value from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxTextIO_ReadFloat(orxFLOAT *_pfOutValue, orxCONST orxSTRING _zMessage, orxBOOL _bLoop);

/** Read a FLOAT Value that must be in a valid range from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _fMin           (IN)  Min value that the output value must have
 * @param _fMax           (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
extern orxDLLAPI orxSTATUS orxFASTCALL  orxTextIO_ReadFloatInRange(orxFLOAT *_pfOutValue, orxFLOAT _fMin, orxFLOAT _fMax, orxCONST orxSTRING _zMessage, orxBOOL _bLoop);

/***************************************************************************
 * OUTPUT                                                                  *
 ***************************************************************************/

/** Prints a formated string
 * @param _zDstString     (OUT) Destination string
 * @param _zSrcString     (IN)  Source formated string
 * @retrun The number of written characters
 */
orxSTATIC orxINLINE orxDLLAPI orxS32    orxTextIO_Printf(orxSTRING _zDstString, orxSTRING _zSrcString, ...)
{
  va_list stArgs;
  orxS32 s32Result;

  /* Checks */
  orxASSERT(_zDstString != orxNULL);
  orxASSERT(_zSrcString != orxNULL);

  /* Parse arguments, print and end it */
  va_start(stArgs, _zSrcString);
  s32Result = vsprintf(_zDstString, _zSrcString, stArgs);
  va_end(stArgs);

  return s32Result;
}

/** Print a message on STDIN
 * @param _zMessage       (IN)  Message to print (with optional parameters. Same syntax as printf
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID   orxTextIO_Print(orxSTRING _zMessage, ...)
{
  /* Declare argument lists */
  va_list args;

  /* Parse arguments, print and end it */
  va_start(args, _zMessage);
  vprintf(_zMessage, args);
  va_end(args);
}

/** Print a message on STDIN and returns to line
 * @param _zMessage       (IN)  Message to print (with optional parameters. Same syntax as printf
 */
orxSTATIC orxINLINE orxDLLAPI orxVOID   orxTextIO_PrintLn(orxSTRING _zMessage, ...)
{
  /* Declare argument lists */
  va_list args;
  
  /* Print message */
  va_start(args, _zMessage);
  vprintf(_zMessage, args);
  va_end(args);

  /* return to line */
  printf("\n");
}


#endif /* _orxTEXTIO_H_ */

/**
 * @file orxTextInput.c
 * 
 * Text input - Offers functions to simply retrieve informations from user
 * 
 */
 
 /***************************************************************************
 orxTextInput.c
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

#include "io/orxTextInput.h"
#include "debug/orxDebug.h"
#include "utils/orxString.h"

#include <stdio.h>
#include <string.h>

#define orxTEXTINPUT_KU32_FLAG_NONE            0x00000000  /**< No flags have been set */
#define orxTEXTINPUT_KU32_FLAG_READY           0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxTEXTINPUT_STATIC_t
{
  orxU32 u32Flags;  /**< Module flags */
} orxTEXTINPUT_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxTEXTINPUT_STATIC sstTextInput;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize the test module
 */
orxSTATUS orxTextInput_Init()
{
  /* Module not already registered ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) != orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Initialize values */
  orxMemory_Set(&sstTextInput, 0, sizeof(orxTEXTINPUT_STATIC));

  /* Module ready */
  sstTextInput.u32Flags |= orxTEXTINPUT_KU32_FLAG_READY;
    
  return orxSTATUS_SUCCESS;
}

/** Uninitialize the test module
 */
orxVOID orxTextInput_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) == orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Module becomes not ready */
  sstTextInput.u32Flags &= ~orxTEXTINPUT_KU32_FLAG_READY;
}

/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
orxSTATUS orxFASTCALL orxTextInput_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxSTRING _zMessage)
{
  orxSTRING _zReturnString; /* String read (orxNULL if a problem has occured) */
  orxU32 u32StringLength;   /* Read string length */
  
  /* Module initialized ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) == orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_zOutputBuffer != orxNULL);

  /* Initialize output string */
  orxMemory_Set(_zOutputBuffer, 0, _u32NbChar * sizeof(orxCHAR));

  /* Print message (if not NULL)*/
  orxString_Print(_zMessage);
  
  /* Read message */
  _zReturnString = fgets(_zOutputBuffer, _u32NbChar, stdin);
  
  /* Set \0 on the last character instead of \n (if present and if string length > 0) */
  u32StringLength = strlen(_zReturnString);
  if ((u32StringLength > 0) && (_zReturnString[u32StringLength - 1] == '\n'))
  {
    _zOutputBuffer[u32StringLength - 1] = '\0';
  }
  
  /* Read a valid value ? */
  if (_zReturnString != orxNULL)
  {
    return orxSTATUS_SUCCESS;
  }
  else
  {
    return orxSTATUS_FAILED;
  }
}

/** Read a S32 Value from STDIN
 * @param _ps32OutValue   (OUT) place where the read value will be stored
 * @param _u32Base        (IN)  Base of the read value
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextInput_ReadS32(orxS32 *_ps32OutValue, orxU32 _u32Base, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop)
{
  /* Module initialized ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) == orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT((_u32Base >= 2) && (_u32Base <= 36));
  
  /* TODO */
  
  /* Returns exit status */
  return orxSTATUS_FAILED;
}

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
orxSTATUS orxFASTCALL orxTextInput_ReadS32InRange(orxS32 *_ps32OutValue, orxU32 _u32Base, orxS32 _s32Min, orxS32 _s32Max, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop)
{
  /* Module initialized ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) == orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT((_u32Base >= 2) && (_u32Base <= 36));
  
  /* TODO */
  
  /* Returns exit status */
  return orxSTATUS_FAILED;
}

/** Read a FLOAT Value from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextInput_ReadFloat(orxS32 *_pfOutValue, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop)
{
  /* Module initialized ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) == orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pfOutValue != orxNULL);
  
  /* TODO */
  
  /* Returns exit status */
  return orxSTATUS_FAILED;
}

/** Read a FLOAT Value that must be in a valid range from STDIN
 * @param _pgOutValue     (OUT) place where the read value will be stored
 * @param _fMin           (IN)  Min value that the output value must have
 * @param _fMax           (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _zError         (IN)  Message that will be displayed if there is an invalid entry
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextInput_ReadFloatInRange(orxS32 *_pfOutValue, orxFLOAT _fMin, orxFLOAT _fMax, orxCONST orxSTRING _zMessage, orxCONST orxSTRING _zError, orxBOOL _bLoop)
{
  /* Module initialized ? */
  orxASSERT((sstTextInput.u32Flags & orxTEXTINPUT_KU32_FLAG_READY) == orxTEXTINPUT_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pfOutValue != orxNULL);
  
  /* TODO */
  
  /* Returns exit status */
  return orxSTATUS_FAILED;
}

/**
 * @file orxTextIO.c
 * 
 * Text input/output - Offers functions to simply retrieve/send informations from/to tty
 * 
 */
 
 /***************************************************************************
 orxTextIO.c
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

#include "io/orxTextIO.h"
#include "memory/orxMemory.h"
#include "utils/orxString.h"

#define orxTEXTIO_KU32_FLAG_NONE            0x00000000  /**< No flags have been set */
#define orxTEXTIO_KU32_FLAG_READY           0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxTEXTIO_STATIC_t
{
  orxU32 u32Flags;  /**< Module flags */
} orxTEXTIO_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxTEXTIO_STATIC sstTextIO;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize the test module
 */
orxSTATUS orxTextIO_Init()
{
  /* Module not already registered ? */
  if(!(sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY))
  {
    /* Initialize values */
    orxMemory_Set(&sstTextIO, 0, sizeof(orxTEXTIO_STATIC));

    /* Module ready */
    sstTextIO.u32Flags |= orxTEXTIO_KU32_FLAG_READY;
  }
    
  return orxSTATUS_SUCCESS;
}

/** Uninitialize the test module
 */
orxVOID orxTextIO_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY) == orxTEXTIO_KU32_FLAG_READY);
  
  /* Module becomes not ready */
  sstTextIO.u32Flags &= ~orxTEXTIO_KU32_FLAG_READY;
}

/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
orxSTATUS orxFASTCALL orxTextIO_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxSTRING _zMessage)
{
  orxSTRING _zReturnString; /* String read (orxNULL if a problem has occured) */
  orxU32 u32StringLength;   /* Read string length */
  
  /* Module initialized ? */
  orxASSERT(sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY);
 
  /* Correct parameters ? */
  orxASSERT(_zOutputBuffer != orxNULL);

  /* Initialize output string */
  orxMemory_Set(_zOutputBuffer, 0, _u32NbChar * sizeof(orxCHAR));

  /* Print message (if not NULL)*/
  orxTextIO_Print(_zMessage);
  
  /* Read message */
  _zReturnString = fgets(_zOutputBuffer, _u32NbChar, stdin);
  
  /* Set \0 on the last character instead of \n (if present and if string length > 0) */
  u32StringLength = orxString_Length(_zReturnString);
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
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextIO_ReadS32(orxS32 *_ps32OutValue, orxU32 _u32Base, orxCONST orxSTRING _zMessage, orxBOOL _bLoop)
{
  orxSTATUS eStatus;        /* status of the conversion */
  orxCHAR zUserValue[64];   /* String where user inputs are stored */
 
  /* Module initialized ? */
  orxASSERT((sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY) == orxTEXTIO_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT((_u32Base >= 2) && (_u32Base <= 36));
  
  do
  {
    /* Read value from user */
    eStatus = orxTextIO_ReadString(zUserValue, 63, _zMessage);
    
    /* Valid value ? */
    if (eStatus != orxSTATUS_FAILED)
    {
      /* Try to convert */
      eStatus = orxString_ToS32(_ps32OutValue, zUserValue, _u32Base);
      
      /* Valid conversion ? */
      if (eStatus == orxSTATUS_FAILED)
      {
        /* Print error message */
        orxTextIO_PrintLn("Invalid number");
      }
    }
  } while ((eStatus == orxSTATUS_FAILED) && _bLoop);
  
  /* Returns exit status */
  return eStatus;
}

/** Read a S32 Value that must be in a valid range from STDIN
 * @param _ps32OutValue   (OUT) place where the read value will be stored
 * @param _u32Base        (IN)  Base of the read value
 * @param _s32Min         (IN)  Min value that the output value must have
 * @param _s32Max         (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextIO_ReadS32InRange(orxS32 *_ps32OutValue, orxU32 _u32Base, orxS32 _s32Min, orxS32 _s32Max, orxCONST orxSTRING _zMessage, orxBOOL _bLoop)
{
  orxSTATUS eStatus;        /* status of the conversion */

  /* Module initialized ? */
  orxASSERT((sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY) == orxTEXTIO_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_ps32OutValue != orxNULL);
  orxASSERT((_u32Base >= 2) && (_u32Base <= 36));
  orxASSERT(_s32Min < _s32Max);
    
  do
  {
    /* Get the value */
    eStatus = orxTextIO_ReadS32(_ps32OutValue, _u32Base, _zMessage, orxFALSE);
    
    /* Valid conversion ? */
    if (eStatus == orxSTATUS_SUCCESS)
    {
      /* Is it in range ? */
      if ((*_ps32OutValue < _s32Min) || (*_ps32OutValue > _s32Max))
      {
        /* Print error message */
        orxTextIO_PrintLn("Value not in range");
        eStatus = orxSTATUS_FAILED;
      }
    }
  } 
  while ((eStatus == orxSTATUS_FAILED) && _bLoop);
  
  /* Returns exit status */
  return eStatus;
}

/** Read a FLOAT Value from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextIO_ReadFloat(orxFLOAT *_pfOutValue, orxCONST orxSTRING _zMessage, orxBOOL _bLoop)
{
  orxSTATUS eStatus;        /* status of the conversion */
  orxCHAR zUserValue[64];   /* String where user inputs are stored */

  /* Module initialized ? */
  orxASSERT((sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY) == orxTEXTIO_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pfOutValue != orxNULL);
  
  do
  {
    /* Read value from user */
    eStatus = orxTextIO_ReadString(zUserValue, 63, _zMessage);
    
    /* Valid value ? */
    if (eStatus != orxSTATUS_FAILED)
    {
      /* Try to convert */
      eStatus = orxString_ToFloat(_pfOutValue, zUserValue);
      
      /* Valid conversion ? */
      if (eStatus == orxSTATUS_FAILED)
      {
        /* Print error message */
        orxTextIO_PrintLn("Invalid number");
      }
    }
  } while ((eStatus == orxSTATUS_FAILED) && _bLoop);
  
  /* Returns exit status */
  return eStatus;
}

/** Read a FLOAT Value that must be in a valid range from STDIN
 * @param _pfOutValue     (OUT) place where the read value will be stored
 * @param _fMin           (IN)  Min value that the output value must have
 * @param _fMax           (IN)  Max value that the output value must have
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @param _bLoop          (IN)  Must be frTRUE if the function has to loop until the user send a valid entry
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status (if no loop asked)
 */
orxSTATUS orxFASTCALL orxTextIO_ReadFloatInRange(orxFLOAT *_pfOutValue, orxFLOAT _fMin, orxFLOAT _fMax, orxCONST orxSTRING _zMessage, orxBOOL _bLoop)
{
  orxSTATUS eStatus;        /* status of the conversion */

  /* Module initialized ? */
  orxASSERT((sstTextIO.u32Flags & orxTEXTIO_KU32_FLAG_READY) == orxTEXTIO_KU32_FLAG_READY);
  
  /* Correct parameters ? */
  orxASSERT(_pfOutValue != orxNULL);
  orxASSERT(_fMin < _fMax);
  
  do
  {
    /* Get the value */
    eStatus = orxTextIO_ReadFloat(_pfOutValue, _zMessage, orxFALSE);
    
    /* Valid conversion ? */
    if (eStatus == orxSTATUS_SUCCESS)
    {
      /* Is it in range ? */
      if ((*_pfOutValue < _fMin) || (*_pfOutValue > _fMax))
      {
        /* Print error message */
        orxTextIO_PrintLn("Value not in range");
        eStatus = orxSTATUS_FAILED;
      }
    }
  } 
  while ((eStatus == orxSTATUS_FAILED) && _bLoop);
  
  /* Returns exit status */
  return eStatus;
}

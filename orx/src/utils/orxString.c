/**
 * @file orxTest.c
 * 
 * String Module - Offers functions to manage Strings and CRC
 * 
 */
 
 /***************************************************************************
 orxString.c
 String Module
 
 begin                : 21/04/2005
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

#include "utils/orxString.h"

#include <stdio.h>
#include <string.h>

#include "memory/orxMemory.h"


#define orxSTRING_KU32_FLAG_NONE  0x00000000  /**< No flags have been set */
#define orxSTRING_KU32_FLAG_READY 0x00000001  /**< The module has been initialized */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/
typedef struct __orxSTRING_STATIC_t
{
  orxU32 u32Flags;  /**< Module flags */
} orxSTRING_STATIC;

/***************************************************************************
 * Module global variable                                                  *
 ***************************************************************************/
static orxSTRING_STATIC sstString;

/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Initialize the test module
 */
orxSTATUS orxString_Init()
{
  /* Module not already registered ? */
  orxASSERT((sstString.u32Flags & orxSTRING_KU32_FLAG_READY) != orxSTRING_KU32_FLAG_READY);
  
  /* Initialize values */
  orxMemory_Set(&sstString, 0, sizeof(orxSTRING_STATIC));

  /* Module ready */
  sstString.u32Flags |= orxSTRING_KU32_FLAG_READY;
    
  return orxSTATUS_SUCCESS;
}

/** Uninitialize the test module
 */
orxVOID orxString_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstString.u32Flags & orxSTRING_KU32_FLAG_READY) == orxSTRING_KU32_FLAG_READY);
  
  /* Module becomes not ready */
  sstString.u32Flags &= ~orxSTRING_KU32_FLAG_READY;
}

/** Read a String from STDIN and store it in the given buffer
 * @param _zOutputBuffer  (OUT) Buffer where the read value will be stored
 * @param _u32NbChar      (IN)  Number of character maximum to read (to avoid overflow)
 * @param _zMessage       (IN)  Message that will be displayed before read
 * @retrun orxSTATUS_SUCCESS if no error has occured, else returns the error status
 */
orxSTATUS orxString_ReadString(orxSTRING _zOutputBuffer, orxU32 _u32NbChar, orxSTRING _zMessage)
{
  orxSTRING _zReturnString; /* String read (orxNULL if a problem has occured) */
  orxU32 u32StringLength;   /* Read string length */
  
  /* Module initialized ? */
  orxASSERT((sstString.u32Flags & orxSTRING_KU32_FLAG_READY) == orxSTRING_KU32_FLAG_READY);
  
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

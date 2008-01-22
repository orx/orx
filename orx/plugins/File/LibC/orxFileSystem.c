/**
 * @file orxFileSystem.c
 */

/***************************************************************************
 orxFileSystem.c
 Lib C implementation of the File System module
 begin                : 21/01/2008
 author               : (C) Arcallians
 email                : keleborn@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "orxInclude.h"
#include "plugin/orxPluginUser.h"
#include "debug/orxDebug.h"
#include "io/orxFileSystem.h"

#include <stdio.h>


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Module flags
 */
#define orxFILESYSTEM_KU32_STATIC_FLAG_NONE   0x00000000  /**< No flags have been set */
#define orxFILESYSTEM_KU32_STATIC_FLAG_READY  0x00000001  /**< The module has been initialized */


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxFILESYSTEM_STATIC_t
{
  orxU32            u32Flags;
  
} orxFILESYSTEM_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
orxSTATIC orxFILESYSTEM_STATIC sstFileSystem;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

orxSTATUS orxFileSystem_LibC_Init()
{
  orxSTATUS eResult = orxSTATUS_SUCCESS;

  /* Was not already initialized? */
  if(!(sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY))
  {
    /* Cleans static controller */
    orxMemory_Set(&sstFileSystem, 0, sizeof(orxFILESYSTEM_STATIC));

    /* Updates status */
    sstFileSystem.u32Flags |= orxFILESYSTEM_KU32_STATIC_FLAG_READY;
  }

  /* Done! */
  return eResult;  
}

orxVOID orxFileSystem_LibC_Exit()
{
  /* Module initialized ? */
  orxASSERT((sstFileSystem.u32Flags & orxFILESYSTEM_KU32_STATIC_FLAG_READY) == orxFILESYSTEM_KU32_STATIC_FLAG_READY);
  
  /* Module not ready now */
  sstFileSystem.u32Flags = orxFILESYSTEM_KU32_STATIC_FLAG_NONE;
}


/********************
 *  Plugin Related  *
 ********************/

orxPLUGIN_USER_CORE_FUNCTION_START(FILESYSTEM);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Init, FILESYSTEM, INIT);
orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Exit, FILESYSTEM, EXIT);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_FindFirst, orxBOOL, orxSTRING, orxFileSystem_LibC_INFO *);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_FindNext, orxBOOL, orxFileSystem_LibC_INFO *);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_FindClose, orxVOID, orxFileSystem_LibC_INFO *);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Info, orxSTATUS, orxSTRING, orxFileSystem_LibC_INFO *);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Copy, orxSTATUS, orxSTRING, orxSTRING);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Rename, orxSTATUS, orxSTRING, orxSTRING);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_Delete, orxSTATUS, orxSTRING);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_CreateDir, orxSTATUS, orxSTRING);
//orxPLUGIN_USER_CORE_FUNCTION_ADD(orxFileSystem_LibC_DeleteDir, orxSTATUS, orxSTRING);
orxPLUGIN_USER_CORE_FUNCTION_END();

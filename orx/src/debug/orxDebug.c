/***************************************************************************
 orxDebug.c
 Debug module
 
 begin                : 10/12/2003
 author               : (C) Arcallians
 email                : iarwain@arcallians.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

#include "debug/orxDebug.h"


#ifdef __orxDEBUG__

/*
 * Platform independant defines
 */

#define orxDEBUG_KU32_CONTROL_FLAG_NONE         0x00000000
#define orxDEBUG_KU32_CONTROL_FLAG_READY        0x00000001


/*
 * Static structure
 */
typedef struct __orxDEBUG_STATIC_t
{
  /* Debug file : 4 */
  orxSTRING zDebugFile;
  
  /* Log file : 8 */
  orxSTRING zLogFile;

  /* Debug flags : 12 */
  orxU32 u32DebugFlags;
  
  /* Backup debug flags : 16 */
  orxU32 u32BackupDebugFlags;

  /* Current buffer : 528 */
  orxCHAR zBuffer[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE];

  /* Current log : 1040 */
  orxCHAR zLog[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE];

  /* Control flags : 1044 */
  orxU32 u32Flags;

} orxDEBUG_STATIC;


/*
 * Static data
 */
orxSTATIC orxDEBUG_STATIC sstDebug;


/***************************************************************************
 ***************************************************************************
 ******                       LOCAL FUNCTIONS                         ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 orxDebug_GetLevelString
 Deletes all cameras.

 returns: orxCONST orxSTRING level string
 ***************************************************************************/
orxINLINE orxCONST orxSTRING orxDebug_GetLevelString(orxDEBUG_LEVEL _eLevel)
{
  /* Depending on level */
  switch(_eLevel)
  {
    case orxDEBUG_LEVEL_MOUSE:              return "MOUSE";
    case orxDEBUG_LEVEL_KEYBOARD:           return "KEYBOARD";
    case orxDEBUG_LEVEL_JOYSTICK:           return "JOYSTICK";
    case orxDEBUG_LEVEL_INTERACTION:        return "INTERACTION";
    case orxDEBUG_LEVEL_GRAPH:              return "GRAPH";
    case orxDEBUG_LEVEL_SOUND:              return "SOUND";
    case orxDEBUG_LEVEL_TIMER:              return "TIMER";
    case orxDEBUG_LEVEL_MEMORY:             return "MEMORY";
    case orxDEBUG_LEVEL_SCREENSHOT:         return "SCREENSHOT";
    case orxDEBUG_LEVEL_FILE:               return "FILE";
    case orxDEBUG_LEVEL_PATHFINDER:         return "PATHFINDER";
    case orxDEBUG_LEVEL_PLUGIN:             return "PLUGIN";
    case orxDEBUG_LEVEL_LOG:                return "LOG";
    case orxDEBUG_LEVEL_ALL:                return "ALL";
    case orxDEBUG_LEVEL_ASSERT:             return "ASSERT";
    case orxDEBUG_LEVEL_CRITICAL_ASSERT:    return "CRITICAL ASSERT";
    
    default:
      return "INVALID DEBUG!";
  }

  return "";
}


/***************************************************************************
 ***************************************************************************
 ******                       PUBLIC FUNCTIONS                        ******
 ***************************************************************************
 ***************************************************************************/

/***************************************************************************
 _orxDebug_Init
 Inits module.

 returns: orxSTATUS_FAILED/orxSTATUS_SUCCESS
 ***************************************************************************/
orxSTATUS _orxDebug_Init()
{
  orxU32 i;
  orxU8 *pu8;

  /* Already Initialized? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_CONTROL_FLAG_READY)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Are debug level correct? */
  if(orxDEBUG_LEVEL_NUMBER > orxDEBUG_LEVEL_MAX_NUMBER)
  {
    /* !!! MSG !!! */

    return orxSTATUS_FAILED;
  }

  /* Cleans static controller */
  for(i = 0, pu8 = (orxU8 *)&sstDebug; i < sizeof(orxDEBUG_STATIC); i++)
  {
    *pu8++ = 0;
  }

  /* Inits default files */
  sstDebug.zDebugFile     = orxDEBUG_KZ_DEFAULT_DEBUG_FILE;
  sstDebug.zLogFile       = orxDEBUG_KZ_DEFAULT_LOG_FILE;

  /* Inits default debug flags */
  sstDebug.u32DebugFlags  = orxDEBUG_KU32_FLAG_DEFAULT;

  /* Done! */
  return orxSTATUS_SUCCESS;
}

/***************************************************************************
 orxDebug_Exit
 Exits from the debug system.

 returns: orxVOID
 ***************************************************************************/
orxVOID _orxDebug_Exit()
{
  /* Initialized? */
  if(sstDebug.u32Flags & orxDEBUG_KU32_CONTROL_FLAG_READY)
  {
    /* Updates flags */
    sstDebug.u32Flags &= ~orxDEBUG_KU32_CONTROL_FLAG_READY;
  }
  else
  {
    /* !!! MSG !!! */
  }

  return;
}

/***************************************************************************
 _orxDebug_Break
 Breaks.

 returns: orxVOID
 ***************************************************************************/
orxVOID _orxDebug_Break()
{
  /* Windows / Linux */
#if defined(__orxWINDOWS__) || defined(__orxLINUX__)

  asm("int $3");

#endif /* __orxWINDOWS__ || __orxLINUX__ */

  return;
}

/***************************************************************************
 _orxDebug_BackupFlags
 Backups flags.

 returns: orxVOID
 ***************************************************************************/
orxVOID _orxDebug_BackupFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_CONTROL_FLAG_READY);

  /* Backups flags */
  sstDebug.u32BackupDebugFlags = sstDebug.u32DebugFlags;

  return;
}

/***************************************************************************
 _orxDebug_RestoreFlags
 Restores flags.

 returns: orxVOID
 ***************************************************************************/
orxVOID _orxDebug_RestoreFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_CONTROL_FLAG_READY);

  /* Restores flags */
  sstDebug.u32DebugFlags = sstDebug.u32BackupDebugFlags;

  return;
}

/***************************************************************************
 _orxDebug_SetFlag
 Sets flags.

 returns: orxVOID
 ***************************************************************************/
orxVOID _orxDebug_SetFlag(orxU32 _u32Add, orxU32 _u32Remove)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_CONTROL_FLAG_READY);

  /* Updates flags */
  sstDebug.u32DebugFlags &= ~_u32Remove;
  sstDebug.u32DebugFlags |= _u32Add;

  return;
}

/***************************************************************************
 _orxDebug_Log
 Logs a debug string.

 returns: orxVOID
 ***************************************************************************/
orxVOID _orxDebug_Log(orxDEBUG_LEVEL _eLevel, orxCONST orxSTRING _zFunction, orxCONST orxSTRING _zFile, orxU32 _u32Line, orxCONST orxSTRING _zFormat, ...)
{
  va_list stArgs;
  FILE *pstFile = orxNULL;

  /* TODO : Checks log mask if display is enable for this level */

  /* Empties current buffer */
  sstDebug.zBuffer[0] = '\0';

  /* Time Stamp? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_FLAG_TIMESTAMP)
  {
    time_t stTime;

    /* Inits Log Time */
    time(&stTime);

    strftime(sstDebug.zBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE, orxDEBUG_KZ_DATE_FORMAT, localtime(&stTime));
  }

  /* Log Type? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_FLAG_TYPE)
  {
    sprintf(sstDebug.zBuffer, "%s <%s>", sstDebug.zBuffer, orxDebug_GetLevelString(_eLevel));
  }

  /* Log FUNCTION, FILE & LINE? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_FLAG_TAGGED)
  {
    sprintf(sstDebug.zBuffer, "%s (%s() - %s:%ld)", sstDebug.zBuffer, _zFunction, _zFile, _u32Line);
  }

  /* Debug Log */
  va_start(stArgs, _zFormat);
  vsprintf(sstDebug.zLog, _zFormat, stArgs);
  va_end(stArgs);

  sprintf(sstDebug.zBuffer, "%s %s", sstDebug.zBuffer, sstDebug.zLog);

  /* Use file? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_FLAG_FILE)
  {
    if(_eLevel == orxDEBUG_LEVEL_LOG)
    {
      pstFile = fopen(sstDebug.zLogFile, "awR");
    }
    else
    {
      pstFile = fopen(sstDebug.zDebugFile, "awR");
    }

    /* Valid? */
    if(pstFile != orxNULL)
    {
      fprintf(pstFile, sstDebug.zBuffer);
      fflush(pstFile);
      fclose(pstFile);
    }
   }

  /* Console Display? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_FLAG_CONSOLE)  
  {
    if(_eLevel == orxDEBUG_LEVEL_LOG)
    {
      pstFile = stdout;
    }
    else
    {
      pstFile = stderr;
    }

    fprintf(pstFile, sstDebug.zBuffer);
    fflush(pstFile);
  }

  /* Done */
  return;
}


#endif /* __orxDEBUG__ */

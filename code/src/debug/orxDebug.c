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
 * @file orxDebug.c
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add mask test for level displaying
 * - Add graphical debug from outside, using a shared debug info array
 * - Add Assert/Assert after code
 * - Enhance logging, use of different log levels
 */


#include "debug/orxDebug.h"

#include <stdlib.h>


#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

#endif /* __orxMSVC__ */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>


/** Platform independant defines
 */

#define orxDEBUG_KU32_STATIC_FLAG_NONE          0x00000000

#define orxDEBUG_KU32_STATIC_FLAG_READY         0x10000000

#define orxDEBUG_KU32_STATIC_MASK_ALL           0xFFFFFFFF


/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
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

  /* Control flags : 1044 */
  orxU32 u32Flags;

} orxDEBUG_STATIC;


/***************************************************************************
 * Static variables                                                        *
 ***************************************************************************/

/** Static data
 */
static orxDEBUG_STATIC sstDebug;


/***************************************************************************
 * Private functions                                                       *
 ***************************************************************************/

/** Gets debug level name
 * @param[in]   _eLevel                       Concerned debug level
  *@return      Corresponding literal string
 */
static orxINLINE orxSTRING orxDebug_GetLevelString(orxDEBUG_LEVEL _eLevel)
{
  orxSTRING zResult;

#define orxDEBUG_DECLARE_LEVEL_ENTRY(ID)    case orxDEBUG_LEVEL_##ID: zResult = #ID; break

  /* Depending on level */
  switch(_eLevel)
  {
    orxDEBUG_DECLARE_LEVEL_ENTRY(ANIM);
    orxDEBUG_DECLARE_LEVEL_ENTRY(CLOCK);
    orxDEBUG_DECLARE_LEVEL_ENTRY(DISPLAY);
    orxDEBUG_DECLARE_LEVEL_ENTRY(FILE);
    orxDEBUG_DECLARE_LEVEL_ENTRY(INPUT);
    orxDEBUG_DECLARE_LEVEL_ENTRY(JOYSTICK);
    orxDEBUG_DECLARE_LEVEL_ENTRY(KEYBOARD);
    orxDEBUG_DECLARE_LEVEL_ENTRY(MEMORY);
    orxDEBUG_DECLARE_LEVEL_ENTRY(MOUSE);
    orxDEBUG_DECLARE_LEVEL_ENTRY(OBJECT);
    orxDEBUG_DECLARE_LEVEL_ENTRY(PARAM);
    orxDEBUG_DECLARE_LEVEL_ENTRY(PATHFINDER);
    orxDEBUG_DECLARE_LEVEL_ENTRY(PHYSICS);
    orxDEBUG_DECLARE_LEVEL_ENTRY(PLUGIN);
    orxDEBUG_DECLARE_LEVEL_ENTRY(RENDER);
    orxDEBUG_DECLARE_LEVEL_ENTRY(SCREENSHOT);
    orxDEBUG_DECLARE_LEVEL_ENTRY(SOUND);
    orxDEBUG_DECLARE_LEVEL_ENTRY(SYSTEM);
    orxDEBUG_DECLARE_LEVEL_ENTRY(TIMER);
    orxDEBUG_DECLARE_LEVEL_ENTRY(USER);
    
    orxDEBUG_DECLARE_LEVEL_ENTRY(ALL);

    orxDEBUG_DECLARE_LEVEL_ENTRY(LOG);

    orxDEBUG_DECLARE_LEVEL_ENTRY(ASSERT);
    orxDEBUG_DECLARE_LEVEL_ENTRY(CRITICAL_ASSERT);
    
    default:                                zResult = "INVALID DEBUG!"; break;
  }

  /* Done! */
  return zResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Inits the debug module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS _orxDebug_Init()
{
  orxU32 i;
  orxU8 *pu8;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Init dependencies */
  
  /* Correct parameters ? */
  if(orxDEBUG_LEVEL_NUMBER > orxDEBUG_LEVEL_MAX_NUMBER)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Internal error. DEBUG_LEVEL_NUMBER(%ld) > DEBUG_LEVEL_MAX_NUMBER(%ld).", orxDEBUG_LEVEL_NUMBER, orxDEBUG_LEVEL_MAX_NUMBER);

    eResult = orxSTATUS_FAILURE;
  }
  else if(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY)
  {
    /* Already Initialized? */
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize debug module when it was already initialized.");

    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    /* Cleans static controller */
    for(i = 0, pu8 = (orxU8 *)&sstDebug; i < sizeof(orxDEBUG_STATIC); i++)
    {
      *pu8++ = 0;
    }
  
    /* Inits default files */
    sstDebug.zDebugFile     = orxDEBUG_KZ_DEFAULT_DEBUG_FILE;
    sstDebug.zLogFile       = orxDEBUG_KZ_DEFAULT_LOG_FILE;
  
    /* Inits default debug flags */
    sstDebug.u32DebugFlags  = orxDEBUG_KU32_STATIC_MASK_DEFAULT;
    
    /* Set module as initialized */
    sstDebug.u32Flags       = orxDEBUG_KU32_STATIC_FLAG_READY;
    
    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the debug module */
void _orxDebug_Exit()
{
  /* Initialized? */
  if(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY)
  {
    /* Updates flags */
    sstDebug.u32Flags &= ~orxDEBUG_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to exit debug module when it wasn't initialized.");
  }

  return;
}

/** Software break function */
void _orxDebug_Break()
{
  /* Windows / Linux */
#if defined(__orxWINDOWS__) || defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxGP2X__)

  /* Compiler specific */

  #ifdef __orxGCC__
  
    #ifdef __orxGP2X__

    //! TODO: Add GP2X software break code

    #else /* __orxGP2X__ */

      #ifdef __orxPPC__

        asm("li r0, 20\nsc\nnop\nli r0, 37\nli r4, 2\nsc\nnop\n" : : : "memory", "r0", "r3", "r4");
  
      #else /* __orxPPC__ */

        asm("int $3");

      #endif /* __orxPPC__ */

    #endif /* __orxGP2X__ */

  #endif /* __orxGCC__ */

  #ifdef __orxMSVC__
    __debugbreak();
  #endif /* __orxMSVC__ */

#endif /* __orxWINDOWS__ || __orxLINUX__ || __orxMAC__ || __orxGP2X__ */

  return;
}

/** Backups current debug flags */
void _orxDebug_BackupFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Backups flags */
  sstDebug.u32BackupDebugFlags = sstDebug.u32DebugFlags;

  return;
}

/** Restores last backuped flags */
void _orxDebug_RestoreFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Restores flags */
  sstDebug.u32DebugFlags = sstDebug.u32BackupDebugFlags;

  return;
}

/** Sets current debug flags */
void orxFASTCALL    _orxDebug_SetFlags(orxU32 _u32Add, orxU32 _u32Remove)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Updates flags */
  sstDebug.u32DebugFlags &= ~_u32Remove;
  sstDebug.u32DebugFlags |= _u32Add;

  return;
}

/** Logs given debug text
 * @param[in]   _eLevel                       Debug level associated with this output
 * @param[in]   _zFunction                    Calling function name
 * @param[in]   _zFile                        Calling file name
 * @param[in]   _u32Line                      Calling file line
 * @param[in]   _zFormat                      Printf formatted text
 */
void orxFASTCALL    _orxDebug_Log(orxDEBUG_LEVEL _eLevel, const orxSTRING _zFunction, const orxSTRING _zFile, orxU32 _u32Line, const orxSTRING _zFormat, ...)
{
  va_list stArgs;
  FILE   *pstFile = orxNULL;
  orxCHAR zBuffer[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE], zLog[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE];


  /* TODO : Checks log mask if display is enable for this level */

  /* Empties current buffer */
  zBuffer[0] = orxCHAR_NULL;

  /* Time Stamp? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP)
  {
    time_t u32Time;

    /* Inits Log Time */
    time(&u32Time);

    strftime(zBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE, orxDEBUG_KZ_DATE_FORMAT, localtime(&u32Time));
  }

  /* Log Type? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TYPE)
  {
    sprintf(zBuffer, "%s <%s>", zBuffer, orxDebug_GetLevelString(_eLevel));
  }

  /* Log FUNCTION, FILE & LINE? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TAGGED)
  {
    const orxCHAR *pc;

    /* Trims relative path */
    for(pc = _zFile;
        (*pc != orxCHAR_EOL) && ((*pc == '.') || (*pc == orxCHAR_DIRECTORY_SEPARATOR_LINUX) || (*pc == orxCHAR_DIRECTORY_SEPARATOR_WINDOWS));
        pc++);

    /* Writes info */
    sprintf(zBuffer, "%s (%s() - %s:%ld)", zBuffer, _zFunction, pc, _u32Line);
  }

  /* Debug Log */
  va_start(stArgs, _zFormat);
  vsprintf(zLog, _zFormat, stArgs);
  va_end(stArgs);

  sprintf(zBuffer, "%s %s\n", zBuffer, zLog);

  /* Use file? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_FILE)
  {
    if(_eLevel == orxDEBUG_LEVEL_LOG)
    {
      pstFile = fopen(sstDebug.zLogFile, "a+");
    }
    else
    {
      pstFile = fopen(sstDebug.zDebugFile, "a+");
    }

    /* Valid? */
    if(pstFile != orxNULL)
    {
      fprintf(pstFile, zBuffer);
      fflush(pstFile);
      fclose(pstFile);
    }
  }

  /* Console Display? */
  if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_CONSOLE)  
  {
    if(_eLevel == orxDEBUG_LEVEL_LOG)
    {
      pstFile = stdout;
    }
    else
    {
      pstFile = stderr;
    }

    fprintf(pstFile, zBuffer);
    fflush(pstFile);
  }

  /* Done */
  return;
}

/** Sets debug file name
 * @param[in]   _zFileName                    Debug file name
 */
void orxFASTCALL _orxDebug_SetDebugFile(const orxSTRING _zFileName)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Had a previous external name? */
  if((sstDebug.zDebugFile != orxNULL) && (sstDebug.zDebugFile != orxDEBUG_KZ_DEFAULT_DEBUG_FILE))
  {
    /* Deletes it */
    free(sstDebug.zDebugFile);
  }

  /* Valid? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY))
  {
    orxU32 u32Size;

    /* Gets string size in bytes */
    u32Size = ((orxU32)strlen(_zFileName) + 1) * sizeof(orxCHAR);

    /* Allocates it */
    sstDebug.zDebugFile = (orxSTRING)malloc(u32Size);

    /* Stores new name */
    memcpy(sstDebug.zDebugFile, _zFileName, u32Size);
  }
  else
  {
    /* Uses default file */
    sstDebug.zDebugFile = orxDEBUG_KZ_DEFAULT_DEBUG_FILE;
  }
}

/** Sets log file name
 * @param[in]   _zFileName                    Log file name
 */
void orxFASTCALL _orxDebug_SetLogFile(const orxSTRING _zFileName)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Had a previous external name? */
  if((sstDebug.zLogFile != orxNULL) && (sstDebug.zLogFile != orxDEBUG_KZ_DEFAULT_LOG_FILE))
  {
    /* Deletes it */
    free(sstDebug.zLogFile);
  }

  /* Valid? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY))
  {
    orxU32 u32Size;

    /* Gets string size in bytes */
    u32Size = ((orxU32)strlen(_zFileName) + 1) * sizeof(orxCHAR);

    /* Allocates it */
    sstDebug.zLogFile = (orxSTRING)malloc(u32Size);

    /* Stores new name */
    memcpy(sstDebug.zLogFile, _zFileName, u32Size);
  }
  else
  {
    /* Uses default file */
    sstDebug.zLogFile = orxDEBUG_KZ_DEFAULT_LOG_FILE;
  }
}

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */

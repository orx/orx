/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2012 Orx-Project
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
 * @file orxDebug.c
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 */


#include "debug/orxDebug.h"

#include <stdlib.h>

#if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

  #include <jni.h>
  #include <android/log.h>

#endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */


#ifdef __orxMSVC__

  #pragma warning(disable : 4996)

#endif /* __orxMSVC__ */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>


/** Platform independent defines
 */

#define orxDEBUG_KU32_STATIC_FLAG_NONE          0x00000000

#define orxDEBUG_KU32_STATIC_FLAG_READY         0x10000000

#define orxDEBUG_KU32_STATIC_MASK_ALL           0xFFFFFFFF

#ifdef __orxDEBUG__

#define orxDEBUG_KU32_STATIC_LEVEL_MASK_DEFAULT 0xFFFFFFFF

#else /* __orxDEBUG__ */

#define orxDEBUG_KU32_STATIC_LEVEL_MASK_DEFAULT (1 << orxDEBUG_LEVEL_LOG)

#endif /* __orxDEBUG__ */

/***************************************************************************
 * Structure declaration                                                   *
 ***************************************************************************/

/** Static structure
 */
typedef struct __orxDEBUG_STATIC_t
{
  /* Debug file name */
  orxSTRING zDebugFile;

  /* Log file name */
  orxSTRING zLogFile;

  /* Debug file */
  FILE *pstDebugFile;

  /* Log file */
  FILE *pstLogFile;

  /* Debug flags */
  orxU32 u32DebugFlags;

  /* Backup debug flags */
  orxU32 u32BackupDebugFlags;

  /* Level flags */
  orxU32 u32LevelFlags;

  /* Control flags */
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
static orxINLINE const orxSTRING orxDebug_GetLevelString(orxDEBUG_LEVEL _eLevel)
{
  const orxSTRING zResult;

#define orxDEBUG_DECLARE_LEVEL_ENTRY(ID)    case orxDEBUG_LEVEL_##ID: zResult = #ID; break

  /* Depending on level */
  switch(_eLevel)
  {
    orxDEBUG_DECLARE_LEVEL_ENTRY(ANIM);
    orxDEBUG_DECLARE_LEVEL_ENTRY(CONFIG);
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
    orxDEBUG_DECLARE_LEVEL_ENTRY(PHYSICS);
    orxDEBUG_DECLARE_LEVEL_ENTRY(PLUGIN);
    orxDEBUG_DECLARE_LEVEL_ENTRY(PROFILER);
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

    default: zResult = "INVALID DEBUG!"; break;
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
orxSTATUS orxFASTCALL _orxDebug_Init()
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

    /* Inits default debug & level flags */
    sstDebug.u32DebugFlags  = orxDEBUG_KU32_STATIC_MASK_DEFAULT;
    sstDebug.u32LevelFlags  = orxDEBUG_KU32_STATIC_LEVEL_MASK_DEFAULT;

    /* Set module as initialized */
    sstDebug.u32Flags       = orxDEBUG_KU32_STATIC_FLAG_READY;

    /* Inits default files */
    _orxDebug_SetDebugFile(orxDEBUG_KZ_DEFAULT_DEBUG_FILE);
    _orxDebug_SetLogFile(orxDEBUG_KZ_DEFAULT_LOG_FILE);

    /* Success */
    eResult = orxSTATUS_SUCCESS;
  }

  /* Done! */
  return eResult;
}

/** Exits from the debug module */
void orxFASTCALL _orxDebug_Exit()
{
  /* Initialized? */
  if(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY)
  {
#if !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__)

    /* Closes files */
    if(sstDebug.pstLogFile != orxNULL)
    {
      fclose(sstDebug.pstLogFile);
      sstDebug.pstLogFile = orxNULL;
    }
    if(sstDebug.pstDebugFile != orxNULL)
    {
       fclose(sstDebug.pstDebugFile);
       sstDebug.pstDebugFile = orxNULL;
    }

#endif /* !__orxANDROID__ && !__orxANDROID_NATIVE__ */

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
void orxFASTCALL _orxDebug_Break()
{
  /* Windows / Linux / Mac / iOS / Android */
#if defined(__orxWINDOWS__) || defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

  /* Compiler specific */

  #ifdef __orxGCC__

    #if defined(__orxIOS__)

      __builtin_trap();

    #elif defined(__orxANDROID__)

      __builtin_trap();

    #elif defined(__orxANDROID_NATIVE__)

      __builtin_trap();

    #else

      #ifdef __orxPPC__

        asm("li r0, 20\nsc\nnop\nli r0, 37\nli r4, 2\nsc\nnop\n" : : : "memory", "r0", "r3", "r4");

      #else /* __orxPPC__ */

        asm("int $3");

      #endif /* __orxPPC__ */

    #endif

  #endif /* __orxGCC__ */

  #ifdef __orxMSVC__

    __debugbreak();

  #endif /* __orxMSVC__ */

#endif /* __orxWINDOWS__ || __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

  return;
}

/** Backups current debug flags */
void orxFASTCALL _orxDebug_BackupFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Backups flags */
  sstDebug.u32BackupDebugFlags = sstDebug.u32DebugFlags;

  return;
}

/** Restores last backuped flags */
void orxFASTCALL _orxDebug_RestoreFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Restores flags */
  sstDebug.u32DebugFlags = sstDebug.u32BackupDebugFlags;

  return;
}

/** Sets current debug flags */
void orxFASTCALL _orxDebug_SetFlags(orxU32 _u32Add, orxU32 _u32Remove)
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
void orxCDECL _orxDebug_Log(orxDEBUG_LEVEL _eLevel, const orxSTRING _zFunction, const orxSTRING _zFile, orxU32 _u32Line, const orxSTRING _zFormat, ...)
{
  /* Is level enabled? */
  if(orxFLAG_TEST(sstDebug.u32LevelFlags, (1 << _eLevel)))
  {
    orxCHAR zBuffer[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE], zLog[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE], *pcBuffer = zBuffer;
    FILE   *pstFile;
    va_list stArgs;

    /* Empties current buffer */
    pcBuffer[0] = orxCHAR_NULL;

    /* Time Stamp? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP)
    {
      time_t u32Time;

      /* Inits Log Time */
      time(&u32Time);

      pcBuffer += strftime(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE, orxDEBUG_KZ_DATE_FORMAT, localtime(&u32Time));
    }

    /* Log Type? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TYPE)
    {

#ifdef __orxMSVC__

      pcBuffer += _snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), " <%s>", orxDebug_GetLevelString(_eLevel));

#else /* __orxMSVC__ */

      pcBuffer += snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), " <%s>", orxDebug_GetLevelString(_eLevel));

#endif /* __orxMSVC__ */

    }

    /* Log FUNCTION, FILE & LINE? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TAGGED)
    {
      const orxCHAR *pc;

      /* Trims relative path */
      for(pc = _zFile;
          (*pc != orxCHAR_EOL) && ((*pc == '.') || (*pc == orxCHAR_DIRECTORY_SEPARATOR_LINUX) || (*pc == orxCHAR_DIRECTORY_SEPARATOR_WINDOWS));
          pc++);

#ifdef __orxMSVC__

      /* Writes info */
      pcBuffer += _snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), " (%s() - %s:%lu)", _zFunction, pc, _u32Line);

#else /* __orxMSVC__ */

      /* Writes info */
      pcBuffer += snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), " (%s() - %s:%lu)", _zFunction, pc, (long unsigned int)_u32Line);

#endif /* __orxMSVC__ */
    }

    /* Debug Log */
    va_start(stArgs, _zFormat);
    vsnprintf(zLog, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), _zFormat, stArgs);
    zLog[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer) - 1] = '\0';
    va_end(stArgs);

#ifdef __orxMSVC__

    pcBuffer += _snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), " %s\n", zLog);

#else /* __orxMSVC__ */

    pcBuffer += snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), " %s\n", zLog);

#endif /* __orxMSVC__ */

    pcBuffer[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE  - (pcBuffer - zBuffer) - 1] = '\0';

    /* Use file? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_FILE)
    {
      if(_eLevel == orxDEBUG_LEVEL_LOG)
      {

#if !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__)

        /* Needs to open the file? */
        if(sstDebug.pstLogFile == orxNULL)
        {
          /* Opens it */
          sstDebug.pstLogFile = fopen(sstDebug.zLogFile, "a+");
        }

#endif /* !__orxANDROID__ && !__orxANDROID_NATIVE__ */

        pstFile = sstDebug.pstLogFile;
      }
      else
      {

#if !defined(__orxANDROID__) && !defined(__orxANDROID_NATIVE__)

        /* Needs to open the file? */
        if(sstDebug.pstDebugFile == orxNULL)
        {
          /* Opens it */
          sstDebug.pstDebugFile = fopen(sstDebug.zDebugFile, "a+");
        }

#endif /* !__orxANDROID__ && !__orxANDROID_NATIVE__ */

        pstFile = sstDebug.pstDebugFile;
      }

      /* Valid? */
      if(pstFile != orxNULL)
      {
        fprintf(pstFile, "%s", zBuffer);
        fflush(pstFile);
      }
    }

    /* Console Display? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_CONSOLE)
    {

#if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

      if(_eLevel == orxDEBUG_LEVEL_LOG)
      {
        __android_log_print(ANDROID_LOG_INFO, "ORX", zBuffer);
      }
      else
      {
        __android_log_print(ANDROID_LOG_DEBUG, "ORX", zBuffer);
      }

#else /* __orxANDROID__ */

      if(_eLevel == orxDEBUG_LEVEL_LOG)
      {
        pstFile = stdout;
      }
      else
      {
        pstFile = stderr;
      }

      fprintf(pstFile, "%s", zBuffer);
      fflush(pstFile);

#endif /* __orxANDROID__ */

    }
  }

  /* Done */
  return;
}

/** Enables/disables a given log level
 * @param[in]   _eLevel                       Debug level to enable/disable
 * @param[in]   _bEnable                      Enable / disable
*/
void orxFASTCALL _orxDebug_EnableLevel(orxDEBUG_LEVEL _eLevel, orxBOOL _bEnable)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);
  orxASSERT(_eLevel < orxDEBUG_LEVEL_NUMBER);

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* Updates level flags */
    orxFLAG_SET(sstDebug.u32LevelFlags, 1 << _eLevel, 0);
  }
  else
  {
    /* Updates level flags */
    orxFLAG_SET(sstDebug.u32LevelFlags, 0, 1 << _eLevel);
  }

  /* Done! */
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
  if((sstDebug.zDebugFile != orxNULL) && (sstDebug.zDebugFile != (orxSTRING)orxDEBUG_KZ_DEFAULT_DEBUG_FILE))
  {
    /* Closes it */
    fclose(sstDebug.pstDebugFile);

    /* Deletes it */
    free(sstDebug.zDebugFile);
  }

  /* Valid? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY) && (_zFileName != (orxSTRING)orxDEBUG_KZ_DEFAULT_DEBUG_FILE))
  {
    orxU32 u32Size;

    /* Gets string size in bytes */
    u32Size = ((orxU32)strlen(_zFileName) + 1) * sizeof(orxCHAR);

    /* Allocates it */
    sstDebug.zDebugFile = (orxSTRING)malloc((size_t)u32Size);

    /* Stores new name */
    memcpy(sstDebug.zDebugFile, _zFileName, (size_t)u32Size);
  }
  else
  {
    /* Uses default file */
    sstDebug.zDebugFile = (orxSTRING)orxDEBUG_KZ_DEFAULT_DEBUG_FILE;
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
  if((sstDebug.zLogFile != orxNULL) && (sstDebug.zLogFile != (orxSTRING)orxDEBUG_KZ_DEFAULT_LOG_FILE))
  {
    /* Closes it */
    fclose(sstDebug.pstLogFile);

    /* Deletes it */
    free(sstDebug.zLogFile);
  }

  /* Valid? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY) && (_zFileName != (orxSTRING)orxDEBUG_KZ_DEFAULT_LOG_FILE))
  {
    orxU32 u32Size;

    /* Gets string size in bytes */
    u32Size = ((orxU32)strlen(_zFileName) + 1) * sizeof(orxCHAR);

    /* Allocates it */
    sstDebug.zLogFile = (orxSTRING)malloc((size_t)u32Size);

    /* Stores new name */
    memcpy(sstDebug.zLogFile, _zFileName, (size_t)u32Size);
  }
  else
  {
    /* Uses default file */
    sstDebug.zLogFile = (orxSTRING)orxDEBUG_KZ_DEFAULT_LOG_FILE;
  }
}

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */

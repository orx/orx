/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2022 Orx-Project
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
#include "core/orxConsole.h"

#include <stdlib.h>

#if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

  #include <jni.h>
  #include <android/log.h>

#endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */

#ifdef __orxWINDOWS__

  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>

#endif /* __orxWINDOWS__ */

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
#define orxDEBUG_KU32_STATIC_FLAG_ANSI          0x20000000
#define orxDEBUG_KU32_STATIC_FLAG_LOGGING       0x40000000

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

  /* Level flags */
  orxU32 u32LevelFlags;

  /* Control flags */
  orxU32 u32Flags;

  /* Log callback function */
  orxDEBUG_CALLBACK_FUNCTION pfnLogCallback;

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

    default: zResult = "INVALID DEBUG LEVEL"; break;
  }

  /* Done! */
  return zResult;
}

/** Gets debug level format
 * @param[in]   _eLevel                       Concerned debug level
 *@return      Corresponding literal format
 */
static orxINLINE const orxSTRING orxDebug_GetLevelFormat(orxDEBUG_LEVEL _eLevel)
{
  const orxSTRING zResult;

  /* Depending on level */
  switch(_eLevel)
  {
    case orxDEBUG_LEVEL_ANIM:
    case orxDEBUG_LEVEL_CONFIG:
    case orxDEBUG_LEVEL_CLOCK:
    case orxDEBUG_LEVEL_DISPLAY:
    case orxDEBUG_LEVEL_FILE:
    case orxDEBUG_LEVEL_INPUT:
    case orxDEBUG_LEVEL_JOYSTICK:
    case orxDEBUG_LEVEL_KEYBOARD:
    case orxDEBUG_LEVEL_MEMORY:
    case orxDEBUG_LEVEL_MOUSE:
    case orxDEBUG_LEVEL_OBJECT:
    case orxDEBUG_LEVEL_PARAM:
    case orxDEBUG_LEVEL_PHYSICS:
    case orxDEBUG_LEVEL_PLUGIN:
    case orxDEBUG_LEVEL_PROFILER:
    case orxDEBUG_LEVEL_RENDER:
    case orxDEBUG_LEVEL_SCREENSHOT:
    case orxDEBUG_LEVEL_SOUND:
    case orxDEBUG_LEVEL_SYSTEM:
    case orxDEBUG_LEVEL_TIMER:
    case orxDEBUG_LEVEL_USER:

    case orxDEBUG_LEVEL_ALL:

      zResult = orxDEBUG_KZ_TYPE_WARNING_FORMAT " ";
      break;

    case orxDEBUG_LEVEL_LOG:

      zResult = orxDEBUG_KZ_TYPE_LOG_FORMAT " ";
      break;

    case orxDEBUG_LEVEL_ASSERT:

      zResult = orxDEBUG_KZ_TYPE_ERROR_FORMAT " ";
      break;

    default: zResult = "INVALID DEBUG LEVEL"; break;
  }

  /* Done! */
  return zResult;
}

/** Clears ANSI codes from a buffer
 * @param[in]   _zBuffer                      Concerned buffer
 */
static orxINLINE void orxDebug_ClearANSICodes(orxSTRING _zBuffer)
{
  orxCHAR *pc, *pcDst;

  /* For all characters */
  for(pcDst = pc = _zBuffer; *pc != orxCHAR_NULL; pc++)
  {
    /* ANSI escape code? */
    if(*pc == orxANSI_KC_MARKER)
    {
      /* Skips code */
      for(;
          (((*pc | 0x20) < 'a')
        || ((*pc | 0x20) > 'z'))
       && (*pc != orxCHAR_NULL);
          pc++)
        ;

      /* Valid? */
      if(*pc != orxCHAR_NULL)
      {
        /* Skips end of code */
        continue;
      }
      else
      {
        /* Stops */
        break;
      }
    }
    else
    {
      /* Should copy? */
      if(pcDst != pc)
      {
        *pcDst = *pc;
      }

      /* Updates destination */
      pcDst++;
    }
  }

  /* Ends string */
  *pcDst = orxCHAR_NULL;

  /* Done! */
  return;
}

/** Has ANSI codes>
 * @param[in]   _zBuffer                      Concerned buffer
 */
static orxINLINE orxBOOL orxDebug_HasANSICodes(orxSTRING _zBuffer)
{
  orxCHAR  *pc;
  orxBOOL   bResult = orxFALSE;

  /* For all characters */
  for(pc = _zBuffer; *pc != orxCHAR_NULL; pc++)
  {
    /* ANSI escape code? */
    if(*pc == orxANSI_KC_MARKER)
    {
      /* Updates result */
      bResult = orxTRUE;
      break;
    }
  }

  /* Done! */
  return bResult;
}


/***************************************************************************
 * Public functions                                                        *
 ***************************************************************************/

/** Inits the debug module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
orxSTATUS orxFASTCALL _orxDebug_Init()
{
  orxU32    i;
  orxSTATUS eResult = orxSTATUS_FAILURE;

  /* Init dependencies */

  /* Correct parameters ? */
  if(orxDEBUG_LEVEL_NUMBER > orxDEBUG_LEVEL_MAX_NUMBER)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Internal error. DEBUG_LEVEL_NUMBER(%d) > DEBUG_LEVEL_MAX_NUMBER(%d).", orxDEBUG_LEVEL_NUMBER, orxDEBUG_LEVEL_MAX_NUMBER);

    eResult = orxSTATUS_FAILURE;
  }
  /* Already Initialized? */
  else if(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY)
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to initialize debug module when it was already initialized.");

    eResult = orxSTATUS_SUCCESS;
  }
  else
  {
    orxU8 *pu8;

    /* Cleans static controller */
    for(i = 0, pu8 = (orxU8 *)&sstDebug; i < sizeof(orxDEBUG_STATIC); i++)
    {
      *pu8++ = 0;
    }

    /* Inits default debug & level flags */
    sstDebug.u32DebugFlags  = orxDEBUG_KU32_STATIC_MASK_DEFAULT;
    sstDebug.u32LevelFlags  = orxDEBUG_KU32_STATIC_LEVEL_MASK_DEFAULT;

#if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

    /* Sets module as initialized */
    sstDebug.u32Flags = orxDEBUG_KU32_STATIC_FLAG_READY;

#else /* __orxANDROID__ || __orxANDROID_NATIVE__ */

    /* Sets module as initialized, with ANSI support */
    sstDebug.u32Flags = orxDEBUG_KU32_STATIC_FLAG_READY | orxDEBUG_KU32_STATIC_FLAG_ANSI;

#endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */

    /* Inits default files */
    _orxDebug_SetDebugFile(orxDEBUG_KZ_DEFAULT_DEBUG_FILE);
    _orxDebug_SetLogFile(orxDEBUG_KZ_DEFAULT_LOG_FILE);

#ifdef __orxWINDOWS__

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
  #define ENABLE_VIRTUAL_TERMINAL_PROCESSING 4
#endif /* !ENABLE_VIRTUAL_TERMINAL_PROCESSING */

    /* Enables ANSI/VT100 features (works only with Windows 10 and up) */
    if(!SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
      /* Removes ANSI flag */
      sstDebug.u32Flags &= ~orxDEBUG_KU32_STATIC_FLAG_ANSI;
    }

#endif /* __orxWINDOWS__ */

#ifdef __orxMAC__

    {
      const char *zTerminal;

      /* Gets TERM environment variable */
      zTerminal = getenv("TERM");

      /* No valid ANSI terminal found? */
      if((zTerminal == orxNULL) || (orxString_SearchString(zTerminal, "color") == orxNULL))
      {
        /* Removes ANSI flag */
        sstDebug.u32Flags &= ~orxDEBUG_KU32_STATIC_FLAG_ANSI;
      }
    }

#endif /* __orxMAC__ */

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

    /* Clears log callback */
    sstDebug.pfnLogCallback = orxNULL;

    /* Updates flags */
    sstDebug.u32Flags &= ~orxDEBUG_KU32_STATIC_FLAG_READY;
  }
  else
  {
    /* Logs message */
    orxDEBUG_PRINT(orxDEBUG_LEVEL_SYSTEM, "Tried to exit debug module when it wasn't initialized.");
  }

  /* Done! */
  return;
}

/** Software break function */
void orxFASTCALL _orxDebug_Break()
{
  /* Windows / Linux / Mac / iOS / Android */
#if defined(__orxWINDOWS__) || defined(__orxLINUX__) || defined(__orxMAC__) || defined(__orxIOS__) || defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

  /* Compiler specific */

  #ifdef __orxMSVC__

    __debugbreak();

  #else /* __orxMSVC__ */

    #ifdef __orxGCC__

      /* Requires GCC >= 4.2.4 */
      orxASSERT((__GNUC__ > 4) || ((__GNUC__ == 4) && ((__GNUC_MINOR__ > 2) || ((__GNUC_MINOR__ == 2) && (__GNUC_PATCHLEVEL__ > 3)))))

    #endif /* __orxGCC__ */

    __builtin_trap();

  #endif /* __orxMSVC__ */

#endif /* __orxWINDOWS__ || __orxLINUX__ || __orxMAC__ || __orxIOS__ || __orxANDROID__ || __orxANDROID_NATIVE__ */

  /* Done! */
  return;
}

/** Sets current debug flags
 * @param[in]   _u32Add                       Flags to add
 * @param[in]   _u32Remove                    Flags to remove
 */
void orxFASTCALL _orxDebug_SetFlags(orxU32 _u32Add, orxU32 _u32Remove)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Updates flags */
  sstDebug.u32DebugFlags &= ~_u32Remove;
  sstDebug.u32DebugFlags |= _u32Add;

  /* Done! */
  return;
}

/** Gets current debug flags
 * @return Current debug flags
 */
orxU32 orxFASTCALL _orxDebug_GetFlags()
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Done! */
  return sstDebug.u32DebugFlags;
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
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Is level enabled and not re-entrant? */
  if(orxFLAG_TEST(sstDebug.u32LevelFlags, (1 << _eLevel)) && !orxFLAG_TEST(sstDebug.u32Flags, orxDEBUG_KU32_STATIC_FLAG_LOGGING))
  {
    orxCHAR   zBuffer[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE], zLog[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE], *pcBuffer = zBuffer;
    orxBOOL   bUseANSICodes = orxFALSE;
    orxSTATUS eStatus = orxSTATUS_SUCCESS;
    va_list   stArgs;

    /* Updates status */
    orxFLAG_SET(sstDebug.u32Flags, orxDEBUG_KU32_STATIC_FLAG_LOGGING, orxDEBUG_KU32_STATIC_FLAG_NONE);

    /* Empties current buffer */
    pcBuffer[0] = orxCHAR_NULL;

    /* Full Time Stamp? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_FULL_TIMESTAMP)
    {
      time_t u32Time;

      /* Inits Log Time */
      time(&u32Time);

      pcBuffer += strftime(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE, orxDEBUG_KZ_DATE_FULL_FORMAT " ", localtime(&u32Time));

      /* Updates status */
      bUseANSICodes = orxTRUE;
    }
    /* Time Stamp? */
    else if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP)
    {
      time_t u32Time;

      /* Inits Log Time */
      time(&u32Time);

      pcBuffer += strftime(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE, orxDEBUG_KZ_DATE_FORMAT " ", localtime(&u32Time));

      /* Updates status */
      bUseANSICodes = orxTRUE;
    }

    /* Log Type? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TYPE)
    {

#ifdef __orxMSVC__

      pcBuffer += _snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), orxDebug_GetLevelFormat(_eLevel), orxDebug_GetLevelString(_eLevel));

#else /* __orxMSVC__ */

      pcBuffer += snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), orxDebug_GetLevelFormat(_eLevel), orxDebug_GetLevelString(_eLevel));

#endif /* __orxMSVC__ */

      /* Updates status */
      bUseANSICodes = orxTRUE;
    }

    /* Log FUNCTION, FILE & LINE? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TAGGED)
    {
      const orxSTRING zFile;

      /* Skips complete path */
      zFile = orxString_SkipPath(_zFile);

#ifdef __orxMSVC__

      /* Writes info */
      pcBuffer += _snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), orxDEBUG_KZ_FILE_FORMAT " ", zFile, _zFunction, _u32Line);

#else /* __orxMSVC__ */

      /* Writes info */
      pcBuffer += snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), orxDEBUG_KZ_FILE_FORMAT " ", zFile, _zFunction, (unsigned int)_u32Line);

#endif /* __orxMSVC__ */

      /* Updates status */
      bUseANSICodes = orxTRUE;
    }

    /* Debug Log */
    va_start(stArgs, _zFormat);
    vsnprintf(zLog, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), _zFormat, stArgs);
    zLog[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer) - 1] = orxCHAR_NULL;
    va_end(stArgs);

    /* Doesn't implicitly use ANSI codes? */
    if(bUseANSICodes == orxFALSE)
    {
      /* Updates status */
      bUseANSICodes = orxDebug_HasANSICodes(zLog);
    }

#ifdef __orxMSVC__

    pcBuffer += _snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), "%s%s%s", zLog, (bUseANSICodes != orxFALSE) ? orxANSI_KZ_COLOR_RESET : orxSTRING_EMPTY, orxSTRING_EOL);

#else /* __orxMSVC__ */

    pcBuffer += snprintf(pcBuffer, orxDEBUG_KS32_BUFFER_OUTPUT_SIZE - (pcBuffer - zBuffer), "%s%s%s", zLog, (bUseANSICodes != orxFALSE) ? orxANSI_KZ_COLOR_RESET : orxSTRING_EMPTY, orxSTRING_EOL);

#endif /* __orxMSVC__ */

    pcBuffer[orxDEBUG_KS32_BUFFER_OUTPUT_SIZE  - (pcBuffer - zBuffer) - 1] = orxCHAR_NULL;

    /* Doesn't have ANSI support? */
    if(!orxFLAG_TEST(sstDebug.u32Flags, orxDEBUG_KU32_STATIC_FLAG_ANSI))
    {
      /* Clears ANSI codes */
      orxDebug_ClearANSICodes(zBuffer);
    }

    /* Should call the log callback? */
    if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_CALLBACK)
    {
      /* Is callback valid? */
      if(sstDebug.pfnLogCallback != orxNULL)
      {
        /* Calls it */
        eStatus = sstDebug.pfnLogCallback(_eLevel, _zFunction, _zFile, _u32Line, zBuffer);
      }
    }

    /* Should continue? */
    if(eStatus != orxSTATUS_FAILURE)
    {
      /* Terminal display? */
      if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TERMINAL)
      {
#if defined(__orxANDROID__) || defined(__orxANDROID_NATIVE__)

#define  LOG_TAG    "orxDebug"
#define  LOGI(...)  __android_log_write(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_write(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

        if(_eLevel == orxDEBUG_LEVEL_LOG)
        {
          LOGI(zBuffer);
        }
        else
        {
          LOGD(zBuffer);
        }

#else /* __orxANDROID__ || __orxANDROID_NATIVE__ */

        FILE *pstFile;

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

#endif /* __orxANDROID__ || __orxANDROID_NATIVE__ */

      }

      /* Has ANSI support? */
      if(orxFLAG_TEST(sstDebug.u32Flags, orxDEBUG_KU32_STATIC_FLAG_ANSI))
      {
        /* Clears ANSI codes */
        orxDebug_ClearANSICodes(zBuffer);
      }

#ifdef __orxWINDOWS__

      /* Terminal display? */
      if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_TERMINAL)
      {
        OutputDebugString(zBuffer);
      }

#endif /* __orxWINDOWS__ */

      /* File print? */
      if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_FILE)
      {
        FILE *pstFile;

        if(_eLevel == orxDEBUG_LEVEL_LOG)
        {

#if !defined(__orxANDROID__) && !defined(__orxANDROID_ANDROID__)

          /* Needs to open the file? */
          if(sstDebug.pstLogFile == orxNULL)
          {
            /* Opens it */
            sstDebug.pstLogFile = fopen(sstDebug.zLogFile, "ab+");
          }

#endif /* !__orxANDROID__ && !__orxANDROID_NATIVE__ */

          pstFile = sstDebug.pstLogFile;
        }
        else
        {

#if !defined(__orxANDROID__) && !defined(__orxANDROID_ANDROID__)

          /* Needs to open the file? */
          if(sstDebug.pstDebugFile == orxNULL)
          {
            /* Opens it */
            sstDebug.pstDebugFile = fopen(sstDebug.zDebugFile, "ab+");
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

      /* Console display? */
      if(sstDebug.u32DebugFlags & orxDEBUG_KU32_STATIC_FLAG_CONSOLE)
      {
        /* Is console initialized? */
        if(orxModule_IsInitialized(orxMODULE_ID_CONSOLE) != orxFALSE)
        {
          /* Logs it */
          orxConsole_Log(zBuffer);
        }
      }
    }

    /* Updates status */
    orxFLAG_SET(sstDebug.u32Flags, orxDEBUG_KU32_STATIC_FLAG_NONE, orxDEBUG_KU32_STATIC_FLAG_LOGGING);
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

  /* Enable? */
  if(_bEnable != orxFALSE)
  {
    /* All levels? */
    if(_eLevel == orxDEBUG_LEVEL_ALL)
    {
      /* Updates level flags */
      orxFLAG_SET(sstDebug.u32LevelFlags, 0xFFFFFFFF, 0);
    }
    else
    {
      /* Checks */
      orxASSERT(_eLevel < orxDEBUG_LEVEL_MAX_NUMBER);

      /* Updates level flags */
      orxFLAG_SET(sstDebug.u32LevelFlags, 1 << _eLevel, 0);
    }
  }
  else
  {
    /* All levels? */
    if(_eLevel == orxDEBUG_LEVEL_ALL)
    {
      /* Updates level flags */
      orxFLAG_SET(sstDebug.u32LevelFlags, 0, 0xFFFFFFFF);
    }
    else
    {
      /* Checks */
      orxASSERT(_eLevel < orxDEBUG_LEVEL_MAX_NUMBER);

      /* Updates level flags */
      orxFLAG_SET(sstDebug.u32LevelFlags, 0, 1 << _eLevel);
    }
  }

  /* Done! */
  return;
}

/** Is a given log level enabled?
 * @param[in]   _eLevel                       Concerned debug level
*/
orxBOOL orxFASTCALL _orxDebug_IsLevelEnabled(orxDEBUG_LEVEL _eLevel)
{
  orxBOOL bResult;

  /* All levels? */
  if(_eLevel == orxDEBUG_LEVEL_ALL)
  {
    /* Updates result */
    bResult = orxFLAG_TEST_ALL(sstDebug.u32LevelFlags, 0xFFFFFFFF) ? orxTRUE : orxFALSE;
  }
  else
  {
    /* Checks */
    orxASSERT(_eLevel < orxDEBUG_LEVEL_MAX_NUMBER);

    /* Updates result */
    bResult = orxFLAG_TEST(sstDebug.u32LevelFlags, 1 << _eLevel) ? orxTRUE : orxFALSE;
  }

  /* Done! */
  return bResult;
}

/** Sets debug file name
 * @param[in]   _zFileName                    Debug file name
 */
void orxFASTCALL _orxDebug_SetDebugFile(const orxSTRING _zFileName)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Was open? */
  if(sstDebug.pstDebugFile != orxNULL)
  {
    /* Closes it */
    fclose(sstDebug.pstDebugFile);
    sstDebug.pstDebugFile = orxNULL;
  }

  /* Had a previous external name? */
  if((sstDebug.zDebugFile != orxNULL) && (strcmp(sstDebug.zDebugFile, orxDEBUG_KZ_DEFAULT_DEBUG_FILE) != 0))
  {
    /* Deletes it */
    free(sstDebug.zDebugFile);
  }

  /* Valid? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY) && (strcmp(_zFileName, orxDEBUG_KZ_DEFAULT_DEBUG_FILE) != 0))
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

  /* Done! */
  return;
}

/** Sets log file name
 * @param[in]   _zFileName                    Log file name
 */
void orxFASTCALL _orxDebug_SetLogFile(const orxSTRING _zFileName)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Was open? */
  if(sstDebug.pstLogFile != orxNULL)
  {
    /* Closes it */
    fclose(sstDebug.pstLogFile);
    sstDebug.pstLogFile = orxNULL;
  }

  /* Had a previous external name? */
  if((sstDebug.zLogFile != orxNULL) && (strcmp(sstDebug.zLogFile, orxDEBUG_KZ_DEFAULT_LOG_FILE) != 0))
  {
    /* Deletes it */
    free(sstDebug.zLogFile);
  }

  /* Valid? */
  if((_zFileName != orxNULL) && (_zFileName != orxSTRING_EMPTY) && (strcmp(_zFileName, orxDEBUG_KZ_DEFAULT_LOG_FILE) != 0))
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

  /* Done! */
  return;
}

/** Sets log callback function, if the callback returns orxSTATUS_FAILURE, the log entry will be entirely inhibited
* @param[in]   _pfnLogCallback                Log callback function, orxNULL to remove it
*/
void orxFASTCALL _orxDebug_SetLogCallback(const orxDEBUG_CALLBACK_FUNCTION _pfnLogCallback)
{
  /* Checks */
  orxASSERT(sstDebug.u32Flags & orxDEBUG_KU32_STATIC_FLAG_READY);

  /* Stores the callback */
  sstDebug.pfnLogCallback = _pfnLogCallback;

  /* Done! */
  return;
}

#ifdef __orxMSVC__

  #pragma warning(default : 4996)

#endif /* __orxMSVC__ */

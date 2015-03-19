/* Orx - Portable Game Engine
 *
 * Copyright (c) 2008-2015 Orx-Project
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
 * @file orxDebug.h
 * @date 10/12/2003
 * @author iarwain@orx-project.org
 *
 * @todo
 * - Add graphical debug from outside, using a shared debug info array
 * - Enhance logging, use of different log levels / colors
 */

/**
 * @addtogroup orxDebug
 *
 * Debug module
 * Module used to output debug info and assert code
 *
 * @{
 */


#ifndef _orxDEBUG_H_
#define _orxDEBUG_H_

#include "orxInclude.h"


/* *** orxDEBUG flags *** */

#define orxDEBUG_KU32_STATIC_FLAG_NONE                0x00000000

#define orxDEBUG_KU32_STATIC_FLAG_TIMESTAMP           0x00000001
#define orxDEBUG_KU32_STATIC_FLAG_FULL_TIMESTAMP      0x00000002
#define orxDEBUG_KU32_STATIC_FLAG_TYPE                0x00000004
#define orxDEBUG_KU32_STATIC_FLAG_TAGGED              0x00000008

#define orxDEBUG_KU32_STATIC_FLAG_FILE                0x00000010
#define orxDEBUG_KU32_STATIC_FLAG_TERMINAL            0x00000020
#define orxDEBUG_KU32_STATIC_FLAG_CONSOLE             0x00000040

#define orxDEBUG_KU32_STATIC_MASK_DEFAULT             0x00000075

#define orxDEBUG_KU32_STATIC_MASK_DEBUG               0x0000003D

#define orxDEBUG_KU32_STATIC_MASK_USER_ALL            0x0FFFFFFF


/* *** Misc *** */

#define orxDEBUG_KZ_DEFAULT_DEBUG_FILE                "orx-debug.log"
#define orxDEBUG_KZ_DEFAULT_LOG_FILE                  "orx.log"
#define orxDEBUG_KZ_DEFAULT_LOG_SUFFIX                ".log"
#define orxDEBUG_KZ_DEFAULT_DEBUG_SUFFIX              "-debug.log"


/* *** Debug Macros *** */

/* Log message, compiler specific */
#if defined(__orxGCC__) || defined(__orxLLVM__)

  #define orxLOG(STRING, ...)                                                                                     \
  do                                                                                                              \
  {                                                                                                               \
    _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__);  \
  } while(orxFALSE)

  #define orxLOG_TERMINAL(STRING, ...)                                                                            \
  do                                                                                                              \
  {                                                                                                               \
    orxU32 u32DebugFlags;                                                                                         \
    u32DebugFlags = _orxDebug_GetFlags();                                                                         \
    _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_TERMINAL,                                                        \
                       orxDEBUG_KU32_STATIC_FLAG_FILE                                                             \
                      |orxDEBUG_KU32_STATIC_FLAG_CONSOLE);                                                        \
    _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__);  \
    _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                        \
  } while(orxFALSE)

  #define orxLOG_FILE(STRING, ...)                                                                                \
  do                                                                                                              \
  {                                                                                                               \
    orxU32 u32DebugFlags;                                                                                         \
    u32DebugFlags = _orxDebug_GetFlags();                                                                         \
    _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_FILE,                                                            \
                       orxDEBUG_KU32_STATIC_FLAG_CONSOLE                                                          \
                      |orxDEBUG_KU32_STATIC_FLAG_TERMINAL);                                                       \
    _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__);  \
    _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                        \
  } while(orxFALSE)

  #define orxLOG_CONSOLE(STRING, ...)                                                                             \
  do                                                                                                              \
  {                                                                                                               \
    orxU32 u32DebugFlags;                                                                                         \
    u32DebugFlags = _orxDebug_GetFlags();                                                                         \
    _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_CONSOLE,                                                         \
                       orxDEBUG_KU32_STATIC_FLAG_FILE                                                             \
                       orxDEBUG_KU32_STATIC_FLAG_TERMINAL);                                                       \
    _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__);  \
    _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                        \
  } while(orxFALSE)

#else /* __orxGCC__ || __orxLLVM__ */
  #ifdef __orxMSVC__

    #define orxLOG(STRING, ...)                                                                                   \
    do                                                                                                            \
    {                                                                                                             \
      _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, __VA_ARGS__);  \
    } while(orxFALSE)

    #define orxLOG_TERMINAL(STRING, ...)                                                                          \
    do                                                                                                            \
    {                                                                                                             \
      orxU32 u32DebugFlags;                                                                                       \
      u32DebugFlags = _orxDebug_GetFlags();                                                                       \
      _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_TERMINAL,                                                      \
                         orxDEBUG_KU32_STATIC_FLAG_FILE                                                           \
                        |orxDEBUG_KU32_STATIC_FLAG_CONSOLE);                                                      \
      _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, __VA_ARGS__);  \
      _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                      \
    } while(orxFALSE)

    #define orxLOG_FILE(STRING, ...)                                                                              \
    do                                                                                                            \
    {                                                                                                             \
      orxU32 u32DebugFlags;                                                                                       \
      u32DebugFlags = _orxDebug_GetFlags();                                                                       \
      _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_FILE,                                                          \
                         orxDEBUG_KU32_STATIC_FLAG_CONSOLE                                                        \
                        |orxDEBUG_KU32_STATIC_FLAG_TERMINAL);                                                     \
      _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, __VA_ARGS__);  \
      _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                      \
    } while(orxFALSE)

    #define orxLOG_CONSOLE(STRING, ...)                                                                           \
    do                                                                                                            \
    {                                                                                                             \
      orxU32 u32DebugFlags;                                                                                       \
      u32DebugFlags = _orxDebug_GetFlags();                                                                       \
      _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_FLAG_CONSOLE,                                                       \
                         orxDEBUG_KU32_STATIC_FLAG_FILE                                                           \
                         orxDEBUG_KU32_STATIC_FLAG_TERMINAL);                                                     \
      _orxDebug_Log(orxDEBUG_LEVEL_LOG, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, __VA_ARGS__);  \
      _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                      \
    } while(orxFALSE)

  #endif /* __orxMSVC__ */
#endif /* __orcGCC__ || __orxLLVM__ */

#define orxDEBUG_INIT()                       _orxDebug_Init()
#define orxDEBUG_EXIT()                       _orxDebug_Exit()

#ifdef __orxDEBUG__

  /* Debug print, compiler specific */
  #if defined(__orxGCC__) || defined(__orxLLVM__)
    #define orxDEBUG_PRINT(LEVEL, STRING, ...)                                                                    \
    do                                                                                                            \
    {                                                                                                             \
      orxU32 u32DebugFlags;                                                                                       \
      u32DebugFlags = _orxDebug_GetFlags();                                                                       \
      _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_MASK_DEBUG, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                    \
      _orxDebug_Log(LEVEL, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__);             \
      _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                      \
    } while(orxFALSE)
  #else /* __orxGCC__ || __orxLLVM__ */
    #ifdef __orxMSVC__
      #define orxDEBUG_PRINT(LEVEL, STRING, ...)                                                                  \
      do                                                                                                          \
      {                                                                                                           \
        orxU32 u32DebugFlags;                                                                                     \
        u32DebugFlags = _orxDebug_GetFlags();                                                                     \
        _orxDebug_SetFlags(orxDEBUG_KU32_STATIC_MASK_DEBUG, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                  \
        _orxDebug_Log(LEVEL, (const orxSTRING)__FUNCTION__, __FILE__, __LINE__, STRING, __VA_ARGS__);             \
        _orxDebug_SetFlags(u32DebugFlags, orxDEBUG_KU32_STATIC_MASK_USER_ALL);                                    \
      } while(orxFALSE)
    #endif /* __orxMSVC__ */
  #endif /* __orcGCC__ || __orxLLVM__ */

  /* End platform specific */

  #define orxDEBUG_ENABLE_LEVEL(LEVEL, ENABLE)_orxDebug_EnableLevel(LEVEL, ENABLE)
  #define orxDEBUG_IS_LEVEL_ENABLED(LEVEL)    _orxDebug_IsLevelEnabled(LEVEL)

  #define orxDEBUG_SET_FLAGS(SET, UNSET)      _orxDebug_SetFlags(SET, UNSET)
  #define orxDEBUG_GET_FLAGS()                _orxDebug_GetFlags()

  /* Break */
  #define orxBREAK()                          _orxDebug_Break()

  /* Files */
  #define orxDEBUG_SETDEBUGFILE(FILE)         _orxDebug_SetDebugFile(FILE)
  #define orxDEBUG_SETLOGFILE(FILE)           _orxDebug_SetLogFile(FILE)
  #define orxDEBUG_SETBASEFILENAME(FILE)                      \
    do                                                        \
    {                                                         \
      orxCHAR   zBuffer[256];                                 \
      zBuffer[255] = orxCHAR_NULL;                            \
      strncpy(zBuffer, FILE, 255);                            \
      strncat(zBuffer, orxDEBUG_KZ_DEFAULT_DEBUG_SUFFIX, 255);\
      _orxDebug_SetDebugFile(zBuffer);                        \
      strncpy(zBuffer, FILE, 255);                            \
      strncat(zBuffer, orxDEBUG_KZ_DEFAULT_LOG_SUFFIX, 255);  \
      _orxDebug_SetLogFile(zBuffer);                          \
    } while(orxFALSE)

  /* Assert */
  #if defined(__orxGCC__) || defined(__orxLLVM__)
    #define orxASSERT(TEST, ...)                                                                  \
      if(!(TEST))                                                                                 \
      {                                                                                           \
        orxDEBUG_PRINT(orxDEBUG_LEVEL_ASSERT, "[ASSERT] : <" #TEST ">", ##__VA_ARGS__); \
        orxBREAK();                                                                               \
      }
  #else /* __orxGCC__ || __orxLLVM__ */
    #ifdef __orxMSVC__
      #define orxASSERT(TEST, ...)                                                                \
        if(!(TEST))                                                                               \
        {                                                                                         \
          orxDEBUG_PRINT(orxDEBUG_LEVEL_ASSERT, "[ASSERT] : <" #TEST ">", __VA_ARGS__); \
          orxBREAK();                                                                             \
        }
    #endif /* __orxMSVC__ */
  #endif /* __orcGCC__ || __orxLLVM__ */

#else /* __orxDEBUG__ */

  #define orxDEBUG_PRINT(LEVEL, STRING, ...)

  #define orxBREAK()

  #define orxDEBUG_ENABLE_LEVEL(LEVEL, ENABLE)_orxDebug_EnableLevel(LEVEL, ENABLE)
  #define orxDEBUG_IS_LEVEL_ENABLED(LEVEL)    _orxDebug_IsLevelEnabled(LEVEL)

  #define orxDEBUG_SETDEBUGFILE(FILE)
  #define orxDEBUG_SETLOGFILE(FILE)           _orxDebug_SetLogFile(FILE)
  #define orxDEBUG_SETBASEFILENAME(FILE)                      \
    do                                                        \
    {                                                         \
      orxCHAR   zBuffer[256];                                 \
      zBuffer[255] = orxCHAR_NULL;                            \
      strncpy(zBuffer, FILE, 255);                            \
      strncat(zBuffer, orxDEBUG_KZ_DEFAULT_LOG_SUFFIX, 255);  \
      _orxDebug_SetLogFile(zBuffer);                          \
    } while(orxFALSE)

  #define orxASSERT(TEST, ...)

  #define orxDEBUG_SET_FLAGS(SET, UNSET)      _orxDebug_SetFlags(SET, UNSET)
  #define orxDEBUG_GET_FLAGS()                _orxDebug_GetFlags()

#endif /* __orxDEBUG__ */



/*****************************************************************************/

/* *** Debug defines. *** */

#define orxDEBUG_KS32_BUFFER_OUTPUT_SIZE      2048

#define orxDEBUG_KZ_DATE_FORMAT               "[%H:%M:%S]"
#define orxDEBUG_KZ_DATE_FULL_FORMAT          "[%Y-%m-%d %H:%M:%S]"


/*****************************************************************************/

/* *** Debug types. *** */
typedef enum __orxDEBUG_LEVEL_t
{
  orxDEBUG_LEVEL_ANIM = 0,                    /**< Anim Debug */
  orxDEBUG_LEVEL_CONFIG,                      /**< Config Debug */
  orxDEBUG_LEVEL_CLOCK,                       /**< Clock Debug */
  orxDEBUG_LEVEL_DISPLAY,                     /**< Display Debug */
  orxDEBUG_LEVEL_FILE,                        /**< File Debug */
  orxDEBUG_LEVEL_INPUT,                       /**< Input Debug */
  orxDEBUG_LEVEL_JOYSTICK,                    /**< Joystick Debug */
  orxDEBUG_LEVEL_KEYBOARD,                    /**< Keyboard Debug */
  orxDEBUG_LEVEL_MEMORY,                      /**< Memory Debug */
  orxDEBUG_LEVEL_MOUSE,                       /**< Mouse Debug */
  orxDEBUG_LEVEL_OBJECT,                      /**< Object Debug */
  orxDEBUG_LEVEL_PARAM,                       /**< Param Debug */
  orxDEBUG_LEVEL_PHYSICS,                     /**< Physics Debug */
  orxDEBUG_LEVEL_PLUGIN,                      /**< Plug-in Debug */
  orxDEBUG_LEVEL_PROFILER,                    /**< Profiler Debug */
  orxDEBUG_LEVEL_RENDER,                      /**< Render Debug */
  orxDEBUG_LEVEL_SCREENSHOT,                  /**< Screenshot Debug */
  orxDEBUG_LEVEL_SOUND,                       /**< Sound Debug */
  orxDEBUG_LEVEL_SYSTEM,                      /**< System Debug */
  orxDEBUG_LEVEL_TIMER,                       /**< Timer Debug */

  orxDEBUG_LEVEL_USER,                        /**< User Debug */

  orxDEBUG_LEVEL_LOG,                         /**< Log Debug */

  orxDEBUG_LEVEL_ASSERT,                      /**< Assert Debug */

  orxDEBUG_LEVEL_NUMBER,

  orxDEBUG_LEVEL_MAX_NUMBER = 32,

  orxDEBUG_LEVEL_ALL = 0xFFFFFFFE,            /**< All Debugs */

  orxDEBUG_LEVEL_NONE = orxENUM_NONE

} orxDEBUG_LEVEL;


/*****************************************************************************/

/* *** Functions *** */

/** Inits the debug module
 * @return      orxSTATUS_SUCCESS / orxSTATUS_FAILURE
 */
extern orxDLLAPI orxSTATUS orxFASTCALL        _orxDebug_Init();

/** Exits from the debug module */
extern orxDLLAPI void orxFASTCALL             _orxDebug_Exit();

/** Logs given debug text
 * @param[in]   _eLevel                       Debug level associated with this output
 * @param[in]   _zFunction                    Calling function name
 * @param[in]   _zFile                        Calling file name
 * @param[in]   _u32Line                      Calling file line
 * @param[in]   _zFormat                      Printf formatted text
 */
extern orxDLLAPI void orxCDECL                _orxDebug_Log(orxDEBUG_LEVEL _eLevel, const orxSTRING _zFunction, const orxSTRING _zFile, orxU32 _u32Line, const orxSTRING _zFormat, ...);

/** Enables/disables a given log level
 * @param[in]   _eLevel                       Debug level to enable/disable
 * @param[in]   _bEnable                      Enable / disable
*/
extern orxDLLAPI void orxFASTCALL             _orxDebug_EnableLevel(orxDEBUG_LEVEL _eLevel, orxBOOL _bEnable);

/** Is a given log level enabled?
 * @param[in]   _eLevel                       Concerned debug level
*/
extern orxDLLAPI orxBOOL orxFASTCALL          _orxDebug_IsLevelEnabled(orxDEBUG_LEVEL _eLevel);

/** Sets current debug flags
 * @param[in]   _u32Add                       Flags to add
 * @param[in]   _u32Remove                    Flags to remove
 */
extern orxDLLAPI void orxFASTCALL             _orxDebug_SetFlags(orxU32 _u32Add, orxU32 _u32Remove);

/** Gets current debug flags
 * @return Current debug flags
 */
extern orxDLLAPI orxU32 orxFASTCALL           _orxDebug_GetFlags();

/** Software break function */
extern orxDLLAPI void orxFASTCALL             _orxDebug_Break();

/** Sets debug file name
 * @param[in]   _zFileName                    Debug file name
 */
extern orxDLLAPI void orxFASTCALL             _orxDebug_SetDebugFile(const orxSTRING _zFileName);

/** Sets log file name
 * @param[in]   _zFileName                    Log file name
 */
extern orxDLLAPI void orxFASTCALL             _orxDebug_SetLogFile(const orxSTRING _zFileName);

#endif /* __orxDEBUG_H_ */

/** @} */

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include "include.h"

/*** DEBUG FLAGS ***/
#define DEBUG_KUL_FLAG_NONE       0x00000000
#define DEBUG_KUL_FLAG_ALL        0xFFFFFFFF

#define DEBUG_KUL_FLAG_TIMESTAMP  0x00000001
#define DEBUG_KUL_FLAG_TYPE       0x00000002
#define DEBUG_KUL_FLAG_TAGGED     0x00000004

#define DEBUG_KUL_FLAG_FILE       0x00000010
#define DEBUG_KUL_FLAG_SCREEN     0x00000020
#define DEBUG_KUL_FLAG_GRAPHICAL  0x00000040

#define DEBUG_KUL_FLAG_DEFAULT    0x00000037

/*** END OF FLAGS ***/

#if defined(DEBUG) || defined(DEBUG_LOG) || defined(DEBUG_MOUSE) || defined(DEBUG_KEYBOARD) || defined(DEBUG_JOYSTICK) || defined(DEBUG_INTERACTION) || defined(DEBUG_GRAPH) || defined(DEBUG_SOUND) || defined(DEBUG_TIMER) || defined(DEBUG_MEM) || defined(DEBUG_SCREENSHOT) || defined(DEBUG_FILE) || defined (DEBUG_PATHFINDER) || defined(DEBUG_ALL)

  #define NEED_DEBUG


  #ifdef DEBUG

    #define DEBUG_ALL
    #undef DEBUG

  #endif /* DEBUG */

/** Debug Macros. */
  #define DEBUG(LEVEL, STRING, ...) _debug(LEVEL, __FUNCTION__, __FILE__, __LINE__, STRING, ##__VA_ARGS__)

  #define DEBUG_INIT() _debug_init()
  #define DEBUG_EXIT()
  #define DEBUG_FLAG_SET(SET, UNSET) _debug_flag_set(SET, UNSET)
  #define DEBUG_FLAG_BACKUP() _debug_flag_backup()
  #define DEBUG_FLAG_RESTORE() _debug_flag_restore()
  #define DEBUG_GRAPH(BITMAP) _debug_graph(BITMAP)

  #define BREAK() _break()

  #define ASSERT(TEST) if(!(TEST)) {DEBUG(D_ASSERT, "!!!" #TEST "!!!"); BREAK();}

#else

  #define DEBUG(LEVEL, STRING, ...)

  #define BREAK()

  #define ASSERT(TEST)

  #define DEBUG_INIT()
  #define DEBUG_EXIT()
  #define DEBUG_FLAG_SET(SET, UNSET)
  #define DEBUG_FLAG_BACKUP()
  #define DEBUG_FLAG_RESTORE()
  #define DEBUG_GRAPH(BITMAP)

#endif /* DEBUG || ... */

/** Debug defines. */
#define DEBUG_KI_BUFFER_SIZE          32
#define DEBUG_KI_BUFFER_OUTPUT_SIZE   512

#define DEBUG_KZ_DATE_FORMAT          "[%Y-%m-%d %H:%M:%S']"

/** Debug types. */
typedef enum debug_enum
{
  D_MOUSE,              /**< Mouse Debug */
  D_KEYBOARD,           /**< Keyboard Debug */
  D_JOYSTICK,           /**< Joystick Debug */
  D_INTERACTION,        /**< Interaction Debug */
  D_GRAPH,              /**< Graph Debug */
  D_SOUND,              /**< Sound Debug */
  D_TIMER,              /**< Timer Debug */
  D_MEMORY,             /**< Memory Debug */
  D_SCREENSHOT,         /**< Screenshot Debug */
  D_FILE,               /**< File Debug */
  D_PATHFINDER,         /**< Pathfinder Debug */
  D_PLUGIN,             /**< Plug-in Debug */
  D_LOG,                /**< Log Debug */
  D_ALL,                /**< All Debugs */
  D_ASSERT,             /**< Assert Debug */
  D_CRITICAL_ASSERT     /**< Critical Assert Debug */
} debug_enum;

/** Debug output function. */
extern void   _debug(debug_enum _e_level, const char *_z_function, const char *_z_file, int32 _i_line, const char *_z_format, ...);
/** Debug flag backup function. */
extern void   _debug_flag_backup();
/** Debug flag restore function. */
extern void   _debug_flag_restore();
/** Debug flag get/set accessor. */
extern void   _debug_flag_set(unsigned long _u32_set, unsigned long _u32_unset);
/** Debug graphical output function. */
extern void   _debug_graph(graph_st_bitmap *_pst_bmp);
/** Debug init function. */
extern uint32 _debug_init();

/** Software break function. */
extern void   _break();

#endif /* _DEBUG_H_ */

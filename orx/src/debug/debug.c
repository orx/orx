#include <string.h>
#include <time.h>

#include "debug/debug.h"

#include "graph/graph.h"


#ifdef NEED_DEBUG
static const char *_debug_type(debug_enum _e)
{
  switch(_e)
  {
    case D_MOUSE:           return "MOUSE";
    case D_KEYBOARD:        return "KEYBOARD";
    case D_JOYSTICK:        return "JOYSTICK";
    case D_INTERACTION:     return "INTERACTION";
    case D_GRAPH:           return "GRAPH";
    case D_SOUND:           return "SOUND";
    case D_TIMER:           return "TIMER";
    case D_MEMORY:          return "MEMORY";
    case D_SCREENSHOT:      return "SCREENSHOT";
    case D_FILE:            return "FILE";
    case D_PATHFINDER:      return "PATHFINDER";
    case D_PLUGIN:          return "PLUGIN";
    case D_LOG:             return "LOG";
    case D_ALL:             return "ALL";
    case D_ASSERT:          return "ASSERT";
    case D_CRITICAL_ASSERT: return "CRITICAL ASSERT";
    
    default:
      return "INVALID DEBUG!";
  }

  return "";
}

static int32 sl_debug_index = 0;
static char *saz_debug_buffer[DEBUG_KI_BUFFER_SIZE];

static char *sz_debug_file_name = "debug.log";
static char *sz_log_file_name = "game.log";


static unsigned long debug_guls_flags = DEBUG_KUL_FLAG_DEFAULT;
static unsigned long debug_guls_flags_backup = DEBUG_KUL_FLAG_NONE;


/* Break(Point) function */
void _break()
{
  /* Windows / Linux */
#if defined(WINDOWS) || defined(LINUX)
  asm ("int $3");
#endif /* WINDOWS || LINUX */
}

void _debug_flag_backup()
{
  debug_guls_flags_backup = debug_guls_flags;

  return;
}

void _debug_flag_restore()
{
  debug_guls_flags = debug_guls_flags_backup;

  return;
}

void _debug_flag_set(unsigned long _u32_set, unsigned long _u32_unset)
{
  debug_guls_flags &= ~_u32_unset;
  debug_guls_flags |= _u32_set;

  return;
}

void _debug(debug_enum _e_level, const char *_z_function, const char *_z_file, int32 _i_line, const char *_z_format, ...)
{
  va_list st_args;

#ifndef DEBUG_ALL

#ifndef DEBUG_MOUSE
  if(_e_level == D_MOUSE)
    return;
#endif /* !DEBUG_MOUSE */

#ifndef DEBUG_KEYBOARD
  if(_e_level == D_KEYBOARD)
    return;
#endif /* !DEBUG_KEYBOARD */

#ifndef DEBUG_JOYSTICK
  if(_e_level == D_JOYSTICK)
    return;
#endif /* !DEBUG_JOYSTICK */

#ifndef DEBUG_INTERACTION
  if(_e_level == D_INTERACTION)
    return;
#endif /* !DEBUG_INTERACTION */

#ifndef DEBUG_GRAPH
  if(_e_level == D_GRAPH)
    return;
#endif /* !DEBUG_GRAPH */

#ifndef DEBUG_SOUND
  if(_e_level == D_SOUND)
    return;
#endif /* !DEBUG_SOUND */

#ifndef DEBUG_TIMER
  if(_e_level == D_TIMER)
    return;
#endif /* !DEBUG_TIMER */

#ifndef DEBUG_MEM
  if(_e_level == D_MEM)
    return;
#endif /* !DEBUG_MEM */

#ifndef DEBUG_SCREENSHOT
  if(_e_level == D_SCREENSHOT)
    return; /* !DEBUG_SCREENSHOT */
#endif

#ifndef DEBUG_FILE
  if(_e_level == D_FILE)
    return;
#endif /* !DEBUG_FILE */

#ifndef DEBUG_PATHFINDER
  if(_e_level == D_PATHFINDER)
    return;
#endif /* !DEBUG_PATHFINDER */

#endif /* DEBUG_ALL */
  
  static char ac_string[DEBUG_KI_BUFFER_OUTPUT_SIZE], ac_log[DEBUG_KI_BUFFER_OUTPUT_SIZE];
  FILE *pst_file = NULL;

  ac_string[0] = '\0';

  /* Time Stamp? */
  if(debug_guls_flags & DEBUG_KUL_FLAG_TIMESTAMP)
  {
    time_t st_time;

    /* Inits Log Time */
    time(&st_time);

    strftime(ac_string, DEBUG_KI_BUFFER_OUTPUT_SIZE, DEBUG_KZ_DATE_FORMAT, localtime(&st_time));
  }

  /* Log Type? */
  if(debug_guls_flags & DEBUG_KUL_FLAG_TYPE)
  {
    sprintf(ac_string, "%s <%s>", ac_string, _debug_type(_e_level));
  }

  /* Log FUNCTION, FILE & LINE? */
  if(debug_guls_flags & DEBUG_KUL_FLAG_TAGGED)
  {
    sprintf(ac_string, "%s (%s() - %s:%ld)", ac_string, _z_function, _z_file, _i_line);
  }

  /* Debug Log */
  va_start(st_args, _z_format);
  vsprintf(ac_log, _z_format, st_args);
  va_end(st_args);

  sprintf(ac_string, "%s %s", ac_string, ac_log);

  /* Graphical Mode? */
  if(debug_guls_flags & DEBUG_KUL_FLAG_GRAPHICAL)
  {
      *(saz_debug_buffer[sl_debug_index]) = '\0';
      strcpy(saz_debug_buffer[sl_debug_index], ac_string);

      sl_debug_index++;
      if(sl_debug_index >= DEBUG_KI_BUFFER_SIZE)
      {
        sl_debug_index = 0;
      }
  }

  /* Use file? */
  if(debug_guls_flags & DEBUG_KUL_FLAG_FILE)
  {
    if(_e_level == D_LOG)
    {
      pst_file = fopen(sz_log_file_name, "awR");
    }
    else
    {
      pst_file = fopen(sz_debug_file_name, "awR");
    }

    fprintf(pst_file, ac_string);
    fflush(pst_file);
    fclose(pst_file);
   }

  /* On Screen Display? */
  if(debug_guls_flags & DEBUG_KUL_FLAG_SCREEN)  
  {
    if(_e_level == D_LOG)
    {
      pst_file = stdout;
    }
    else
    {
      pst_file = stderr;
    }

    fprintf(pst_file, ac_string);
    fflush(pst_file);
  }

  return;
}

uint32 _debug_init()
{
  int32 i;

  for(i = 0; i < DEBUG_KI_BUFFER_SIZE; i++)
  {
      saz_debug_buffer[i] = (char *)malloc(DEBUG_KI_BUFFER_OUTPUT_SIZE * sizeof(char));
      *(saz_debug_buffer[i]) = '\0';
  }

  return EXIT_SUCCESS;
}

void _debug_graph(graph_st_bitmap *_pst_bmp)
{
	int32 startline = DEBUG_KI_BUFFER_SIZE;
	int32 currentline = startline;
	int32 indexPos = sl_debug_index + 1;
	
	if(*(saz_debug_buffer[indexPos]) == '\0')
  {
		indexPos = 0;
	}

	while((indexPos != sl_debug_index) && (*(saz_debug_buffer[indexPos]) != '\0'))
  {
		graph_printf(_pst_bmp, 0, currentline, -1, "%s", saz_debug_buffer[indexPos]);

		currentline += 10;

		indexPos++;
		if(indexPos >= DEBUG_KI_BUFFER_SIZE)
    {
			indexPos = 0;
		}
	}

  return;
}

#endif /* NEED_DEBUG */

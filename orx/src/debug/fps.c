#include "debug/fps.h"

#include "core/clock.h"
#include "debug/debug.h"

static uint32 sui_fps_frame_count = 0;
static volatile uint32 svui_fps = 0;

static inline void fps_proc()
{
  svui_fps = sui_fps_frame_count;
  sui_fps_frame_count = 0;

  return;
}


uint32 fps_init()
{
  if(clock_cb_function_add(fps_proc, 1000) == FALSE)
  {
    DEBUG(D_TIMER, MSG_FPS_KZ_INIT_FAILED);
  }

  return EXIT_SUCCESS;
}

void fps_exit()
{
  clock_cb_function_remove(fps_proc, 1000);
/*
  timer_int_remove(fps_proc);
*/

  return;
}


inline void fps_counter_increase()
{
  sui_fps_frame_count++;

  return;
}

inline uint32 fps_get()
{
  return svui_fps;
}

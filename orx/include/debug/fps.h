#ifndef _FPS_H_
#define _FPS_H_

#include "include.h"

#include "msg/msg_fps.h"


extern uint32 fps_init();
extern void   fps_exit();

extern void   fps_counter_increase();
extern uint32 fps_get();


#endif /* _FPS_H_ */

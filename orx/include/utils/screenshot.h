#ifndef _SCREENSHOT_H_
#define _SCREENSHOT_H_

#include "include.h"

#include "msg/msg_screenshot.h"


#define SCREENSHOT_KZ_DIRECTORY "../data"
#define SCREENSHOT_KZ_PREFIX    "shot"
#define SCREENSHOT_KZ_EXT       "bmp"


extern uint32 screenshot_init();
extern void   screenshot_exit();
extern void   screenshot_take();

#endif /* _SCREENSHOT_H_ */

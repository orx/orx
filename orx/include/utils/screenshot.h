#ifndef _SCREENSHOT_H_
#define _SCREENSHOT_H_

#include "orxInclude.h"

#include "msg/msg_screenshot.h"


#define SCREENSHOT_KZ_DIRECTORY "../data"
#define SCREENSHOT_KZ_PREFIX    "shot"
#define SCREENSHOT_KZ_EXT       "bmp"


extern orxU32 screenshot_init();
extern orxVOID   screenshot_exit();
extern orxVOID   screenshot_take();

#endif /* _SCREENSHOT_H_ */

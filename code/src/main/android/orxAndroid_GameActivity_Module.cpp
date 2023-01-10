#if defined(TARGET_OS_ANDROID)

#pragma clang diagnostic push
/* In GameActivity 1.1.0 there is indeed some sloppy code... */
#pragma clang diagnostic ignored "-Wreturn-type"
#pragma clang diagnostic ignored "-Wformat-insufficient-args"
#pragma clang diagnostic ignored "-Wmacro-redefined"

#include <game-activity/native_app_glue/android_native_app_glue.c>
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

#pragma clang diagnostic pop

#endif

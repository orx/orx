#include <jni.h>
#include <android/log.h>

#include "thread.h"
#include "apk_file.h"

JNIEXPORT jint JNICALL
JNI_OnLoad (JavaVM * vm, void * reserved)
{
	__android_log_print(ANDROID_LOG_DEBUG, "init",  "JNI_OnLoad\n");
	ThreadInit(vm);
	APKInit();
	return JNI_VERSION_1_4;
}

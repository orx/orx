LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest
### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := orxTest.c
LOCAL_STATIC_LIBRARIES := orxd-standalone android_native_app_glue

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,orx/code/build/android-native)
$(call import-module,android/native_app_glue)

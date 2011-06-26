LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest
### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := orxTest.c
LOCAL_STATIC_LIBRARIES := orxd-standalone
LOCAL_SHARED_LIBRARIES := apkfile

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,orx-android/code/build/android)

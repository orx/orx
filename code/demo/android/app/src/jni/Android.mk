LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest

LOCAL_SRC_FILES := orxTest.c
LOCAL_STATIC_LIBRARIES := orxd

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,lib/static/android)


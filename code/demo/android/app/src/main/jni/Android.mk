LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := orxDemo
LOCAL_SRC_FILES := orxDemo.cpp
LOCAL_CFLAGS := -DNO_MALLINFO=1 -DTARGET_OS_ANDROID
LOCAL_STATIC_LIBRARIES := orx

LOCAL_LDFLAGS += -v

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,prefab/orx)

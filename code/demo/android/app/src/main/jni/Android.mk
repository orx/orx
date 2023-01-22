LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := orxTest
LOCAL_SRC_FILES := orxTest.cpp
LOCAL_CFLAGS := -DNO_MALLINFO=1 -DTARGET_OS_ANDROID
LOCAL_STATIC_LIBRARIES := orx cpufeatures

LOCAL_LDFLAGS += -v

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,prefab/orx)

$(call import-module,android/cpufeatures)

LOCAL_PATH := $(call my-dir)
SRC_PATH := $(LOCAL_PATH)

ifeq ($(strip $(ORX)),)
$(error ORX environment variable not set)
endif

include $(CLEAR_VARS)

LOCAL_MODULE := orxTest
LOCAL_SRC_FILES := $(SRC_PATH)/orxTest.cpp
LOCAL_STATIC_LIBRARIES := orx

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-add-path,$(ORX))
$(call import-module,build/android/jni)

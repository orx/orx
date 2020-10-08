LOCAL_PATH := $(call my-dir)

SRC_PATH := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest

LOCAL_SRC_FILES := $(SRC_PATH)/orxTest.cpp
LOCAL_STATIC_LIBRARIES := android_orx
LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
$(call import-add-path,C:\Users\krist\Desktop\git\github\orx\code)
$(call import-module,lib/static/android-native)

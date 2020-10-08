LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest

LOCAL_SRC_FILES := orxTest.cpp
LOCAL_STATIC_LIBRARIES := orx_native
LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)
$(call import-add-path,$(ORX))
$(call import-module,lib/static/android-native)

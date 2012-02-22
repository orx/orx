LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := orx-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborx.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -landroid -ldl -lEGL -lGLESv2 -lOpenSLES
LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__
TARGET_PLATFORM = android-9
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := orxd-prebuilt
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborxd.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -landroid -ldl -lEGL -lGLESv2 -lOpenSLES
LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__ -D__orxDEBUG__
TARGET_PLATFORM = android-9
include $(PREBUILT_STATIC_LIBRARY)

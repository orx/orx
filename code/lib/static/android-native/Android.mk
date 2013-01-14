LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := orx
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborx.a
LOCAL_STATIC_LIBRARIES := SOIL-prebuilt Box2D-prebuilt nv_and_util
TARGET_PLATFORM = android-9

LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lOpenSLES

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := orxd
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborxd.a
LOCAL_STATIC_LIBRARIES := SOIL-prebuilt Box2D-prebuilt nv_and_util
TARGET_PLATFORM = android-9

LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__ -D__orxDEBUG__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lOpenSLES

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := orxp
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborxp.a
LOCAL_STATIC_LIBRARIES := SOIL-prebuilt Box2D-prebuilt nv_and_util
TARGET_PLATFORM = android-9

LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__ -D__orxPROFILER__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -landroid -lEGL -lGLESv2 -lOpenSLES

include $(PREBUILT_STATIC_LIBRARY)

$(call import-module,extern/nv_and_util)
$(call import-module,extern/SOIL/lib/android-native)
$(call import-module,extern/Box2D_2.1.3/lib/android-native)


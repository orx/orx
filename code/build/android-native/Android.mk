LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orx-standalone
LOCAL_SRC_FILES :=   $(TARGET_ARCH_ABI)/liborx.a
LOCAL_LDFLAGS := -llog -landroid -ldl -lEGL -lGLESv2
LOCAL_EXPORT_LDLIBS := -llog -landroid -ldl -lEGL -lGLESv2 -lOpenSLES $(LOCAL_PATH)/../../../extern/SOIL/build/android-native/$(TARGET_ARCH_ABI)/libSOIL.a $(LOCAL_PATH)/../../../extern/Box2D_2.1.3/build/android-native/$(TARGET_ARCH_ABI)/libBox2D.2.1.3.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__
LOCAL_EXPORT_CPPFLAGS := -D__orxANDROID_NATIVE__

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxd-standalone
LOCAL_SRC_FILES :=   $(TARGET_ARCH_ABI)/liborxd.a
LOCAL_LDFLAGS := -llog -landroid -ldl -lEGL -lGLESv2
LOCAL_EXPORT_LDLIBS := -llog -landroid -ldl -lEGL -lGLESv2 -lOpenSLES $(LOCAL_PATH)/../../../extern/SOIL/build/android-native/$(TARGET_ARCH_ABI)/libSOIL.a $(LOCAL_PATH)/../../../extern/Box2D_2.1.3/build/android-native/$(TARGET_ARCH_ABI)/libBox2D.2.1.3.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_EXPORT_CFLAGS := -D__orxANDROID_NATIVE__ -D__orxDEBUG__
LOCAL_EXPORT_CPPFLAGS := -D__orxANDROID_NATIVE__ -D__orxDEBUG__

include $(PREBUILT_STATIC_LIBRARY)


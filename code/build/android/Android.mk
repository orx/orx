LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE    := apkfile
LOCAL_SRC_FILES := apkfile/thread.c apkfile/apk_file.c apkfile/init.c
LOCAL_LDLIBS := -llog

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_EXPORT_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := orx-standalone
LOCAL_SRC_FILES :=   $(TARGET_ARCH_ABI)/liborx.a
LOCAL_SHARED_LIBRARIES := apkfile
LOCAL_LDFLAGS := -llog -ldl -lGLESv2
LOCAL_EXPORT_LDLIBS := -llog -ldl -lGLESv2 $(LOCAL_PATH)/../../../extern/SOIL/build/android/$(TARGET_ARCH_ABI)/libSOIL.a $(LOCAL_PATH)/../../../extern/Box2D_2.1.3/build/android/$(TARGET_ARCH_ABI)/libBox2D.2.1.3.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_EXPORT_CFLAGS := -D__orxANDROID__
LOCAL_EXPORT_CPPFLAGS := -D__orxANDROID__

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxd-standalone
LOCAL_SRC_FILES :=   $(TARGET_ARCH_ABI)/liborxd.a
LOCAL_SHARED_LIBRARIES := apkfile
LOCAL_LDFLAGS := -llog -ldl -lGLESv2
LOCAL_EXPORT_LDLIBS := -llog -ldl -lGLESv2 $(LOCAL_PATH)/../../../extern/SOIL/build/android/$(TARGET_ARCH_ABI)/libSOIL.a $(LOCAL_PATH)/../../../extern/Box2D_2.1.3/build/android/$(TARGET_ARCH_ABI)/libBox2D.2.1.3.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include
LOCAL_EXPORT_CFLAGS := -D__orxANDROID__ -D__orxDEBUG__
LOCAL_EXPORT_CPPFLAGS := -D__orxANDROID__ -D__orxDEBUG__

include $(PREBUILT_STATIC_LIBRARY)



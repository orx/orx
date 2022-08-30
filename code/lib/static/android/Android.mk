LOCAL_PATH := $(call my-dir)

BASE_PATH := $(LOCAL_PATH)/../../../

include $(CLEAR_VARS)
LOCAL_MODULE := android_orx
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := orx LiquidFun-prebuilt WebP-prebuilt

LOCAL_EXPORT_CFLAGS := -DTARGET_OS_ANDROID
LOCAL_EXPORT_C_INCLUDES := $(BASE_PATH)include
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -landroid -lEGL

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := android_orxd
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := orxd LiquidFun-prebuilt WebP-prebuilt

LOCAL_EXPORT_CFLAGS := -DTARGET_OS_ANDROID -D__orxDEBUG__
LOCAL_EXPORT_C_INCLUDES := $(BASE_PATH)include
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -landroid -lEGL

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := android_orxp
LOCAL_ARM_MODE := arm
LOCAL_STATIC_LIBRARIES := orxp LiquidFun-prebuilt WebP-prebuilt

LOCAL_EXPORT_CFLAGS := -DTARGET_OS_ANDROID -D__orxPROFILER__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -landroid -lEGL

include $(BUILD_STATIC_LIBRARY)

$(call import-add-path,$(BASE_PATH))
$(call import-module,build/android/jni)
$(call import-module,../extern/LiquidFun-1.1.0/lib/android)
$(call import-module,../extern/libwebp/lib/android)

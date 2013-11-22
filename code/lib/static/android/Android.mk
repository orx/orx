LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := orx
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborx.a
LOCAL_STATIC_LIBRARIES := SOIL-prebuilt Box2D-prebuilt OpenAL-prebuilt Tremolo-prebuilt orxAndroidSupport

TARGET_PLATFORM = android-9

LOCAL_EXPORT_CFLAGS := -D__orxANDROID__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -landroid -lEGL

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_EXPORT_CFLAGS += -mhard-float
    ifeq (,$(filter -fuse-ld=mcld,$(APP_LDFLAGS) $(LOCAL_LDFLAGS)))
        LOCAL_EXPORT_LDLIBS += -Wl,--no-warn-mismatch
    endif
endif

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := orxd
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborxd.a
LOCAL_STATIC_LIBRARIES := SOIL-prebuilt Box2D-prebuilt OpenAL-prebuilt Tremolo-prebuilt orxAndroidSupport

TARGET_PLATFORM = android-9

LOCAL_EXPORT_CFLAGS := -D__orxANDROID__ -D__orxDEBUG__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -landroid -lEGL

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_EXPORT_CFLAGS += -mhard-float
    ifeq (,$(filter -fuse-ld=mcld,$(APP_LDFLAGS) $(LOCAL_LDFLAGS)))
        LOCAL_EXPORT_LDLIBS += -Wl,--no-warn-mismatch
    endif
endif

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := orxp
LOCAL_SRC_FILES := $(TARGET_ARCH_ABI)/liborxp.a
LOCAL_STATIC_LIBRARIES := SOIL-prebuilt Box2D-prebuilt OpenAL-prebuilt Tremolo-prebuilt orxAndroidSupport

TARGET_PLATFORM = android-9

LOCAL_EXPORT_CFLAGS := -D__orxANDROID__ -D__orxPROFILER__
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_EXPORT_LDLIBS := -llog -lGLESv2 -landroid -lEGL

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
    LOCAL_EXPORT_CFLAGS += -mhard-float
    ifeq (,$(filter -fuse-ld=mcld,$(APP_LDFLAGS) $(LOCAL_LDFLAGS)))
        LOCAL_EXPORT_LDLIBS += -Wl,--no-warn-mismatch
    endif
endif

include $(PREBUILT_STATIC_LIBRARY)

$(call import-module,../extern/SOIL/lib/android)
$(call import-module,../extern/Box2D_2.1.3/lib/android)
$(call import-module,../extern/openal-soft/lib/android)
$(call import-module,../extern/Tremolo/lib/android)
$(call import-module,../extern/android/jni)

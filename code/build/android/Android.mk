LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orx
LOCAL_SRC_FILES :=   $(TARGET_ARCH_ABI)/liborx.a
LOCAL_EXPORT_LDLIBS := -L$(LOCAL_PATH)/../../../extern/SOIL/build/android/$(TARGET_ARCH_ABI) \
                       -L$(LOCAL_PATH)/../../../extern/Box2D_2.1.3/build/android/$(TARGET_ARCH_ABI) \
                       -L$(LOCAL_PATH)/../../../extern/NvEvent/obj/local/$(TARGET_ARCH_ABI)
LOCAL_EXPORT_LDLIBS += -llog -ldl -lGLESv2 -lstdc++ -lc -lm -llog -ldl -lGLESv2 -lnvmath -lnvglesutil \
                       -lnvhhdds -lnvlog -lnvshader -lnvfile -lnvapkfile -lnvevent -lnvtime -lEGL -lnvthread \
                       -lSOIL -lBox2D.2.1.3
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include $(LOCAL_PATH)/../../../extern/NvEvent/jni
LOCAL_EXPORT_CFLAGS := -D__orxANDROID__
LOCAL_EXPORT_CPPFLAGS := -D__orxANDROID__

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxd
LOCAL_SRC_FILES :=   $(TARGET_ARCH_ABI)/liborxd.a
LOCAL_EXPORT_LDLIBS := -L$(LOCAL_PATH)/../../../extern/SOIL/build/android/$(TARGET_ARCH_ABI) \
                       -L$(LOCAL_PATH)/../../../extern/Box2D_2.1.3/build/android/$(TARGET_ARCH_ABI) \
                       -L$(LOCAL_PATH)/../../../extern/NvEvent/obj/local/$(TARGET_ARCH_ABI)
LOCAL_EXPORT_LDLIBS += -llog -ldl -lGLESv2 -lstdc++ -lc -lm -llog -ldl -lGLESv2 -lnvmath -lnvglesutil \
                       -lnvhhdds -lnvlog -lnvshader -lnvfile -lnvapkfile -lnvevent -lnvtime -lEGL -lnvthread \
                       -lSOIL -lBox2D.2.1.3
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../include $(LOCAL_PATH)/../../../extern/NvEvent/jni
LOCAL_EXPORT_CFLAGS := -D__orxANDROID__ -D__orxDEBUG__
LOCAL_EXPORT_CPPFLAGS := -D__orxANDROID__ -D__orxDEBUG__

include $(PREBUILT_STATIC_LIBRARY)



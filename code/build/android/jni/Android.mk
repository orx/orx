
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ORX_NAME := orx

ifeq ($(ORX_DEBUG),true)
ORX_NAME := $(ORX_NAME)d
endif

ifeq ($(ORX_EM),true)
ORX_NAME := $(ORX_NAME)EM
endif


LOCAL_MODULE := $(ORX_NAME)

Box2D := $(LOCAL_PATH)/../../../../extern/Box2D_2.1.3
Dlmalloc := $(LOCAL_PATH)/../../../../extern/dlmalloc

LOCAL_CFLAGS := -I$(LOCAL_PATH)/../../../include \
-I$(Box2D)/include \
-I$(LOCAL_PATH)/include \
-D__orxANDROID__ \
-DNO_MALLINFO=1 \
-I$(Dlmalloc) 
#-Wall

LOCAL_CPPFLAGS := -I$(LOCAL_PATH)/../../../include \
-I$(Box2D)/include \
-I$(LOCAL_PATH)/include \
-I$(Dlmalloc) \
-D__orxANDROID__ \
-DNO_MALLINFO=1 \
-I$(Dlmalloc) 
#-Wall

ifeq ($(ORX_DEBUG),true)
LOCAL_CFLAGS += -D__orxDEBUG__
LOCAL_CPPFLAGS += -D__orxDEBUG__
endif

ifeq ($(ORX_EM),true)
LOCAL_CFLAGS += -D__ORX_ANDROID_EMULATOR__
LOCAL_CPPFLAGS += -D__ORX_ANDROID_EMULATOR__
endif

ORX_SRCS := ../../../src/main/orxParam.c \
../../../src/anim/*.c \
../../../src/base/*.c \
../../../src/core/*.c \
../../../src/debug/*.c \
../../../src/display/*.c \
../../../src/io/*.c \
../../../src/math/*.c \
../../../src/memory/*.c \
../../../src/object/*.c \
../../../src/physics/*.c \
../../../src/plugin/*.c \
../../../src/render/*.c \
../../../src/sound/*.c \
../../../src/utils/*.c \
../../../plugins/Display/android/orxDisplay.cpp \
../../../plugins/Display/android/android-support.cpp \
../../../plugins/Joystick/android/orxJoystick.c \
../../../plugins/Keyboard/Dummy/orxKeyboard.c \
../../../plugins/Physics/Box2D/orxPhysics.cpp \
../../../plugins/Render/Home/orxRender.c \
../../../plugins/Sound/android/orxSoundSystem.c \
../../../plugins/Mouse/android/orxMouse.c

LOCAL_CPP_EXTENSION := .cpp

# Note this "simple" makefile var substitution, you can find even more complex examples in different Android projects
LOCAL_SRC_FILES := $(foreach F, $(ORX_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))

ifneq ($(ORX_EM),true)
LOCAL_LDLIBS := -lGLESv2
endif

LOCAL_LDLIBS += -lGLESv1_CM -ldl -llog -lgcc -lm -lBox2D  -L$(Box2D)/lib/android

include $(BUILD_STATIC_LIBRARY)

# second lib, which will depend on and include the first one
#
#include $(CLEAR_VARS)
#
#LOCAL_MODULE    := libtwolib-second
#LOCAL_SRC_FILES := second.c

#LOCAL_STATIC_LIBRARIES := $(ORX_NAME)

#include $(BUILD_SHARED_LIBRARY)

#$(shell cp $(LOCAL_PATH)/../libs/armeabi/liborx.so  $(LOCAL_PATH)/../../../lib/android/)

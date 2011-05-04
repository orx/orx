
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ORX_NAME := orx

ifeq ($(ORX_DEBUG),true)
ORX_NAME := $(ORX_NAME)d
endif


LOCAL_MODULE := $(ORX_NAME)

Box2D := $(LOCAL_PATH)/../../../../extern/Box2D_2.1.3
SOIL := $(LOCAL_PATH)/../../../../extern/SOIL
Dlmalloc := $(LOCAL_PATH)/../../../../extern/dlmalloc

#LOCAL_CFLAGS := -I$(LOCAL_PATH)/../../../include \
#-I$(Box2D)/include \
#-I$(SOIL)/include \
#-I$(LOCAL_PATH)/include \
#-D__orxANDROID__ \
#-DNO_MALLINFO=1 \
#-I$(Dlmalloc) 

#LOCAL_CPPFLAGS := -D__orxANDROID__ \
#-DNO_MALLINFO=1 \
#-I$(LOCAL_PATH)/../../../include \
#-I$(Box2D)/include \
#-I$(SOIL)/include \
#-I$(LOCAL_PATH)/include \
#-I$(Dlmalloc) \

LOCAL_CPPFLAGS := -D__orxANDROID__ \
-DNO_MALLINFO=1

LOCAL_CFLAGS := -D__orxANDROID__ \
-DNO_MALLINFO=1

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../include
LOCAL_C_INCLUDES += $(Box2D)/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)/include
LOCAL_C_INCLUDES += $(SOIL)/include
LOCAL_C_INCLUDES += $(Dlmalloc)

ifeq ($(ORX_DEBUG),true)
LOCAL_CFLAGS += -D__orxDEBUG__
LOCAL_CPPFLAGS += -D__orxDEBUG__
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
../../../plugins/Joystick/android/orxJoystick.c \
../../../plugins/Keyboard/Dummy/orxKeyboard.c \
../../../plugins/Physics/Box2D/orxPhysics.cpp \
../../../plugins/Render/Home/orxRender.c \
../../../plugins/Sound/android/orxSoundSystem.c \
../../../plugins/Mouse/android/orxMouse.c \
../../../plugins/Display/android/orx_apk_file.c \
../../../plugins/Display/android/orxDisplay.cpp \
../../../plugins/Display/android/android-support.cpp \

LOCAL_CPP_EXTENSION := .cpp

# Note this "simple" makefile var substitution, you can find even more complex examples in different Android projects
LOCAL_SRC_FILES := $(foreach F, $(ORX_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))

LOCAL_LDLIBS += -ldl -llog -lgcc -lm -lBox2D -lSOIL -L$(SOIL)/lib/android -L$(Box2D)/lib/android

include $(BUILD_STATIC_LIBRARY)

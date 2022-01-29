LOCAL_PATH := $(call my-dir)/../../../src

ORX_SRC_FILES := \
  base/orxType.c            \
  base/orxModule.c          \
  io/orxFile.c              \
  io/orxInput.c             \
  io/orxKeyboard.c          \
  io/orxJoystick.c          \
  io/orxMouse.c             \
  utils/orxHashTable.c      \
  utils/orxTree.c           \
  utils/orxLinkList.c       \
  utils/orxString.c         \
  anim/orxAnimPointer.c     \
  anim/orxAnimSet.c         \
  anim/orxAnim.c            \
  display/orxScreenshot.c   \
  display/orxGraphic.c      \
  display/orxDisplay.c      \
  display/orxFont.c         \
  display/orxText.c         \
  display/orxTexture.c      \
  render/orxCamera.c        \
  render/orxViewport.c      \
  render/orxShaderPointer.c \
  render/orxRender.c        \
  render/orxShader.c        \
  math/orxMath.c            \
  math/orxVector.c          \
  core/orxCommand.c         \
  core/orxConfig.c          \
  core/orxConsole.c         \
  core/orxEvent.c           \
  core/orxResource.c        \
  core/orxSystem.c          \
  core/orxThread.c          \
  core/orxClock.c           \
  core/orxLocale.c          \
  main/orxParam.c           \
  physics/orxPhysics.c      \
  physics/orxBody.c         \
  object/orxStructure.c     \
  object/orxTimeLine.c      \
  object/orxFrame.c         \
  object/orxFX.c            \
  object/orxFXPointer.c     \
  object/orxObject.c        \
  object/orxSpawner.c       \
  plugin/orxPlugin.c        \
  sound/orxSoundPointer.c   \
  sound/orxSoundSystem.c    \
  sound/orxSound.c          \
  memory/orxBank.c          \
  memory/orxMemory.c        \
  debug/orxFPS.c            \
  debug/orxDebug.c          \
  debug/orxProfiler.c       \
  plugin/orxPlugin_EmbeddedList.cpp \
  main/orxAndroidSupport.cpp

ORX_INCLUDES := $(LOCAL_PATH)/../include \
                    $(LOCAL_PATH)/../../extern/rpmalloc/rpmalloc \
                    $(LOCAL_PATH)/../../extern/xxHash \
                    $(LOCAL_PATH)/../../extern/miniaudio \
                    $(LOCAL_PATH)/../../extern/stb_image \
                    $(LOCAL_PATH)/../../extern/qoi \
                    $(LOCAL_PATH)/../../extern/stb_vorbis \
                    $(LOCAL_PATH)/../../extern/LiquidFun-1.1.0/include \
                    $(LOCAL_PATH)/../../extern/libwebp/include

ORX_CFLAGS := -DNO_MALLINFO=1 -DTARGET_OS_ANDROID

include $(CLEAR_VARS)

LOCAL_MODULE = orx
LOCAL_SRC_FILES = $(ORX_SRC_FILES)
LOCAL_C_INCLUDES := $(ORX_INCLUDES)
LOCAL_CFLAGS := $(ORX_CFLAGS)
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE = orxp
LOCAL_SRC_FILES = $(ORX_SRC_FILES)
LOCAL_C_INCLUDES := $(ORX_INCLUDES)
LOCAL_CFLAGS := $(ORX_CFLAGS) -D__orxPROFILER__
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE = orxd
LOCAL_SRC_FILES = $(ORX_SRC_FILES)
LOCAL_C_INCLUDES := $(ORX_INCLUDES)
LOCAL_CFLAGS := $(ORX_CFLAGS) -D__orxDEBUG__ -O0
LOCAL_ARM_MODE := arm

include $(BUILD_STATIC_LIBRARY)

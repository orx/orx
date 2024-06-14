ifeq ($(strip $(ORX)),)
  $(error ORX environment variable not set)
endif

LOCAL_PATH := $(ORX)/src

ORX_EXTERN := $(ORX)/../extern

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
  main/android/orxAndroidSupport.cpp \
  plugin/orxPlugin_EmbeddedList.cpp

ORX_INCLUDES := \
  $(ORX)/include \
  $(ORX)/include/main/android \
  $(ORX_EXTERN)/rpmalloc/rpmalloc \
  $(ORX_EXTERN)/xxHash \
  $(ORX_EXTERN)/miniaudio \
  $(ORX_EXTERN)/stb_image \
  $(ORX_EXTERN)/qoi \
  $(ORX_EXTERN)/stb_vorbis \
  $(ORX_EXTERN)/LiquidFun-1.1.0/include \
  $(ORX_EXTERN)/libwebp/include

include $(CLEAR_VARS)
LOCAL_MODULE := orx
LOCAL_SRC_FILES := $(ORX_SRC_FILES)
LOCAL_C_INCLUDES := $(ORX_INCLUDES)
LOCAL_CFLAGS := -DNO_MALLINFO=1 -DTARGET_OS_ANDROID -Wno-atomic-alignment
LOCAL_STATIC_LIBRARIES := webpdecoder_static liquidfun_static game-activity_static paddleboat_static swappy_static

LOCAL_ARM_MODE := arm
include $(BUILD_STATIC_LIBRARY)

$(call import-add-path,$(ORX_EXTERN))

$(call import-module,LiquidFun-1.1.0/src/liquidfun/Box2D/jni)
$(call import-module,libwebp)
$(call import-module,prefab/game-activity)
$(call import-module,prefab/games-controller)
$(call import-module,prefab/games-frame-pacing)

LOCAL_PATH := $(call my-dir)/../../../src

#  ======== orx ==========

include $(CLEAR_VARS)

LOCAL_MODULE = orx
LOCAL_SRC_FILES = \
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
	core/orxConfig.c          \
	core/orxEvent.c           \
	core/orxSystem.c          \
	core/orxClock.c           \
	core/orxLocale.c          \
	main/orxParam.c           \
	main/orxAndroidNativeSupport.c  \
	physics/orxPhysics.c      \
	physics/orxBody.c         \
	object/orxStructure.c     \
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
        plugin/orxPlugin_EmbeddedList.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/../../extern/dlmalloc $(LOCAL_PATH)/../../extern/SOIL/include $(LOCAL_PATH)/../../extern/Box2D_2.1.3/include $(LOCAL_PATH)/../../extern/stb_vorbis

LOCAL_CFLAGS := -DNO_MALLINFO=1 -D__orxANDROID_NATIVE__

LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)


#  ======== orxd =========

include $(CLEAR_VARS)

LOCAL_MODULE = orxd
LOCAL_SRC_FILES = \
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
	core/orxConfig.c          \
	core/orxEvent.c           \
	core/orxSystem.c          \
	core/orxClock.c           \
	core/orxLocale.c          \
	main/orxParam.c           \
	main/orxAndroidNativeSupport.c  \
	physics/orxPhysics.c      \
	physics/orxBody.c         \
	object/orxStructure.c     \
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
        plugin/orxPlugin_EmbeddedList.cpp

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/../../extern/dlmalloc $(LOCAL_PATH)/../../extern/SOIL/include $(LOCAL_PATH)/../../extern/Box2D_2.1.3/include $(LOCAL_PATH)/../../extern/stb_vorbis

LOCAL_CFLAGS := -DNO_MALLINFO=1 -D__orxANDROID_NATIVE__ -D__orxDEBUG__

LOCAL_ARM_MODE := arm
TARGET_PLATFORM = android-9

include $(BUILD_STATIC_LIBRARY)


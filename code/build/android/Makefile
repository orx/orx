VPATH = ../../src

C_SRC = \
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
	core/orxEvent.c           \
	core/orxSystem.c          \
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
	debug/orxProfiler.c

CPP_SRC = \
  plugin/orxPlugin_EmbeddedList.cpp \
	main/orxAndroidSupport.cpp


armeabi_debug_OBJ = $(addprefix armeabi/debug/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi/debug/,$(C_SRC:.c=.o))
armeabi-v7a_debug_OBJ = $(addprefix armeabi-v7a/debug/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi-v7a/debug/,$(C_SRC:.c=.o))

armeabi_profile_OBJ = $(addprefix armeabi/profile/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi/profile/,$(C_SRC:.c=.o))
armeabi-v7a_profile_OBJ = $(addprefix armeabi-v7a/profile/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi-v7a/profile/,$(C_SRC:.c=.o))

armeabi_OBJ = $(addprefix armeabi/release/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi/release/,$(C_SRC:.c=.o))
armeabi-v7a_OBJ = $(addprefix armeabi-v7a/release/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi-v7a/release/,$(C_SRC:.c=.o))

armeabi_debug_OUT = armeabi/liborxd.a
armeabi-v7a_debug_OUT = armeabi-v7a/liborxd.a

armeabi_profile_OUT = armeabi/liborxp.a
armeabi-v7a_profile_OUT = armeabi-v7a/liborxp.a

armeabi_OUT = armeabi/liborx.a
armeabi-v7a_OUT = armeabi-v7a/liborx.a

INCLUDES = -I../../include -I../../../extern/SOIL/include -I../../../extern/Box2D_2.1.3/include \
           -I../../../extern/dlmalloc -I../../../extern/NvEvent/jni -I../../../extern/NvEvent/jni/include

CXX = arm-linux-androideabi-g++
CC = arm-linux-androideabi-gcc
AR = arm-linux-androideabi-ar
CPPFLAGS = -fno-exceptions -fno-rtti -DNO_MALLINFO=1 -D__orxANDROID__ -DANDROID -D__orxBANK_ALIGN__
CFLAGS = -DNO_MALLINFO=1 -D__orxANDROID__ -DANDROID -D__orxBANK_ALIGN__
ARMV7A_FLAGS_RELEASE = -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -O1
ARMV5TE_FLAGS_RELEASE = -O1
ARMV7A_FLAGS_DEBUG = -march=armv7-a -mfloat-abi=softfp -mfpu=vfpv3-d16 -O0 -g
ARMV5TE_FLAGS_DEBUG = -O0 -g

all: $(armeabi_debug_OUT) $(armeabi-v7a_debug_OUT) $(armeabi_profile_OUT) $(armeabi-v7a_profile_OUT) $(armeabi_OUT) $(armeabi-v7a_OUT)

armeabi/debug/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(ARMV5TE_FLAGS_DEBUG) -D__orxDEBUG__ -c $< -o $@

armeabi/debug/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) $(ARMV5TE_FLAGS_DEBUG) -D__orxDEBUG__ -c $< -o $@

armeabi-v7a/debug/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(ARMV7A_FLAGS_DEBUG) -D__orxDEBUG__ -c $< -o $@

armeabi-v7a/debug/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) $(ARMV7A_FLAGS_DEBUG) -D__orxDEBUG__ -c $< -o $@

armeabi/profile/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(ARMV5TE_FLAGS_PROFILE) -D__orxPROFILER__ -c $< -o $@

armeabi/profile/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) $(ARMV5TE_FLAGS_PROFILE) -D__orxPROFILER__ -c $< -o $@

armeabi-v7a/profile/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(ARMV7A_FLAGS_PROFILE) -D__orxPROFILER__ -c $< -o $@

armeabi-v7a/profile/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) $(ARMV7A_FLAGS_PROFILE) -D__orxPROFILER__ -c $< -o $@

armeabi/release/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(ARMV5TE_FLAGS_RELEASE) -c $< -o $@

armeabi/release/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) $(ARMV5TE_FLAGS_RELEASE) -c $< -o $@

armeabi-v7a/release/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) $(ARMV7A_FLAGS_RELEASE) -c $< -o $@

armeabi-v7a/release/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) $(ARMV7A_FLAGS_RELEASE) -c $< -o $@

$(armeabi_OUT): $(armeabi_OBJ)
	$(AR) rcs $(armeabi_OUT) $(armeabi_OBJ)

$(armeabi-v7a_OUT): $(armeabi-v7a_OBJ)
	$(AR) rcs $(armeabi-v7a_OUT) $(armeabi-v7a_OBJ)

$(armeabi_profile_OUT): $(armeabi_profile_OBJ)
	$(AR) rcs $(armeabi_profile_OUT) $(armeabi_profile_OBJ)

$(armeabi-v7a_profile_OUT): $(armeabi-v7a_profile_OBJ)
	$(AR) rcs $(armeabi-v7a_profile_OUT) $(armeabi-v7a_profile_OBJ)

$(armeabi_debug_OUT): $(armeabi_debug_OBJ)
	$(AR) rcs $(armeabi_debug_OUT) $(armeabi_debug_OBJ)

$(armeabi-v7a_debug_OUT): $(armeabi-v7a_debug_OBJ)
	$(AR) rcs $(armeabi-v7a_debug_OUT) $(armeabi-v7a_debug_OBJ)

clean:
	rm -f $(armeabi_debug_OBJ) $(armeabi-v7a_debug_OBJ) $(armeabi_profile_OBJ) $(armeabi-v7a_profile_OBJ) $(armeabi_OBJ) $(armeabi-v7a_OBJ) $(armeabi_debug_OUT) $(armeabi-v7a_debug_OUT) $(armeabi_profile_OUT) $(armeabi-v7a_profile_OUT) $(armeabi_OUT) $(armeabi-v7a_OUT)

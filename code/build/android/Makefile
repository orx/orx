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
	render/orxFXPointer.c     \
	render/orxRender.c        \
	render/orxFX.c            \
	render/orxShader.c        \
	math/orxMath.c            \
	math/orxVector.c          \
	core/orxConfig.c          \
	core/orxEvent.c           \
	core/orxSystem.c          \
	core/orxClock.c           \
	core/orxLocale.c          \
	main/orxParam.c           \
	physics/orxPhysics.c      \
	physics/orxBody.c         \
	object/orxStructure.c     \
	object/orxFrame.c         \
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
	main/orxAndSupport.c

CPP_SRC = \
  plugin/orxPlugin_EmbeddedList.cpp
	
armeabi_debug_OBJ = $(addprefix armeabi/debug/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi/debug/,$(C_SRC:.c=.o))
armeabi-v7a_debug_OBJ = $(addprefix armeabi-v7a/debug/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi-v7a/debug/,$(C_SRC:.c=.o))

armeabi_OBJ = $(addprefix armeabi/release/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi/release/,$(C_SRC:.c=.o))
armeabi-v7a_OBJ = $(addprefix armeabi-v7a/release/,$(CPP_SRC:.cpp=.o)) $(addprefix armeabi-v7a/release/,$(C_SRC:.c=.o))
	
armeabi_debug_OUT = armeabi/liborxd.a
armeabi-v7a_debug_OUT = armeabi-v7a/liborxd.a

armeabi_OUT = armeabi/liborx.a
armeabi-v7a_OUT = armeabi-v7a/liborx.a

INCLUDES = -I../../include -I../../../extern/SOIL/include -I../../../extern/Box2D_2.1.3/include -I../../../extern/dlmalloc -I./apkfile
	
CXX = arm-linux-androideabi-g++
CC = arm-linux-androideabi-gcc
AR = arm-linux-androideabi-ar
CPPFLAGS = -fno-exceptions -fno-rtti -DNO_MALLINFO=1 -D__orxANDROID__
CFLAGS = -DNO_MALLINFO=1 -D__orxANDROID__
	
all: $(armeabi_debug_OUT) $(armeabi-v7a_debug_OUT) $(armeabi_OUT) $(armeabi-v7a_OUT)

armeabi/debug/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) -g -O0 -D__orxDEBUG__ -c $< -o $@

armeabi/debug/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -g -O0 -D__orxDEBUG__ -c $< -o $@

armeabi-v7a/debug/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) -g -O0 -D__orxDEBUG__ -march=armv7-a -mfloat-abi=softfp -c $< -o $@

armeabi-v7a/debug/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -g -O0 -D__orxDEBUG__ -march=armv7-a -mfloat-abi=softfp -c $< -o $@

armeabi/release/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) -O2 -c $< -o $@

armeabi/release/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -O2 -c $< -o $@

armeabi-v7a/release/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(INCLUDES) $(CPPFLAGS) -march=armv7-a -mfloat-abi=softfp -O2 -c $< -o $@

armeabi-v7a/release/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(INCLUDES) $(CFLAGS) -march=armv7-a -mfloat-abi=softfp -O2 -c $< -o $@

$(armeabi_OUT): $(armeabi_OBJ)
	$(AR) rcs $(armeabi_OUT) $(armeabi_OBJ)

$(armeabi-v7a_OUT): $(armeabi-v7a_OBJ)
	$(AR) rcs $(armeabi-v7a_OUT) $(armeabi-v7a_OBJ)

$(armeabi_debug_OUT): $(armeabi_debug_OBJ)
	$(AR) rcs $(armeabi_debug_OUT) $(armeabi_debug_OBJ)

$(armeabi-v7a_debug_OUT): $(armeabi-v7a_debug_OBJ)
	$(AR) rcs $(armeabi-v7a_debug_OUT) $(armeabi-v7a_debug_OBJ)

clean:
	rm -f $(armeabi_debug_OBJ) $(armeabi-v7a_debug_OBJ) $(armeabi_OBJ) $(armeabi-v7a_OBJ) $(armeabi_debug_OUT) $(armeabi-v7a_debug_OUT) $(armeabi_OUT) $(armeabi-v7a_OUT)

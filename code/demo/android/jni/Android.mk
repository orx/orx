#----------assign seome params--------------------
#if set it to true if will compile in the debug mode
#ORX_DEBUG := false

# two function in this macro
# first, set the app name, the main feature is to set the default .ini filename
# using the last "/" token
# second, set the res folder contain .ini, all of images and other stuff. 
# name of folder is the remaining "/" token 
# for exameple: orx_demo/orxDemoApp. the folder will be /sdcard/orx_demo (add /sdcard as prefix)
# while, the app name is orxDemoApp
#the name of the lib
ORX_MODULE_NAME := orxApp
#set all of sources
ORXAPP_SRCS := ../jni/*.c \
			   ../jni/*.cpp
			   
ORXLIB     := ../../../lib/android
SOILLIB    := ../../../../extern/SOIL/lib/android
ORXINCLUDE := ../../../include
GRAPGICJNI := ../../../../extern/jni/lib/android
BOX2DLIB     := ../../../../extern/Box2D_2.1.3/lib/android
BOX2DORXINCLUDE := ../../../../extern/Box2D_2.1.3/include
			   
			  
#--------- end of params set----------
			   
# The namespace in Java file, with dots replaced with underscores
ORX_JAVA_PACKAGE_PATH := org_orxproject_lib_AnOrxActivity
ORX_JAVA_ACTITY_PATH := org/orxproject/lib/AnOrxActivity
			 
ORX_LIB_NAME := orx
BOX2D_LIB_NAME := Box2D
SOIL_LIB_NAME := SOIL

ifeq ($(ORX_DEBUG),true)
ORX_MODULE_NAME := $(ORX_MODULE_NAME)d
ORX_LIB_NAME := $(ORX_LIB_NAME)d
endif

ifeq ($(ORX_EM),true)
ORX_LIB_NAME := $(ORX_LIB_NAME)EM
endif

LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE := $(ORX_LIB_NAME)
LOCAL_SRC_FILES := $(ORXLIB)/lib$(ORX_LIB_NAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := jnigraphics
LOCAL_SRC_FILES := $(GRAPGICJNI)/libjnigraphics.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := $(BOX2D_LIB_NAME)
LOCAL_SRC_FILES := $(BOX2DLIB)/lib$(BOX2D_LIB_NAME).a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := $(ORX_MODULE_NAME)

LOCAL_CFLAGS := -DANDROID \
				-D__orxANDROID__ \
                -I$(LOCAL_PATH)/$(ORXINCLUDE) \
                -I$(LOCAL_PATH)/$(BOX2DORXINCLUDE)
                
LOCAL_CPPFLAGS := -DANDROID \
				-D__orxANDROID__ \
                 -I$(LOCAL_PATH)/$(ORXINCLUDE) \
                -I$(LOCAL_PATH)/$(BOX2DORXINCLUDE)
                
		
ifeq ($(ORX_DEBUG),true)
LOCAL_CFLAGS += -D__orxDEBUG__
LOCAL_CPPFLAGS += -D__orxDEBUG__
endif

# Note this "simple" makefile var substitution, you can find even more complex examples in different Android projects
LOCAL_SRC_FILES := $(foreach F, $(ORXAPP_SRCS), $(addprefix $(dir $(F)),$(notdir $(wildcard $(LOCAL_PATH)/$(F)))))

ifneq ($(ORX_EM),true)
LOCAL_LDLIBS := -lGLESv2
endif

LOCAL_LDLIBS += -lGLESv1_CM -ldl -llog -lm -ljnigraphics -l$(ORX_LIB_NAME) -l$(SOIL_LIB_NAME) -L$(LOCAL_PATH)/$(SOILLIB) -L$(LOCAL_PATH)/$(ORXLIB)  -l$(BOX2D_LIB_NAME) -L$(LOCAL_PATH)/$(BOX2DLIB) -lgcc

include $(BUILD_SHARED_LIBRARY)

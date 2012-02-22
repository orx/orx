LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest
### Add all source file names to be included in lib separated by a whitespace
LOCAL_SRC_FILES := orxTest.c
LOCAL_STATIC_LIBRARIES := android_native_app_glue orxd-prebuilt Box2D-prebuilt SOIL-prebuilt 

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
$(call import-module,orx/code/lib/static/android-native)
$(call import-module,orx/extern/SOIL/lib/android-native)
$(call import-module,orx/extern/Box2D_2.1.3/lib/android-native)

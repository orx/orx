LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := orxTest

LOCAL_SRC_FILES := orxTest.c
LOCAL_STATIC_LIBRARIES := orxd NvEvent
LOCAL_SHARED_LIBRARIES := Box2D SOIL

LOCAL_ARM_MODE := arm

include $(BUILD_SHARED_LIBRARY)

$(call import-module,orx/code/build/android)
$(call import-module,orx/extern/Box2D_2.1.3/build/android)
$(call import-module,orx/extern/SOIL/build/android)
$(call import-module,orx/extern/NvEvent/build)
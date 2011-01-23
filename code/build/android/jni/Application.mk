#if set it to true if will compile in the debug mode

# The ARMv7 is significanly faster due to the use of the hardware FPU
APP_ABI := armeabi armeabi-v7a
APP_PLATFORM := android-9
ifeq ($(ORX_DEBUG),true)
APP_OPTIM    := debug 
endif

#the default is release, debug: debug level will be set.
# no optim code gen




APP_ABI := armeabi-v7a x86 arm64-v8a x86_64
APP_MODULES = orxTest
APP_PLATFORM = android-17

# For single-lib apps, linking STL statically reduces the APK size significantly.
APP_STL:=c++_static

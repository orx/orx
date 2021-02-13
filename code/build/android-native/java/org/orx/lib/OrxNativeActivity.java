package org.orx.lib;

import android.annotation.TargetApi;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.app.NativeActivity;
import android.content.Context;
import android.hardware.input.InputManager;
import android.view.InputDevice;

/**
    Base class for the apps NativeActivity adapted to orxAndroidNativeSupport.cpp in orxlib.

    NativeActivity is used with the android_native_app_glue.
 */
public class OrxNativeActivity extends NativeActivity implements InputManager.InputDeviceListener {

    public static int orxANDROID_KU32_MAX_JOYSTICK_NUMBER = 16; // same as in Orx /code/include/io/orxJoystick.h

    // C functions we call
    public native void nativeOnInputDeviceAdded(int deviceId);
    public native void nativeOnInputDeviceChanged(int deviceId);
    public native void nativeOnInputDeviceRemoved(int deviceId);

    private InputManager mInputManager;

    /*
      NOTE: Do NOT to call any native methods before App is created (before this callback is done).
      The native app structure is initialized in new thread when super.onCreate() is being executed.
      So best is to wait until onStart() below has been called.
    */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mInputManager = (InputManager)this.getSystemService(Context.INPUT_SERVICE);
    }

    @Override
    protected void onStart() {
      Log.d("OrxNativeActivity", "onStart()");
    	super.onStart();

      mInputManager.registerInputDeviceListener(this, null);
    }

    @Override
    protected void onStop() {
      Log.d("OrxNativeActivity", "onStop()");
      mInputManager.unregisterInputDeviceListener(this);
      super.onStop();
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        Log.d("OrxNativeActivity", "onInputDeviceAdded() deviceId: "+deviceId);
        if (isGameController(deviceId)) {
            nativeOnInputDeviceAdded(deviceId);
        }
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
        Log.d("OrxNativeActivity", "onInputDeviceChanged() deviceId: "+deviceId);
        if (isGameController(deviceId)) {
            nativeOnInputDeviceChanged(deviceId);
        }
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        Log.d("OrxActivity", "onInputDeviceRemoved() deviceId: "+deviceId);
        // cannot use isGameController(deviceId) for removed device beacuse getInputDevice(deviceId) return null.
        // But it is not a problem to send this deviceId to Orx even if it's not a game controller, just a debug-print "unknown device" in OrxJoystick
        nativeOnInputDeviceRemoved(deviceId);
    }

    private boolean isGameController(int deviceId) {
      InputDevice dev = mInputManager.getInputDevice(deviceId);
      if (dev == null) {
        Log.e("OrxNativeActivity", "NULL device with deviceId: "+deviceId);
        return false;
      }
      int sources = dev.getSources();
      // if the device is a gamepad/joystick
      return ((((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
            ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)));
    }

    // Called from android/OrxJoystick via orxAndroidNativeSupport to get initial deviceIds for Orx Joysticks if UseJoystick = true
    @SuppressWarnings("UnusedDeclaration")
    public int[] getDeviceIds() {
        int deviceIds[] = mInputManager.getInputDeviceIds();
        int result[] = new int[orxANDROID_KU32_MAX_JOYSTICK_NUMBER];
        int i = 0;

        for (int deviceId : deviceIds) {
            InputDevice dev = mInputManager.getInputDevice(deviceId);
            int sources = dev.getSources();
            if (isGameController(deviceId) && i < orxANDROID_KU32_MAX_JOYSTICK_NUMBER) {
                result[i++] = deviceId;
                Log.i("OrxNativeActivity", "Game controller '"+dev+"' connected, sources:"+sources+", deviceID: "+deviceId);
            }
        }
        Log.i("OrxNativeActivity", "Nr of Game controllers connected: "+i);

        return result;
    }
}


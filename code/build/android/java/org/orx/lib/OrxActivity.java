package org.orx.lib;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.content.Context;
import android.hardware.input.InputManager;
import android.os.Bundle;
import android.util.Log;
import android.view.InputDevice;
import android.view.KeyCharacterMap;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;

import androidx.fragment.app.FragmentActivity;

import java.util.concurrent.atomic.AtomicBoolean;

/**
 * Orx Activity
 * Base Activity class for use in Orx "android" apps (not "android-native").
 *
 * NOTE: Must be in sync with native methods in orxAndroidSupport.cpp
*/
public class OrxActivity extends FragmentActivity implements SurfaceHolder.Callback,
    View.OnKeyListener, View.OnTouchListener, InputManager.InputDeviceListener {

  public static int orxANDROID_KU32_MAX_JOYSTICK_NUMBER = 16; // same as in Orx /code/include/io/orxJoystick.h

    private SurfaceHolder mCurSurfaceHolder;
    private SurfaceView mSurface;
    private InputManager mInputManager;

    private AtomicBoolean mRunning = new AtomicBoolean(false);
    private Thread mOrxThread;
    private boolean mDestroyed;

    @Override
    protected void onCreate(Bundle arg0) {
          super.onCreate(arg0);

        nativeOnCreate();
        mInputManager = (InputManager)this.getSystemService(Context.INPUT_SERVICE);
        mOrxThread = new Thread("OrxThread") {
            @Override
            public void run() {
                startOrx(OrxActivity.this);
                mRunning.set(false);
            }
        };
    }

    @Override
    @SuppressLint("ClickableViewAccessibility")
    protected void onStart() {
        super.onStart();

        mInputManager.registerInputDeviceListener(this, null);

        if(mSurface == null) {
            int surfaceId = getResources().getIdentifier("id/orxSurfaceView", null, getPackageName());

            if(surfaceId != 0) {
                mSurface = (SurfaceView) findViewById(surfaceId);

                if(mSurface == null) {
                    Log.d("OrxActivity", "SurfaceView with identifier orxSurfaceView not found in layout.");
                    mSurface = new SurfaceView(getApplication());
                    setContentView(mSurface);
                }
            } else {
                Log.d("OrxActivity", "No identifier orxSurfaceView found.");
                mSurface = new SurfaceView(getApplication());
                setContentView(mSurface);
            }

            mSurface.getHolder().addCallback(this);
            mSurface.setFocusable(true);
            mSurface.setFocusableInTouchMode(true);
            mSurface.setOnKeyListener(this);
            mSurface.setOnTouchListener(this);
            mSurface.setOnGenericMotionListener(new OrxOnGenericMotionListener(this));
          }

        if(!mRunning.getAndSet(true)) {
            mOrxThread.start();
        }
    }

    @Override
    protected void onStop() {
        mInputManager.unregisterInputDeviceListener(this);
        super.onStop();
    }

    @Override
    protected void onPause() {
        super.onPause();

        if(mRunning.get()) {
            nativeOnPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        if(mRunning.get()) {
            nativeOnResume();
        }
    }

    @Override
    protected void onDestroy() {
        mDestroyed = true;

        if(mCurSurfaceHolder != null) {
            nativeOnSurfaceDestroyed();
            mCurSurfaceHolder = null;
        }

        if(mRunning.get()) {
            stopOrx();
        }

        super.onDestroy();
    }

    // Called when we have a valid drawing surface
    @SuppressLint("NewApi")
    public void surfaceCreated(SurfaceHolder holder) {
        if(!mDestroyed) {
            mCurSurfaceHolder = holder;
            nativeOnSurfaceCreated(holder.getSurface());
        }
    }

    // Called when we lose the surface
    public void surfaceDestroyed(SurfaceHolder holder) {
        mCurSurfaceHolder = null;
        if(!mDestroyed) {
            nativeOnSurfaceDestroyed();
        }
    }

    // Called when the surface is resized
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if(!mDestroyed) {
            nativeOnSurfaceChanged(width, height);
        }
    }

    // Key events
    public boolean onKey(View v, int keyCode, KeyEvent event) {
        int source = event.getSource();

        if(keyCode != KeyEvent.KEYCODE_BACK && // BACK is a keyboard event
                ((source & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK ||
                 (source & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD)) {

            if(event.getRepeatCount() == 0) {
                int deviceId = event.getDeviceId();

                switch (event.getAction()) {
                    case KeyEvent.ACTION_DOWN:
                        nativeOnJoystickDown(deviceId, keyCode);
                        break;

                    case KeyEvent.ACTION_UP:
                        nativeOnJoystickUp(deviceId, keyCode);
                        break;
                }

                if (keyCode != KeyEvent.KEYCODE_VOLUME_UP
                        && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN)
                    return true;
            }

            return false;
        }

        if((source & InputDevice.SOURCE_KEYBOARD) == InputDevice.SOURCE_KEYBOARD ||
                (source & InputDevice.SOURCE_DPAD) == InputDevice.SOURCE_DPAD) {
            switch (event.getAction()) {
                case KeyEvent.ACTION_DOWN:
                    nativeOnKeyDown(keyCode, event.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK);
                    break;

                case KeyEvent.ACTION_UP:
                    nativeOnKeyUp(keyCode);
                    break;

                case KeyEvent.ACTION_MULTIPLE:
                    if(keyCode == KeyEvent.KEYCODE_UNKNOWN) {
                        final KeyCharacterMap m = KeyCharacterMap.load(event.getDeviceId());
                        final KeyEvent[] es = m.getEvents(event.getCharacters().toCharArray());

                        if (es != null) {
                            for (KeyEvent s : es) {
                                switch(s.getAction()) {
                                    case KeyEvent.ACTION_DOWN:
                                        nativeOnKeyDown(s.getKeyCode(), event.getUnicodeChar() & KeyCharacterMap.COMBINING_ACCENT_MASK);
                                        break;
                                    case KeyEvent.ACTION_UP:
                                        nativeOnKeyUp(s.getKeyCode());
                                        break;
                                }
                            }
                        }

                        return true;
                    }
            }

            if (keyCode != KeyEvent.KEYCODE_VOLUME_UP
                    && keyCode != KeyEvent.KEYCODE_VOLUME_DOWN)
                return true;

            return false;
        }

        return false;
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        mSurface.requestFocus();
        nativeOnFocusChanged(hasFocus);
    }

    // Touch events
    @SuppressLint("ClickableViewAccessibility")
    public boolean onTouch(View v, MotionEvent event) {
        final int touchDevId = event.getDeviceId();
        final int pointerCount = event.getPointerCount();
        // touchId, pointerId, action, x, y, pressure
        int actionPointerIndex = event.getActionIndex();
        int pointerFingerId = event.getPointerId(actionPointerIndex);
        int action = event.getActionMasked();

        int x = (int) event.getX(actionPointerIndex);
        int y = (int) event.getY(actionPointerIndex);
        int p = (int) event.getPressure(actionPointerIndex);

        if (action == MotionEvent.ACTION_MOVE && pointerCount > 1) {
            for (int i = 0; i < pointerCount; i++) {
                pointerFingerId = event.getPointerId(i);
                x = (int) event.getX(i);
                y = (int) event.getY(i);
                p = (int) event.getPressure(i);
                nativeOnTouch(touchDevId, pointerFingerId, action, x, y, p);
            }
        } else if(action == MotionEvent.ACTION_CANCEL) {
            for (int i = 0; i < pointerCount; i++) {
                pointerFingerId = event.getPointerId(i);
                x = (int) event.getX(i);
                y = (int) event.getY(i);
                p = (int) event.getPressure(i);
                nativeOnTouch(touchDevId, pointerFingerId, MotionEvent.ACTION_UP, x, y, p);
            }
        } else {
            nativeOnTouch(touchDevId, pointerFingerId, action, x, y, p);
        }
        return true;
    }

    @Override
    public void onInputDeviceAdded(int deviceId) {
        nativeOnInputDeviceAdded(deviceId);
    }

    @Override
    public void onInputDeviceChanged(int deviceId) {
        nativeOnInputDeviceChanged(deviceId);
    }

    @Override
    public void onInputDeviceRemoved(int deviceId) {
        nativeOnInputDeviceRemoved(deviceId);
    }

    // C functions we call

    private native void startOrx(Activity activity);
    private native void nativeOnPause();
    private native void nativeOnResume();
    private native void stopOrx();

    native void nativeOnCreate();
    native void nativeOnSurfaceCreated(Surface surface);
    native void nativeOnSurfaceDestroyed();
    native void nativeOnSurfaceChanged(int width, int height);
    native void nativeOnKeyDown(int keycode, int unicode);
    native void nativeOnKeyUp(int keycode);
    native void nativeOnTouch(int touchDevId, int pointerFingerId,
                                            int action, int x,
                                            int y, int p);
    native void nativeOnFocusChanged(boolean hasFocus);
    native void nativeOnInputDeviceAdded(int deviceId);
    native void nativeOnInputDeviceChanged(int deviceId);
    native void nativeOnInputDeviceRemoved(int deviceId);
    native void nativeOnJoystickMove(int deviceId, float[] axis);
    native void nativeOnJoystickDown(int deviceId, int keycode);
    native void nativeOnJoystickUp(int deviceId, int keycode);

    // Java functions called from C

    @SuppressWarnings("UnusedDeclaration")
    public int getRotation() {
        WindowManager windowMgr = (WindowManager) getSystemService(WINDOW_SERVICE);
        return windowMgr.getDefaultDisplay().getRotation();
    }

    @SuppressWarnings("UnusedDeclaration")
    public void showKeyboard(final boolean show) {
        runOnUiThread(new Runnable() {

            @Override
            public void run() {
                InputMethodManager imm = (InputMethodManager) getSystemService(INPUT_METHOD_SERVICE);

                if (show) {
                    imm.showSoftInput(mSurface, InputMethodManager.SHOW_IMPLICIT);
                } else {
                    imm.hideSoftInputFromWindow(mSurface.getWindowToken(), InputMethodManager.HIDE_IMPLICIT_ONLY);
                }
            }
        });
    }

    @SuppressWarnings("UnusedDeclaration")
    public int[] getDeviceIds() {
        int deviceIds[] = mInputManager.getInputDeviceIds();
        int result[] = new int[orxANDROID_KU32_MAX_JOYSTICK_NUMBER];
        int i = 0;

        for (int deviceId : deviceIds) {
            InputDevice dev = mInputManager.getInputDevice(deviceId);
            int sources = dev.getSources();
            // if the device is a gamepad/joystick
            if ((((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                    ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) &&
                    i < orxANDROID_KU32_MAX_JOYSTICK_NUMBER ) {
                result[i++] = deviceId;
            }
        }

        return result;
    }
}


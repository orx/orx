package org.orx.lib;

import android.annotation.TargetApi;
import android.os.Build;
import android.view.InputDevice;
import android.view.MotionEvent;
import android.view.View;

/**
 * Created by philippe on 29/10/14.
 */
@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
public class OrxOnGenericMotionListener implements View.OnGenericMotionListener {

    private OrxActivity mOrxActivity;

    public OrxOnGenericMotionListener(OrxActivity activity) {
        mOrxActivity = activity;
    }

    @Override
    public boolean onGenericMotion(View view, MotionEvent motionEvent) {
        int eventSource = motionEvent.getSource();
        if ((((eventSource & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
            ((eventSource & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK))
            && motionEvent.getAction() == MotionEvent.ACTION_MOVE) {

            // Process the current movement sample in the batch.
            processJoystickInput(motionEvent, -1);

            return true;
        }

        return view.onGenericMotionEvent(motionEvent);
    }

    static float axis[] = new float[8];
    private void processJoystickInput(MotionEvent event, int historyPos) {
        InputDevice device = event.getDevice();

        axis[0] = getCenteredAxis(event, device, MotionEvent.AXIS_X, historyPos);
        axis[1] = getCenteredAxis(event, device, MotionEvent.AXIS_Y, historyPos);
        axis[2] = getCenteredAxis(event, device, MotionEvent.AXIS_Z, historyPos);
        axis[3] = getCenteredAxis(event, device, MotionEvent.AXIS_RZ, historyPos);
        axis[4] = getCenteredAxis(event, device, MotionEvent.AXIS_HAT_X, historyPos);
        axis[5] = getCenteredAxis(event, device, MotionEvent.AXIS_HAT_Y, historyPos);
        axis[6] = getCenteredAxis(event, device, MotionEvent.AXIS_RTRIGGER, historyPos);
        axis[7] = getCenteredAxis(event, device, MotionEvent.AXIS_LTRIGGER, historyPos);

        mOrxActivity.nativeOnJoystickMove(event.getDeviceId(), axis);
    }

    private static float getCenteredAxis(MotionEvent event, InputDevice device,
                                         int axis, int historyPos) {
        final InputDevice.MotionRange range = device.getMotionRange(axis, event.getSource());
        if (range != null) {
            final float flat = range.getFlat();
            final float value = historyPos < 0 ? event.getAxisValue(axis)
                    : event.getHistoricalAxisValue(axis, historyPos);

            // Ignore axis values that are within the 'flat' region of the
            // joystick axis center.
            // A joystick at rest does not always report an absolute position of
            // (0,0).
            if (Math.abs(value) > flat) {
                return value;
            }
        }
        return 0;
    }
}

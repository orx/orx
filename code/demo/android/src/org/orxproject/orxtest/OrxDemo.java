package org.orxproject.orxtest;

import org.orx.lib.OrxActivity;

import android.os.Bundle;

public class OrxDemo extends OrxActivity {
	
    @Override
	protected void onCreate(Bundle savedInstanceState) {
    	
    	/* if you want to use orx in a custom layout,
    	 * call setContentView with your layout, retrieve the OrxGLSurfaceView
    	 * and call setOrxGLSurfaceView() before super.onCreate().
    	 */
    	
		super.onCreate(savedInstanceState);

		// call this if you need accelerometer for orx joystick input
		enableAccelerometer();
	}

	static
    {
		// load your native module here.
        System.loadLibrary("orxTest");
    }

	@Override
	protected boolean requireDepthBuffer() {
		// return true if you need OpenGL depth buffer
		return false;
	}
}

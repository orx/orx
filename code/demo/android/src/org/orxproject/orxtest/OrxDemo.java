package org.orxproject.orxtest;

import org.orx.lib.OrxActivity;

import android.os.Bundle;

public class OrxDemo extends OrxActivity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {

		super.onCreate(savedInstanceState);

		// call this if you need accelerometer for orx joystick input
		enableAccelerometer();
	}

	static {
		// load your native module here.
		System.loadLibrary("orxTest");
	}

	@Override
	protected boolean requireDepthBuffer() {
		// return true if you need OpenGL depth buffer
		return false;
	}

	@Override
	protected int getLayoutId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the layout id
		 */
		return 0;
	}

	@Override
	protected int getOrxGLSurfaceViewId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the OrxGLSurfaceView id
		 */
		return 0;
	}
}


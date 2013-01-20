package org.orxproject.orxtest;

import org.orx.lib.OrxActivity;

import android.os.Bundle;

public class OrxDemo extends OrxActivity {

	static {
		// load your native module here.
		System.loadLibrary("orxTest");
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


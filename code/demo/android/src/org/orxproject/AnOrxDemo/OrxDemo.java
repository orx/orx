package org.orxproject.AnOrxDemo;

import org.orxproject.lib.AnOrxActivity;
import org.orxproject.lib.ORXSurface;

import android.view.*;
import android.os.*;

/**
 * ORX Activity
 */
public class OrxDemo extends AnOrxActivity {

	// Setup
	protected void onCreate(Bundle savedInstanceState) {
		// isDebug = false;
		// open the debug mode or no
		// appPath = "orx_demo";
		// the base folder to store the resource
		// orxAppLibName = "orxApp";
		// the lib name of this application
		// this three should be set in advance of creating father AnOrxActivity
		isDebug = false;
		//when you need to run in the emulator it should set to true
		isInEmulator = false;
		appPath = "bounce_demo";
		appName = "orxTest";
		orxAppLibName = "orxApp";
		

		// load the .so lib in AnOrxActivity
		super.onCreate(savedInstanceState);

		// So we can call stuff from static callbacks
		// TODO keep the screen on, I know it is not a perfect decision,
		// acticity should resume from pausing.
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		//build the gl surface, start to init and run the loop
		mSurface = new ORXSurface(this);
		setContentView(mSurface);
		SurfaceHolder holder = mSurface.getHolder();
		holder.setType(SurfaceHolder.SURFACE_TYPE_GPU);
	}
}

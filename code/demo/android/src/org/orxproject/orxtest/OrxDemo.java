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

	static
    {
        System.loadLibrary("orxTest");
    }

}

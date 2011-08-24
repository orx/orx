package org.orxproject.orxtest;

import android.os.Bundle;

import com.nvidia.devtech.NvEventQueueActivity;

public class OrxDemo extends NvEventQueueActivity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	wantsAccelerometer = true;
    	
        super.onCreate(savedInstanceState);
    }

    static
    {
    	System.loadLibrary("Box2D");
        System.loadLibrary("orxTest");
    }

}

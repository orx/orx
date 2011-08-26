package org.orxproject.orxtest;

import android.os.Bundle;

import com.nvidia.devtech.NvEventQueueActivity;

public class OrxDemo extends NvEventQueueActivity {
    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	wantsAccelerometer = true;
      /** The number of bits requested for the red component */
      redSize     = 5;
      /** The number of bits requested for the green component */
      greenSize   = 6;
      /** The number of bits requested for the blue component */
      blueSize    = 5;
      /** The number of bits requested for the alpha component */
      alphaSize   = 0;
      /** The number of bits requested for the stencil component */
      stencilSize = 0;
      /** The number of bits requested for the depth component */
      depthSize   = 0;
    	
        super.onCreate(savedInstanceState);
    }

    static
    {
    	System.loadLibrary("Box2D");
    	System.loadLibrary("SOIL");
        System.loadLibrary("orxTest");
    }

}

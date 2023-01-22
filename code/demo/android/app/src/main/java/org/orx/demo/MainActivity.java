package org.orx.demo;

import org.orx.lib.OrxGameActivity;

// Note: if Game controller support is not needed then you can instead extend android.app.NativeActivity
public class MainActivity extends OrxGameActivity {
    static {
        System.loadLibrary("orxTest");
    }

    protected void onResume() {
        super.onResume();
        hideSystemBars();
    }
}

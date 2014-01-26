package org.orxproject.orxtest;

import android.annotation.TargetApi;
import android.os.Build;
import android.os.Bundle;
import android.view.View;

import org.orx.lib.OrxActivity;

public class OrxDemo extends OrxActivity {
    private View mDecorView;
    private VersionedOnWindowFocusChanged mOnWindowFocusChanged;

    static {
    	// load openal-soft module first.
    	System.loadLibrary("openal-soft");
		// load your native module here.
		System.loadLibrary("orxTest");
	}

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mDecorView = getWindow().getDecorView();

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            mOnWindowFocusChanged = new KitKatOnWindowFocusChanged();
        } else {
            mOnWindowFocusChanged = new PreKitKatOnWindowFocusChanged();
        }
    }

    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        mOnWindowFocusChanged.onWindowFocusChanged(hasFocus, mDecorView);
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
	protected int getSurfaceViewId() {
		/*
		 * Override this if you want to use a custom layout
		 * return the OrxGLSurfaceView id
		 */
		return 0;
	}

    public interface VersionedOnWindowFocusChanged {
        public void onWindowFocusChanged(boolean hasFocus, View decorView);
    }

    private static class PreKitKatOnWindowFocusChanged implements VersionedOnWindowFocusChanged {

        @Override
        public void onWindowFocusChanged(boolean hasFocus, View decorView) {
            // nothing to do
        }
    }

    private static class KitKatOnWindowFocusChanged implements VersionedOnWindowFocusChanged {
        @TargetApi(Build.VERSION_CODES.HONEYCOMB)
        @Override
        public void onWindowFocusChanged(boolean hasFocus, View decorView) {
            if (hasFocus) {
                decorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            }
        }
    }
}


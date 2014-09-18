package org.orxproject.orxtest;

import android.annotation.TargetApi;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnSystemUiVisibilityChangeListener;
import org.orx.lib.OrxActivity;

public class OrxDemo extends OrxActivity {
    private View mDecorView;

    static {
	// load your native module here.
	System.loadLibrary("orxTest");
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        // call setContentView() here if you need a custom layout.
        // The custom layout needs to include a SurfaceView with @+id/orxSurfaceView.
        
        mDecorView = getWindow().getDecorView();

        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            mDecorView.setOnSystemUiVisibilityChangeListener(new OnSystemUiVisibilityChangeListener() {
				
				@Override
				public void onSystemUiVisibilityChange(int visibility) {
					if ((visibility & View.SYSTEM_UI_FLAG_FULLSCREEN) == 0) {
						mDecorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
		                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
		                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
		                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
		                        | View.SYSTEM_UI_FLAG_FULLSCREEN
		                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
					}
				}
			});
        }
    }

    @TargetApi(Build.VERSION_CODES.KITKAT)
    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        super.onWindowFocusChanged(hasFocus);
        
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
        	if (hasFocus) {
                mDecorView.setSystemUiVisibility(View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                        | View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                        | View.SYSTEM_UI_FLAG_FULLSCREEN
                        | View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY);
            }
        }
    }
}


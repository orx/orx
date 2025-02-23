package org.orx.lib;

import androidx.core.view.ViewCompat;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import com.google.androidgamesdk.GameActivity;

public class OrxGameActivity extends GameActivity {
    protected void onResume() {
        super.onResume();
        hideSystemBars();
    }

    @Override
    protected void onDestroy() {
        // See https://issuetracker.google.com/issues/398193010
        if (mSurfaceView != null) {
            ViewCompat.setOnApplyWindowInsetsListener(mSurfaceView, null);
        }

        super.onDestroy();
    }

    protected void hideSystemBars() {
        WindowInsetsControllerCompat windowInsetsController = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        windowInsetsController.hide(WindowInsetsCompat.Type.systemBars());
    }
}

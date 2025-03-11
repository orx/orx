package org.orx.lib;

import android.view.View;

import androidx.annotation.NonNull;
import androidx.core.view.WindowCompat;
import androidx.core.view.WindowInsetsCompat;
import androidx.core.view.WindowInsetsControllerCompat;

import com.google.androidgamesdk.GameActivity;

public class OrxGameActivity extends GameActivity {
    protected void onResume() {
        super.onResume();
        hideSystemBars();
    }

    @NonNull
    @Override
    public WindowInsetsCompat onApplyWindowInsets(View v, WindowInsetsCompat insets) {
        // See https://issuetracker.google.com/issues/398193010
        if (mDestroyed) {
            return insets;
        }

        return super.onApplyWindowInsets(v, insets);
    }

    protected void hideSystemBars() {
        WindowInsetsControllerCompat windowInsetsController = WindowCompat.getInsetsController(getWindow(), getWindow().getDecorView());
        windowInsetsController.hide(WindowInsetsCompat.Type.systemBars());
    }
}

package org.orxproject.lib;

import java.io.IOException;

import android.app.*;
import android.view.*;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.os.*;
import android.test.MoreAsserts;
import android.util.Log;
import android.graphics.*;
import android.graphics.Bitmap.Config;
import android.hardware.*;
import android.content.res.AssetManager;

/**
 * ORX Activity
 */
public class AnOrxActivity extends Activity {

	// Main components
	protected static ORXSurface mSurface;

	//debug mode using orxd
	protected static boolean isDebug = true;
	//when you need to run in the emulator it should set to true
	protected static boolean usingGLES1 = false;
	
	protected static boolean isTranslucent = true;

	/*
	* first, set the app name, the main feature is to set the default .ini filename
	*/
	protected static String appPath = "orx_demo";
	
	/*
	 * set the res folder contain .ini, all of images and other stuff. the virtual app name
	 */
	protected static String appName = "orxTest";

	protected static String orxAppLibName = "orxApp";

	public static AnOrxActivity anOrxActivity;

	private static Bitmap lastBmp;

	public static String saveImagePath;

	public static int saveImageState;

	/**
	 * eg when the surface create this will be set as true, vice verse
	 */
	public static boolean mHasSurface;

	/**
	 * eg when the app is paused or push to the back
	 */
	public static boolean mPause;

	/**
	 * when the egl is created
	 */
	public static boolean mInit;

	/**
	 * when orx view is changed evert time complete switching from background to
	 * foreground, the size of view will be changed
	 */
	public static boolean mSizeChange;

	// these two indicate the width and height of the orx view
	public static int mWidth;

	public static int mHeight;

	public static byte[] ORXThreadLock = new byte[0]; // thread lock

	// C functions we call
	public static native void nativeInit();

	public static native void nativeQuit();

	public static native void nativeSetScreenSize(int width, int height);

	public static native void onNativeKeyDown(int keycode);

	public static native void onNativeKeyUp(int keycode);

	public static native void onNativeTouch(int action, int pointid, float x,
			float y, float p);

	public static native void onNativeResize(int x, int y);

	/**
	 * 
	 * @param focusEventID
	 *            0 is pull the view into background 1 is pull the view into
	 *            foreground
	 */
	public static native void nativeSendFocusEvent(int focusEventID);

	public static native void onNativeAccel(float x, float y, float z);

	public static native void nativeSetAssetManager(AssetManager assetManager);

	public static native void nativeSetShaderSupport(boolean bShaderSupport);
	
	public static native void nativeSetMainAppPath(String mainAppPath);

	private AnOrxMovingAsset movingAsset;
	private String fullAppPath;

	// Setup
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		fullAppPath = appPath+"/"+appName;
		if (isDebug) {

			System.loadLibrary(orxAppLibName + "d");
			fullAppPath += "d";
		} else {
			System.loadLibrary(orxAppLibName);
		}

		mWidth = 0;
		mHeight = 0;
		mPause = false;
		mHasSurface = false;
		mSizeChange = false;
		mInit = false;

		anOrxActivity = this;

		movingAsset = new AnOrxMovingAsset(getAssets());
		nativeSetMainAppPath(fullAppPath);
		try {
			movingAsset.copyDirectory(appPath, "/sdcard");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			Log.e("orx app", "error when moving asset");
			e.printStackTrace();
		}
		
		//create the apkFileHelper
		AnOrxAPKFileHelper.getInstance().setContext(this);

	}

	// Accel
	public static boolean initAccel() {
		mSurface.enableSensor(Sensor.TYPE_ACCELEROMETER, true);
		return true;
	}

	public static boolean closeAccel() {
		mSurface.enableSensor(Sensor.TYPE_ACCELEROMETER, false);
		return true;
	}

	// Events
	protected void onPause() {
		Log.d("orx app", "pause");
		synchronized (ORXThreadLock) {
			mPause = true;
		}

		super.onPause();
	}

	protected void onResume() {
		Log.d("orx app", "resume");
		synchronized (ORXThreadLock) {
			mPause = false;
			ORXThreadLock.notifyAll();
		}
		super.onResume();
	}

	@Override
	protected void onDestroy() {
		// TODO Auto-generated method stub
		AnOrxActivity.nativeQuit();
		System.exit(0);
		super.onDestroy();
	}

	// Java functions called from C
	public static void createGLContext(int colorDepth, boolean depthBuffer) {
		mSurface.initEGL(colorDepth, depthBuffer);
		mInit = true;
	}

	/**
	 * will be called every run loop
	 * 
	 * @return
	 */
	public static int startLoop() {
		boolean storePause;
		// in orx init and orx pause state,the game thread will be slept and
		// wait for waking up in onResume
		synchronized (ORXThreadLock) {
			storePause = mPause;
			if (mPause && mInit) {
				nativeSendFocusEvent(0);
			}
			while (needToWait()) {
				try {

					ORXThreadLock.wait();
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					Log.e("orx app", "thread lock error");
					e.printStackTrace();
				}
			}
			if (mSizeChange && mInit) {
				mSurface.createEGLSurface();
				mSizeChange = false;
				// the only possible to pull the view to the foreground is here

			}
			if (storePause && mInit) {
				nativeSendFocusEvent(1);
			}
		}
		return 1;
	}

	public static boolean needToWait() {

		if (mPause || (!mHasSurface)) {
			return true;
		}

		if ((mWidth > 0) && (mHeight > 0)) {
			return false;
		}

		return true;

	}

	public static int endLoop() {
		return 1;
	}

	public static void flipBuffers() {
		mSurface.flipEGL();
	}

	/*
	 * this function does not test. I hope it could work
	 */
	public static void saveScreenImage(String filename, boolean bPNG) {

		saveImagePath = filename;
		saveImageState = bPNG ? 2 : 1;

	}

	public static boolean setMediaPlayerDataSource(MediaPlayer mediaPlayer,
			String assetFileName) {
		try {
			mediaPlayer.setDataSource(anOrxActivity.getAssets()
					.openFd(assetFileName).getFileDescriptor());
			return true;
		} catch (IllegalArgumentException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.e("orx", "an error when loading music");
			return false;
		} catch (IllegalStateException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.e("orx", "an error when loading music");
			return false;
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.e("orx", "an error when loading music");
			return false;
		}
	}
	
	public static int loadSound(SoundPool soundpool, String assetFileName){
		try {
			return soundpool.load(anOrxActivity.getAssets()
					.openFd(assetFileName), 1);
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
			Log.e("orx", "an error when loading sound");
			return 0;
		}
	}

}

/**
 * Simple nativeInit() runnable
 */
class ORXRunner implements Runnable {
	public void run() {
		// AnOrxActivity.initAudio();

		// Runs orxMain()
		AnOrxActivity.startLoop();
		AnOrxActivity.initAccel();
		AnOrxActivity.nativeInit();

	}
}

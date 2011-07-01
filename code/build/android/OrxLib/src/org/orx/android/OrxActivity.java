package org.orx.android;

import static android.view.ViewGroup.LayoutParams.FILL_PARENT;

import java.io.File;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.content.pm.PackageManager;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.os.Environment;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.Log;
import android.view.Gravity;
import android.widget.FrameLayout.LayoutParams;

public class OrxActivity extends Activity implements SensorEventListener {
	private static String TAG = "OrxActivity";
	private static final String META_DATA_LIB_NAME = "android.orx.lib_name";
	
	private boolean mOrxRunning;
	private boolean mOrxExited;
	private EGLContext mContext;
	private EGLSurface mSurface;
	private SensorManager mSensorManager;
	private int mOrientation;
	public boolean mExternalStorageAvailable = false;
	public boolean mExternalStorageWriteable = false;
	
	private PowerManager mPowerManager;
	private WakeLock mWakeLock;

	protected OrxView mView;
	protected OrxRenderer mRenderer;

	@Override
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		ActivityInfo ai;
		String libname = "orxjni";
		
		try {
            ai = getPackageManager().getActivityInfo(
                    getIntent().getComponent(), PackageManager.GET_META_DATA);
            if (ai.metaData != null) {
                String ln = ai.metaData.getString(META_DATA_LIB_NAME);
                if (ln != null) libname = ln;
            }
        } catch (PackageManager.NameNotFoundException e) {
            throw new RuntimeException("Error getting activity info", e);
        }
        // load orx
        OrxLib.setLibName(libname);
        OrxLib.load();
        
		
		mOrientation = getWindowManager().getDefaultDisplay().getOrientation();
		APKFileHelper.getInstance().setContext(getApplication());

		mRenderer = new OrxRenderer();
		
		mSensorManager = (SensorManager) getSystemService("sensor");

		mOrxExited = false;
		mOrxRunning = false;

		// Get an instance of the PowerManager
        mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
		// Create a bright wake lock
        mWakeLock = mPowerManager.newWakeLock(PowerManager.SCREEN_BRIGHT_WAKE_LOCK, getClass().getName());
        
		onSetContentView();
	}

	@Override
	protected void onPause() {
		super.onPause();
		mSensorManager.unregisterListener(this,
				mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
		mView.onPause();
		try {
	        mWakeLock.release();
		} catch (SecurityException e) {
			Log.w(TAG,  "add permission android.permission.WAKE_LOCK to your AndroidManfiest.xml");
		}
	}

	@Override
	protected void onRestart() {
		super.onRestart();
		mView.queueEvent(new Runnable() {
			
			@Override
			public void run() {
				OrxLib.send_orxSYSTEM_EVENT_FOREGROUND();
			}
		});
	}

	@Override
	protected void onStop() {
		super.onStop();
		if(mOrxRunning) {
			mView.queueEvent(new Runnable() {
				
				@Override
				public void run() {
					OrxLib.send_orxSYSTEM_EVENT_BACKGROUND();
				}
			});
		}
	}

	@Override
	protected void onResume() {
		super.onResume();
		mView.onResume();
		mSensorManager.registerListener(this,
				mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER),
				SensorManager.SENSOR_DELAY_GAME, null);
		try {
			mWakeLock.acquire();
		} catch (SecurityException e) {
			Log.w(TAG,  "add permission android.permission.WAKE_LOCK to your AndroidManfiest.xml");
		}
	}

	/* ==== OrxRenderer class ==== */

	private class OrxRenderer implements GLSurfaceView.Renderer {
		@Override
		public void onDrawFrame(GL10 gl) {
			if (!mOrxExited) {
				if (!OrxLib.step()) {
					OrxLib.exit();
					mOrxExited = true;
					mOrxRunning = false;
					OrxActivity.this.finish();
				}
			}
		}

		@Override
		public void onSurfaceChanged(GL10 gl, int width, int height) {
			if (!mOrxRunning) {
				OrxLib.init(OrxActivity.this, width, height);
				mOrxRunning = true;
			}
		}

		@Override
		public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		}
	}

	protected class OrxContextFactory implements GLSurfaceView.EGLContextFactory {
		private static final int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

		public EGLContext createContext(EGL10 egl, EGLDisplay display,
				EGLConfig eglConfig) {
			Log.w(TAG, "creating OpenGL ES 2.0 context");
			checkEglError("Before eglCreateContext", egl);
			int[] attrib_list = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL10.EGL_NONE };
			EGLContext context = egl.eglCreateContext(display, eglConfig,
					EGL10.EGL_NO_CONTEXT, attrib_list);
			checkEglError("After eglCreateContext", egl);
			mContext = context;
			return context;
		}

		public void destroyContext(EGL10 egl, EGLDisplay display,
				EGLContext context) {
			egl.eglDestroyContext(display, context);
			mContext = null;
		}
	}

	protected class OrxWindowSurfaceFactory implements
			GLSurfaceView.EGLWindowSurfaceFactory {

		public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay display,
				EGLConfig config, Object nativeWindow) {
			mSurface = egl.eglCreateWindowSurface(display, config,
					nativeWindow, null);
			if (mOrxRunning) {
				egl.eglMakeCurrent(display, mSurface, mSurface, mContext);
				OrxLib.send_orxDISPLAY_EVENT_RESTORE_CONTEXT();
				egl.eglMakeCurrent(display, EGL10.EGL_NO_SURFACE,
						EGL10.EGL_NO_SURFACE, mContext);
			}
			return mSurface;
		}

		public void destroySurface(EGL10 egl, EGLDisplay display,
				EGLSurface surface) {
			if (mOrxRunning) {
				egl.eglMakeCurrent(display, mSurface, mSurface, mContext);
				OrxLib.send_orxDISPLAY_EVENT_SAVE_CONTEXT();
				egl.eglMakeCurrent(display, EGL10.EGL_NO_SURFACE,
						EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
			}
			egl.eglDestroySurface(display, surface);
			mSurface = null;
		}
	}

	private static void checkEglError(String prompt, EGL10 egl) {
		int error;
		while ((error = egl.eglGetError()) != EGL10.EGL_SUCCESS) {
			Log.e(TAG, String.format("%s: EGL error: 0x%x", prompt, error));
		}
	}

	@Override
	public void onAccuracyChanged(Sensor sensor, int accuracy) {
	}

	@Override
	public void onSensorChanged(SensorEvent event) {
		if (mOrxRunning && event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
			if (mOrientation == 0) {
				// portrait mode
				final float x = -event.values[0];
				final float y = -event.values[1];
				final float z = event.values[2];
				mView.queueEvent(new Runnable() {

					@Override
					public void run() {
						OrxLib.onNativeAccel(x, y, z);
					}
				});
			} else {
				// landscape mode
				final float x = event.values[1];
				final float y = -event.values[0];
				final float z = event.values[2];
				mView.queueEvent(new Runnable() {

					@Override
					public void run() {
						OrxLib.onNativeAccel(x, y, z);
					}
				});
			}
		}
	}
	
	protected int getScreenDepth() {
		//default to 16bpp surface
		return 16;
	}
	
	protected boolean isUsingDepthBuffer() {
		//default to no depth buffer
		return false;
	}

	protected void onSetContentView() {
		mView = new OrxView(this, (getScreenDepth() == 32), isUsingDepthBuffer() ? 8 : 0, 0);
		/*
		 * Setup the context factory for 2.0 rendering. See ContextFactory class
		 * definition below
		 */
		mView.setEGLContextFactory(new OrxContextFactory());
		mView.setEGLWindowSurfaceFactory(new OrxWindowSurfaceFactory());

		mView.setRenderer(mRenderer);

		this.setContentView(mView, createSurfaceViewLayoutParams());
	}

	protected LayoutParams createSurfaceViewLayoutParams() {
		final LayoutParams layoutParams = new LayoutParams(FILL_PARENT,
				FILL_PARENT);
		layoutParams.gravity = Gravity.CENTER;
		return layoutParams;
	}
	
	public String getExternalStorageDirectory() {
		String directory = null;
		String state = Environment.getExternalStorageState();

		if (Environment.MEDIA_MOUNTED.equals(state)) {
		    // We can read and write the media
		    mExternalStorageAvailable = mExternalStorageWriteable = true;
		} else if (Environment.MEDIA_MOUNTED_READ_ONLY.equals(state)) {
		    // We can only read the media
		    mExternalStorageAvailable = true;
		    mExternalStorageWriteable = false;
		} else {
		    // Something else is wrong. It may be one of many other states, but all we need
		    //  to know is we can neither read nor write
		    mExternalStorageAvailable = mExternalStorageWriteable = false;
		}
		
		if(mExternalStorageAvailable) {
			File sd_dir = Environment.getExternalStorageDirectory();
			String activity_dir = "/Android/data/" + getApplication().getPackageName() + "/files/";
			File working_dir = new File(sd_dir, activity_dir);
			if(!working_dir.exists()) {
				if(mExternalStorageWriteable) {
					if(working_dir.mkdirs()) {
						Log.i(TAG, "directory " + working_dir.getAbsolutePath() + " created.");
						directory = working_dir.getAbsolutePath() + "/";
					}
					else {
						Log.w(TAG, "external directory " + working_dir.getAbsolutePath() + " doesn't exists, but external storage is not writable!");
					}
				}
				else {
					Log.w(TAG, "external directory " + working_dir.getAbsolutePath() + " doesn't exists, but external storage is not writable!");
				}
			}
			else {
				directory = working_dir.getAbsolutePath() + "/";
			}
		}
		
		return directory;
	}
}
package org.orxproject.lib;

import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.IntBuffer;
import java.security.spec.MGF1ParameterSpec;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL10;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.PixelFormat;
import android.graphics.Bitmap.CompressFormat;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.opengl.GLSurfaceView;
import android.provider.Settings.System;
import android.util.AttributeSet;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;

/**
 * ORXSurface. This is what we draw on, so we need to know when it's created in
 * order to do anything usef Because of this, that's where we set up the ORX
 * thread
 */
public class ORXSurface extends SurfaceView implements SurfaceHolder.Callback,
		View.OnKeyListener, View.OnTouchListener, SensorEventListener {

	// This is what ORX runs in. It invokes orxMain(), eventually
	private Thread mORXThread;

	// EGL private objects
	private EGLContext mEGLContext;
	private EGLSurface mEGLSurface;
	private EGLDisplay mEGLDisplay;
	private EGLConfig mEGLConfig;

	private static int EGL_CONTEXT_CLIENT_VERSION = 0x3098;

	// Sensors
	private static SensorManager mSensorManager;

	// Startup
	public ORXSurface(Context context) {
		super(context);
		getHolder().addCallback(this);

		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
		setOnKeyListener(this);
		setOnTouchListener(this);

		mSensorManager = (SensorManager) context.getSystemService("sensor");

		int width = getWidth();
		int height = getHeight();

		// Set the width and height variables in C before we start ORX so we
		// haveit available on init
		// AnOrxActivity.onNativeResize(width, height);

		mORXThread = new Thread(new ORXRunner(), "ORXThread");
		mORXThread.start();
	}

	public ORXSurface(Context context, AttributeSet attrs) {
		super(context, attrs);
		setClickable(true);
		setFocusable(true);

		getHolder().addCallback(this);

		setFocusable(true);
		setFocusableInTouchMode(true);
		requestFocus();
		setOnKeyListener(this);
		setOnTouchListener(this);

		mSensorManager = (SensorManager) context.getSystemService("sensor");

		int width = getWidth();
		int height = getHeight();

		// Set the width and height variables in C before we start ORX so we
		// haveit available on init
		AnOrxActivity.onNativeResize(width, height);


		mORXThread = new Thread(new ORXRunner(), "ORXThread");
		mORXThread.start();
	}

	// Called when we have a valid drawing surface
	public void surfaceCreated(SurfaceHolder holder) {
		synchronized (AnOrxActivity.ORXThreadLock) {
			AnOrxActivity.mHasSurface = true;
			setFocusable(true);
			setFocusableInTouchMode(true);
			requestFocus();
			AnOrxActivity.ORXThreadLock.notifyAll(); // to wake up the game
			// thread
		}
	}

	// Called when we lose the surface
	public void surfaceDestroyed(SurfaceHolder holder) {

		synchronized (AnOrxActivity.ORXThreadLock) {
			AnOrxActivity.mHasSurface = false;
		}

	}

	// Called when the surface is resized
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		synchronized (AnOrxActivity.ORXThreadLock) {
			EGL10 mEgl = (EGL10) EGLContext.getEGL();

			AnOrxActivity.mWidth = width;
			AnOrxActivity.mHeight = height;
			AnOrxActivity.mSizeChange = true;
			AnOrxActivity.onNativeResize(width, height);
			AnOrxActivity.ORXThreadLock.notify();

		}

	}

	// unused
	public void onDraw(Canvas canvas) {
	}

	public void destroySurface() {
		EGL10 mEgl = (EGL10) EGLContext.getEGL();
		if (mEGLSurface != null) {
			mEgl.eglMakeCurrent(mEGLDisplay, EGL10.EGL_NO_SURFACE,
					EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
			mEgl.eglDestroySurface(mEGLDisplay, mEGLSurface);
			mEGLSurface = null;
		}
		if (mEGLContext != null) {
			mEgl.eglDestroyContext(mEGLDisplay, mEGLContext);
			mEGLContext = null;
		}
		if (mEGLDisplay != null) {
			mEgl.eglTerminate(mEGLDisplay);
			mEGLDisplay = null;
		}
	}

	public boolean createEGLSurface() {
		EGL10 egl = (EGL10) EGLContext.getEGL();
		if (mEGLSurface != null) {

			/*
			 * Unbind and destroy the old EGL surface, if there is one.
			 */
			egl.eglMakeCurrent(mEGLDisplay, EGL10.EGL_NO_SURFACE,
					EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
			egl.eglDestroySurface(mEGLDisplay, mEGLSurface);
		}

		EGLSurface surface = egl.eglCreateWindowSurface(mEGLDisplay,
				mEGLConfig, this.getHolder(), null);
		if (surface == null) {
			int i = egl.eglGetError();
			if (i == 12299)
				Log.e("orx app",
						"createWindowSurface returned EGL_BAD_NATIVE_WINDOW.");
		}
		boolean isMake = egl.eglMakeCurrent(mEGLDisplay, surface, surface,
				mEGLContext);

		mEGLSurface = surface;
		return true;
	}

	// EGL functions
	public boolean initEGL(int colorDepth, boolean depthBuffer) {
		int depthBufferSize = depthBuffer ? 16 : 0;
		int r = 0, g = 0, b = 0, a = 0;
		if (colorDepth == 16) {
			r = 5;
			g = 6;
			b = 5;
		} else if (colorDepth == 24) {
			r = 8;
			g = 8;
			b = 8;
		} else if (colorDepth == 32) {
			r = 8;
			g = 8;
			b = 8;
			a = 8;
		}

		Log.d("ORX", "Starting up " + r + ", " + g + ", " + b + ", " + a + ", "
				+ depthBufferSize);
		if (a > 0) {
			this.getHolder().setFormat(PixelFormat.TRANSLUCENT);
		}

		try {

			EGL10 egl = (EGL10) EGLContext.getEGL();

			EGLDisplay dpy = egl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);

			int[] version = new int[2];
			egl.eglInitialize(dpy, version);

			ConfigChooser configChooser = new ConfigChooser(r, g, b, a,
					depthBufferSize, 0);

			EGLConfig config = configChooser.chooseConfig(egl, dpy);

			EGLContext ctx;
			if (configChooser.gles2Support) {
				int[] attrib_list = { EGL_CONTEXT_CLIENT_VERSION, 2,
						EGL10.EGL_NONE };

				ctx = egl.eglCreateContext(dpy, config, EGL10.EGL_NO_CONTEXT,
						attrib_list);
			} else {

				ctx = egl.eglCreateContext(dpy, config, EGL10.EGL_NO_CONTEXT,
						null);
			}
			mEGLConfig = config;
			mEGLContext = ctx;
			mEGLDisplay = dpy;
			createEGLSurface();

		} catch (Exception e) {
			Log.e("ORX", e + "");
			for (StackTraceElement s : e.getStackTrace()) {
				Log.v("ORX", s.toString());
			}
		}
		Log.e("ORX", "Done making!");

		return true;
	}

	// EGL buffer flip
	public void flipEGL() {
		try {

			EGL10 egl = (EGL10) EGLContext.getEGL();
			GL10 gl = (GL10) mEGLContext.getGL();

			egl.eglWaitNative(EGL10.EGL_NATIVE_RENDERABLE, null);

			// drawing here

			egl.eglWaitGL();

			if (AnOrxActivity.saveImageState > 0) {
				int b[] = new int[getWidth() * getHeight()];
				int bt[] = new int[getWidth() * getHeight()];
				IntBuffer ib = IntBuffer.wrap(b);
				ib.position(0);

				gl.glReadPixels(0, 0, getWidth(), getHeight(), GL10.GL_RGBA,
						GL10.GL_UNSIGNED_BYTE, ib);

				for (int i = 0; i < getHeight(); i++) {
					// remember, that OpenGL bitmap is incompatible with Android
					// bitmap and so, some correction need.
					for (int j = 0; j < getWidth(); j++) {
						int pix = b[i * getWidth() + j];
						int pb = (pix >> 16) & 0xff;
						int pr = (pix << 16) & 0x00ff0000;
						int pix1 = (pix & 0xff00ff00) | pr | pb;
						bt[(getHeight() - i - 1) * getWidth() + j] = pix1;
					}
				}
				Bitmap screenBitmap = Bitmap.createBitmap(bt, getWidth(),
						getHeight(), Bitmap.Config.ARGB_8888);

				CompressFormat format = AnOrxActivity.saveImageState == 1 ? CompressFormat.JPEG
						: CompressFormat.PNG;
				FileOutputStream fos = new FileOutputStream("/sdcard/"
						+ AnOrxActivity.appPath + AnOrxActivity.saveImagePath);
				screenBitmap.compress(format, 90, fos);
				try {
					fos.flush();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				try {
					fos.close();
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				AnOrxActivity.saveImageState = 0;
			}

			egl.eglSwapBuffers(mEGLDisplay, mEGLSurface);

		} catch (Exception e) {
			Log.e("ORX", "flipEGL(): " + e);

			for (StackTraceElement s : e.getStackTrace()) {
				Log.e("ORX", s.toString());
			}
		}
	}

	private static class ConfigChooser implements
			GLSurfaceView.EGLConfigChooser {

		public boolean gles2Support = false;

		public ConfigChooser(int r, int g, int b, int a, int depth, int stencil) {
			mRedSize = r;
			mGreenSize = g;
			mBlueSize = b;
			mAlphaSize = a;
			mDepthSize = depth;
			mStencilSize = stencil;
		}

		/*
		 * This EGL config specification is used to specify 2.0 rendering. We
		 * use a minimum size of 4 bits for red/green/blue, but will perform
		 * actual matching in chooseConfig() below.
		 */
		private static int EGL_OPENGL_ES2_BIT = 4;
		private static int[] s_configAttribs2 = { EGL10.EGL_RED_SIZE, 4,
				EGL10.EGL_GREEN_SIZE, 4, EGL10.EGL_BLUE_SIZE, 4, EGL10.EGL_NONE };

		private static int[] s_configAttribsgles2 = { EGL10.EGL_RED_SIZE, 4,
				EGL10.EGL_GREEN_SIZE, 4, EGL10.EGL_BLUE_SIZE, 4,
				EGL10.EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT, EGL10.EGL_NONE };

		public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {

			/*
			 * Get the number of minimally matching EGL configurations
			 */
			int[] num_config = new int[1];
			egl.eglChooseConfig(display, s_configAttribsgles2, null, 0,
					num_config);

			int numConfigs = num_config[0];
			if (AnOrxActivity.usingGLES1) {
				numConfigs = 0;
			}

			if (numConfigs <= 0) {
				Log.d("ORX", "do not support gles 2.0");
				gles2Support = false;
				AnOrxActivity.nativeSetShaderSupport(false);

				egl.eglChooseConfig(display, s_configAttribs2, null, 0,
						num_config);

				numConfigs = num_config[0];
				if (numConfigs <= 0) {
					throw new IllegalArgumentException(
							"No configs match configSpec");
				}
				/*
				 * Allocate then read the array of minimally matching EGL
				 * configs
				 */
				EGLConfig[] configs = new EGLConfig[numConfigs];
				egl.eglChooseConfig(display, s_configAttribs2, configs,
						numConfigs, num_config);
				EGLConfig configResult = chooseConfig(egl, display, configs);
				printConfig(egl, display, configResult);

				// do other thing to notify the gles support

				return configResult;
			}

			Log.d("ORX", "support gles 2.0 shader is coming");
			gles2Support = true;
			AnOrxActivity.nativeSetShaderSupport(true);

			/*
			 * Allocate then read the array of minimally matching EGL configs
			 */
			EGLConfig[] configs = new EGLConfig[numConfigs];
			egl.eglChooseConfig(display, s_configAttribsgles2, configs,
					numConfigs, num_config);

			EGLConfig configResult = chooseConfig(egl, display, configs);
			printConfig(egl, display, configResult);

			// do other thing to notify the gles support
			/*
			 * Now return the "best" one
			 */
			return configResult;
		}

		public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display,
				EGLConfig[] configs) {
			for (EGLConfig config : configs) {
				int d = findConfigAttrib(egl, display, config,
						EGL10.EGL_DEPTH_SIZE, 0);
				int s = findConfigAttrib(egl, display, config,
						EGL10.EGL_STENCIL_SIZE, 0);


				// We need at least mDepthSize and mStencilSize bits
				if (d < mDepthSize || s < mStencilSize)
					continue;

				// We want an *exact* match for red/green/blue/alpha
				int r = findConfigAttrib(egl, display, config,
						EGL10.EGL_RED_SIZE, 0);
				int g = findConfigAttrib(egl, display, config,
						EGL10.EGL_GREEN_SIZE, 0);
				int b = findConfigAttrib(egl, display, config,
						EGL10.EGL_BLUE_SIZE, 0);
				int a = findConfigAttrib(egl, display, config,
						EGL10.EGL_ALPHA_SIZE, 0);

				if (r == mRedSize && g == mGreenSize && b == mBlueSize
						&& a == mAlphaSize)
					return config;
			}
			return null;
		}

		private int findConfigAttrib(EGL10 egl, EGLDisplay display,
				EGLConfig config, int attribute, int defaultValue) {

			if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
				return mValue[0];
			}
			return defaultValue;
		}

		private void printConfigs(EGL10 egl, EGLDisplay display,
				EGLConfig[] configs) {
			int numConfigs = configs.length;
			Log.w("orxJAVA", String.format("%d configurations", numConfigs));
			for (int i = 0; i < numConfigs; i++) {
				Log.w("orxJAVA", String.format("Configuration %d:\n", i));
				printConfig(egl, display, configs[i]);
			}
		}

		private void printConfig(EGL10 egl, EGLDisplay display, EGLConfig config) {
			int[] attributes = { EGL10.EGL_BUFFER_SIZE, EGL10.EGL_ALPHA_SIZE,
					EGL10.EGL_BLUE_SIZE,
					EGL10.EGL_GREEN_SIZE,
					EGL10.EGL_RED_SIZE,
					EGL10.EGL_DEPTH_SIZE,
					EGL10.EGL_STENCIL_SIZE,
					EGL10.EGL_CONFIG_CAVEAT,
					EGL10.EGL_CONFIG_ID,
					EGL10.EGL_LEVEL,
					EGL10.EGL_MAX_PBUFFER_HEIGHT,
					EGL10.EGL_MAX_PBUFFER_PIXELS,
					EGL10.EGL_MAX_PBUFFER_WIDTH,
					EGL10.EGL_NATIVE_RENDERABLE,
					EGL10.EGL_NATIVE_VISUAL_ID,
					EGL10.EGL_NATIVE_VISUAL_TYPE,
					0x3030, // EGL10.EGL_PRESERVED_RESOURCES,
					EGL10.EGL_SAMPLES,
					EGL10.EGL_SAMPLE_BUFFERS,
					EGL10.EGL_SURFACE_TYPE,
					EGL10.EGL_TRANSPARENT_TYPE,
					EGL10.EGL_TRANSPARENT_RED_VALUE,
					EGL10.EGL_TRANSPARENT_GREEN_VALUE,
					EGL10.EGL_TRANSPARENT_BLUE_VALUE,
					0x3039, // EGL10.EGL_BIND_TO_TEXTURE_RGB,
					0x303A, // EGL10.EGL_BIND_TO_TEXTURE_RGBA,
					0x303B, // EGL10.EGL_MIN_SWAP_INTERVAL,
					0x303C, // EGL10.EGL_MAX_SWAP_INTERVAL,
					EGL10.EGL_LUMINANCE_SIZE, EGL10.EGL_ALPHA_MASK_SIZE,
					EGL10.EGL_COLOR_BUFFER_TYPE, EGL10.EGL_RENDERABLE_TYPE,
					0x3042 // EGL10.EGL_CONFORMANT
			};
			String[] names = { "EGL_BUFFER_SIZE", "EGL_ALPHA_SIZE",
					"EGL_BLUE_SIZE", "EGL_GREEN_SIZE", "EGL_RED_SIZE",
					"EGL_DEPTH_SIZE", "EGL_STENCIL_SIZE", "EGL_CONFIG_CAVEAT",
					"EGL_CONFIG_ID", "EGL_LEVEL", "EGL_MAX_PBUFFER_HEIGHT",
					"EGL_MAX_PBUFFER_PIXELS", "EGL_MAX_PBUFFER_WIDTH",
					"EGL_NATIVE_RENDERABLE", "EGL_NATIVE_VISUAL_ID",
					"EGL_NATIVE_VISUAL_TYPE", "EGL_PRESERVED_RESOURCES",
					"EGL_SAMPLES", "EGL_SAMPLE_BUFFERS", "EGL_SURFACE_TYPE",
					"EGL_TRANSPARENT_TYPE", "EGL_TRANSPARENT_RED_VALUE",
					"EGL_TRANSPARENT_GREEN_VALUE",
					"EGL_TRANSPARENT_BLUE_VALUE", "EGL_BIND_TO_TEXTURE_RGB",
					"EGL_BIND_TO_TEXTURE_RGBA", "EGL_MIN_SWAP_INTERVAL",
					"EGL_MAX_SWAP_INTERVAL", "EGL_LUMINANCE_SIZE",
					"EGL_ALPHA_MASK_SIZE", "EGL_COLOR_BUFFER_TYPE",
					"EGL_RENDERABLE_TYPE", "EGL_CONFORMANT" };
			int[] value = new int[1];
			for (int i = 0; i < attributes.length; i++) {
				int attribute = attributes[i];
				String name = names[i];
				if (egl.eglGetConfigAttrib(display, config, attribute, value)) {
					Log.w("orxJAVA", String
							.format("  %s: %d\n", name, value[0]));
				} else {
					// Log.w(TAG, String.format("  %s: failed\n", name));
					while (egl.eglGetError() != EGL10.EGL_SUCCESS)
						;
				}
			}
		}

		// Subclasses can adjust these values:
		protected int mRedSize;
		protected int mGreenSize;
		protected int mBlueSize;
		protected int mAlphaSize;
		protected int mDepthSize;
		protected int mStencilSize;
		private int[] mValue = new int[1];
	}

	// Key events
	public boolean onKey(View v, int keyCode, KeyEvent event) {

		if (event.getAction() == KeyEvent.ACTION_DOWN) {
			AnOrxActivity.onNativeKeyDown(keyCode);
			return false;
		}

		else if (event.getAction() == KeyEvent.ACTION_UP) {
			AnOrxActivity.onNativeKeyUp(keyCode);
			return false;
		}

		return false;
	}

	// Touch events
	public boolean onTouch(View v, MotionEvent event) {

		// TODO: Anything else we need to pass?

		for (int i = 0; i < event.getPointerCount(); i++) {
			int action = -1;
			if (event.getAction() == MotionEvent.ACTION_DOWN)
				action = 0;
			if (event.getAction() == MotionEvent.ACTION_UP)
				action = 2;
			if (event.getAction() == MotionEvent.ACTION_MOVE)
				action = 1;
			if (action >= 0) {
				AnOrxActivity.onNativeTouch(action, event.getPointerId(i),
						event.getX(i), event.getY(i), (float) (event
								.getPressure(i) * 1000.0));

			}
		}

		try {
			Thread.sleep(10);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		return true;
	}

	// Sensor events
	public void enableSensor(int sensortype, boolean enabled) {
		// TODO: This uses getDefaultSensor - what if we have >1 accels?
		if (enabled) {
			mSensorManager.registerListener(this, mSensorManager
					.getDefaultSensor(sensortype),
					SensorManager.SENSOR_DELAY_GAME, null);
		} else {
			mSensorManager.unregisterListener(this, mSensorManager
					.getDefaultSensor(sensortype));
		}
	}

	public void onAccuracyChanged(Sensor sensor, int accuracy) {
		// TODO
	}

	public void onSensorChanged(SensorEvent event) {
		if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) {
			AnOrxActivity.onNativeAccel(-event.values[0], -event.values[1],
					event.values[2]);
		}
	}

}

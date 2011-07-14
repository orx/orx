package org.orx.android;

import android.app.Activity;

public class OrxLib {
	private static String orx_libname = null;
	private static boolean sLoaded = false;
	
	public static void load() {
		if(!sLoaded) {
			if(orx_libname == null) {
				throw new RuntimeException("orx_libname not set");
			}
			System.loadLibrary("apkfile");
			System.loadLibrary(orx_libname);
			sLoaded = true;
		}
	}
	
	public static void setLibName(String name) {
		orx_libname = name;
	}

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void init(Activity activity, int width, int height);
     public static native boolean step();
     public static native void exit();
     public static native void send_orxSYSTEM_EVENT_CLOSE();
     public static native void send_orxSYSTEM_EVENT_BACKGROUND();
     public static native void send_orxSYSTEM_EVENT_FOREGROUND();
     public static native void send_orxDISPLAY_EVENT_SAVE_CONTEXT();
     public static native void send_orxDISPLAY_EVENT_RESTORE_CONTEXT();
     public static native void onNativeTouch(int action,int actionPointer,int pointerCount, int[] uidArray, float[] fXArray, float[] fYArray, float[] fPressionArray);
     public static native void onNativeAccel(float x, float y, float z);
}

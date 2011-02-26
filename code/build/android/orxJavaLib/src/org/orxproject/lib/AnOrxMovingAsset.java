package org.orxproject.lib;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.res.AssetManager;
import android.util.Log;

public class AnOrxMovingAsset {
	private AssetManager assetManager;

	public AnOrxMovingAsset(AssetManager assetManager) {
		this.assetManager = assetManager;
	}

	public void copyDirectory(String assetName, String destDirPath)
			throws IOException {
		File destDir = new File(destDirPath + "/" + assetName);
		if (!destDir.exists()) {
			Log.i("orx app", "build a folder " + destDir.mkdirs());
		} else {
			return;
		}
		Log.i("orx app", "dir Path is " + destDirPath);
		Log.i("orx app", " assetName is " + assetName + " destdir is "
				+ destDir.getAbsolutePath());

		String[] children = assetManager.list(assetName);
		for (String sourceChild : children) {
			// File destChild = new File(destDir, sourceChild);
			Log.i("orxApp",
					"name is "
							+ sourceChild
							+ assetManager.list(assetName + "/" + sourceChild).length);
			if (assetManager.list(assetName + "/" + sourceChild).length != 0) {
				copyDirectory(assetName + "/" + sourceChild, destDirPath);
			} else {
				Log.i("orxApp", "a normal file name is " + sourceChild);
				
				copyFile(assetManager.open(assetName + "/" + sourceChild), new File(destDir.getAbsolutePath()+"/"+sourceChild));
			}
		}
	}

	public static void copyFile(InputStream in, File dest) throws IOException {
		if (!dest.exists()) {
			dest.createNewFile();
		}
		OutputStream out = null;
		try {
			out = new FileOutputStream(dest);
			byte[] buf = new byte[1024];
			int len;
			while ((len = in.read(buf)) > 0) {
				out.write(buf, 0, len);
			}
		} finally {
			in.close();
			out.close();
		}
	}
}

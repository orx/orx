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

	private final static String SDCARD_PATH = "/sdcard";
	private String mAppDirectory;

	public AnOrxMovingAsset(AssetManager assetManager, String appDirectory) {
		this.assetManager = assetManager;
		this.mAppDirectory = appDirectory;
	}

	public boolean isExistedFile(String currentPath) {
		File destDir = new File(SDCARD_PATH + "/" + mAppDirectory + "/"
				+ currentPath);

		return destDir.exists();
	}

	public void copyDirectory(String currentPath) throws IOException {
		File destDir = new File(SDCARD_PATH + "/" + mAppDirectory + "/"
				+ currentPath);
		if (!destDir.exists()) {
			destDir.mkdirs();
		}
		Log.i("Orx app", "copy to " + destDir);

		String[] children = assetManager.list(currentPath);
		for (String sourceChild : children) {
			String assetName;
			if (currentPath == "")
				assetName =  sourceChild;
			else
				assetName = currentPath + "/" + sourceChild;

			if (assetManager.list(assetName).length != 0) {
				copyDirectory(assetName);
			} else {
				// it's a config file ? => copy it
				if (sourceChild.endsWith(".ini")) {
					String sFileName = destDir.getAbsolutePath() + "/"
							+ sourceChild;
					copyFile(assetManager.open(assetName), new File(sFileName));
					Log.d("Moving assets:", "File copied " + sFileName);
				}
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

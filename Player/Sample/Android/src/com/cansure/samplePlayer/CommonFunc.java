/*******************************************************************************
	File:		CommonFunc.java

	Contains:	common func implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.samplePlayer;

import java.io.File;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.os.Build;

public final class CommonFunc {
	public static String model = Build.MODEL;
	public static String product = Build.PRODUCT;

	public static int getAndroidVer() {
		return Build.VERSION.SDK_INT;
	}

	public static boolean checkSDCard() {
		if (android.os.Environment.getExternalStorageState().equals(
				android.os.Environment.MEDIA_MOUNTED)) {
			return true;
		} else {
			return false;
		}
	}

	public static int checkFileExt(String str) {
		Pattern audioPattern = Pattern
				.compile(
						".+\\.(mp3|amr|aac|wma|m4a|wav|ec3|ac3|mp2|ogg|ra|isma|flac|evrc|qcelp|pcm|adpcm|au|awb)$",
						Pattern.CASE_INSENSITIVE);
		Pattern videoPattern = Pattern
				.compile(
						".+\\.(avi|asf|rm|rmvb|mp4|m4v|3gp|3g2|wmv|3g2|mpg|mpeg|qt|mkv|flv|mov|asx|m3u8|m3u|manifest|mpd|ts|webm|ismv|ismc|k3g|sdp)$",
						Pattern.CASE_INSENSITIVE);

		Matcher matcher = audioPattern.matcher(str);
		if (matcher.find()) {
			return R.drawable.audio;
		}

		matcher = videoPattern.matcher(str);
		if (matcher.matches()) {
			return R.drawable.video;
		}

		String strTest = str.toLowerCase();

		if (strTest.startsWith(Definition.PREFIX_MEDIAFILE_MTV) == true)
			return R.drawable.video;

		if (strTest.endsWith("/manifest") == true)
			return R.drawable.video;

		if (strTest.contains("/manifest?") == true)
			return R.drawable.video;

		if (strTest.contains("m3u8?") == true)
			return R.drawable.video;

		if (strTest.contains("m3u?") == true)
			return R.drawable.video;

		int index = strTest.lastIndexOf("_");
		if (index != -1) {
			strTest = strTest.substring(index);
			if (strTest.length() == 5) {

				if (isNumeric(strTest.substring(1))) {
					return R.drawable.video;
				}
			}
		}

		return R.drawable.folder;
	}

	public static boolean isNumeric(String str) {
		Pattern pattern = Pattern.compile("[0-9]*");
		Matcher isNum = pattern.matcher(str);
		if (!isNum.matches()) {
			return false;
		}
		return true;
	}

	public static boolean isFileExist(String filename) {
		File file = new File(filename);
		return file.exists();
	}

	public static boolean isLink(String url) {
		boolean bLink = false;

		int index = url.lastIndexOf(".");
		if (index == -1)
			return false;

		String strUrlSuffix = url.substring(index);

		if (strUrlSuffix.equalsIgnoreCase(Definition.SUFFIX_HTM)
				|| strUrlSuffix.equalsIgnoreCase(Definition.SUFFIX_HTML)
				|| strUrlSuffix.equalsIgnoreCase(Definition.SUFFIX_PHP)
				|| strUrlSuffix.equalsIgnoreCase(Definition.SUFFIX_ASP)
				|| strUrlSuffix.equalsIgnoreCase(Definition.SUFFIX_ASPX)) {
			bLink = true;
		}

		return bLink;
	}
}

/*******************************************************************************
	File:		ExtData.java

	Contains:	ExtData wrap implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-12-24		Fenger			Create file

*******************************************************************************/
package com.cansure.ExtData;

import android.os.Message;
import android.util.Log;

public class ExtData {
	private static final String TAG = "YYExtData";
	
    private int 				m_NativeContext = 0;; 
    private int					m_ExtDataAPI = 0;;    

    private native int nativeGetAPI(Object player, String apkPath);     
    private native int nativeFree(int nNativeContext);
	 
	static {
		System.loadLibrary("yyExtDataJNI");
	}

	public int Init(Object player, String apkPath) {
		int nRet = nativeGetAPI (player, apkPath);
		return nRet;
	}

	public void Uninit() {
		nativeFree (m_NativeContext);
	}
}

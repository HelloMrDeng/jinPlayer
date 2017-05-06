/*******************************************************************************
	File:		BasePlayer.java

	Contains:	player interface file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-12-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.content.Context;
import android.view.SurfaceView;
import android.graphics.Bitmap;

public interface BasePlayer {	
	// Define id of event listener.
	public static final int 	YY_EV_Open_Complete 		= 0x01;
	public static final int 	YY_EV_Open_Failed 			= 0x02;
	public static final int 	YY_EV_Play_Complete 		= 0x05;
	public static final int 	YY_EV_Play_Status	 		= 0x06;
	public static final int 	YY_EV_Play_Duration 		= 0x07;			
	
	// The video size was changed. The player need to resize the 
	// surface pos and size.
	public static final int 	YY_EV_VideoFormat_Change 	= 0x21;
	// The audio format was changed.
	public static final int 	YY_EV_AudioFormat_Change 	= 0x22;
	

	// The event listener function
	public interface onEventListener{
		public int onEvent (int nID, Object obj);
	}
	
	// Define the functions
	public int 		Init(Context context, String apkPath);
	public void 	setEventListener(onEventListener listener);	
	public int 		Open (String strPath);
	public void 	Play();
	public void 	Pause();
	public void 	Stop();	
	public long 	GetDuration();	
	public int 		GetPos();
	public void 	SetPos(int nPos);
	public int	 	GetParam(int nParamId, Object objParam);
	public int	 	SetParam(int nParamId, int nParam, Object objParam);
	public void 	Uninit();
	public void 	SetVolume(float left, float right);
}



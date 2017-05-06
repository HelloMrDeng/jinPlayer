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
	// define the param id
	public static final int 	PARAM_PID_EVENT_DONE		= 0X100;
	// 0 for default (YUV or RGB), 1 force to use RGB8888
	public static final int 	PARAM_PID_COLOR_FORMAT		= 0X400;	
	public static final int 	PARAM_PID_VIDEODEC_MODE		= 0X405;
	public static final int 	PARAM_PID_SunTT_Enable		= 0X410;	
	// 1, UTF-8, 2, GB2312, 3....
	public static final int 	PARAM_PID_SunTT_Charset		= 0X411;	
	// set the view bitmap handle 
	public static final int		PARAM_PID_BITMAP_VIDEO		= 0X500;
	public static final int		PARAM_PID_BITMAP_DRAW		= 0X501;
	
	// Define id of event listener.
	public static final int 	YY_EV_Open_Complete 		= 0x01;
	public static final int 	YY_EV_Open_Failed 			= 0x02;
	public static final int 	YY_EV_Play_Complete 		= 0x05;
	public static final int 	YY_EV_Play_Status	 		= 0x06;
	public static final int 	YY_EV_Play_Duration 		= 0x07;	
	// The first frame video was displayed.
	public static final int 	YY_EV_Draw_FirstFrame 		= 0x10;
	
	public static final int		YY_EV_Create_ExtVD			= 0x11;
	public static final int		YY_EV_Create_ExtAD			= 0x12;
		
	
	// The video size was changed. The player need to resize the 
	// surface pos and size.
	public static final int 	YY_EV_VideoFormat_Change 	= 0x21;
	// The audio format was changed.
	public static final int 	YY_EV_AudioFormat_Change 	= 0x22;
	
	// The audio format was changed.
	public static final int 	YY_EV_Subtitle_Text		 	= 0x102;
	
	// video dec mode value
	public static final int		YY_VDMODE_Auto				= 0XFF;
	public static final int		YY_VDMODE_Soft				= 0X01;
	public static final int		YY_VDMODE_IOMX				= 0X02;
	public static final int		YY_VDMODE_MediaCodec		= 0X04;

	// The event listener function
	public interface onEventListener{
		public int onEvent (int nID, Object obj);
		public int OnSubTT (String strText, int nTime);	
	}
	
	// Define the functions
	public int 		Init(Context context, String apkPath);
	public void 	setEventListener(onEventListener listener);	
	public void 	SetView(SurfaceView sv);
	public int 		Open (String strPath);
	public int 		OpenExt (int nExtData, int nFlag);
	public void 	Play();
	public void 	Pause();
	public void 	Stop();	
	public long 	GetDuration();	
	public int 		GetPos();
	public void 	SetPos(int nPos);
	public int	 	GetParam(int nParamId, Object objParam);
	public int	 	SetParam(int nParamId, int nParam, Object objParam);
	public int	 	GetThumb(String strFile, int nWidth, int nHeight, Bitmap objBitmap);	
	public int		ReadVideo (byte[] pData);
	public int		ReadAudio (byte[] pData);
	public void 	Uninit();
	
	public int 		GetVideoWidth();
	public int 		GetVideoHeight();
	public long		GetVideoTime ();
	public int		GetVideoFlag ();
	public int		SetVideoFlag (int nFlag);
	public void 	SetVolume(float left, float right);
}



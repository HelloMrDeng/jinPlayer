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


public interface BasePlayer {
	// Define id of event listener.
	public static final int 	YY_EV_Open_Complete 		= 0x01;
	public static final int 	YY_EV_Open_Failed 			= 0x02;
	public static final int 	YY_EV_Play_Complete 		= 0x05;
	// The video size was changed. The player need to resize the 
	// surface pos and size.
	public static final int 	YY_EV_VideoFormat_Change 	= 0x06;
	// The audio format was changed.
	public static final int 	YY_EV_AudioFormat_Change 	= 0x07;
	// The first frame video was displayed.
	public static final int 	YY_EV_Draw_FirstFrame 		= 0x10;
	
	// The event listener function
	public interface onEventListener{
		public int onEvent(int nID, Object obj);
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
	public void 	Uninit();
	
	public int 		GetVideoWidth();
	public int 		GetVideoHeight();
	public void 	SetVolume(float left, float right);
}

/*
public void onVideoSizeChanged (int width, int height) 
{	
	if (m_nSurfaceWidth == width && m_nSurfaceHeight == height)
		return;

	Log.v(TAG, "Video Size width = " + width + ", height = " + height);
	if ((m_nSurfaceWidth == 0 && width > 0) || m_nSurfaceWidth * height != width * m_nSurfaceHeight)
	{
		RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams)m_svMain.getLayoutParams();
		DisplayMetrics dm  = new DisplayMetrics();
		getWindowManager().getDefaultDisplay().getMetrics(dm);
		if (width != 0 && height != 0 && lp.width == LayoutParams.FILL_PARENT && lp.height == LayoutParams.FILL_PARENT) 
		{
			int nMaxOutW = dm.widthPixels;
			int nMaxOutH = dm.heightPixels;
			int w = 0, h = 0;
			if (width != 0 && height != 0) 
			{
				nMaxOutW &= ~0x3;
				nMaxOutH &= ~0x1;

				if (nMaxOutW * height > width * nMaxOutH)
				{
					h = nMaxOutH;
					w = width * h / height;
				}
				else
				{
					w = nMaxOutW;
					h = height * w / width;
				}
				w &= ~0x3;
				h &= ~0x1;
				
				lp.width = w;
				lp.height = h;
			}

			m_svMain.setLayoutParams(lp);
			Log.v(TAG, String.format("setSurfaceSize width = %d, height = %d", lp.width , lp.height));
		}
	}

	m_nSurfaceWidth = width;
	m_nSurfaceHeight = height;

	if (m_nSurfaceWidth != 0 && m_nSurfaceHeight != 0) 
	{
		m_shMain.setFixedSize(m_nSurfaceWidth, m_nSurfaceHeight);
		m_svMain.setVisibility(View.VISIBLE); 
	}   
}
*/
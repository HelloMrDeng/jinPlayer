/*******************************************************************************
	File:		BasePlayer.java

	Contains:	player wrap implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.os.Handler;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.graphics.PixelFormat;
import android.util.Log;

public class BasePlayer {
	private static final String TAG = "YYLOGBasePlayer";
	
	public static final int 	YY_EV_Open_Complete 		= 0x01;
	public static final int 	YY_EV_Open_Failed 			= 0x02;
	public static final int 	YY_EV_Play_Complete 		= 0x05;
	public static final int 	YY_EV_VideoFormat_Change 	= 0x06;
	public static final int 	YY_EV_Draw_FirstFrame 		= 0x10;
	
	public static final int 	PARAM_PID_SURFACE_CHANGED	= 0X100;
	public static final int 	PARAM_PID_VIEW_ACTIVE		= 0X200;
	public static final int 	PARAM_PID_AUDIO_OFFSEST		= 0X300;
	public static final int 	PARAM_PID_TRY_CLOSE			= 0X500;
	
    private int 				m_NativeContext;     
    private Surface 			m_NativeSurface;
    private SurfaceView 		m_SurfaceView;

    private int					m_nVideoWidth = 0;
    private int					m_nVideoHeight = 0;
    
	private int 				m_nSampleRate = 0;
	private int		 			m_nChannels = 0;	

	private AudioRender			m_AudioRender;
	private VideoRender			m_VideoRender;	
	private EventCheck			m_EventCheck;	
	
	private onEventListener 	m_EventListener;

	public interface onEventListener{
		public int onEvent(int nID, Object obj);
	}

    private native int nativeInit(Object player, String apkPath);     
    private native int nativeUninit(int nNativeContext);
    private native int nativeOpen(int nNativeContext,String strPath);  
    private native int nativeOpenExt(int nNativeContext, int nExtData, int nFlag);  
    private native int nativePlay(int nNativeContext);    
    private native int nativePause(int nNativeContext);   
    private native int nativeStop(int nNativeContext);     
    private native int nativeGetPos(int nNativeContext);    
    private native int nativeSetPos(int nNativeContext,int nPos);
    private native long nativeGetDuration(int nNativeContext);
	private native long nativeGetVideoData (int nNativeContext, byte[] data);	
	private native long nativeGetAudioData (int nNativeContext, byte[] data);	
	private native long nativeGetEventStatus (int nNativeContext);		
    private native int nativeGetParam(int nNativeContext,int nParamId, Object objParam);
    private native int nativeSetParam(int nNativeContext,int nParamId, Object objParam);
	 
	private Handler mHandle = new Handler() 
	{
		@Override
		public void handleMessage(Message msg) 
		{
			if (m_EventListener != null)
				m_EventListener.onEvent(msg.what, msg.obj);
		}
	};

	static {
		System.loadLibrary("yyMediaJNI");
	}

	public void setEventListener(onEventListener listener) {
		m_EventListener = listener;
	}

	public int Init(Object player, String apkPath) {
		int nRet = nativeInit(player, apkPath);

		m_EventCheck = new EventCheck (this);
		
		return nRet;
	}
	
	public void SetView(SurfaceView sv) {
		m_SurfaceView = sv;

		if (sv != null)
		{
			SurfaceHolder sh = sv.getHolder();
			m_NativeSurface = sh.getSurface();
		
			sh.setFormat(PixelFormat.RGBA_8888);				
		}
		else
		{
			m_NativeSurface = null;
		}
	}

	public int Open (String strPath) {
		
		if (m_EventCheck != null)
			m_EventCheck.run ();	
		
		int nRet = nativeOpen (m_NativeContext, strPath);

		return nRet;
	}
	
	public void Stop() 
	{
		nativeSetParam (m_NativeContext, PARAM_PID_TRY_CLOSE, 0);
		
		if (m_VideoRender != null)
			m_VideoRender.stop();
		if (m_AudioRender != null)
			m_AudioRender.stop();
		
		nativeStop(m_NativeContext);			
	}

	public void Uninit() {
		
		if (m_VideoRender != null)
			m_VideoRender.stop();
		if (m_AudioRender != null)
			m_AudioRender.stop();
		if (m_EventCheck != null)
			m_EventCheck.stop ();
		
		nativeUninit(m_NativeContext);
		
		m_VideoRender = null;
		m_AudioRender = null;
		m_EventCheck = null;
	}

	public long GetDuration() {
		return nativeGetDuration(m_NativeContext);
	}

	public void Play() {
		nativePlay(m_NativeContext);
		
		if (m_AudioRender != null)
			m_AudioRender.run ();
		if (m_VideoRender != null)
			m_VideoRender.run();
	}
	
	public void Pause() {	
		
		if (m_VideoRender != null)
			m_VideoRender.pause();
		if (m_AudioRender != null)
			m_AudioRender.pause ();		
		
		nativePause(m_NativeContext);
	}

	public int GetPos() {
		return nativeGetPos(m_NativeContext);
	}

	public void SetPos(int nPos) {
		nativeSetPos(m_NativeContext, nPos);
	}

	public Object GetParam(int nParamId) {
		Integer objParam = Integer.valueOf(1);
		int nRet = nativeGetParam(m_NativeContext, nParamId, objParam);
		if (nRet != 0)
			return null;

		return objParam;
	}

	public void SetParam(int nParamId, Object objParam) {
		nativeSetParam(m_NativeContext, nParamId, objParam);
	}

	public long GetVideoData(byte[] data) {
		return nativeGetVideoData(m_NativeContext, data);
	}

	public long GetAudioData(byte[] data) {
		return nativeGetAudioData(m_NativeContext, data);
	}

	public long GetEventStatus () {
		return nativeGetEventStatus (m_NativeContext);
	}
	
	public int GetVideoWidth() {
		return m_nVideoWidth;
	}

	public int GetVideoHeight() {
		return m_nVideoHeight;
	}
	
	public int GetAudioSampleRate() {
		return m_nSampleRate;
	}

	public int GetAudioChannels() {
		return m_nChannels;
	}
	
	public int HasVideo () {
		if (m_nVideoWidth <= 0)
			return 0;
		
		if (m_VideoRender != null)
		{
			if (m_VideoRender.IsEOS () > 0 && m_VideoRender.GetRnd () < 10)
				return 0;
		}
		
		
		return 1;
	}

	public int onEventCallback(int nID, Object objParam) 
	{
		if (nID == YY_EV_Open_Complete)
		{
			if (m_nVideoWidth > 0 && m_nVideoHeight > 0)
				m_VideoRender = new VideoRender(this);
			
			if (m_nSampleRate > 0 && m_nChannels > 0)
				m_AudioRender = new AudioRender(this);
			
			if (m_AudioRender != null)
				m_AudioRender.openTrack(m_nSampleRate, m_nChannels, 0, 0);
		}
		else  if (nID == YY_EV_Play_Complete)
		{
			if (HasVideo () <= 0 )
			{
				SetPos (0);
				return 0;
			}
		}

		Message msg = mHandle.obtainMessage(nID, objParam);
		msg.sendToTarget();
		return 0;
	}
}

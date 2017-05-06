/*******************************************************************************
	File:		MediaPlayer.java

	Contains:	player wrap implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.content.Context;
import android.graphics.Bitmap;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

import java.io.UnsupportedEncodingException;
import java.lang.ref.WeakReference;
import android.util.Log;

public class MediaPlayer implements BasePlayer {
	private static final String TAG = "YYLOGMediaPlayer";
	
	private Context				m_context = null;
    private int 				m_NativeContext = 0;     
    private Surface 			m_NativeSurface = null;
    private SurfaceView 		m_SurfaceView = null;

    private int					m_nVideoWidth = 0;
    private int					m_nVideoHeight = 0;
    
	private int 				m_nSampleRate = 0;
	private int		 			m_nChannels = 0;	

	private AudioRender			m_AudioRender = null;
	private onEventListener 	m_EventListener = null;

	static {
		System.loadLibrary("yyMediaJNI");
	}

	public int Init(Context context, String apkPath) {
		m_context = context;
		m_NativeContext =  nativeInit(new WeakReference<MediaPlayer>(this), apkPath);
		if (m_NativeContext == 0)
			return -1;
	
		if (m_NativeSurface != null)
			nativeSetView (m_NativeContext, m_NativeSurface);
		
		return 0;
	}
	
	public void setEventListener(onEventListener listener) {
		m_EventListener = listener;
	}
	
	public void SetView(SurfaceView sv) {
		m_SurfaceView = sv;
		if (sv != null)
		{
			SurfaceHolder sh = sv.getHolder();
			m_NativeSurface = sh.getSurface();					
		}
		else
		{
			m_NativeSurface = null;
		}	
		
		if (m_NativeContext != 0)
			nativeSetView (m_NativeContext, m_NativeSurface);		
	}

	public int Open (String strPath) {
		return nativeOpen (m_NativeContext, strPath);
	}
	
	public int OpenExt (int nExtData, int nFlag) {
		return nativeOpenExt (m_NativeContext, nExtData, nFlag);
	}

	public void Play() {
		nativePlay(m_NativeContext);
	}
	
	public void Pause() {			
		nativePause(m_NativeContext);		
	}

	public void Stop() {	
		nativeStop(m_NativeContext);			
	}
	
	public long GetDuration() {
		return nativeGetDuration(m_NativeContext);
	}
	
	public int GetPos() {
		return nativeGetPos(m_NativeContext);
	}

	public void SetPos(int nPos) {
		nativeSetPos(m_NativeContext, nPos);
	}

	public int GetParam(int nParamId, Object objParam) {
		return nativeGetParam(m_NativeContext, nParamId, objParam);
	}

	public int SetParam(int nParamId, int nParam, Object objParam) {
		return nativeSetParam(m_NativeContext, nParamId, nParam, objParam);
	}

	public int GetThumb(String strFile, int nWidth, int nHeight, Bitmap objBitmap) {
		return nativeGetThumb (m_NativeContext, strFile, nWidth, nHeight, objBitmap);
	}
	
	public void Uninit() {		
		if (m_AudioRender != null)
			m_AudioRender.closeTrack();
		
		nativeUninit(m_NativeContext);
		m_AudioRender = null;
	}
	
	public int GetVideoWidth() {
		return m_nVideoWidth;
	}

	public int GetVideoHeight() {
		return m_nVideoHeight;
	}
	
	public void SetVolume(float left, float right){
		if (m_AudioRender != null)
			m_AudioRender.SetVolume (left, right);
	}
	
	private static void postEventFromNative(Object baselayer_ref, int what, int ext1, int ext2, Object obj)
	{
		Log.v(TAG, "postEventFromNative this id is  " + what);
		
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;
		
		if (what == YY_EV_VideoFormat_Change)
		{
			player.m_nVideoWidth = ext1;
			player.m_nVideoHeight = ext2;
		}		
		else if (what == YY_EV_AudioFormat_Change)
		{
			player.m_nSampleRate = ext1;
			player.m_nChannels = ext2;
			
			if (player.m_AudioRender == null)	
				player.m_AudioRender = new AudioRender(player.m_context, player);
			player.m_AudioRender.openTrack (player.m_nSampleRate, player.m_nChannels);
			return;
		}
			
		Message msg = player.mHandle.obtainMessage(what, obj);
		msg.sendToTarget();	
	}
		
	private static void audioDataFromNative(Object baselayer_ref, byte[] data, int size)
	{
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;
		
		player.m_AudioRender.writeData(data,  size);
	}	

	private static void textSubtitleFromNative(Object baselayer_ref, String text, int size, int nDur)
	{
		Log.v ("@@@YYLOG", "000 The Text is " + text + " size " + text.length());	
		
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;
		
		Log.v ("@@@YYLOG", "The Text is " + text);
		
		if (player.m_EventListener != null)
			player.m_EventListener.onSubtitle (text, size, nDur);
	}	
	
	private static void textBytesFromNative(Object baselayer_ref, byte[] data, int size, int nDur)
	{	
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;
		
		String strEncode = "GB2312";
	
		String strText = null;
		try {
			strText = new String (data, strEncode);
		} catch (UnsupportedEncodingException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		
		Log.v ("@@@YYLOG", "The Text is " + strText);
		
		if (player.m_EventListener != null)
			player.m_EventListener.onSubtitle (strText, size, nDur);
	}		
	
	private Handler mHandle = new Handler() 
	{
		@Override
		public void handleMessage(Message msg) 
		{
			if (m_EventListener != null)
				m_EventListener.onEvent(msg.what, msg.obj);
		}
	};
	
	// the native functions
    private native int 	nativeInit(Object player, String apkPath);     
    private native int 	nativeUninit(int nNativeContext);
    private native int 	nativeSetView(int nNativeContext, Object view);  
    private native int 	nativeOpen(int nNativeContext,String strPath);  
    private native int 	nativeOpenExt (int nNativeContext, int nExtData, int nFlag);  
    private native int 	nativePlay(int nNativeContext);    
    private native int 	nativePause(int nNativeContext);   
    private native int 	nativeStop(int nNativeContext);     
    private native int 	nativeGetPos(int nNativeContext);    
    private native int 	nativeSetPos(int nNativeContext,int nPos);
    private native long nativeGetDuration(int nNativeContext);	
    private native int 	nativeGetParam(int nNativeContext,int nParamId, Object objParam);
    private native int 	nativeSetParam(int nNativeContext,int nParamId, int nParam, Object objParam);
    private native int 	nativeGetThumb(int nNativeContext,String strFile, int nWidth, int nHeight, Object objBitmap);	
}

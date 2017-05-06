/*******************************************************************************
	File:		MediaPlayer.java

	Contains:	player wrap implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuffXfermode;
import android.graphics.PorterDuff;

import java.lang.ref.WeakReference;

import android.util.DisplayMetrics;
import android.util.Log;
import java.io.UnsupportedEncodingException;

public class MediaPlayer implements BasePlayer {
	private static final String TAG = "YYLOGMediaPlayer";
	
	private Context				m_context = null;
    private int 				m_NativeContext = 0;     

	private int 				m_nSampleRate = 0;
	private int		 			m_nChannels = 0;	

	private AudioRender			m_AudioRender = null;
	private onEventListener 	m_EventListener = null;

	static {
		System.loadLibrary("yyRockVJNI");
	}

	public int Init(Context context, String apkPath) {
		m_context = context;
		m_NativeContext =  nativeInit(new WeakReference<MediaPlayer>(this), apkPath);
		if (m_NativeContext == 0)
			return -1;
		return 0;
	}
	
	public void setEventListener(onEventListener listener) {
		m_EventListener = listener;
	}

	public int Open (String strPath) {
		return nativeOpen (m_NativeContext, strPath);
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
	
	public int GetItemCount () {
		return nativeGetItemCount (m_NativeContext);
	}
	
	public int SetItemSel (int nItem) {
		return nativeSelectItem (m_NativeContext, nItem);
	}

	public int GetTrackCount () {
		return nativeGetTrackCount (m_NativeContext);
	}
	
	public int GetTrackVolume (int nTrack) {
		return nativeGetTrackVolume (m_NativeContext, nTrack);
	}
	
	public int SetTrackVolume (int nTrack, int nVolume) {
		return nativeSetTrackVolume (m_NativeContext, nTrack, nVolume);
	}	

	public int GetTrackEnable (int nTrack) {
		return nativeGetTrackEnable (m_NativeContext, nTrack);
	}
	
	public int SetTrackEnable (int nTrack, int nEnable) {
		return nativeSetTrackEnable (m_NativeContext, nTrack, nEnable);
	}	

	public int GetRepeatCount () {
		return nativeGetRepeatCount (m_NativeContext);
	}
	
	public int GetRepeatSel () {
		return nativeGetRepeatSelect (m_NativeContext);
	}
	
	public int SetRepeatSel (int nIndex) {
		return nativeSetRepeatSelect (m_NativeContext, nIndex);
	}
	
	public int GetParam(int nParamId, Object objParam) {
		return nativeGetParam(m_NativeContext, nParamId, objParam);
	}

	public int SetParam(int nParamId, int nParam, Object objParam) {
		return nativeSetParam(m_NativeContext, nParamId, nParam, objParam);
	}	

	public void Uninit() {		
		if (m_AudioRender != null)
			m_AudioRender.closeTrack();	
		nativeUninit(m_NativeContext);
		m_AudioRender = null;
	}
	
	public void SetVolume(float left, float right){
		if (m_AudioRender != null)
			m_AudioRender.SetVolume (left, right);
	}
	
	private static void postEventFromNative(Object baselayer_ref, int what, int ext1, int ext2, Object obj) {
		Log.v(TAG, "postEventFromNative this id is  " + what);
		
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;
		
		if (what == YY_EV_AudioFormat_Change) {
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
		
	private static void audioDataFromNative(Object baselayer_ref, byte[] data, int size) {
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;		
		player.m_AudioRender.writeData(data,  size);
	}
	
	private Handler mHandle = new Handler()  {
		public void handleMessage(Message msg) {			
			if (m_EventListener != null) {
				m_EventListener.onEvent(msg.what, msg.obj);	
			}
		}
	};
	
	// the native functions
    private native int 	nativeInit(Object player, String apkPath);     
    private native int 	nativeUninit(int nNativeContext);
    private native int 	nativeOpen(int nNativeContext,String strPath);  
    private native int 	nativePlay(int nNativeContext);    
    private native int 	nativePause(int nNativeContext);   
    private native int 	nativeStop(int nNativeContext);     
    private native int 	nativeGetPos(int nNativeContext);    
    private native int 	nativeSetPos(int nNativeContext,int nPos);
    private native long nativeGetDuration(int nNativeContext);	
    private native int 	nativeGetItemCount(int nNativeContext);   
    private native int 	nativeSelectItem(int nNativeContext,int nItem);   
    private native int 	nativeGetTrackCount(int nNativeContext);   
    private native int 	nativeGetTrackVolume(int nNativeContext,int nIndex);   
    private native int 	nativeSetTrackVolume(int nNativeContext,int nIndex,int nVolume);   
    private native int 	nativeGetTrackEnable(int nNativeContext,int nIndex);   
    private native int 	nativeSetTrackEnable(int nNativeContext,int nIndex,int nVolume);    
    private native int 	nativeGetRepeatCount(int nNativeContext);     
    private native int 	nativeGetRepeatSelect(int nNativeContext); 
    private native int 	nativeSetRepeatSelect(int nNativeContext,int nSel);    
    private native int 	nativeGetParam(int nNativeContext,int nParamId, Object objParam);
    private native int 	nativeSetParam(int nNativeContext,int nParamId, int nParam, Object objParam);   
}





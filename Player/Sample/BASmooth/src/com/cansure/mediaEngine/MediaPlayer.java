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
	// The draw the video on view in main thread
	public static final int 	YY_EV_Draw_Video	 	= 0x1001;
	
	private Context				m_context = null;
    private int 				m_NativeContext = 0;     
    private Surface 			m_NativeSurface = null;
    private SurfaceView 		m_SurfaceView = null;
    private BitmapView			m_VideoView = null;
    private Bitmap				m_bmpVideo = null;
    private Bitmap				m_bmpDraw = null;

    private boolean				m_bIsStream = false;
    private boolean				m_bExtVD = false;
    private int					m_nVideoFlag = 0;
    private int					m_lVideoTime = 0;
    private int					m_nVideoWidth = 0;
    private int					m_nVideoHeight = 0;
    
	private int 				m_nSampleRate = 0;
	private int		 			m_nChannels = 0;	
	
	private String				m_strSubTT = null;
	private String				m_strEncode = null;	
	private int					m_nSubTTDur = 0;

	private AudioRender			m_AudioRender = null;
	private MCVideoRender		m_VideoRender = null;
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
		if (sv != null) {
			SurfaceHolder sh = sv.getHolder();
			m_NativeSurface = sh.getSurface();					
		} else {
			m_NativeSurface = null;
		}		
		if (m_NativeContext != 0)
			nativeSetView (m_NativeContext, m_NativeSurface);		
	}
	
	public void SetVideoView (BitmapView vVideo) {	
		m_VideoView = vVideo;
		m_VideoView.setPlayer(this);
	}

	public int Open (String strPath) {
		m_bExtVD = false;
		m_bIsStream = false;
		String strProt = strPath.substring(0, 4);
		String strHttp = "http";
		if (strProt.compareToIgnoreCase(strHttp) == 0)
			m_bIsStream = true;
		return nativeOpen (m_NativeContext, strPath);
	}
	
	public int OpenExt (int nExtData, int nFlag) {
		m_bExtVD = false;
		return nativeOpenExt (m_NativeContext, nExtData, nFlag);
	}

	public void Play() {
		nativePlay(m_NativeContext);
		if (m_VideoRender != null)
			m_VideoRender.start();		
	}
	
	public void Pause() {			
		nativePause(m_NativeContext);		
	}

	public void Stop() {	
		if (m_VideoRender != null)
			m_VideoRender.stop();
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
	
	public int ReadVideo (byte[] pBuff) {
		m_nVideoFlag = 0;
		m_lVideoTime = 0;
		return nativeReadVideo (m_NativeContext, pBuff);
	}
	
	public int ReadAudio (byte[] pBuff) {
		return nativeReadAudio (m_NativeContext, pBuff);
	}	

	public void Uninit() {		
		if (m_AudioRender != null)
			m_AudioRender.closeTrack();	
		if (m_VideoRender != null)
			m_VideoRender.stop();
		nativeUninit(m_NativeContext);
		m_AudioRender = null;
		m_VideoView.SetBitmap(null);
		if (m_bmpVideo != null) {
			m_bmpVideo.recycle();
			m_bmpVideo = null;
		}
	}
	
	public int GetVideoWidth() {
		return m_nVideoWidth;
	}

	public int GetVideoHeight() {
		return m_nVideoHeight;
	}
	
	public long GetVideoTime () {
		return m_lVideoTime;
	}
	
	public int GetVideoFlag () {
		return m_nVideoFlag;
	}
	
	public int SetVideoFlag (int nFlag) {
		m_nVideoFlag = nFlag;
		return 0;
	}
	
	public void SetVolume(float left, float right){
		if (m_AudioRender != null)
			m_AudioRender.SetVolume (left, right);
	}
	
	public void OnOpenComplete () {
		if (!m_bExtVD)
			return;
		if (m_VideoRender == null)
			m_VideoRender = new MCVideoRender (m_context, this);
		m_VideoRender.setSurface(m_NativeSurface);				
	}
	
	public void drawVideo (boolean bDraw) {		
		if (bDraw) {
			if (m_bmpDraw == null)
				return ;			
			m_VideoView.SetBitmap(m_bmpVideo);
			m_VideoView.invalidate();
			m_bmpDraw = null;			
		} else {
			m_VideoView.SetBitmap(null);			
		}
		if (bDraw)
		{
			if (!m_VideoView.isShown())
				m_VideoView.setVisibility(View.VISIBLE);
		}
		else
		{
			if (m_VideoView.isShown())
				m_VideoView.setVisibility(View.INVISIBLE);	
		}
		return;
	}

	public void onVideoSizeChanged () {	
		RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams)m_SurfaceView.getLayoutParams();
		DisplayMetrics dm = m_context.getResources().getDisplayMetrics();
		if (m_nVideoWidth != 0 && m_nVideoHeight != 0 && lp.width == LayoutParams.FILL_PARENT && lp.height == LayoutParams.FILL_PARENT) 
		{
			int nMaxOutW = dm.widthPixels;
			int nMaxOutH = dm.heightPixels;
			if (nMaxOutW * m_nVideoHeight > m_nVideoWidth * nMaxOutH) {
				lp.height = nMaxOutH;
				lp.width  = m_nVideoWidth * nMaxOutH / m_nVideoHeight;
			} else {
				lp.width  = nMaxOutW;
				lp.height = m_nVideoHeight * nMaxOutW / m_nVideoWidth;
			}	
			//lp.width = lp.width * 2;
			//lp.height = lp.height * 2;		
			m_SurfaceView.setLayoutParams(lp);	
			m_VideoView.setLayoutParams(lp);
			Log.v("PlayerView", String.format("setSurfaceSize width = %d, height = %d", lp.width , lp.height));			
		}
		if (m_nVideoWidth != 0 && m_nVideoHeight != 0) {
			if (m_bmpVideo != null) {
				m_bmpVideo.recycle ();
				m_bmpVideo = null;
			}
			if (m_bIsStream) {
				m_bmpVideo = Bitmap.createBitmap(m_nVideoWidth, m_nVideoHeight, Bitmap.Config.ARGB_8888);	
				SetParam (PARAM_PID_BITMAP_VIDEO, 0, m_bmpVideo);	
			}
		}
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
	
	private static int videoDataFromNative(Object baselayer_ref, Object obj)
	{
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return -1;	
		if (player.m_bmpDraw != null)
			return 1;
		player.m_bmpDraw = (Bitmap)obj;
		Message msg = player.mHandle.obtainMessage(YY_EV_Draw_Video, obj);
		msg.sendToTarget();	
		return 0;
	}	
	
	private static void textBytesFromNative(Object baselayer_ref, byte[] data, int size, int nDur)
	{	
		MediaPlayer player = (MediaPlayer)((WeakReference)baselayer_ref).get();
		if (player == null) 
			return;
		
		if (player.m_strEncode == null) {
			int nCharset = player.nativeGetParam (player.m_NativeContext, PARAM_PID_SunTT_Charset, null);
			if (nCharset == 1)
				player.m_strEncode = "UTF-8";
			else if (nCharset == 2)
				player.m_strEncode = "GB2312";
			else
				player.m_strEncode = "UTF-8";	
		}

		player.m_nSubTTDur = nDur;
		player.m_strSubTT = null;
		if (size > 0) {
			try {
				player.m_strSubTT = new String (data, player.m_strEncode);
			} catch (UnsupportedEncodingException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}	
		}
		//player.m_EventListener.OnSubTT (strText, nDur);
		Message msg = player.mHandle.obtainMessage(YY_EV_Subtitle_Text, 0);
		msg.sendToTarget();			
	}
	
	private Handler mHandle = new Handler()  
	{
		public void handleMessage(Message msg) 
		{	
			int nRC = 0;
			if (msg.what == YY_EV_Create_ExtVD) {
				m_bExtVD = true;
				return;
			} else if (msg.what == YY_EV_Open_Complete) {
				OnOpenComplete ();
			}
				
			if (m_EventListener != null) {
				if (msg.what == YY_EV_Subtitle_Text)
					m_EventListener.OnSubTT(m_strSubTT, m_nSubTTDur);
				else
					nRC = m_EventListener.onEvent(msg.what, msg.obj);	
			}

			if (msg.what == YY_EV_VideoFormat_Change && nRC == 0) 
				onVideoSizeChanged ();	
			else if (msg.what == YY_EV_Draw_Video) {
				if (msg.obj == null)
					drawVideo (false);
				else
					drawVideo (true);	
			}
			if (msg.what == YY_EV_VideoFormat_Change)
				SetParam (PARAM_PID_EVENT_DONE, 0, null);
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
    private native int 	nativeReadVideo (int nNativeContext, byte[] pBuff);
    private native int 	nativeReadAudio (int nNativeContext, byte[] pBuff);    
}

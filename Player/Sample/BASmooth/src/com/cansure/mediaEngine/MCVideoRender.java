/*******************************************************************************
	File:		MCVideoRender.java

	Contains:	Video media codec render implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-09-23		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import java.nio.ByteBuffer;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.Surface;

import android.media.MediaCodec;
import android.media.MediaCodec.BufferInfo;
import android.media.MediaCodecInfo;
import android.media.MediaCodecList;
import android.media.MediaFormat;
import android.os.Build;

import android.util.Log;

public class MCVideoRender {
	private static final String TAG = "YYLOGVideoRender";
	
	public static final int YYBUFF_NEW_POS		= 0X01;
	public static final int YYBUFF_NEW_FORMAT	= 0X02;
	public static final int YYBUFF_EOS			= 0X04;
	public static final int YYBUFF_KEY_FRAME	= 0X10;	
	
	private	static final String MIME_TYPE_AVC = "video/avc";
	private static final int	WAIT_INTIME = 3000;
	private static final int	WAIT_OUTTIME = 5000;
	private static final int	MC_MAX_WIDTH = 1920;
	private static final int	MC_MAX_HEIGHT = 1080;
	
	private BasePlayer		mPlayer = null;
	private Surface			mSurface = null;
	
	private Thread			mThread = null;	
	private runPlayback		mPlayback = null;
	private boolean			mStop = false;
	
	private	MediaCodec 		mCodec = null;
	private ByteBuffer[] 	mInBuffs = null;
	private ByteBuffer[] 	mOutBuffs = null;
	private MediaFormat		mFormat = null;
	private boolean			mAdaptivePlay = false;
	
    private class runPlayback implements Runnable {
        private MCVideoRender 	mRender = null;
        public runPlayback (MCVideoRender vr) {
        	mRender = vr;
        }
        public void run () { 
        	mRender.playback();
        }
    }		

	public MCVideoRender(Context context, BasePlayer player) {
		mPlayer = player;		
		CheckMediaCodecInfo ();
	}
	
	public void setSurface (Surface pSurface) {
		mSurface = pSurface;
	}
	
	@SuppressLint("NewApi")
	public void playback () {
		int 			nOutdx = -1;	
		BufferInfo		info = new BufferInfo();	
		int 			nIndx = -1;
		int				nSize = 0;
		int 			nFlag = 0;
		long			lTime = 0;
		ByteBuffer		buffer = null;
		byte[]			pBuff = new byte[MC_MAX_WIDTH * MC_MAX_HEIGHT / 2];
		
		if (mCodec == null)
			CreateMediaCodec ();
		
		while (!mStop) {			
			if (nIndx < 0)
				nIndx = mCodec.dequeueInputBuffer(WAIT_INTIME);
			if (nIndx < 0)
				continue;	
			// get the buffer from native 
			if (buffer == null) {
				nSize = mPlayer.ReadVideo (pBuff);
				if (nSize < 0) 
					continue;
				buffer = ByteBuffer.wrap (pBuff, 0, nSize);
			}
			// Check the flag from buffer
			nFlag = 0;
			if (mPlayer.GetVideoFlag () == YYBUFF_NEW_FORMAT) {
				nFlag = MediaCodec.BUFFER_FLAG_END_OF_STREAM;
				mCodec.queueInputBuffer(nIndx, 0, 0, 0, nFlag);
				FlushOutBuffers (true);
				if (!mAdaptivePlay) {
					CreateMediaCodec ();	
					nIndx = mCodec.dequeueInputBuffer(WAIT_INTIME);
					if (nIndx < 0)
						continue;	
				}		
				nFlag = MediaCodec.BUFFER_FLAG_CODEC_CONFIG;
			} else if (mPlayer.GetVideoFlag () == YYBUFF_NEW_POS) {	
				Log.v (TAG, "@@@YYLOG mediacodec start Flush ******!");
				//mCodec.queueInputBuffer(nIndx, 0, 0, 0, 0);
				nIndx = -1;			
				buffer = null;
				FlushOutBuffers (false);
				mCodec.flush();
				Log.v (TAG, "@@@YYLOG mediacodec exit Flush ######!");
				continue;	
			} else if (mPlayer.GetVideoFlag () == YYBUFF_EOS) {
				nFlag = MediaCodec.BUFFER_FLAG_END_OF_STREAM;
			} else if (mPlayer.GetVideoFlag () == YYBUFF_KEY_FRAME) {
				nFlag = MediaCodec.BUFFER_FLAG_SYNC_FRAME;
			}
			mPlayer.SetVideoFlag (0);
			
			lTime = mPlayer.GetVideoTime() * 1000;		
			mInBuffs[nIndx].clear();
			mInBuffs[nIndx].put(buffer);
			mCodec.queueInputBuffer(nIndx, 0, nSize, lTime, nFlag);
			nIndx = -1;
			buffer = null;		
	
			nOutdx = mCodec.dequeueOutputBuffer(info,  WAIT_OUTTIME);
			if (nOutdx == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
				Log.v (TAG, "@@@YYLOG BufferChanged!");
				ResetBuffers ();			
			} else if (nOutdx == MediaCodec.INFO_OUTPUT_BUFFERS_CHANGED) {
				Log.v (TAG, "@@@YYLOG Format changed!");
			}
			if (nOutdx < 0)
				continue;
			if (WaitRenderTime (info.presentationTimeUs))
				mCodec.releaseOutputBuffer(nOutdx, true);	
			else
				mCodec.releaseOutputBuffer(nOutdx, false);	
		}
		mThread = null;
	}
	
	@SuppressLint("NewApi")
	public boolean WaitRenderTime (long lVideoTime) {
		long lPlayTime = mPlayer.GetPos() * 1000;	
		while (lPlayTime < lVideoTime) {
			Sleep (2);
			lPlayTime = mPlayer.GetPos() * 1000;
			if ((lVideoTime - lPlayTime > 500000) || (lVideoTime - lPlayTime < - 500000)) 
				break;				
			if (mStop)
				break;
		}
		return mStop? false : true;
	}
	
	@SuppressLint("NewApi")
	public void start () 
	{			
		mStop = false;
		if (mPlayback == null)
			mPlayback = new runPlayback (this);
		if (mThread == null) {
			mThread = new Thread (mPlayback, "yyVideo Playback");
			mThread.start();
		}
	}
	
	@SuppressLint("NewApi")
	public void stop()
	{
		mStop = true;
		while (mThread != null) {
			try {
				Thread.sleep(2);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}		
		}
		if (mCodec != null) {
			FlushOutBuffers (false);
			mCodec.stop();
			mCodec.release();
		}
		mCodec = null;
	}
	
	@SuppressLint("NewApi")
	private void FlushOutBuffers (boolean bRend) {
		if (mCodec == null)
			return;
		BufferInfo		info = new BufferInfo();	
		int 			nOutdx = 0;
		while (nOutdx >= 0) {
			nOutdx = mCodec.dequeueOutputBuffer(info,  WAIT_OUTTIME);
			if (nOutdx >= 0) {
				if (bRend)
					WaitRenderTime (info.presentationTimeUs);
				mCodec.releaseOutputBuffer(nOutdx, bRend);	
			} else {
				break;
			}
		}			
	}
	
	@SuppressLint("NewApi")
	private boolean CreateMediaCodec () {
		if (mCodec != null) {
			FlushOutBuffers (false);
			mCodec.stop();
			mCodec.release();
		}
		mCodec = MediaCodec.createDecoderByType(MIME_TYPE_AVC);
		if (mAdaptivePlay) {		
			mFormat = MediaFormat.createVideoFormat(MIME_TYPE_AVC, MC_MAX_WIDTH, MC_MAX_HEIGHT);
			mFormat.setInteger("max-width", MC_MAX_WIDTH);
			mFormat.setInteger("max-height", MC_MAX_HEIGHT);			
		} else {
			mFormat = MediaFormat.createVideoFormat(MIME_TYPE_AVC, mPlayer.GetVideoWidth(), mPlayer.GetVideoHeight());		
		}	
		mCodec.configure(mFormat, mSurface, null, 0);

		mCodec.start();
		ResetBuffers ();
		
		return true;
	}
	
	@SuppressLint("NewApi")
	private void ResetBuffers () {	
		if (mCodec == null)
			return;
		mInBuffs = mCodec.getInputBuffers();
		mOutBuffs = mCodec.getOutputBuffers();
	}
	
	private void Sleep (int nTime) {
		try {
			Thread.sleep(nTime);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	@SuppressLint("NewApi")
	private void CheckMediaCodecInfo () {		
		 int numCodecs = MediaCodecList.getCodecCount();		 
	     for (int i = 0; i < numCodecs; i++) {
	         MediaCodecInfo codecInfo = MediaCodecList.getCodecInfoAt(i);
//	         String	strName = codecInfo.getName ();
//			 Log.v(TAG, "@@@YYLOG Support codec " + i + " Name:  " + strName);			 
	         if (codecInfo.isEncoder()) 
	             continue;

	         String[] types = codecInfo.getSupportedTypes();
	         for (int j = 0; j < types.length; j++) {
	        	 if (types[j].endsWith(MIME_TYPE_AVC))
	        		 continue;
//	        	 Log.v(TAG, "@@@YYLOG  " + strName + "  Support  " + j + " Type:  " + types[j]);	        	 
	        	 MediaCodecInfo.CodecCapabilities capVideo  = codecInfo.getCapabilitiesForType(types[j]);
/*	        	 
	        	 int [] nColors = capVideo.colorFormats;
	        	 for (int m = 0; m < nColors.length; m++)
	        		 Log.v(TAG, "@@@YYLOG: support color format is: " + nColors[m]);        	 
	        	 MediaCodecInfo.CodecProfileLevel[] nProfiles = capVideo.profileLevels;
	        	 for (int n = 0; n < nProfiles.length; n++)
	        		 Log.v(TAG, "@@@YYLOG: support profile is: " + nProfiles[n]);	
*/	        	 
	        	 if (Build.VERSION.SDK_INT < Build.VERSION_CODES.KITKAT)
	        		 continue; 
	        	 mAdaptivePlay = capVideo.isFeatureSupported(MediaCodecInfo.CodecCapabilities .FEATURE_AdaptivePlayback);
	        	 if (mAdaptivePlay)
	        		 Log.v(TAG, "@@@YYLOG It supports adaptive playback!");
	        	 return;
	         }
	     }		
	}
}

/*
"video/x-vnd.on2.vp8" - VP8 video (i.e. video in .webm)
"video/x-vnd.on2.vp9" - VP9 video (i.e. video in .webm)
"video/avc" - H.264/AVC video
"video/mp4v-es" - MPEG4 video
"video/3gpp" - H.263 video
"audio/3gpp" - AMR narrowband audio
"audio/amr-wb" - AMR wideband audio
"audio/mpeg" - MPEG1/2 audio layer III
"audio/mp4a-latm" - AAC audio (note, this is raw AAC packets, not packaged in LATM!)
"audio/vorbis" - vorbis audio
"audio/g711-alaw" - G.711 alaw audio
"audio/g711-mlaw" - G.711 ulaw audio
*/

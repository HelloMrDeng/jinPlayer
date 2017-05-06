/*******************************************************************************
	File:		VideoRender.java

	Contains:	Video Render implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.util.Log;

public class VideoRender {
	private final String TAG = "YYLOGVR";

	private final int VIDEOSTATUS_STOP = 0;
	private final int VIDEOSTATUS_PLAY = 1;
	private final int VIDEOSTATUS_PAUSE = 2;

	private BasePlayer mPlayer;

	Thread mThreadPlayback;
	runPlayback mrunPlayback;
	private int mStatus; // 0 stop, 1 play, 2 pause
	private int m_bEOS = 0;
	private int m_nRnd = 0;
	
	private class runPlayback implements Runnable {
		private VideoRender mVideoRender;

		public runPlayback(VideoRender vr) {
			mVideoRender = vr;
		}

		public void run() {
			mVideoRender.playback();
		}
	}

	public VideoRender(BasePlayer pPlayer) {
		mPlayer = pPlayer;
		m_bEOS = 0;

		setStatus(VIDEOSTATUS_STOP);
	}
	
	public int IsEOS () {
		return m_bEOS;
	}

	public int GetRnd (){
		return m_nRnd;
	}
	
	public void run() {
		if (mStatus == VIDEOSTATUS_PLAY)
			return;

		m_nRnd = 0;
		
		setStatus(VIDEOSTATUS_PLAY);

		if (mrunPlayback == null)
			mrunPlayback = new runPlayback(this);

		if (mThreadPlayback == null) {
			mThreadPlayback = new Thread(mrunPlayback, "Video Playback");
			mThreadPlayback.start();
		}
	}

	public void pause() {
		setStatus(VIDEOSTATUS_PAUSE);
	}

	public void stop() {
		setStatus(VIDEOSTATUS_STOP);

		while (mThreadPlayback != null) {
			waitForSleep(100);
		}
	}

	public void playback() {
		long nRC = 0;
		
//		mThreadPlayback.setPriority(Thread.NORM_PRIORITY + 2);

		while (mStatus == VIDEOSTATUS_PLAY || mStatus == VIDEOSTATUS_PAUSE) 
		{	
			if (mStatus == VIDEOSTATUS_PLAY) 
			{
				nRC = mPlayer.GetVideoData(null);
				if (nRC == 1)
					m_bEOS = 1;
				else if (nRC == 0)
					m_nRnd++;
			}
			else
			{
				waitForSleep(2);
			}
		}
		mThreadPlayback = null;
	}

	public boolean init(int nWidth, int nHeight) {

		if (nWidth == 0 || nHeight == 0)
			return false;

		return true;
	}

	private synchronized void setStatus(int status) {
		if (status == VIDEOSTATUS_STOP || status == VIDEOSTATUS_PLAY
				|| status == VIDEOSTATUS_PAUSE) {
			mStatus = status;
		} else {
			Log.e(TAG, "Error: wrong status value is " + status);
		}

	}

	private void waitForSleep(long ltime) {
		try {
			Thread.sleep(ltime);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}

/*******************************************************************************
	File:		EventCheck.java

	Contains:	Video Render implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.util.Log;

public class EventCheck {
	private final String TAG = "YYLOGEC";

	private final int ETCHKSTATUS_STOP 	= 0;
	private final int ETCHKSTATUS_PLAY 	= 1;
	private final int ETCHKSTATUS_PAUSE = 2;

	private BasePlayer mPlayer;

	Thread mThreadPlayback;
	runPlayback mrunPlayback;
	private int mStatus; // 0 stop, 1 play, 2 pause

	private class runPlayback implements Runnable {
		private EventCheck mEventCheck;

		public runPlayback(EventCheck vr) {
			mEventCheck = vr;
		}

		public void run() {
			mEventCheck.playback();
		}
	}

	public EventCheck(BasePlayer pPlayer) {
		mPlayer = pPlayer;

		setStatus(ETCHKSTATUS_STOP);
	}

	public void run() {
		if (mStatus == ETCHKSTATUS_PLAY)
			return;

		setStatus(ETCHKSTATUS_PLAY);

		if (mrunPlayback == null)
			mrunPlayback = new runPlayback(this);

		if (mThreadPlayback == null) {
			mThreadPlayback = new Thread(mrunPlayback, "Event Checking");
			mThreadPlayback.start();
		}
	}

	public void pause() {
		setStatus(ETCHKSTATUS_PAUSE);
	}

	public void stop() {
		setStatus(ETCHKSTATUS_STOP);

		while (mThreadPlayback != null) {
			waitForSleep(100);
		}
	}

	public void playback() {

		mThreadPlayback.setPriority(Thread.NORM_PRIORITY - 1);

		while (mStatus == ETCHKSTATUS_PLAY || mStatus == ETCHKSTATUS_PAUSE) 
		{	
			if (mStatus == ETCHKSTATUS_PLAY) 
			{
				mPlayer.GetEventStatus();
			}
			else
			{
				waitForSleep(2);
			}
		}
		mThreadPlayback = null;
	}

	private synchronized void setStatus(int status) {
		if (status == ETCHKSTATUS_STOP || status == ETCHKSTATUS_PLAY
				|| status == ETCHKSTATUS_PAUSE) {
			mStatus = status;
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

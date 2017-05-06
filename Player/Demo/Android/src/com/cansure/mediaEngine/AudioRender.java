/*******************************************************************************
	File:		AudioRener.java

	Contains:	Audio data render implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.util.Log;

import java.nio.ByteBuffer;

public class AudioRender {
	private BasePlayer mPlayer;
	private AudioTrack mAudioTrack;
	
	private final int AUDIOSTATUS_STOP 	= 0;
	private final int AUDIOSTATUS_PLAY 	= 1;
	private final int AUDIOSTATUS_PAUSE = 2;

	private int 		mSampleRate;
	private int 		mChannels;

	private ByteBuffer 	mByteBuffer;
	private int			mSizeBuffer;

	Thread 				mThreadPlayback;
	runPlayback 		mrunPlayback;
	private int 		mStatus; // 0 stop, 1 play, 2 pause
	private boolean 	mbWrite;

	private static String TAG = "YYLOGAR";

	private class runPlayback implements Runnable {
		private AudioRender mAudioRender;

		public runPlayback(AudioRender ar) {
			mAudioRender = ar;
		}

		public void run() {
			mAudioRender.playback();
		}
	}

	public AudioRender(BasePlayer pPlayer) {
		mPlayer = pPlayer;
		mAudioTrack = null;

		mSampleRate = 0;
		mChannels = 0;

		mByteBuffer = null;

		mStatus = AUDIOSTATUS_STOP;

		mbWrite = false;
	}

	public void run() {
		if (mStatus == AUDIOSTATUS_PLAY) {
			return;
		}

		mStatus = AUDIOSTATUS_PLAY;
		if (mrunPlayback == null)
			mrunPlayback = new runPlayback(this);

		if (mThreadPlayback == null) {
			mThreadPlayback = new Thread(mrunPlayback, "AudioRender Playback");
			mThreadPlayback.start();
		}
	}

	public void pause() {
		mStatus = AUDIOSTATUS_PAUSE;

		while (mbWrite) {
			waitForSleep(2);
		}

		if (mAudioTrack != null && mAudioTrack.getPlayState() == AudioTrack.PLAYSTATE_PLAYING)
			mAudioTrack.pause();
	}

	public void stop() {
		mStatus = AUDIOSTATUS_STOP;

		while (mThreadPlayback != null) {
			waitForSleep(100);
		}
	}

	public void flush() {
		if (mAudioTrack != null)
			mAudioTrack.flush();
	}

	public void playback() {
		long lRC = 0;

//		mThreadPlayback.setPriority(Thread.MAX_PRIORITY - 2);
//		mThreadPlayback.setPriority(Thread.NORM_PRIORITY + 4);
		
		while (mStatus == AUDIOSTATUS_PLAY || mStatus == AUDIOSTATUS_PAUSE) 
		{
			if (mStatus == AUDIOSTATUS_PLAY)
			{
				mbWrite = true;
				if (mByteBuffer == null)
					lRC = mPlayer.GetAudioData(null);
				else
					lRC = mPlayer.GetAudioData(mByteBuffer.array());

				if (lRC == 0) 
				{
					if (mPlayer.GetAudioSampleRate() != mSampleRate || mPlayer.GetAudioChannels() != mChannels) 
					{
						openTrack(mPlayer.GetAudioSampleRate(), mPlayer.GetAudioChannels(), 0, 0);
					}
				}

				if (lRC > 0) 
				{
					writeData(mByteBuffer.array(), lRC);
				}
				mbWrite = false;
			} else {
				waitForSleep(2);
			}
		}

		mThreadPlayback = null;
		closeTrack();

		Log.v(TAG, "playbackaudio stopped!");
	}

	public long writeData(byte[] audioData, long nSize) 
	{
		if (mAudioTrack != null && nSize > 0) 
		{
			if (mAudioTrack.getPlayState() != AudioTrack.PLAYSTATE_PLAYING) 
			{
				mAudioTrack.play();
			}

			mAudioTrack.write(audioData, 0, (int) nSize);
		}

		return 0;
	}

	public int openTrack(int sampleRate, int channelCount, int format, int bufferCount) 
	{
		if (mAudioTrack != null)
			closeTrack();

		@SuppressWarnings("deprecation")
		int nChannelConfig = (channelCount == 1) ? AudioFormat.CHANNEL_CONFIGURATION_MONO : AudioFormat.CHANNEL_CONFIGURATION_STEREO;
		int nMinBufSize = AudioTrack.getMinBufferSize(sampleRate, nChannelConfig, AudioFormat.ENCODING_PCM_16BIT);
		if (nMinBufSize == AudioTrack.ERROR_BAD_VALUE || nMinBufSize == AudioTrack.ERROR)
			return -1;
		
		int param = nMinBufSize * 1000 / (sampleRate*channelCount);//+100;
		if(mPlayer!=null)
			mPlayer.SetParam (mPlayer.PARAM_PID_AUDIO_OFFSEST,  param);

		nMinBufSize = nMinBufSize * 2;

		if (nMinBufSize < 2048)
			nMinBufSize = 2048;
		mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, sampleRate,
									nChannelConfig, AudioFormat.ENCODING_PCM_16BIT, nMinBufSize,
									AudioTrack.MODE_STREAM);
		Log.i(TAG, "The SampleRate = " + sampleRate
				+ "  Channel = " + channelCount + "  nMinBufSize = "
				+ nMinBufSize + "  Offset = " + param);

		mSampleRate = sampleRate;
		mChannels = channelCount;
		mSizeBuffer = mSampleRate * mChannels * 2;
		mByteBuffer = ByteBuffer.allocate(mSizeBuffer);
		if (mByteBuffer == null) {
			Log.e(TAG, "Failed to allocate buffer");
			return -1;
		}

		return 0;
	}

	public void closeTrack() {
		if (mAudioTrack != null) {
			mAudioTrack.stop();
			mAudioTrack.release();
			mAudioTrack = null;
			mSampleRate = 0;
			mChannels = 0;
			mByteBuffer = null;
		}
	}

	public void arsetVolume(float left, float right) {
		if (mAudioTrack != null)
			mAudioTrack.setStereoVolume(left, right);
	}
	
	private void waitForSleep(long ltime){
		try {
			Thread.sleep(ltime);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}

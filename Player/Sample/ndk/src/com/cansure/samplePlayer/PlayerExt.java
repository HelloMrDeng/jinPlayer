/*******************************************************************************
	File:		PlayerExt.java

	Contains:	player UI implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.samplePlayer;

import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import android.app.Activity;
import android.graphics.PixelFormat;
import android.util.DisplayMetrics;
import android.view.ViewGroup.LayoutParams;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.text.format.DateUtils;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.WindowManager;
import android.widget.ImageButton;
import android.widget.ProgressBar;
import android.widget.RelativeLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import com.cansure.mediaEngine.*;

public class PlayerExt extends Activity implements SurfaceHolder.Callback,
		BasePlayer.onEventListener {
	private static final String TAG = "YYLOGPlayerExt";

	private static final int MSG_SHOW_CONTROLLER 	= 0X1001;
	private static final int MSG_HIDE_CONTROLLER 	= 0X1002;
	private static final int MSG_UPDATE_UI 			= 0X1003;

	private SurfaceView 	m_svMain;
	private SurfaceHolder 	m_shMain;
	
	private int 			m_nSurfaceWidth = 0;
	private int 			m_nSurfaceHeight = 0;	

	private RelativeLayout 	m_rlTop;
	private RelativeLayout 	m_rlBottom;

	private TextView 		m_tvCurrentTime;
	private TextView 		m_tvTotalTime;
	private SeekBar 		m_sbMain;
	private ImageButton 	m_ibPlayPause;
	private ProgressBar		m_pbLoadingProgress = null;

	private BasePlayer 		m_Player = null;
	
	private Date 			m_dateUIDisplayStartTime;

	private boolean 		m_bTrackProgressing = false;
	private boolean			m_bPaused = false;

	private String 			m_strVideoPath = "";

	private int 			m_nDuration;
	private int 			m_nPos = 0;
	private boolean 		m_bStop = false;

	private Timer 			m_timerMain = null;
	private TimerTask 		m_ttMain = null;

	private Handler m_handlerEvent = new Handler() 
	{
		public void handleMessage(Message msg) 
		{
			if (m_Player == null)
				return;

			if (msg.what == MSG_SHOW_CONTROLLER) 
			{
				showControllerImpl();
			}
			else if (msg.what == MSG_HIDE_CONTROLLER) 
			{
				hideControllerImpl();
			} 
			else if (msg.what == MSG_UPDATE_UI) 
			{

				if (m_rlBottom.getVisibility() != View.VISIBLE)
					return;

				if (m_bStop || m_bTrackProgressing)
					return;

				Date dateNow = new Date(System.currentTimeMillis());
				long timePeriod = (dateNow.getTime() - m_dateUIDisplayStartTime
						.getTime()) / 1000;
				if (timePeriod >= 7 && m_bPaused == false)
					hideControllerImpl();

				if (m_nDuration <= 0) {
					setTextFast(m_tvCurrentTime, "00:00");
					m_tvCurrentTime.setTextColor((R.drawable.darkgray));
					setTextFast(m_tvTotalTime, "00:00");
					m_tvTotalTime.setTextColor((R.drawable.darkgray));
					m_sbMain.setEnabled(false);
				} else {
					m_nPos = (int) m_Player.GetPos();
					int currPos = m_nPos / 100 * 100 / (m_nDuration / 100);
					if (m_sbMain.getProgress() != currPos)
						m_sbMain.setProgress(currPos);
					String str = DateUtils.formatElapsedTime(m_nPos / 1000);
					setTextFast(m_tvCurrentTime, str);
				}
			} 
		}
	};

	/** Called when the activity is first created. */
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		Log.v(TAG, "Player onCreate");

		/* Screen always on */
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		setContentView(R.layout.playerext);
          
		init();
		
		Uri uri = getIntent().getData();
		if (uri != null) {
			m_strVideoPath = uri.getPath();

			Log.v(TAG, "Path is " + m_strVideoPath);

		} else {
			m_strVideoPath = "test.mp4";
		}

		if ((m_strVideoPath == null) || (m_strVideoPath.trim().length() <= 0)) {
			onError(m_Player, 1, -1);
			return;
		}

		getWindow().setFormat(PixelFormat.UNKNOWN);

		m_svMain = (SurfaceView) findViewById(R.id.svMain);
		m_shMain = m_svMain.getHolder();
		m_shMain.addCallback(this);

		m_pbLoadingProgress.setVisibility(View.VISIBLE);
		
		m_nSurfaceWidth = m_svMain.getWidth();
		m_nSurfaceHeight = m_svMain.getHeight();		
	}

	private boolean setTextFast(TextView vw, String str) {
		if (str == null)
			str = "";
		String dtrOld = vw.getText().toString();
		if (str.compareTo(dtrOld) != 0) {
			vw.setText(str);
			return true;
		}
		return false;
	}

	private void resetUIDisplayStartTime() {
		m_dateUIDisplayStartTime = new Date(System.currentTimeMillis());
	}

	private void init() {
		initLayout();
		initLayoutTop();
		initLayoutCenter();
	}

	private void initLayout() {
		m_rlTop = (RelativeLayout) findViewById(R.id.rlTop);
		m_rlBottom = (RelativeLayout) findViewById(R.id.rlBottom);
	}

	private void initLayoutTop() {
		m_tvCurrentTime = (TextView) findViewById(R.id.tvCurrentTime);
		m_tvTotalTime = (TextView) findViewById(R.id.tvTotalTime);
		m_sbMain = (SeekBar) findViewById(R.id.sbMain);

		m_sbMain.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
			public void onStopTrackingTouch(SeekBar seekBar) {
				int iCurrent = seekBar.getProgress();
				int iMax = seekBar.getMax();
				int iNewPosition = iCurrent * 100 / iMax * m_nDuration / 100;

				m_bTrackProgressing = false;

				if (m_Player != null) {
					Log.v(TAG, "Seek To " + iNewPosition
							+ ", After stop track touch.");
					seek(iNewPosition);
				}
			}

			public void onStartTrackingTouch(SeekBar seekBar) {
				m_bTrackProgressing = true;
			}

			public void onProgressChanged(SeekBar seekBar, int progress,
					boolean fromUser) {
			}
		});

	}

	private void initLayoutCenter() {
		m_ibPlayPause = (ImageButton) findViewById(R.id.ibPlayPause);
		m_pbLoadingProgress = (ProgressBar) findViewById(R.id.pbLoadingProgress);

		m_ibPlayPause.setOnClickListener(new View.OnClickListener() {

			public void onClick(View v) {
				playerPauseRestart();
			}
		});
	}

	private void playVideo(String strPath) {
		int nRet;
		Log.v(TAG, "play video " + strPath);
		
		m_Player = new MediaPlayer();

		String apkPath = "/data/data/" + this.getPackageName() + "/lib/";
		m_Player.SetView(m_svMain);
		nRet = m_Player.Init(this, apkPath);
		if (nRet != 0) {
			onError(m_Player, nRet, 0);
			return;
		}
				
		m_Player.setEventListener(this);
		nRet = m_Player.Open (strPath);
		if (nRet != 0) {
			onError(m_Player, nRet, 0);
			
			PlayStop();
			finish();	
			
			return;
		}
	}

	public void surfaceChanged(SurfaceHolder surfaceholder, int format, int w, int h) 
	{
			
	}

	public void surfaceCreated(SurfaceHolder surfaceholder) 
	{		
		if (m_Player != null) 
		{
			m_Player.SetView(m_svMain);					
			showMediaController();
			return;
		}

		playVideo(m_strVideoPath);

	}

	public void surfaceDestroyed(SurfaceHolder surfaceholder) 
	{	
		if (m_Player != null)
		{
			m_Player.SetView(null);
		}
	}

	public void PlayStop() {
		m_bPaused = false;
		m_bStop = true;

		if (m_Player != null) {
			m_Player.Stop();
			m_Player.Uninit();
			m_Player = null;
			Log.v(TAG, "MediaPlayer release.");
		}
	}

	@Override
	protected void onDestroy() {
		if (m_timerMain != null) {
			m_timerMain.cancel();
			m_timerMain.purge();
			m_timerMain = null;
			m_ttMain = null;
		}

		if (m_shMain != null) {
			m_shMain.removeCallback(this);
			m_shMain = null;
		}

		super.onDestroy();
		Log.v(TAG, "Player onDestroy Completed!");
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onPause()
	 */
	@Override
	protected void onPause() 
	{
		super.onPause();
		Log.v(TAG, "Player onPause");

		if (m_Player != null) 
		{
			if (m_Player.GetDuration() <= 0) 
			{
				PlayStop();
				finish();
			} 
			else
			{
				m_Player.Pause();
				m_ibPlayPause.setBackgroundDrawable(getResources().getDrawable(R.drawable.btn_play_on));
				m_bPaused = true;
				Log.v(TAG, "Player pause");

				showMediaController();
			}
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onRestart()
	 */
	@Override
	protected void onRestart() {
		Log.v(TAG, "Player onRestart");

		super.onRestart();
	}

	private void showMediaController() {
		if (m_Player == null)
			return;

		if (m_Player.GetDuration() <= 0)
			return;

		m_handlerEvent.sendEmptyMessage(MSG_SHOW_CONTROLLER);
	}

	private void showControllerImpl() {
		Log.v(TAG,
				"Touch screen, layout status is " + m_rlBottom.getVisibility());
		resetUIDisplayStartTime();

		if (m_rlBottom.getVisibility() != View.VISIBLE) {
			Log.v(TAG, "mIsStop is " + m_bStop);

			if (m_ttMain != null)
				m_ttMain = null;

			m_ttMain = new TimerTask() {
				public void run() {
					m_handlerEvent.sendEmptyMessage(MSG_UPDATE_UI);
				}
			};

			if (m_timerMain == null) {
				m_timerMain = new Timer();
			}

			m_timerMain.schedule(m_ttMain, 0, 1000);

			m_rlBottom.setVisibility(View.VISIBLE);
			m_rlTop.setVisibility(View.VISIBLE);

			Log.v(TAG, "mLayoutTime show " + m_rlTop.getVisibility());
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onTouchEvent(android.view.MotionEvent)
	 */
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_DOWN) {

			if (m_rlBottom.getVisibility() != View.VISIBLE) {
				showControllerImpl();
			} else {
				hideControllerImpl();
			}
		}

		return super.onTouchEvent(event);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onKeyDown(int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		
		if (keyCode == KeyEvent.KEYCODE_BACK) 
		{
			PlayStop();
			finish();
			return true;
		}

		return super.onKeyDown(keyCode, event);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onStart()
	 */
	@Override
	protected void onStart() {
		Log.v(TAG, "Player onStart");
		super.onStart();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onStop()
	 */
	@Override
	protected void onStop() {
		Log.v(TAG, "Player onStop");
		super.onStop();
	}

	public void hideController() {
		m_handlerEvent.sendEmptyMessage(MSG_HIDE_CONTROLLER);
	}

	public void hideControllerImpl() {
		if (m_timerMain != null) {
			m_timerMain.cancel();
			m_timerMain.purge();
			m_timerMain = null;
			m_ttMain = null;
		}

		m_rlBottom.setVisibility(View.INVISIBLE);
		m_rlTop.setVisibility(View.INVISIBLE);
	}

	public boolean onError(BasePlayer mp, int what, int extra) {
		Log.v(TAG, "Error message, what is 0x" + Integer.toHexString(what)
				+ " extra is " + extra);

		return true;
	}

	private void playerPauseRestart() 
	{
		if (m_Player != null) 
		{
			if (m_bPaused == false) 
			{
				m_Player.Pause();
				showMediaController();
				m_ibPlayPause.setBackgroundDrawable(getResources().getDrawable(R.drawable.selector_player_play));
				m_bPaused = true;
			} else if (m_bPaused == true) 
			{
				m_Player.Play();
				m_ibPlayPause.setBackgroundDrawable(getResources().getDrawable(R.drawable.selector_player_pause));
				m_bPaused = false;
			}
		}
	}

	// @Override
	public int onEvent(int nID, Object obj) 
	{
		Log.v(TAG, "onEvent ID is " + Integer.toHexString(nID));

		if (nID == BasePlayer.YY_EV_VideoFormat_Change) 
		{
			Log.v(TAG, "Video format changed!");		
			onVideoSizeChanged (m_Player.GetVideoWidth (), m_Player.GetVideoHeight ());			
		}
		else if (nID == BasePlayer.YY_EV_Open_Complete) 
		{
			Log.v(TAG, "Open Completely.");		
			
			m_pbLoadingProgress.setVisibility(View.INVISIBLE);
			m_nDuration = (int)m_Player.GetDuration();
			
			String str = DateUtils.formatElapsedTime(m_nDuration / 1000);
			setTextFast(m_tvTotalTime, str);			
			
			m_Player.Play ();
		} 
		if (nID == BasePlayer.YY_EV_Open_Failed) 
		{
			Log.v(TAG, "Open Failed!");			
			PlayStop();
			finish();				
		}		
		else if (nID == BasePlayer.YY_EV_Play_Complete) 
		{
			//m_Player.Pause ();
			//seek(0);

			PlayStop();
			finish();		

			Log.v(TAG, "Seek to 0, after play completed.");
		}		

		return 0;
	}

	private void seek(int nPos)
	{
		if ((m_Player != null)) {
			m_Player.SetPos(nPos);
		}
	}
	
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
}

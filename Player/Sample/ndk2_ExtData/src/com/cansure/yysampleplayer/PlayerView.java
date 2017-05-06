/*******************************************************************************
	File:		Player view.java

	Contains:	the player UI implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-05		Fenger			Create file

*******************************************************************************/
package com.cansure.yysampleplayer;

import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.util.DisplayMetrics;
import android.util.Log;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.view.View;
import android.view.ViewGroup.LayoutParams;
import android.widget.RelativeLayout;
import android.widget.ImageButton;
import android.widget.SeekBar;

import com.cansure.mediaEngine.*;
import com.cansure.ExtData.*;

public class PlayerView extends Activity 
						implements SurfaceHolder.Callback,
						BasePlayer.onEventListener {
	
	private static final int MSG_UPDATE_UI 	= 0X1001;
	
	private SurfaceView 	m_svVideo = null;
	private SurfaceHolder 	m_shVideo = null;
	private int 			m_nSurfaceWidth = 0;
	private int 			m_nSurfaceHeight = 0;	
	
	private RelativeLayout 	m_layButtons = null;
	private ImageButton		m_btnPlay = null;
	private ImageButton		m_btnPause = null;
	private SeekBar 		m_sbPlayer = null;	
	
	private String			m_strFile = null;
	private MediaPlayer		m_Player = null;
	private int				m_nDuration = 0;
	private ExtData			m_ExtData = null;
	private int				m_nExtData = 0;
	
	private Timer			m_tmPlay = null;
	private TimerTask		m_ttPlay = null;
	private Date 			m_dateShowTime = null;	
	private Handler 		m_handlerEvent = null;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);     
        
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

        setContentView(R.layout.activity_player_view);
        
        initControls ();
		
		Uri uri = getIntent().getData();
		if (uri != null) 
			m_strFile = uri.getPath();	
		
		Log.v ("PlayerView", m_strFile);
    }
    
	public void surfaceChanged(SurfaceHolder surfaceholder, int format, int w, int h) 
	{
		Log.v ("PlayerView", "SurfaceChanged" + format + w + h);
	}

	public void surfaceCreated(SurfaceHolder surfaceholder) 
	{		
		Log.v ("PlayerView", "SurfaceCreated");
		if (m_Player != null) 
		{
			m_Player.SetView(m_svVideo);	
			//m_Player.Play();
			return;
		}
		
		OpenFile (m_strFile);
		
		m_dateShowTime = new Date(System.currentTimeMillis());
		showControls ();		
	}

	public void surfaceDestroyed(SurfaceHolder surfaceholder) 
	{	
		Log.v ("PlayerView", "surfaceDestroyed");
	}
	
	// @Override
	public int onEvent(int nID, Object obj) 
	{
		if (nID == BasePlayer.YY_EV_VideoFormat_Change) 
		{
			onVideoSizeChanged (m_Player.GetVideoWidth (), m_Player.GetVideoHeight ());			
		}
		else if (nID == BasePlayer.YY_EV_Open_Complete) 
		{			
			m_nDuration = (int)m_Player.GetDuration();
			m_Player.Play ();
		} 
		if (nID == BasePlayer.YY_EV_Open_Failed) 
		{
			Close ();				
		}		
		else if (nID == BasePlayer.YY_EV_Play_Complete) 
		{
			Close ();	
		}		

		return 0;
	}
	
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) 
			Close ();

		return super.onKeyDown(keyCode, event);
	}	
	
	@Override
	public boolean onTouchEvent(MotionEvent event) {
		if (event.getAction() == MotionEvent.ACTION_DOWN) {
			m_dateShowTime = new Date(System.currentTimeMillis());
			if (m_layButtons.getVisibility() != View.VISIBLE) {
				showControls ();
			} else {
				hideControls ();
			}
		}

		return super.onTouchEvent(event);
	}	
	
	@Override
	protected void onPause() 
	{
		super.onPause();
		Log.v("PlayerView", "Player onPause");
		
		if (m_Player != null) 
		{
			if (m_Player.GetDuration() <= 0) {
				Close();
			}  else {
				m_Player.Pause();
				m_btnPause.setVisibility(View.INVISIBLE);
				m_btnPlay.setVisibility(View.VISIBLE);
				m_layButtons.setVisibility(View.VISIBLE);
			}
		}		
	}
	
	@Override
	protected void onDestroy() {
		hideControls ();

		if (m_shVideo != null) {
			m_shVideo.removeCallback(this);
			m_shVideo = null;
		}

		super.onDestroy();
	}
	
	private void OpenFile (String strPath) {
		int nRet;
		String apkPath = "/data/data/" + this.getPackageName() + "/lib/";
				
		m_Player = new MediaPlayer();
		m_Player.SetView(m_svVideo);
		nRet = m_Player.Init(this, apkPath);
		if (nRet != 0) {
			Close ();
			return;
		}				
		m_Player.setEventListener(this);
		
		m_ExtData = new ExtData ();
		m_nExtData = m_ExtData.Init(this, apkPath);		
		//nRet = m_Player.Open (strPath);
		m_Player.SetParam(m_Player.PARAM_PID_COLOR_FORMAT, 0, null);
		nRet = m_Player.OpenExt(m_nExtData, 0);		
		if (nRet != 0) {
			Close ();		
			return;
		}	
	}	
	
	private void Close () {
		if (m_Player != null) {
			m_Player.Stop();
			m_Player.Uninit();
			m_Player = null;
		}	
		
		if (m_ExtData != null)
		{
			m_ExtData.Uninit ();
			m_ExtData = null;
		}
		
		finish ();
	}
	
	private void initControls (){
		m_svVideo = (SurfaceView) findViewById(R.id.svVideo);
		m_shVideo = m_svVideo.getHolder();
		m_shVideo.addCallback(this);

		m_layButtons = (RelativeLayout) findViewById(R.id.layControls);
		m_btnPlay = (ImageButton) findViewById (R.id.btnPlay);
		m_btnPause = (ImageButton) findViewById (R.id.btnPause);
		m_sbPlayer = (SeekBar) findViewById (R.id.sbPlayer);
		
		m_btnPlay.setVisibility(View.INVISIBLE);
	//	m_layButtons.setVisibility(View.INVISIBLE);
		
		m_btnPlay.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (m_Player != null)
					m_Player.Play();
				m_btnPlay.setVisibility(View.INVISIBLE);
				m_btnPause.setVisibility(View.VISIBLE);
			}
		});
		
		m_btnPause.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (m_Player != null) 
					m_Player.Pause();
				m_btnPause.setVisibility(View.INVISIBLE);
				m_btnPlay.setVisibility(View.VISIBLE);
			}
		});	
		
		m_sbPlayer.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
			public void onStopTrackingTouch(SeekBar seekBar) {
				int nPos = seekBar.getProgress() * m_nDuration / 100;
				if (m_Player != null)
					m_Player.SetPos(nPos);
			}
			
			public void onStartTrackingTouch(SeekBar seekBar) {
			}

			public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
			}
		});	
		
		m_handlerEvent = new Handler() 
		{
			public void handleMessage(Message msg) 
			{
				if (m_Player == null)
					return;

				if (msg.what == MSG_UPDATE_UI) 
				{			
					Date dateNow = new Date(System.currentTimeMillis());
					long timePeriod = (dateNow.getTime() - m_dateShowTime.getTime()) / 1000;
					if (timePeriod >= 10)
						hideControls ();
					
					if (m_nDuration > 0) {
						int nPos = m_Player.GetPos() * 100 / m_nDuration;
						m_sbPlayer.setProgress(nPos);	
					}
				} 
			}
		};		
	}
	
	private void showControls (){
		m_layButtons.setVisibility(View.VISIBLE);
		if (m_ttPlay != null)
			m_ttPlay = null;
		m_ttPlay = new TimerTask() {
			public void run() {
				m_handlerEvent.sendEmptyMessage(MSG_UPDATE_UI);
			}
		};			
		
		if (m_tmPlay == null)
			m_tmPlay = new Timer ();
		
		m_tmPlay.schedule(m_ttPlay, 0, 1000);		
	}
	
	private void hideControls () {
		if (m_tmPlay != null) {
			m_tmPlay.cancel();
			m_tmPlay.purge();
			m_tmPlay = null;
			m_ttPlay = null;
		}
		
		m_layButtons.setVisibility(View.INVISIBLE);		
	}
	
	public void onVideoSizeChanged (int width, int height) 
	{	
		if (m_nSurfaceWidth == width && m_nSurfaceHeight == height)
			return;

		Log.v("PlayerView", "Video Size width = " + width + ", height = " + height);
		if ((m_nSurfaceWidth == 0 && width > 0) || m_nSurfaceWidth * height != width * m_nSurfaceHeight)
		{
			RelativeLayout.LayoutParams lp = (RelativeLayout.LayoutParams)m_svVideo.getLayoutParams();
			DisplayMetrics dm  = new DisplayMetrics();
			getWindowManager().getDefaultDisplay().getMetrics(dm);
			if (width != 0 && height != 0 && lp.width == LayoutParams.FILL_PARENT && lp.height == LayoutParams.FILL_PARENT) 
			{
				int nMaxOutW = dm.widthPixels;
				int nMaxOutH = dm.heightPixels;
				int w = 0, h = 0;
				if (width != 0 && height != 0) {
					nMaxOutW &= ~0x3;
					nMaxOutH &= ~0x1;
					if (nMaxOutW * height > width * nMaxOutH) {
						h = nMaxOutH;
						w = width * h / height;
					} else {
						w = nMaxOutW;
						h = height * w / width;
					}
					w &= ~0x3;
					h &= ~0x1;
					
					lp.width = w;
					lp.height = h;
				}

				m_svVideo.setLayoutParams(lp);
				Log.v("PlayerView", String.format("setSurfaceSize width = %d, height = %d", lp.width , lp.height));
			}
		}

		m_nSurfaceWidth = width;
		m_nSurfaceHeight = height;

		if (m_nSurfaceWidth != 0 && m_nSurfaceHeight != 0) {
			m_shVideo.setFixedSize(m_nSurfaceWidth, m_nSurfaceHeight);
			m_svVideo.setVisibility(View.VISIBLE); 
		}   
	}	
    
}

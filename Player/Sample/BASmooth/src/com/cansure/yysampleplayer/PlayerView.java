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
import android.content.SharedPreferences;
import android.util.Log;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;

import com.cansure.mediaEngine.*;

public class PlayerView extends Activity 
						implements SurfaceHolder.Callback,
						BasePlayer.onEventListener {
	
	private static final int MSG_UPDATE_UI 	= 0X1001;
	
	private SurfaceView 	m_svVideo = null;
	private SurfaceHolder 	m_shVideo = null;
	private BitmapView 		m_vvVideo = null;
	private TextView 		m_txtSubTT = null;

	private RelativeLayout 	m_layButtons = null;
	private ImageButton		m_btnPlay = null;
	private ImageButton		m_btnPause = null;
	private SeekBar 		m_sbPlayer = null;	
	
	private String			m_strFile = null;
	private MediaPlayer		m_Player = null;
	private int				m_nDuration = 0;
	
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
			m_strFile = uri.toString();
		
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
			m_Player.SetVideoView(m_vvVideo);
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
	public int onEvent(int nID, Object obj) {
		if (nID == BasePlayer.YY_EV_Open_Complete) {			
			m_nDuration = (int)m_Player.GetDuration();
			m_Player.Play ();
		} 
		else if (nID == BasePlayer.YY_EV_Play_Duration) 			
			m_nDuration = (int)m_Player.GetDuration();
		else if (nID == BasePlayer.YY_EV_Open_Failed) 
			Close ();					
		else if (nID == BasePlayer.YY_EV_Play_Complete)
			Close ();		

		return 0;
	}
	
	public int OnSubTT (String strText, int nTime) {
		m_txtSubTT.setText(strText);
		if (nTime >= 0) {
			m_txtSubTT.setText(strText);
			m_txtSubTT.setVisibility(View.VISIBLE);
		} else {
			m_txtSubTT.setText("");		
			m_txtSubTT.setVisibility(View.INVISIBLE);			
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
		m_Player = new MediaPlayer();

		String apkPath = "/data/data/" + this.getPackageName() + "/lib/";
		m_Player.SetView(m_svVideo);
		m_Player.SetVideoView(m_vvVideo);
		nRet = m_Player.Init(this, apkPath);
		if (nRet != 0) {
			Close ();
			return;
		}
				
		m_Player.setEventListener(this);
		ConfigPlayer ();
		nRet = m_Player.Open (strPath);
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
		
		finish ();
	}
	
	
	private void ConfigPlayer () {
		if (m_Player == null)
			return;
		SharedPreferences settings = this.getSharedPreferences("Player_Setting", 0);
//		int nVideoQuality 	= settings.getInt("VideoQuality", 0);
		int nSubTT     	= settings.getInt("Subtitle", 1); 
		int nColorType     = settings.getInt("ColorType", 0);   	   
		int nVideoDec      = settings.getInt("VideoDec", 1);   	

		m_Player.SetParam(BasePlayer.PARAM_PID_COLOR_FORMAT, nColorType, null);	
		m_Player.SetParam(BasePlayer.PARAM_PID_SunTT_Enable, nSubTT, null);	
		if (nVideoDec == 0)
			m_Player.SetParam(BasePlayer.PARAM_PID_VIDEODEC_MODE, BasePlayer.YY_VDMODE_Auto, null);	
		else if (nVideoDec == 1)
			m_Player.SetParam(BasePlayer.PARAM_PID_VIDEODEC_MODE, BasePlayer.YY_VDMODE_Soft, null);			
		else if (nVideoDec == 2)
			m_Player.SetParam(BasePlayer.PARAM_PID_VIDEODEC_MODE, BasePlayer.YY_VDMODE_IOMX, null);
		else if (nVideoDec == 3)
			m_Player.SetParam(BasePlayer.PARAM_PID_VIDEODEC_MODE, BasePlayer.YY_VDMODE_MediaCodec, null);		
	}	
	
	private void initControls (){
		m_svVideo = (SurfaceView) findViewById(R.id.svVideo);
		m_vvVideo = (BitmapView) findViewById (R.id.vvVideo);
		m_shVideo = m_svVideo.getHolder();
		m_shVideo.addCallback(this);

		m_txtSubTT = (TextView) findViewById(R.id.tvSubTT);
		m_layButtons = (RelativeLayout) findViewById(R.id.layControls);
		m_btnPlay = (ImageButton) findViewById (R.id.btnPlay);
		m_btnPause = (ImageButton) findViewById (R.id.btnPause);
		m_sbPlayer = (SeekBar) findViewById (R.id.sbPlayer);
		
		m_btnPlay.setVisibility(View.INVISIBLE);
	//	m_layButtons.setVisibility(View.INVISIBLE);
		m_txtSubTT.setVisibility(View.INVISIBLE);
		
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
}

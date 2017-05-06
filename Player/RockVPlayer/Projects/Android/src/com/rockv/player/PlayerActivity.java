/*******************************************************************************
	File:		Player view.java

	Contains:	the player UI implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-05		Fenger			Create file

*******************************************************************************/
package com.rockv.player;

import java.util.Date;
import java.util.Timer;
import java.util.TimerTask;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.content.DialogInterface.OnClickListener;
import android.util.Log;

import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.WindowManager;
import android.view.View;
import android.widget.RelativeLayout;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.RadioGroup;
import android.widget.RadioButton;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.SeekBar;
import android.widget.TextView;

import com.cansure.mediaEngine.*;

public class PlayerActivity extends Activity 
						implements BasePlayer.onEventListener {
	
	private static final int MSG_UPDATE_UI 	= 0X1001;

	private	TextView		m_txtTrack1 = null;
	private	TextView		m_txtTrack2 = null;
	private	TextView		m_txtTrack3 = null;
	private SeekBar			m_skbTrack1 = null;
	private SeekBar			m_skbTrack2 = null;
	private SeekBar			m_skbTrack3 = null;
	private CheckBox		m_chbTrack1 = null;
	private CheckBox		m_chbTrack2 = null;
	private CheckBox		m_chbTrack3 = null;
	
	private RadioButton		m_rdbRepeatNone = null;
	private RadioButton		m_rdbRepeatA = null;
	private RadioButton		m_rdbRepeatB = null;
	private RadioButton		m_rdbRepeatAll = null;
		
	private Button			m_btnPlay = null;
	private Button			m_btnStop = null;
	
	private SeekBar 		m_skbPos = null;	
	private	TextView		m_txtPos = null;
	private	TextView		m_txtDur = null;
	
	private String			m_strFile = null;
	private MediaPlayer		m_Player = null;
	private int				m_nDuration = 0;
	
	private Timer			m_tmPlay = null;
	private TimerTask		m_ttPlay = null;
	private Handler 		m_handlerEvent = null;
	
	boolean					m_bPlaying = false;
	boolean					m_bRepeatAll = false;
	int						m_nRepeatNum = 0;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);           
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
				WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        setContentView(R.layout.activity_player);
        
        initControls ();
		
		Uri uri = getIntent().getData();
		if (uri != null) {
			m_strFile = uri.toString();	
			Log.v ("PlayerActivity", m_strFile);
		}
    }
   
	private void OpenFile (String strPath) {
		if (m_Player != null)
			return;
		m_Player = new MediaPlayer();
		String apkPath = "/data/data/" + this.getPackageName() + "/lib/";
		int nRet = m_Player.Init(this, apkPath);
		if (nRet != 0) {
			return;
		}		
		m_Player.setEventListener(this);
		nRet = m_Player.Open (strPath);
		if (nRet != 0) {	
			return;
		}	
		UpdateControls ();
//		m_Player.Play ();
	}	
	
	private void Close () {
		if (m_Player != null) {
			m_Player.Stop();
			m_Player.Uninit();
			m_Player = null;
		}		
		finish ();
	}
	
	private View.OnClickListener ctrlListener = new View.OnClickListener(){ 
		@Override 
		public void onClick(View view) { 
			if (view.getId() == m_btnPlay.getId()) {
				if (m_bPlaying) {
					m_Player.Pause();
					m_bPlaying = false;
					m_btnPlay.setText("播放");
				} else {
					m_Player.Play ();
					m_bPlaying = true;
					m_btnPlay.setText("暂停");
				}
			} else if (view.getId() == m_btnStop.getId()) {
				m_Player.Pause();
				m_Player.SetPos(0);
				m_bPlaying = false;
				m_btnPlay.setText("Play");
			}
		} 
	}; 
	
	private CompoundButton.OnCheckedChangeListener chbListener = new CompoundButton.OnCheckedChangeListener () {
		public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {			
			int nTrack = 0;
			if (buttonView.getId() == m_chbTrack1.getId()) 
				nTrack = 0;
			else if (buttonView.getId() == m_chbTrack2.getId()) 
				nTrack = 1;
			else if (buttonView.getId() == m_chbTrack3.getId()) 
				nTrack = 2;		
			if (isChecked) 
				m_Player.SetTrackEnable(nTrack, 1);
			else 
				m_Player.SetTrackEnable(nTrack, 0);
		}
	};
	
	private View.OnClickListener rbClickListener = new View.OnClickListener() {
		@Override
		public void onClick(View buttonView) {
			m_rdbRepeatNone.setChecked(false);
			m_rdbRepeatA.setChecked(false);
			m_rdbRepeatB.setChecked(false);
			m_rdbRepeatAll.setChecked(false);		
			m_bRepeatAll = false;
			int nRepeat = 0;
			if (buttonView.getId() == m_rdbRepeatNone.getId())  {
				m_rdbRepeatNone.setChecked(true);
				nRepeat = -1;
			}
			else if (buttonView.getId() == m_rdbRepeatA.getId()) {
				m_rdbRepeatA.setChecked(true);
				nRepeat = 0;
			}
			else if (buttonView.getId() == m_rdbRepeatB.getId())  {
				m_rdbRepeatB.setChecked(true);
				nRepeat = 1;	
			}
			else if (buttonView.getId() == m_rdbRepeatAll.getId())  {
				m_rdbRepeatAll.setChecked(true);
				nRepeat = -1;	
				m_bRepeatAll = true;
			}
			m_Player.SetRepeatSel(nRepeat);			
		}
	};
	
	private SeekBar.OnSeekBarChangeListener skbListener = new SeekBar.OnSeekBarChangeListener() {
		public void onStopTrackingTouch(SeekBar seekBar) {
			if (m_Player == null)
				return;
			int nPos = seekBar.getProgress();
			if (seekBar.getId() == m_skbPos.getId()) {
				m_Player.SetPos(nPos * m_nDuration / 100);
			} else if (seekBar.getId() == m_skbTrack1.getId())  {
				m_Player.SetTrackVolume(0, nPos);
			} else if (seekBar.getId() == m_skbTrack2.getId())  {
				m_Player.SetTrackVolume(1, nPos);
			} else if (seekBar.getId() == m_skbTrack3.getId())  {
				m_Player.SetTrackVolume(2, nPos);
			}
		}
		public void onStartTrackingTouch(SeekBar seekBar) {
		}
		public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
		}		
	};
	
	private void initControls (){		
		m_txtTrack1 = (TextView) findViewById (R.id.txtTrack1);
		m_txtTrack2 = (TextView) findViewById (R.id.txtTrack2);
		m_txtTrack3 = (TextView) findViewById (R.id.txtTrack3);	
		m_skbTrack1 = (SeekBar) findViewById (R.id.skbTrack1);	
		m_skbTrack2 = (SeekBar) findViewById (R.id.skbTrack2);
		m_skbTrack3 = (SeekBar) findViewById (R.id.skbTrack3);
		m_chbTrack1 = (CheckBox) findViewById (R.id.chbTrack1);
		m_chbTrack2 = (CheckBox) findViewById (R.id.chbTrack2);
		m_chbTrack3 = (CheckBox) findViewById (R.id.chbTrack3);	
		
		m_rdbRepeatNone = (RadioButton) findViewById (R.id.rbNone);
		m_rdbRepeatA = (RadioButton) findViewById (R.id.rbA);
		m_rdbRepeatB = (RadioButton) findViewById (R.id.rbB);
		m_rdbRepeatAll = (RadioButton) findViewById (R.id.rbAll);
		
		m_btnPlay = (Button) findViewById (R.id.btnPlay);
		m_btnStop = (Button) findViewById (R.id.btnStop);	
		
		m_skbPos = (SeekBar) findViewById (R.id.skbPos);		
		m_txtPos = (TextView) findViewById (R.id.txtPos);	
		m_txtDur = (TextView) findViewById (R.id.txtDur);	
		
		m_btnPlay.setOnClickListener(ctrlListener);
		m_btnStop.setOnClickListener(ctrlListener);
		
		m_skbTrack1.setOnSeekBarChangeListener (skbListener);
		m_skbTrack2.setOnSeekBarChangeListener (skbListener);
		m_skbTrack3.setOnSeekBarChangeListener (skbListener);
		m_skbPos.setOnSeekBarChangeListener (skbListener);
		
		m_chbTrack1.setOnCheckedChangeListener(chbListener);
		m_chbTrack2.setOnCheckedChangeListener(chbListener);
		m_chbTrack3.setOnCheckedChangeListener(chbListener);
		
		m_rdbRepeatNone.setChecked(true);
		m_rdbRepeatA.setChecked(false);
		m_rdbRepeatB.setChecked(false);
		m_rdbRepeatB.setChecked(false);	
		m_rdbRepeatNone.setOnClickListener(rbClickListener);
		m_rdbRepeatA.setOnClickListener(rbClickListener);
		m_rdbRepeatB.setOnClickListener(rbClickListener);
		m_rdbRepeatAll.setOnClickListener(rbClickListener);

		m_handlerEvent = new Handler() {
			public void handleMessage(Message msg) {
				if (msg.what == MSG_UPDATE_UI) {								
					if (m_nDuration > 0) {
						int nPos = 0;
						if (m_Player != null) {					
							nPos = m_Player.GetPos() * 100 / m_nDuration;
							m_skbPos.setProgress(nPos);	
							nPos = m_Player.GetPos();
							String strPos = String.format ("%02d:%02d", nPos / 60000, (nPos / 1000) % 60);
							m_txtPos.setText(strPos);
						}
					}
				} 
			}
		};		
		m_ttPlay = new TimerTask() {
			public void run() {
				m_handlerEvent.sendEmptyMessage(MSG_UPDATE_UI);
			}
		};			
		m_tmPlay = new Timer ();		
		m_tmPlay.schedule(m_ttPlay, 0, 1000);			
	}
	
	private void UpdateControls () {
		if (m_Player == null)
			return;
		m_nDuration = (int)m_Player.GetDuration();
		String strDur = String.format ("%02d:%02d", m_nDuration / 60000, (m_nDuration / 1000) % 60);
		m_txtDur.setText(strDur);
		
		if (m_Player.GetTrackCount() < 3) {
			m_skbTrack3.setEnabled(false);
			m_chbTrack3.setEnabled(false);
		}
		if (m_Player.GetTrackCount() < 2) {
			m_skbTrack2.setEnabled(false);
			m_chbTrack2.setEnabled(false);
		}
		if (m_Player.GetTrackCount() < 1) {
			m_skbTrack1.setEnabled(false);
			m_chbTrack1.setEnabled(false);
		}
		int nTrack = 0;
		int nVolume = 0;
		int nEnable = 0;
		while (nTrack < m_Player.GetTrackCount()) {
			nVolume = m_Player.GetTrackVolume(nTrack);
			m_skbTrack1.setProgress(nVolume);
			nEnable = m_Player.GetTrackEnable(nTrack);
			m_chbTrack1.setChecked (nEnable > 0);
			nTrack++;
			if (nTrack >= m_Player.GetTrackCount())
				break;
			nVolume = m_Player.GetTrackVolume(nTrack);
			m_skbTrack2.setProgress(nVolume);
			nEnable = m_Player.GetTrackEnable(nTrack);
			m_chbTrack2.setChecked(nEnable > 0);
			nTrack++;
			if (nTrack >= m_Player.GetTrackCount())
				break;
			nVolume = m_Player.GetTrackVolume(nTrack);
			m_skbTrack3.setProgress(nVolume);
			nEnable = m_Player.GetTrackEnable(nTrack);
			m_chbTrack3.setChecked(nEnable > 0);
			break;						
		}
		if (m_Player.GetRepeatCount() < 2)
			m_rdbRepeatB.setEnabled(false);
		if (m_Player.GetRepeatCount() < 1)
			m_rdbRepeatA.setEnabled(false);	
		int nRepeat = m_Player.GetRepeatSel();
		if (nRepeat < 0)
			m_rdbRepeatNone.setChecked(true);
		else if (nRepeat == 0)
			m_rdbRepeatA.setChecked(true);		
		else if (nRepeat == 1)
			m_rdbRepeatB.setChecked(true);			
	}
	
	// @Override
	public int onEvent(int nID, Object obj) {
		if (nID == BasePlayer.YY_EV_Open_Complete) {			
			return 0;
		} else if (nID == BasePlayer.YY_EV_Play_Duration) 			
			m_nDuration = (int)m_Player.GetDuration();
		else if (nID == BasePlayer.YY_EV_Open_Failed) 
			Close ();					
		else if (nID == BasePlayer.YY_EV_Play_Complete) {
			if (m_bRepeatAll) { 
				m_Player.Play();
				m_bPlaying = true;
				m_btnPlay.setText("暂停");
			} else {
				m_bPlaying = false;
				m_btnPlay.setText("播放");
			}
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
	protected void onStart () 
	{
		super.onStart();
		Log.v("@@@YYLOG PlayerActivity", "Player Start");	
		OpenFile (m_strFile);		
	}
	
	@Override
	protected void onPause() 
	{
		super.onPause();
		Log.v("@@@YYLOG PlayerActivity", "Player onPause");		
		if (m_Player != null) {
			m_Player.Pause();
		}		
	}
	
	@Override
	protected void onDestroy() {
		Log.v("@@@YYLOG PlayerActivity", "Player onDestroy");		
		Close ();
		super.onDestroy();
	}	
}

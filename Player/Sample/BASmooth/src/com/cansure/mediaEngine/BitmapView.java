/*******************************************************************************
	File:		AudioRener.java

	Contains:	Audio data render implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.mediaEngine;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Canvas;
import android.graphics.Paint;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffXfermode;
import android.util.AttributeSet;
import android.graphics.Rect;
import android.view.View;

public class BitmapView extends View {
    private Bitmap 	mBitmap = null;
    private Paint	mPaint = null; 
    
    private Rect	m_rcBmp = null;
    private Rect	m_rcView = null;
    
	private BasePlayer	mPlayer = null;
    
    public BitmapView (Context context, AttributeSet attrs) {
        super(context, attrs);
       // setFocusable(true);
        
		mPaint = new Paint();
		mPaint.setXfermode(new PorterDuffXfermode(PorterDuff.Mode.SRC));
		mPaint.setDither(true);
		mPaint.setFilterBitmap(false);	 
		
		m_rcBmp = new Rect ();
		m_rcBmp.top = 0;
		m_rcBmp.left = 0;		
		m_rcView = new Rect ();
		m_rcView.top = 0;
		m_rcView.left = 0;	
	}
    
    void setPlayer (BasePlayer	pPlayer) {
    	mPlayer = pPlayer;
    }
    
    void SetBitmap (Bitmap bmpVideo) {
    	if (mBitmap != bmpVideo) {
    		mBitmap = bmpVideo;
    		if (mBitmap != null) {
	    		m_rcBmp.right = mBitmap.getWidth();
	    		m_rcBmp.bottom = mBitmap.getHeight();
    		}
    	}
    }
    
    @Override 
    protected void onDraw(Canvas canvas) {
    	if (mBitmap != null) {
    		m_rcView.right = getWidth();
    		m_rcView.bottom = getHeight();  		
    		canvas.drawBitmap(mBitmap, m_rcBmp, m_rcView, mPaint);
    		mPlayer.SetParam (BasePlayer.PARAM_PID_BITMAP_DRAW, 1, null);	
    	}
    }
}


package com.cansure.yysampleplayer;

import java.io.File;
import java.lang.reflect.Field;

import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.ActionBar.TabListener;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.Process;
import android.support.v4.view.ViewPager;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewConfiguration;
import android.view.Window;

@SuppressLint("NewApi")
public class MainActivity extends FragmentActivity implements TabListener {
	public	ViewPager 		mViewPager;
	public 	FileListView 	mFragFileList;
	public 	FragLastPlay	mFragLastPlay;
	public 	FragMyBox		mFragMyBox;    
	public 	FragPages		mFragPages;
	
	private int				mCurPage = 0;
	
    
    ViewPager.OnPageChangeListener mListener = new ViewPager.OnPageChangeListener() {
        @Override
        public void onPageSelected(int id) {
            // TODO Auto-generated method stub
        	mCurPage = id;
            getActionBar().setSelectedNavigationItem(id);
        }

        @Override
        public void onPageScrolled(int arg0, float arg1, int arg2) {
            // TODO Auto-generated method stub
        }

        @Override
        public void onPageScrollStateChanged(int arg0) {
            // TODO Auto-generated method stub
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);   
        setContentView(R.layout.activity_main);
        getOverFlow();
        init();
    }

    private void getOverFlow() {
        try {
            ViewConfiguration viewConfig = ViewConfiguration.get(this);
            Field menuKeyField = ViewConfiguration.class.getDeclaredField("sHasPermanentMenuKey");
            if (null != menuKeyField) {
                menuKeyField.setAccessible(true);
                menuKeyField.setBoolean(viewConfig, false);
            }
        } catch (Exception e) {
            // TODO: handle exception     
        }
    }

    private void init() { 
        mFragFileList = new FileListView ();
        mFragLastPlay = new FragLastPlay ();
        mFragMyBox = new FragMyBox();
        mViewPager = (ViewPager) findViewById(R.id.pager);
        mFragPages = new FragPages (this, getSupportFragmentManager());
        mViewPager.setAdapter(mFragPages);
        mViewPager.setPageMarginDrawable(R.drawable.bg_split);
        mViewPager.setPageMargin(3);
        mViewPager.setOnPageChangeListener(mListener);
     
        ActionBar mActionBar = getActionBar();
        if (mActionBar != null) {
	        mActionBar.setHomeButtonEnabled(false);
	        mActionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
	        mActionBar.addTab(mActionBar.newTab().setText(R.string.tabFileList).setTabListener(this));
	        mActionBar.addTab(mActionBar.newTab().setText(R.string.tabLastPlay).setTabListener(this));
	        mActionBar.addTab(mActionBar.newTab().setText(R.string.tabMyBox).setTabListener(this));
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.activity_file_list_view, menu);      
        return true;
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO Auto-generated method stub
        int id = item.getItemId();
        switch (id) {      
            case R.id.menu_settings:
               startActivity(new Intent(this, SettingView.class));
                break;
            case R.id.menu_exit:
                finish();
                System.exit(RESULT_OK);
                break;
            default:
                break;        
        }
        return super.onOptionsItemSelected(item);
    }     

    @Override
    public void onTabReselected(Tab arg0, FragmentTransaction arg1) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onTabSelected(Tab tab, FragmentTransaction arg1) {
        // TODO Auto-generated method stub
        if (mViewPager.getCurrentItem() == tab.getPosition())
            return;
        mViewPager.setCurrentItem(tab.getPosition());
    }

    @Override
    public void onTabUnselected(Tab arg0, FragmentTransaction arg1) {
        // TODO Auto-generated method stub

    }

    private Handler refreshHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
        }
    };

    
    @Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
    	if (mCurPage == 0) { // File list view
    		if (mFragFileList.onKeyDown(keyCode, event))
    			return true;
    	} 	
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			finish();
			System.exit(0);
		}
    	return true;
	}	
}

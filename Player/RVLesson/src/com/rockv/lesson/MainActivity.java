package com.rockv.lesson;

import android.support.v7.app.ActionBarActivity;
import android.os.Bundle;
import android.view.KeyEvent;
import android.content.Intent;
import android.net.Uri;
import android.view.Menu;
import android.view.MenuItem;
import android.os.Environment;
import java.io.File;

import android.webkit.WebView;
import android.webkit.WebSettings;
import android.webkit.WebViewClient;


public class MainActivity extends ActionBarActivity {
	
	private WebView			m_webView = null;	

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
		m_webView = (WebView) findViewById(R.id.webView);	
		m_webView.getSettings().setJavaScriptEnabled(true);
		//Don't show Horizontal & Vertical ScrollBar
		m_webView.setHorizontalScrollBarEnabled(false); 
		m_webView.setVerticalScrollBarEnabled(false);
		m_webView.getSettings().setCacheMode(WebSettings.LOAD_NO_CACHE);
		m_webView.getSettings().setBuiltInZoomControls(true); 
		m_webView.getSettings().setSupportZoom(true); 
		m_webView.setWebViewClient(new RVWebViewClient());
//		m_webView.setWebChromeClient(new RVWebChromeClient());   
		
		
		String strDir = Environment.getExternalStorageDirectory().getPath () + "/RockVL";
		File destDir = new File(strDir);
		if (!destDir.exists()) {
			destDir.mkdirs();
		}
		strDir = strDir + "/DownLoad";
		destDir = new File(strDir);
		if (!destDir.exists()) {
			destDir.mkdirs();
		}
	
//		m_webView.loadUrl("file:///sdcard/RVLesson/L01/index.html");
		m_webView.loadUrl("http://www.rockv.net/tt/");

    }


    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();
        if (id == R.id.action_settings) {
    		m_webView.loadUrl("http://www.baidu.com");
            return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
	    	if (m_webView.canGoBack()) {
				m_webView.goBack();
			} else {
		 		System.exit(0);
			}
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}	    
    
    private class RVWebViewClient extends WebViewClient {
        String m_strPath = Environment.getExternalStorageDirectory().getPath () + "/RockVL/Download/";
        
        @Override
        public boolean shouldOverrideUrlLoading(WebView view, String url) {    	
			String strExt = url.substring(url.length() - 4);
			String strFile = "";
			String strURL = url;
			if (strExt.equalsIgnoreCase(".mkv") || strExt.equalsIgnoreCase(".mp4")) {
				int 	nPos = url.length() - 5;
				char 	chPos;
				while (nPos > 0) {
					chPos = url.charAt(nPos--);
					if (chPos == '/') {
						strFile = url.substring(nPos+2);
						strFile = m_strPath + strFile;
						break;
					}
				}
				if (strFile.length() > 4) {
					File file = new File (strFile);
					if (file.exists()) {
						strURL = strFile;
					} else {
						strURL = "file:pdp" + url.substring(4);
					}
				}
										
				Intent intent = new Intent(MainActivity.this, PlayerView.class);				
				intent.setData(Uri.parse(strURL));															
				startActivity(intent);	
			} else {
		       	view.loadUrl(url);
			}
        	return true;
        }  
      } 
    
}

/*******************************************************************************
	File:		file list view.java

	Contains:	the file list UI implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-05		Fenger			Create file

*******************************************************************************/
package com.cansure.yysampleplayer;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.content.Intent;
import android.graphics.Bitmap;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.SimpleAdapter.ViewBinder;

import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;

import android.util.Log;

import com.cansure.mediaEngine.*;

public class FileListView extends Fragment {
	private static final String		m_strExtVideo = ".avi, .divx, .mp4, .m4v, .mov, .mkv, .3gp, .3g2, .rmvb, .rm, .real, .rv, .asf, .wmv, .flv, .ts, .mpeg, .mpg, .vob, .gif, ";
	private static final String		m_strExtAudio = ".mp3, .mp2, .aac, .amr, .ogg, .wav, .ac3, .awb, .ape, .flac, .wma, .ra, .m4a, ";
	private static final String 	m_strExtImage = ".jpg, .jpeg, .bmp, .png, ";
	private static final String 	m_strExtStream = ".m3u8, .url, ";
	
	private static final String		YY_INFO_NAME = "Name";
	private static final String		YY_INFO_PATH = "Path";
	private static final String		YY_INFO_IMAGE = "Image";
	private static final String		YY_INFO_DIR = "Dir";
	private static final String		YY_INFO_TYPE = "Type";
	
	private static final String		YY_FILE_VIDEO = "Video";
	private static final String		YY_FILE_AUDIO = "Audio";
	private static final String		YY_FILE_IMAGE = "Image";
	private static final String		YY_FILE_STREAM = "Stream";
	
	private String							m_strRootPath = null;
	static private String					m_strListPath = null;
	private ListView						m_lstFiles = null;
	private AdapterView.OnItemClickListener m_lvListener = null;
	
	private Thread							mThread = null;	
	private getThumbnailProc				mThumbProc = null;
	private boolean							mStop = false;
	private int								m_nThumbWidth = 96;
	private int								m_nThumbHeight = 64;	
	private MediaPlayer						m_Player = null;
	

	 @Override
	 public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
		View view = inflater.inflate(R.layout.activity_file_list_view, container, false);	
        initListView (view);
		m_Player = new MediaPlayer();
		String apkPath = "/data/data/" + getActivity().getPackageName() + "/lib/";
		m_Player.Init (getActivity(), apkPath);
	
        m_strRootPath = "/";
        if (m_strListPath == null) {
        	File file = Environment.getExternalStorageDirectory();   
        	updateFileList (file.getPath());
        } else {
            updateFileList (m_strListPath);	
        }    
	    return view;
	 }
	
	 @Override
	 public void onCreate(Bundle savedInstanceState) {			
	     super.onCreate(savedInstanceState);
	     setHasOptionsMenu(true);
	 }	
    
	public void onDestroy() {
		if (m_Player != null) {
			m_Player.Stop();
			m_Player.Uninit();
			m_Player = null;
		}	
	}
	
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK && !m_strListPath.equals(m_strRootPath)) {
			stopThumbProc ();
			File file = new File (m_strListPath);
			m_strListPath = file.getParent();
			updateFileList (m_strListPath);
			return true;
		}
		return false;
	}	
    
    private void initListView (View view) {
       	m_lvListener = new AdapterView.OnItemClickListener() {
    		@Override
    		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
    			SimpleAdapter adapter = (SimpleAdapter)m_lstFiles.getAdapter();
    			Map<String, Object> map = (Map<String, Object>)adapter.getItem(position);		
    			String  strFile = (String) map.get(YY_INFO_PATH);
    			String	strDir = (String) map.get(YY_INFO_DIR);
    
				stopThumbProc ();
				
    			if (strDir.equals("1")) {
    				updateFileList (strFile);
        		} else {  				
    				String strExt = strFile.substring(strFile.length() - 4);
    				if (strExt.equalsIgnoreCase(".url")) {
    					updateURLList (strFile);
    				} else {
        				Intent intent = new Intent(getActivity(), PlayerView.class);		
	    				intent.setData(Uri.parse(strFile));																
	    				startActivity(intent);	
    				}
    			}			
    		}
    	};
    	m_lstFiles = (ListView) view.findViewById(R.id.listViewFile);  	
        m_lstFiles.setOnItemClickListener(m_lvListener);
    }
    
    private void updateURLList (String strFile) {
    	m_strListPath = m_strListPath + "/url";
		ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
		HashMap<String, Object> map;
		
    	try {
	    	FileInputStream fis = new FileInputStream (strFile);
	    	BufferedReader br = new BufferedReader(new InputStreamReader(fis));
	    	String line = null;
	    	while((line = br.readLine())!=null) {
				map = new HashMap<String, Object>();
				map.put(YY_INFO_NAME, line);
				map.put(YY_INFO_PATH, line);
				map.put(YY_INFO_IMAGE, R.drawable.item_video);
				map.put(YY_INFO_DIR, "2");		
				map.put(YY_INFO_TYPE, YY_FILE_STREAM);		
				list.add(map);
	    	}  
    	}catch (Exception e) {
    	    e.printStackTrace();
    	}	   
    	
		SimpleAdapter adapter = new SimpleAdapter(getActivity(), list, R.layout.file_list_item,
									new String[]{YY_INFO_NAME, YY_INFO_IMAGE}, new int[]{R.id.name, R.id.img});    
		m_lstFiles.setAdapter(adapter);	     	
    }
	
    private void updateFileList (String strPath){  	
    	m_strListPath = strPath;
    	
    	ArrayList<Map<String, Object>> list = getFileList(strPath); 	
		SimpleAdapter adapter = new SimpleAdapter(getActivity(), list, R.layout.file_list_item,
									new String[]{YY_INFO_NAME, YY_INFO_IMAGE}, new int[]{R.id.name, R.id.img});    
		
		thumbViewBinder thumbView = new thumbViewBinder ();
		adapter.setViewBinder(thumbView);		
		
		Comparator comp = new nameComparator();
		Collections.sort(list, comp);
		
		m_lstFiles.setAdapter(adapter);	  
		
		startThumbProc ();
    }
    
	private ArrayList<Map<String, Object>> getFileList(String strPath) {
		ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
		File 	fPath = new File(strPath);
		File[] 	fList = fPath.listFiles();
		String	strName = null;
		int		nPointPos = 0;
		CharSequence csExt;
	
		HashMap<String, Object> map;
		if (fList != null) {
			for (int i = 0; i < fList.length; i++) 
			{
				File file = fList[i];
				if (file.isHidden()) 
					continue;					
				map = new HashMap<String, Object>();
				map.put(YY_INFO_NAME, file.getName());
				map.put(YY_INFO_PATH, file.getPath());
				if (file.isDirectory()){
					if (getSubFileCount (file.getParent()) <= 0) {
						map = null;
						continue;
					}
					map.put(YY_INFO_IMAGE, R.drawable.item_folder);
					map.put(YY_INFO_DIR, "1");
				}
				else {	
					strName = file.getName();
					nPointPos = strName.lastIndexOf('.');
					if (nPointPos <= 0)
						continue;
					strName.toLowerCase();
					csExt = strName.subSequence(nPointPos,  strName.length());
					if (m_strExtVideo.contains(csExt))
						map.put(YY_INFO_TYPE, YY_FILE_VIDEO);						
					else if (m_strExtAudio.contains(csExt))
						map.put(YY_INFO_TYPE, YY_FILE_AUDIO);	
					else if (m_strExtImage.contains(csExt)) {
						map.put(YY_INFO_TYPE, YY_FILE_IMAGE);	
						map = null;
						continue;
					}
					else if (m_strExtStream.contains(csExt))
						map.put(YY_INFO_TYPE, YY_FILE_STREAM);	
					else {
						map = null;
						continue;
					}						
					map.put(YY_INFO_IMAGE, R.drawable.item_video);
					map.put(YY_INFO_DIR, "0");
				}
				list.add(map);
			}
		}		
		return list;
	} 	
	
	private int getSubFileCount (String strPath) {
		File 	fPath = new File(strPath);
		File[] 	fList = fPath.listFiles();
		String	strName = null;
		int		nPointPos = 0;
		CharSequence csExt;
		
		if (fList == null || fList.length <= 0)
			return 0;
		for (int i = 0; i < fList.length; i++) 
		{
			File file = fList[i];
			if (file.isDirectory()){
				return 1;
			} else {	
				strName = file.getName();
				nPointPos = strName.lastIndexOf('.');
				if (nPointPos <= 0)
					return 0;
				strName.toLowerCase();
				csExt = strName.subSequence(nPointPos,  strName.length());
				if (m_strExtVideo.contains(csExt))
					return 1;					
				else if (m_strExtAudio.contains(csExt))
					return 1;
				else if (m_strExtImage.contains(csExt)) {
					return 0;
				}
				else if (m_strExtStream.contains(csExt))
					return 1;
				else {
					continue;
				}						
			}
		}			
		return 0;
	}	
	
	public class thumbViewBinder implements ViewBinder {
		public boolean setViewValue(View view, Object data, String textRepresentation) {
            if((view instanceof ImageView) && (data instanceof Bitmap)) {  
                ImageView imageView = (ImageView) view;  
                Bitmap bmp = (Bitmap) data;  
                imageView.setImageBitmap(bmp);  
                return true;  
            } 
			return false;
		}
	}
	
	public class nameComparator implements Comparator<Object> {
		@SuppressWarnings("unchecked")
		public int compare(Object o1, Object o2) {
			HashMap<String, Object> p1 = (HashMap<String, Object>) o1;
			HashMap<String, Object> p2 = (HashMap<String, Object>) o2;
			String strName1, strName2, strDir1, strDir2;
			strName1 = (String) p1.get(YY_INFO_NAME);
			strName2 = (String) p2.get(YY_INFO_NAME);
			strDir1 = (String) p1.get(YY_INFO_DIR);
			strDir2 = (String) p2.get(YY_INFO_DIR);

			if (strDir1.equals("1") && strDir2.equals("0") ){
				return -1;
			} else if (strDir1.equals("0") && strDir2.equals("1") ){
				return 1;
			} else {
				if (strName1.compareToIgnoreCase(strName2) > 0) {
					return 1;
				} else {
					return -1;
				}
			}
		}
	}	
	
	private boolean startThumbProc () {
		mStop = false;
		if (mThumbProc == null)
			mThumbProc = new getThumbnailProc (this);
		if (mThread == null) {
			mThread = new Thread (mThumbProc, "yyGetThumbProc");
			mThread.setPriority(Thread.NORM_PRIORITY - 1);
			mThread.start();
		}	
		return true;
	}
	
	private void stopThumbProc () {
		mStop = true;
		while (mThread != null) {
			try {
				Thread.sleep(5);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}			
		}
	}
	
	public void getThumbnailLoop () {
		Map<String, Object> map = null;
		SimpleAdapter adapter = (SimpleAdapter)m_lstFiles.getAdapter();
		int 	nCount = adapter.getCount();
		String 	strType = null;
		for (int i = 0; i < nCount; i++) {
			map = (Map<String, Object>)adapter.getItem(i);	
			strType = (String)map.get(YY_INFO_TYPE);
			if (strType == null || !strType.equalsIgnoreCase (YY_FILE_VIDEO))
				continue;
			String  strFile = (String) map.get(YY_INFO_PATH);
			String	strDir = (String) map.get(YY_INFO_DIR);
			if (!strDir.equals("1")) {							
				Log.v ("@@@YYLOG", strFile);
				Bitmap bmpImg = Bitmap.createBitmap(m_nThumbWidth, m_nThumbHeight, Bitmap.Config.ARGB_8888 );			
				int nRC = m_Player.GetThumb(strFile, m_nThumbWidth, m_nThumbHeight, bmpImg);
				if (nRC >= 0) {
					map.put(YY_INFO_IMAGE, bmpImg);		
					Message msg = mHandle.obtainMessage ();
					msg.sendToTarget();	
				}
			}
			if (mStop)
				break;
		}

		mThread = null;
	}
	
    private class getThumbnailProc implements Runnable {
        private FileListView 	mLstView = null;
        public getThumbnailProc (FileListView lstView) {
        	mLstView = lstView;
        }
        public void run () { 
        	mLstView.getThumbnailLoop();
        }
    }
    
	private Handler mHandle = new Handler()  {
		public void handleMessage(Message msg) {	
			m_lstFiles.invalidateViews();
		}
	};   	
}

/*******************************************************************************
	File:		file list view.java

	Contains:	the file list UI implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-01-05		Fenger			Create file

*******************************************************************************/
package com.rockv.player;

import java.io.File;
import android.net.Uri;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.Map;

import android.os.Bundle;
import android.os.Environment;
import android.app.Activity;
import android.content.Intent;
import android.view.KeyEvent;
import android.view.View;
import android.widget.AdapterView;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;

public class MainActivity extends Activity {
	private String							m_strRootPath = null;
	private ListView						m_lstFiles = null;
	private AdapterView.OnItemClickListener m_lvListener = null;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {			
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        
        initListView ();
        File file = Environment.getExternalStorageDirectory();   
        m_strRootPath = file.getPath();
        m_strRootPath = m_strRootPath + "/RockV/Class";
        updateFileList (m_strRootPath);
    }
    
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
	    	System.exit(0);
			return true;
		}
		return super.onKeyDown(keyCode, event);
	}	
    
    private void initListView () {
       	m_lvListener = new AdapterView.OnItemClickListener() {
    		@Override
    		public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
    			SimpleAdapter adapter = (SimpleAdapter)m_lstFiles.getAdapter();
    			Map<String, Object> map = (Map<String, Object>)adapter.getItem(position);		
    			String  strFile = (String) map.get("path");
    			String	strDir = (String) map.get("dir");
    			
    			if (strDir.equals("1")) {
    				updateFileList (strFile);
        		} else {  				
    				Intent intent = new Intent(MainActivity.this, PlayerActivity.class);				
    				intent.setData(Uri.parse(strFile));																
    				startActivity(intent);	
    			}			
    		}
    	};
    	m_lstFiles = (ListView)findViewById (R.id.listViewFile);   	
        m_lstFiles.setOnItemClickListener(m_lvListener);
    }
	
    private void updateFileList (String strPath){  	    	
    	ArrayList<Map<String, Object>> list = getFileList(strPath); 	
		SimpleAdapter adapter = new SimpleAdapter(this, list, R.layout.list_item,
				new String[]{"name","img"}, new int[]{R.id.name, R.id.img});    
		
		Comparator comp = new nameComparator();
		Collections.sort(list, comp);
		
		m_lstFiles.setAdapter(adapter);	   	
    }
    
	private ArrayList<Map<String, Object>> getFileList(String strPath) {
		ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
		File 	fPath = new File(strPath);
		File[] 	fList = fPath.listFiles();
		String	strName = "";
		
		HashMap<String, Object> map;
		if (fList != null) {
			for (int i = 0; i < fList.length; i++) 
			{
				File file = fList[i];
				if (file.isHidden()) 
					continue;
				if (file.isDirectory()){
					continue;
				}	
				String strFile = file.getName();
				String strExt = strFile.substring(strFile.length() - 4);
				if (!strExt.equalsIgnoreCase(".rkv")) 
					continue;
				strName = strFile.substring(0, strFile.length() - 4);
				
				map = new HashMap<String, Object>();
				map.put("name", strName);
				map.put("path", file.getPath());
				map.put("img", R.drawable.item_video);
				map.put("dir", "0");
				list.add(map);
			}
		}		
		return list;
	}    

	public class nameComparator implements Comparator<Object> {
		@SuppressWarnings("unchecked")
		public int compare(Object o1, Object o2) {
			HashMap<String, Object> p1 = (HashMap<String, Object>) o1;
			HashMap<String, Object> p2 = (HashMap<String, Object>) o2;
			String strName1, strName2, strDir1, strDir2;
			strName1 = (String) p1.get("name");
			strName2 = (String) p2.get("name");
			strDir1 = (String) p1.get("dir");
			strDir2 = (String) p2.get("dir");

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
}

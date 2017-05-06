/*******************************************************************************
	File:		FileListView.java

	Contains:	List file implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-28		Fenger			Create file

*******************************************************************************/
package com.cansure.demoplayer;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;

import android.app.ListActivity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.TextView;
import android.widget.Toast;

public class FileListView extends ListActivity {
	private static final String TAG = "FileListView";
	private static final int PLAYER_RETURN_CODE = 0;

	private String rootPath = "/sdcard";
	private TextView mPath;
	private LinearLayout mLayout;
	private TextView mSDText;
	private LinearLayout mListView;
	private List<String> mURLList;
	private List<String> mURLPlayList = null;


	private SimpleAdapter mSchedule;
	private ArrayList<HashMap<String, Object>> mylist = null;

	BroadcastReceiver mReceiver = new BroadcastReceiver() {
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			Log.v(TAG, action);
			if (action.equals(Intent.ACTION_MEDIA_UNMOUNTED)) {
				mLayout.setVisibility(View.VISIBLE);
				mListView.setVisibility(View.INVISIBLE);
				mSDText.setText(R.string.str_err_nosd);
				mPath.setText("");
			} else if (action.equals(Intent.ACTION_MEDIA_REMOVED)) {
				mSDText.setText(R.string.str_err_nosd);
				mPath.setText("");
			} else if (action.equals(Intent.ACTION_MEDIA_EJECT)) {
				mSDText.setText(R.string.str_err_nosd);
				mPath.setText("");
			} else if (action.equals(Intent.ACTION_MEDIA_MOUNTED)) {
				mLayout.setVisibility(View.INVISIBLE);
				mListView.setVisibility(View.VISIBLE);

				getFileDir(rootPath);
			}
		}
	};

	// Called when the activity is first created
	protected void onCreate(Bundle icicle) {
		super.onCreate(icicle);
		setContentView(R.layout.filelist_page);
		Log.v(TAG, "FilelistView onCreate");
		mPath = (TextView) findViewById(R.id.mPath);
		mLayout = (LinearLayout) findViewById(R.id.sdstatuslayout);
		mSDText = (TextView) findViewById(R.id.sdstatus);
		mListView = (LinearLayout) findViewById(R.id.listView);

		// CommonFunc.copyConfigfiles(this);
		// Log.v(TAG, "product: " + CommonFunc.product
		// + "; model: " + CommonFunc.model);

		rootPath = "/";

		if (checkSDCard()) {
			mLayout.setVisibility(View.INVISIBLE);
			String sdpath = searchSDCard();
			if (sdpath.length() == 0)
				sdpath = rootPath;
			getFileDir(sdpath);
		}
	}

	private String searchSDCard() {
		String[] searchRoots = { "/mnt", "/" };
		for (int i = 0; i < 2; ++i) {
			File dir = new File(searchRoots[i]);
			File[] files = dir.listFiles();
			if (files == null)
				continue;
			for (int j = 0; j < files.length; ++j) {
				File f = files[j];
				if (f.getName().contentEquals("sdcard")) {
					return (j != 0) ? "/sdcard" : searchRoots[i] + "/sdcard";
				}
			}
		}
		return "";
	}

	@SuppressWarnings("unchecked")
	private void getFileDir(String filePath) {
		mPath.setText(filePath);
		mylist = new ArrayList<HashMap<String, Object>>();
		File f = new File(filePath);
		File[] files = f.listFiles();
		HashMap<String, Object> map;

		if (!filePath.equals(rootPath)) {
			map = new HashMap<String, Object>();
			map.put("icon", R.drawable.folder_up);
			map.put("text1",
					getResources().getString(R.string.str_Back_To_Parent));
			map.put("path", f.getParent());

			mylist.add(map);
		}

		if (files != null) 
		{
			for (int i = 0; i < files.length; i++) 
			{
				File file = files[i];
				if (file.isHidden()) 
				{
					continue;
				}

				// if (file.isDirectory()||(
				// !file.isDirectory()&&checkFileExt(file.getName())!=R.drawable.folder))

				map = new HashMap<String, Object>();
				map.put("icon", CommonFunc.checkFileExt(file.getPath()));
				map.put("text1", file.getName());
				map.put("path", file.getPath());
				if (file.isFile()) 
				{
					map.put("icon", R.drawable.file);
					java.text.DecimalFormat df = new java.text.DecimalFormat("#0.0");
					map.put("text2", df.format((float) file.length() / 1024 / 1024) + "M");
				}
				mylist.add(map);
			}
		}

		Comparator comp = new Mycomparator();
		Collections.sort(mylist, comp);

		setListAdapter(initAdapter());
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * android.app.Activity#onConfigurationChanged(android.content.res.Configuration
	 * )
	 */
	public void onConfigurationChanged(Configuration newConfig) {
		// TODO Auto-generated method stub
		super.onConfigurationChanged(newConfig);
		Log.v(TAG, "Current orientation is " + newConfig.orientation);
	}

	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		menu.clear();

		return super.onPrepareOptionsMenu(menu);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
	 */
	public boolean onCreateOptionsMenu(Menu menu) {
		return super.onCreateOptionsMenu(menu);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onOptionsItemSelected(android.view.MenuItem)
	 */
	public boolean onOptionsItemSelected(MenuItem item) {
		return super.onOptionsItemSelected(item);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onStart()
	 */
	protected void onStart() {
		Log.v(TAG, "FilelistView onStart");
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
		intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_STARTED);
		intentFilter.addAction(Intent.ACTION_MEDIA_SCANNER_FINISHED);
		intentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
		intentFilter.addAction(Intent.ACTION_MEDIA_REMOVED);
		intentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
		intentFilter.addDataScheme("file");

		registerReceiver(mReceiver, intentFilter);
		super.onStart();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onDestroy()
	 */
	protected void onDestroy() {
		Log.v(TAG, "FilelistView onDestroy");
		unregisterReceiver(mReceiver);
		super.onDestroy();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onKeyDown(int, android.view.KeyEvent)
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		Log.v(TAG, "Key click is " + keyCode);

		if (keyCode == KeyEvent.KEYCODE_BACK) {
			super.onKeyDown(keyCode, event);

//			System.exit(0);
			finish ();

			return true;
		}

		return super.onKeyDown(keyCode, event);
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onPause()
	 */
	protected void onPause() {
		Log.v(TAG, "FilelistView onPause");
		super.onPause();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onRestart()
	 */
	protected void onRestart() {
		Log.v(TAG, "FilelistView onRestart");
		super.onRestart();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onResume()
	 */
	protected void onResume() {
		Log.v(TAG, "FilelistView onResume");
		super.onResume();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see android.app.Activity#onStop()
	 */
	protected void onStop() {
		Log.v(TAG, "FilelistView onStop");
		super.onStop();
	}

	public SimpleAdapter initAdapter() {
		mSchedule = new SimpleAdapter(this, mylist, R.layout.file_row,
				new String[] { "icon", "text1", "text2" }, new int[] {
						R.id.ItemImage, R.id.text1, R.id.text2 });
		return mSchedule;
	}

	protected void onListItemClick(ListView l, View v, int position, long id) 
	{
		HashMap<String, Object> map = new HashMap<String, Object>();
		map = mylist.get(position);
		File file = new File((String) map.get("path"));
		if (file.isDirectory()) 
		{
			getFileDir((String) map.get("path"));
		} 
		else 
		{
			Intent intent = new Intent();
			String str = file.getPath();
			intent.setData(Uri.parse(str));
			intent.putExtra("fileName", str);
			int iTop = l.getTop() + v.getTop();
			intent.putExtra("Top", iTop);
			intent.putExtra("Position", position);
			intent.putExtra("playlist", mylist);
			intent.setClass(FileListView.this, PlayerExt.class);
			String s = "urlPlayList.txt";
			if (str.substring(str.length() - 15).compareToIgnoreCase(s) != 0)
			{																
				startActivity(intent);
			}
			else
			{
				mURLPlayList = new ArrayList<String>();
				mURLPlayList.clear();
				this.ReadUrlInfoToList(this.mURLPlayList, str);
				if (mURLPlayList.size() > 0)
				{
					intent.setData(Uri.parse(mURLPlayList.get(0)));
					intent.putExtra("fileName", mURLPlayList.get(0));
					startActivityForResult(intent, PLAYER_RETURN_CODE);
				}
			}
		}
	}

	private boolean checkSDCard() {
		if (android.os.Environment.getExternalStorageState().equals(
				android.os.Environment.MEDIA_MOUNTED)) {
			return true;
		} else {
			return false;
		}
	}

	public class Mycomparator implements Comparator<Object> {
		@SuppressWarnings("unchecked")
		public int compare(Object o1, Object o2) {
			HashMap<String, Object> p1 = (HashMap<String, Object>) o1;
			HashMap<String, Object> p2 = (HashMap<String, Object>) o2;
			String str1, str2;
			str1 = (String) p1.get("text1");
			File Path1 = new File((String) p1.get("path"));
			str2 = (String) p2.get("text1");
			File Path2 = new File((String) p2.get("path"));

			if (str1.compareToIgnoreCase(getResources().getString(
					R.string.str_Back_To_Parent)) == 0) {
				return -1;
			}

			if (str2.compareToIgnoreCase(getResources().getString(
					R.string.str_Back_To_Parent)) == 0) {
				return 1;
			}

			if (Path1.isDirectory() && Path2.isFile()) {
				return -1;
			} else if (Path1.isFile() && Path2.isDirectory()) {
				return 1;
			} else if (Path1.isDirectory() && Path2.isDirectory()) {
				if (str1.compareToIgnoreCase(str2) > 0) {
					return 1;
				} else {
					return -1;
				}
			}

			if (str1.compareToIgnoreCase(str2) > 0) {
				return 1;
			} else {
				return -1;
			}
		}
	}

	public void onBackPressed() {
		this.finish();
	}

	public void ReadUrlInfoToList(List<String> listUrl, String configureFile) {
		String sUrl, line = "";
		sUrl = configureFile;
		File UrlFile = new File(sUrl);
		if (!UrlFile.exists()) {
			Toast.makeText(this, "Don't find " + sUrl + " file!",
					Toast.LENGTH_LONG).show();
			return;
		}
		FileReader fileread;

		listUrl.add(getResources().getString(R.string.str_SelURL_FirstLine));

		try {
			fileread = new FileReader(UrlFile);
			BufferedReader bfr = new BufferedReader(fileread);
			try {
				while (line != null) {
					line = bfr.readLine();
					if (line != null) {
						listUrl.add(line);
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		}
	}

	public void ReadUrlInfo() {
		ReadUrlInfoToList(mURLList, "/sdcard/url.txt");
	}
}

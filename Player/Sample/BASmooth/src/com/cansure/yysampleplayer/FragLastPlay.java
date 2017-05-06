
package com.cansure.yysampleplayer;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.SimpleAdapter;

public class FragLastPlay extends Fragment {
    private String[] mlist = {
            "b_1", "b_2", "b_3", "b_4", "b_5"
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        View view = inflater.inflate(R.layout.temp_list, container, false);
        ListView mListView = (ListView) view.findViewById(R.id.list);
//        mListView.setAdapter(new ArrayAdapter<String>(getActivity(), R.layout.temp_item, mlist));
        
		ArrayList<Map<String, Object>> list = new ArrayList<Map<String, Object>>();
		HashMap<String, Object> map;
		
		map = new HashMap<String, Object>();
		map.put("name", "It doesn't support in current version!");
		map.put("image", R.drawable.item_video);
		list.add(map);
		
		SimpleAdapter adapter = new SimpleAdapter(getActivity(), list, R.layout.file_list_item,
								new String[]{"name", "image"}, new int[]{R.id.name, R.id.img});    
		mListView.setAdapter(adapter);

        return view;       
    }
}

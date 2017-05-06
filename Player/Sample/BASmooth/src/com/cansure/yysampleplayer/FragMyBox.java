
package com.cansure.yysampleplayer;

import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;

public class FragMyBox extends Fragment {
    private String[] mlist = {
            "It doesn't support in current version!"
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO Auto-generated method stub

        View view = inflater.inflate(R.layout.temp_list, container, false);
        ListView mListView = (ListView) view.findViewById(R.id.list);
        mListView.setAdapter(new ArrayAdapter<String>(getActivity(), R.layout.temp_item, mlist));
        return view;
    }

}

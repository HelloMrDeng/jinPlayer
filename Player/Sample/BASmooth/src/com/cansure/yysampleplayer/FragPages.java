
package com.cansure.yysampleplayer;

import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentManager;
import android.support.v4.app.FragmentPagerAdapter;
import android.util.Log;

public class FragPages extends FragmentPagerAdapter {

    private MainActivity mainActivity;

    public FragPages (MainActivity mainActivity, FragmentManager fm) {
        super(fm);
        this.mainActivity = mainActivity;
    }

    @Override
    public Fragment getItem(int id) {
        // TODO Auto-generated method stub
        Fragment mFragment;
        switch (id) {
            case 0:
                mFragment = mainActivity.mFragFileList;
                break;
            case 1:
                mFragment = mainActivity.mFragLastPlay;
                break;
            case 2:
                mFragment = mainActivity.mFragMyBox;
                break;
            default:
                mFragment = mainActivity.mFragFileList;
                break;
        }
        return mFragment;
    }

    @Override
    public int getCount() {        
        return 3;
    }

}

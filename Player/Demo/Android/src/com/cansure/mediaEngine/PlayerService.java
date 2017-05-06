
package com.cansure.mediaEngine;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;

public class PlayerService extends Service {
	
	public static BasePlayer	m_Player = null;
	
	public void CreatePlayer () {
		m_Player = new BasePlayer ();
	}
	
	public void ResetPlayer () {
 	
    	if (m_Player != null)
    	{
			m_Player.Stop();
			m_Player.Uninit();
			m_Player = null;
    	}
   		
	}
	
    /**
     * Class for clients to access.  Because we know this service always
     * runs in the same process as its clients, we don't need to deal with
     * IPC.
     */
    public class PlayerBinder extends Binder {
    	PlayerService getService() {
            return PlayerService.this;
        }
    }
    
    @Override
    public void onCreate() {
    	super.onCreate (); 	
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i("LocalService", "Received start id " + startId + ": " + intent);
        // We want this service to continue running until it is explicitly
        // stopped, so return sticky.
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
    	
    	super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    // This is the object that receives interactions from clients.  See
    // RemoteService for a more complete example.
    private final IBinder mBinder = new PlayerBinder();

}


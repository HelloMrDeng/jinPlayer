/*******************************************************************************
	File:		RPlayerDef.h

	Contains:	The player resource define header file

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-06		Fenger			Create file

*******************************************************************************/
#ifndef __RPlayerDef_H__
#define __RPlayerDef_H__

// play window MESSAGE and TIMER
#define WM_PLAYER_BASE				WM_USER + 100
#define WM_PLAYER_SetWinText		WM_PLAYER_BASE + 1

// list window MESSAGE and TIMER
#define WM_LiST_BASE				WM_USER + 200
#define WM_LIST_PlayFile			WM_LiST_BASE + 1
#define WT_LIST_BASE				100
#define WT_TYPE_PassWord			WT_LIST_BASE + 1
#define WT_MYBOX_CreateBox			WT_LIST_BASE + 2
#define WT_TYPE_NewFolder			WT_LIST_BASE + 3

// play window MESSAGE and TIMER
#define WM_PLAY_BASE				WM_USER + 300
#define WM_PLAY_Play				WM_PLAY_BASE + 1
#define WM_PLAY_Close				WM_PLAY_BASE + 2
#define WT_PLAY_BASE				200
#define	WT_PLAY_PlayNextFile		WT_PLAY_BASE + 1
#define	WT_PLAY_LButtonUP			WT_PLAY_BASE + 2
#define	WT_PLAY_HideBar				WT_PLAY_BASE + 3
#define	WT_PLAY_HScroll				WT_PLAY_BASE + 4
#define	WT_PLAY_TurnOnMonitor		WT_PLAY_BASE + 5
#define	WT_PLAY_HideCursor			WT_PLAY_BASE + 6
#define	WT_PLAY_ZoomSelect			WT_PLAY_BASE + 7

#define	WPT_LButtonUp_Delay			400
#define	WPT_BAR_ShowTime			5000

// Play bar MESSAGE and TIMER
#define WT_BAR_BASE					300
#define	WT_BAR_UI_Update			WT_BAR_BASE + 1

#define ID_BUTTON_PLAY				1001
#define ID_BUTTON_PAUSE				1002
#define ID_BUTTON_BF				1003
#define ID_BUTTON_FF				1004
#define ID_BUTTON_LIST				1006
#define ID_BUTTON_FULL				1008
#define ID_BUTTON_NORMAL			1009
#define ID_BUTTON_MUTE				1011
#define ID_BUTTON_AUDIO				1012

#endif //__RPlayerDef_H__
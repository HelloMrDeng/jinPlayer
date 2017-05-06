/*******************************************************************************
	File:		CListRes.cpp

	Contains:	The list resource implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-03		Fenger			Create file

*******************************************************************************/
#include "windows.h"

#include "CListRes.h"
#include "resource.h"

#pragma warning (disable : 4996)

CListRes::CListRes(HINSTANCE hInst)
	: CBaseObject ()
	, m_hInst (hInst)
{
	SetObjectName ("CListRes");
	m_hBmpExit = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_EXIT));
	m_hBmpHome = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_HOME));
	m_hBmpFolder = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_FOLDER));
	m_hBmpVideo = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_VIDEO));
	m_hBmpAudio = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_AUDIO));
	m_hBmpImage = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_IMAGE));
	m_hBmpNewFile = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_VIDEO_NEW));
	m_hBmpNewFolder = LoadBitmap (m_hInst, MAKEINTRESOURCE(IDB_BITMAP_FOLDER_NEW));
}	

CListRes::~CListRes(void)
{
	DeleteObject (m_hBmpHome);
	DeleteObject (m_hBmpFolder);
	DeleteObject (m_hBmpVideo);
	DeleteObject (m_hBmpAudio);
	DeleteObject (m_hBmpImage);
	DeleteObject (m_hBmpNewFolder);
	DeleteObject (m_hBmpNewFile);
}


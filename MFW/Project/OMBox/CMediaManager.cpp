/*******************************************************************************
	File:		CMediaManager.cpp

	Contains:	Media Engine implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-24		Fenger			Create file

*******************************************************************************/
#include "CMediaManager.h"

#include "UStringFunc.h"

int YYPlay_SetNotify (void * hPlayer, YYMediaNotifyEvent pFunc, void * pUserData)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;	
	pPlayEng->SetNotifyFunc (pFunc, pUserData);
	return YY_ERR_NONE;
}

int YYPlay_SetView (void * hPlayer, void * hWnd, YYRND_TYPE nType)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;	
	pPlayEng->SetDisplay ((HWND)hWnd, nType);
	return YY_ERR_NONE;
}

int YYPlay_Open (void * hPlayer, const TCHAR *pFile, int nFormat)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	YYM_Player *pPlayer = (YYM_Player *)hPlayer;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	// It was -1 as default in older version.
	if (nFormat == -1)
		nFormat = 0;
	int nRC = pPlayEng->Open (pFile, nFormat);

	return nRC;
}

int YYPlay_Close (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->Close ();
}

int YYPlay_Run (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->Start ();
}

int YYPlay_Pause (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->Pause ();
}

int YYPlay_Stop (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->Stop ();
}

YYPLAY_STATUS YYPlay_GetStatus (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_PLAY_Init;

	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->GetStatus ();
}

int YYPlay_GetDur (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;	
	return pPlayEng->GetDuration ();
}

int YYPlay_GetPos (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->GetPos ();
}

int YYPlay_SetPos (void * hPlayer, int nPos)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->SetPos (nPos);
}

int	YYPlay_SetVolume (void * hPlayer, int nVolume)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->SetVolume (nVolume);
}

int	YYPlay_GetVolume (void * hPlayer)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->GetVolume ();
}

int YYPlay_UpdateView (void * hPlayer, RECT * rcView)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->UpdateView (rcView);
}

HBITMAP YYPlay_GetThumb (void * hPlayer, const TCHAR * pFile, YYINFO_Thumbnail * pThumbInfo)
{
	if (hPlayer == NULL)
		return NULL;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return (HBITMAP)pPlayEng->GetThumb (pFile, pThumbInfo);
}

int YYPLay_MediaInfo (void * hPlayer, TCHAR * pInfo, int nSize)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->GetMediaInfo (pInfo, nSize);
}

int YYPlay_GetParam (void * hPlayer, int nID, void * pParam)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;	
	return pPlayEng->GetParam (nID, pParam);
}

int YYPlay_SetParam (void * hPlayer, int nID, void * pParam)
{
	if (hPlayer == NULL)
		return YY_ERR_ARG;
	COMBoxMng * pPlayEng = (COMBoxMng *)((YYM_Player *)hPlayer)->hPlayer;
	return pPlayEng->SetParam (nID, pParam);
}

CMediaManager::CMediaManager(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
{
	for (int i = 0; i < MAX_PLAYER_NUM; i++)
	{
		m_mmPlayer[i].pInterface = NULL;
		m_mmPlayer[i].pPlayer = NULL;
	}
}

CMediaManager::~CMediaManager(void)
{
	for (int i = 0; i < MAX_PLAYER_NUM; i++)
	{
		if (m_mmPlayer[i].pInterface != NULL)
		{
			delete m_mmPlayer[i].pInterface;
			m_mmPlayer[i].pInterface = NULL;
		}

		if (m_mmPlayer[i].pPlayer != NULL)
		{
			m_mmPlayer[i].pPlayer->Stop ();
			delete m_mmPlayer[i].pPlayer;
			m_mmPlayer[i].pPlayer = NULL;
		}
	}
}

int CMediaManager::CreateMedia (int nType, void ** ppMedia)
{
	if (ppMedia == NULL)
		return YY_ERR_ARG;
	*ppMedia = NULL;

	if ((nType & 0X0F) == YYME_PLAYER)
	{
		for (int i = 0; i < MAX_PLAYER_NUM; i++)
		{
			if (m_mmPlayer[i].pInterface != NULL)
				continue;

			m_mmPlayer[i].pInterface = new YYM_Player ();
			if (m_mmPlayer[i].pInterface == NULL)
				return YY_ERR_MEMORY;

			m_mmPlayer[i].pPlayer = new COMBoxMng (m_hInst, m_mmPlayer[i].pInterface);
			if (m_mmPlayer[i].pPlayer == NULL)
				return YY_ERR_MEMORY;

			YYM_Player * pITF = m_mmPlayer[i].pInterface;
			pITF->nVersion = YYMM_PLAYER_VERSION;
			pITF->hPlayer = m_mmPlayer[i].pPlayer;

			pITF->SetNotify = YYPlay_SetNotify;
			pITF->SetView   = YYPlay_SetView;
			pITF->Open	    = YYPlay_Open;
			pITF->Close	    = YYPlay_Close;
			pITF->Run	    = YYPlay_Run;
			pITF->Pause	    = YYPlay_Pause;
			pITF->Stop	    = YYPlay_Stop;
			pITF->GetStatus = YYPlay_GetStatus;
			pITF->GetDur    = YYPlay_GetDur;
			pITF->GetPos    = YYPlay_GetPos;
			pITF->SetPos    = YYPlay_SetPos;
			pITF->SetVolume = YYPlay_SetVolume;
			pITF->GetVolume = YYPlay_GetVolume;
			pITF->UpdateView= YYPlay_UpdateView;
			pITF->MediaInfo = YYPLay_MediaInfo;
			pITF->GetThumb  = YYPlay_GetThumb;
			pITF->GetParam  = YYPlay_GetParam;
			pITF->SetParam  = YYPlay_SetParam;

			*ppMedia = pITF;

			return YY_ERR_NONE;
		}
	}
	
	return YY_ERR_IMPLEMENT;
}

int CMediaManager::DestroyMedia (int nType, void * pMedia)
{
	if (pMedia == NULL)
		return YY_ERR_ARG;

	for (int i = 0; i < MAX_PLAYER_NUM; i++)
	{
		if (m_mmPlayer[i].pInterface == pMedia)
		{
			delete m_mmPlayer[i].pInterface;
			m_mmPlayer[i].pInterface = NULL;

			m_mmPlayer[i].pPlayer->Stop ();
			delete m_mmPlayer[i].pPlayer;
			m_mmPlayer[i].pPlayer = NULL;

			return YY_ERR_NONE;
		}
	}

	return YY_ERR_IMPLEMENT;
}

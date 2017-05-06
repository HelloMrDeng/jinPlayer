/*******************************************************************************
	File:		CVideoRender.cpp

	Contains:	file info dialog implement code

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-14		Fenger			Create file

*******************************************************************************/
#include "windows.h"
#include "commctrl.h"

#include "tchar.h"
#include "stdint.h"

#include "CDlgFileProp.h"
#include "Resource.h"
#include "CBaseUtils.h"
#include "USystemFunc.h"

#define CHKRET(fmt, ...) \
if (nRC < 0) \
{			 \
	TCHAR szMsg[256]; \
	_stprintf(szMsg, _T(" ") fmt _T("return 0X%08X \r\n"), __VA_ARGS__, nRC); \
	MessageBox (m_hDlg, szMsg, _T("Error"), MB_OK); \
	return nRC; \
}

CDlgFileProp::CDlgFileProp(HINSTANCE hInst, HWND hParent)
	: m_hInst (hInst)
	, m_hParent (hParent)
	, m_hDlg (NULL)
	, m_hCmbFile (NULL)
	, m_hEdtInfo (NULL)
	, m_pFmtCtx (NULL)
	, m_pInfoFile (NULL)
	, m_ppInfoStream (NULL)
	, m_hThreadReadPacket (NULL)
	, m_nReadStatus (YY_PLAY_Stop)
{
	_tcscpy (m_szFileName, _T(""));

	CBaseUtils::FillExtIOFunc (&m_ioFileExt);
}

CDlgFileProp::~CDlgFileProp(void)
{
	CloseFile ();
}

int CDlgFileProp::OpenDlg (const TCHAR * pFile)
{
	if (pFile != NULL)
		_tcscpy (m_szFileName, pFile);
	int nRC = DialogBoxParam (m_hInst, MAKEINTRESOURCE(IDD_DIALOG_FILEINFO), m_hParent, FileInfoDlgProc, (LPARAM)this);

	if (nRC == -1)
	{
		DWORD dwErr = GetLastError ();
		dwErr = dwErr;
	}
	return nRC;
}

INT_PTR CALLBACK CDlgFileProp::FileInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int				wmId, wmEvent;
	RECT			rcDlg;
	CDlgFileProp *	pDlgInfo = NULL;

	if (hDlg != NULL)
	{
		GetClientRect (hDlg, &rcDlg);
		pDlgInfo = (CDlgFileProp *)GetWindowLong (hDlg, GWL_USERDATA);
	}

	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetWindowLong (hDlg, GWL_USERDATA, lParam);
		pDlgInfo = (CDlgFileProp *)lParam;
		pDlgInfo->m_hDlg = hDlg;
		pDlgInfo->m_hCmbFile = GetDlgItem (hDlg, IDC_COMBO_FILE);
		pDlgInfo->m_hEdtInfo = GetDlgItem (hDlg, IDC_EDIT_INFO);

		EnableWindow (GetDlgItem (hDlg, IDC_BUTTON_READ), FALSE);
		SetWindowPos (hDlg, NULL, (GetSystemMetrics (SM_CXSCREEN) - rcDlg.right) / 2, 
						(GetSystemMetrics (SM_CYSCREEN) - rcDlg.bottom ) / 2, 0, 0, SWP_NOSIZE);
		
		if (_tcslen (pDlgInfo->m_szFileName) > 0)
			pDlgInfo->GetFileInfo (pDlgInfo->m_szFileName);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		if (wmEvent == CBN_SELCHANGE)
		{
			int nIndex = SendMessage (pDlgInfo->m_hCmbFile, CB_GETCURSEL, 0, 0);
			pDlgInfo->FillStreamInfo (nIndex);
			break;
		}

		// Parse the menu selections:
		switch (wmId)
		{
		case IDC_BUTTON_OPEN:
		{
			TCHAR szFile[1024];
			memset (szFile, 0, sizeof (szFile));
			if (pDlgInfo->GetFileName (szFile) >= 0)
				pDlgInfo->GetFileInfo (szFile);
		}
			break;

		case IDC_BUTTON_READ:
			if (pDlgInfo->m_nReadStatus == YY_PLAY_Pause)
			{
				pDlgInfo->m_nReadStatus = YY_PLAY_Run;
				SetWindowText (GetDlgItem (hDlg, IDC_BUTTON_READ), _T("Pause"));
			}
			else if (pDlgInfo->m_nReadStatus = YY_PLAY_Run)
			{
				pDlgInfo->m_nReadStatus = YY_PLAY_Pause;
				SetWindowText (GetDlgItem (hDlg, IDC_BUTTON_READ), _T("Resume"));
				yySleep (200000);

				int nCurSel = SendMessage (pDlgInfo->m_hCmbFile, CB_GETCURSEL, 0, 0);
				if (nCurSel > 0)
					SetWindowText (pDlgInfo->m_hEdtInfo, pDlgInfo->m_ppInfoStream[nCurSel - 1]);
			}
			break;

		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, LOWORD(wParam));
			break;

		default:
			break;
		}
		break;

	case WM_DROPFILES:
	{
		HDROP hDrop = (HDROP) wParam;
		int nFiles = DragQueryFile (hDrop, 0XFFFFFFFF, NULL, 0);
		if (nFiles > 0)
		{
			TCHAR szFile[1024];
			memset (szFile, 0, sizeof (szFile));
			int nNameLen = DragQueryFile (hDrop, 0, szFile, sizeof (szFile));
			if (nNameLen > 0)
				pDlgInfo->GetFileInfo (szFile);
		}	
	}
		break;

	default:
		break;
	}

	return (INT_PTR)FALSE;
}

int CDlgFileProp::GetFileInfo (const TCHAR * pFile)
{
	int nRC = 0;

	CloseFile ();

	SetWindowText (GetDlgItem (m_hDlg, IDC_EDIT_FILE), pFile);
	SendMessage (m_hCmbFile, CB_RESETCONTENT, 0, 0);
	SendMessage (GetDlgItem (m_hDlg, IDC_PROGRESS_READ), PBM_SETPOS, 0, 0);

	char szFileName[1024];
	memset (szFileName, 0, sizeof (szFileName));
	sprintf (szFileName, "ExtIO:%08X", &m_ioFileExt);
	char * pFileName = szFileName + strlen (szFileName);
#ifdef _UNICODE
	CBaseUtils::ConvertDataToBase64 ((unsigned char *)pFile, _tcslen (pFile) * sizeof (TCHAR),
										pFileName, sizeof (szFileName) - strlen (szFileName));
#else
	strcat (szFileName, pFile);
#endif // UNICODE

	// open input file, and allocate format context
	nRC = avformat_open_input(&m_pFmtCtx, szFileName, NULL, NULL);
	CHKRET (_T("Open file failed!"));

    // retrieve stream information
    nRC = avformat_find_stream_info(m_pFmtCtx, NULL);
	CHKRET (_T("Find stream info failed!"));

	TCHAR wzItem[256];
	char  szItem[256];
	_stprintf (wzItem, _T("File Info..."));
	SendMessage (m_hCmbFile, CB_ADDSTRING, 0, (LPARAM)wzItem);

	int nStreamCount = m_pFmtCtx->nb_streams;
	AVStream * pStream = NULL;

	for (int i = 0; i < nStreamCount; i++)
	{
		pStream = m_pFmtCtx->streams[i];
		sprintf (szItem, ("Stream %d  %s"), i, av_get_media_type_string (pStream->codec->codec_type));
#ifdef _UNICODE
		MultiByteToWideChar (CP_ACP, 0, szItem, -1, wzItem, sizeof (wzItem));
#else
		strcpy (wzItem, szItem);
#endif // _UNICODE
		SendMessage (m_hCmbFile, CB_ADDSTRING, 0, (LPARAM)wzItem);
	}
	SendMessage (m_hCmbFile, CB_SETCURSEL, 0, 0);

	FillFileInfo ();

	return 0;
}

int CDlgFileProp::FillStreamInfo (int nIndex)
{
	if (m_ppInfoStream == NULL)
	{
		if (m_hThreadReadPacket == NULL)
		{
			DWORD dwID = 0;
			m_nReadStatus = YY_PLAY_Run;
			m_hThreadReadPacket = CreateThread (NULL, 0, (LPTHREAD_START_ROUTINE) ReadPacketProc, this, 0, &dwID);
		}
	}
	else
	{
		int nCurSel = SendMessage (m_hCmbFile, CB_GETCURSEL, 0, 0);
		if (nCurSel > 0)
			SetWindowText (m_hEdtInfo, m_ppInfoStream[nCurSel - 1]);
		else
			SetWindowText (m_hEdtInfo, m_pInfoFile);
	}

	return 0;
}

int CDlgFileProp::FillFileInfo (void)
{
	if (m_pInfoFile != NULL)
	{
		SetWindowText (m_hEdtInfo, m_pInfoFile);
		return 0;
	}

	int		nInfoSize = 1024 * 32;
	m_pInfoFile = new TCHAR[nInfoSize];
	memset (m_pInfoFile, 0, nInfoSize * sizeof (TCHAR));

	char *	pInfoText = new char[nInfoSize];
	memset (pInfoText, 0, nInfoSize);
	char	szInfoLine[256];

	sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("File Format:"), m_pFmtCtx->iformat->name);
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Stream Num:"), m_pFmtCtx->nb_streams);
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Audio Index:"), av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0));
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Video Index:"), av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0));
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Start Time:"), m_pFmtCtx->start_time);
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Duration:"), m_pFmtCtx->duration / 1000);
	strcat (pInfoText, szInfoLine);

	sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Bit Rate:"), m_pFmtCtx->bit_rate);
	strcat (pInfoText, szInfoLine);

	for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
	{
		AVStream *			pStream = m_pFmtCtx->streams[i];
		AVCodecContext *	pCodec = pStream->codec;

		sprintf (szInfoLine, "\r\n\r\nStream %d\r\n", i);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("Media Type:"), av_get_media_type_string (pCodec->codec_type));
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %s\r\n", GetLineText ("Codec Name:"), avcodec_get_name (pCodec->codec_id));
		strcat (pInfoText, szInfoLine);

		if (GetProfileName (pCodec, szInfoLine) > 0)
			strcat (pInfoText, szInfoLine);

		if (pCodec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			sprintf (szInfoLine, "%s  %d X %d\r\n", GetLineText ("Video Size:"), pCodec->width, pCodec->height);
			strcat (pInfoText, szInfoLine);

			sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Gop Size:"), pCodec->gop_size);
			strcat (pInfoText, szInfoLine);

			sprintf (szInfoLine, "%s  %d / %d\r\n", GetLineText ("Aspect Ratio:"), pStream->sample_aspect_ratio.num, pStream->sample_aspect_ratio.den);
			strcat (pInfoText, szInfoLine);
		}
		else if (pCodec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			sprintf (szInfoLine, "%s  %d  %d\r\n", GetLineText ("Audio Format:"), pCodec->sample_rate, pCodec->channels);
			strcat (pInfoText, szInfoLine);
		}

		sprintf (szInfoLine, "%s  %d / %d\r\n", GetLineText ("Time Base:"), pStream->time_base.num, pStream->time_base.den);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Start Time:"), pStream->start_time * 1000 * pStream->time_base.num / pStream->time_base.den);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Duration:"), pStream->duration * 1000 * pStream->time_base.num / pStream->time_base.den);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Frames Num:"), pStream->nb_frames);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %d / %d\r\n", GetLineText ("Frame Rate:"), pStream->avg_frame_rate.num, pStream->avg_frame_rate.den);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Chunk Size:"), pStream->interleaver_chunk_size);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %lld\r\n", GetLineText ("Chunk Duration:"), pStream->interleaver_chunk_duration);
		strcat (pInfoText, szInfoLine);

		sprintf (szInfoLine, "%s  %d\r\n", GetLineText ("Extradata Size:"), pCodec->extradata_size);
		strcat (pInfoText, szInfoLine);

		if (pCodec->extradata_size > 0)
			GetHexString (pCodec->extradata, pCodec->extradata_size, 16, pInfoText + strlen (pInfoText));
	}

#ifdef _UNICODE
	MultiByteToWideChar (CP_ACP, 0, pInfoText, -1, m_pInfoFile, nInfoSize);
#else
	strcpy (m_pInfoFile, pInfoText);
#endif // _UNICODE
	delete []pInfoText;

	SetWindowText (m_hEdtInfo, m_pInfoFile);

	return 0;
}

void CDlgFileProp::CloseFile (void)
{
	m_nReadStatus = YY_PLAY_Stop;
	while (m_hThreadReadPacket != NULL)
		yySleep (10000);

	if (m_pInfoFile != NULL)
		delete []m_pInfoFile;
	m_pInfoFile = NULL;

	if (m_ppInfoStream != NULL)
	{
		for (int i = 0; i < m_pFmtCtx->nb_streams; i++)
			delete []m_ppInfoStream[i];
		delete []m_ppInfoStream;
	}
	m_ppInfoStream = NULL;

	if (m_pFmtCtx != NULL)
		avformat_close_input (&m_pFmtCtx);
	m_pFmtCtx = NULL;
}

char *	CDlgFileProp::GetLineText (char * pLine)
{
	memset (m_szLineText, 0, sizeof (m_szLineText));

	int	nWidth = 32;
	int nLen = 0;
	if (strlen (pLine) < nWidth)
		nLen = nWidth - strlen (pLine);
	strcpy (m_szLineText, pLine);
	for (int i = 0; i < nLen; i++)
		strcat (m_szLineText, " ");

	return m_szLineText;
}

int CDlgFileProp::GetFileName (TCHAR * pFileName)
{
	DWORD				dwID = 0;
	OPENFILENAME		ofn;

	memset( &(ofn), 0, sizeof(ofn));
	ofn.lStructSize	= sizeof(ofn);
	ofn.hwndOwner = m_hDlg;

	ofn.lpstrFilter = TEXT("Media File (*.*)\0*.*\0");	
	ofn.lpstrFile = pFileName;
	ofn.nMaxFile = MAX_PATH;

	ofn.lpstrTitle = TEXT("Open Media File");
	ofn.Flags = OFN_EXPLORER;
			
	if (!GetOpenFileName(&ofn))
		return -1;

	return 0;
}

int CDlgFileProp::GetHexString (unsigned char * pBuffer, int nBuffSize, int nLineSize, char * pText)
{
	char	szHex[8192];
	char	szLine[256];
	char	szWord[32];

	if (pBuffer == NULL || nBuffSize <= 0)
		return 0;

	int nLineNum = nLineSize;
	int nIndex = 0;

	memset (szHex, 0, sizeof (szHex));
	unsigned char * pHexBuff = pBuffer;

	while (pHexBuff - pBuffer < nBuffSize)
	{
		if (nBuffSize - (pHexBuff - pBuffer) < nLineSize)
			nLineNum = nBuffSize - (pHexBuff - pBuffer);

		sprintf (szWord, "0X%04X  ", nIndex);
		strcpy (szLine, szWord);

		int i = 0;
		for (i = 0; i < nLineNum; i++)
		{
			sprintf (szWord, "%02X ", pHexBuff[i]);
			strcat (szLine, szWord);

			if (i == 7)
				strcat (szLine, ("  "));
		}

		strcat (szLine, ("  "));

		if (nLineNum < nLineSize)
		{
			for (i = 0; i < nLineSize - nLineNum; i++)
				strcat (szLine, ("  "));
			strcat (szLine, ("     "));
		}

		for (i = 0; i < nLineNum; i++)
		{
			sprintf (szWord, ("%c"), pHexBuff[i]);
			strcat (szLine, szWord);

			if (i == 7)
				strcat (szLine, ("  "));
		}
		strcat (szLine, ("\r\n"));

		pHexBuff += nLineNum;
		nIndex += nLineNum;

		strcat (szHex, szLine);

		if (strlen (szHex) > 2048)
			break;
	}

	strcpy (pText, szHex);

	int nSize = pHexBuff - pBuffer;
	return nSize;
}

long long CDlgFileProp::BaseToTime (long long llBase, AVStream * pStream)
{
	return llBase * 1000 * pStream->time_base.num / pStream->time_base.den;
}

int CDlgFileProp::ReadPacketProc (void * pParam)
{
	CDlgFileProp * pDlgInfo = (CDlgFileProp *)pParam;

	return pDlgInfo->ReadPacketLoop ();
}

int CDlgFileProp::ReadPacketLoop (void)
{
	int			nRC = 0;
	int			i = 0;
	int			nIdx = 0;
	int			nLineLen = 64;
	int			nStreams = m_pFmtCtx->nb_streams;
	int *		nnStreamSize = new int[nStreams];
	int *		nnIndex = new int[nStreams];
	long long *	lllPrevPTS = new long long[nStreams];
	long long *	lllPrevDTS = new long long[nStreams];
	long long	llDuration = 0;

	TCHAR		szLine[256];
	AVStream *	pStream = NULL;
	AVPacket	avPacket;

	_tcscpy (szLine, _T(" Index   Flag  Type     Size        DTS         Step         PTS        Step   Dur     Pos\r\n"));
	nLineLen = _tcslen (szLine);

	m_ppInfoStream = new TCHAR *[nStreams];
	for (i = 0; i < nStreams; i++)
	{
		pStream = m_pFmtCtx->streams[i];

		if (pStream->duration * pStream->time_base.num / pStream->time_base.den > llDuration)
			llDuration = pStream->duration * pStream->time_base.num / pStream->time_base.den;

		if (pStream->nb_frames > 0)
		{
			nnStreamSize[i] = (pStream->nb_frames + 100) * nLineLen;
		}
		else if (pStream->duration > 0)
		{
			long long llDur = pStream->duration * 1000 * pStream->time_base.num / pStream->time_base.den;
			if (pStream->codec->codec_type == AVMEDIA_TYPE_VIDEO)
				nnStreamSize[i] = (llDur / 30 + 100) * nLineLen;
			else if (pStream->codec->codec_type == AVMEDIA_TYPE_AUDIO)
				nnStreamSize[i] = (llDur / 20 + 100) * nLineLen;
			else
				nnStreamSize[i] = (llDur / 30 + 100) * nLineLen;

		}
		else
		{
			// define two hours len
			nnStreamSize[i] = 7200 * 30 * nLineLen;
		}

		if (nnStreamSize[i] <= 0 || nnStreamSize[i] > 7200 * 30 * nLineLen)
			nnStreamSize[i] = 7200 * 30 * nLineLen;

		m_ppInfoStream[i] = new TCHAR[nnStreamSize[i]];
		_tcscpy (m_ppInfoStream[i], szLine);
		nnIndex[i] = 0;
		lllPrevPTS[i] = 0;
		lllPrevDTS[i] = 0;
	}

	if (llDuration == 0)
		llDuration = m_pFmtCtx->duration / 1000000;

	EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_READ), TRUE);
	LPARAM lParam = MAKELPARAM (0, (int)llDuration);
	SendMessage (GetDlgItem (m_hDlg, IDC_PROGRESS_READ), PBM_SETRANGE, 0, lParam);
	SendMessage (GetDlgItem (m_hDlg, IDC_PROGRESS_READ), PBM_SETPOS, 0, 0);

	av_init_packet (&avPacket);

    AVCodecContext *	pDecCtxVideo = NULL;
	int					nIdxVideo = -1;
	AVCodec *			pDecVideo = NULL;
	AVFrame *			pFrmVideo = NULL;
	int					nGotFrame = 0;
	TCHAR				szType[6];
	long long			llLastVpts = 0;
	bool				bVideodec = false;
	 
	if (SendMessage (GetDlgItem (m_hDlg, IDC_CHECK_VIDEODEC), BM_GETCHECK, 0, 0) == BST_CHECKED)
		bVideodec = true;
	
	CObjectList <PacketInfo>	lstPkt;
	PacketInfo *				pPktInfo = NULL;
	NODEPOS					pos = NULL;

	nIdxVideo = av_find_best_stream(m_pFmtCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (nIdxVideo >= 0)
	{
		pDecCtxVideo = m_pFmtCtx->streams[nIdxVideo]->codec;
		pDecVideo = avcodec_find_decoder (pDecCtxVideo->codec_id);
		if (pDecVideo != NULL)
		{
			pDecCtxVideo->thread_count = 1;
			pDecCtxVideo->thread_type = 1;
			nRC = avcodec_open2 (pDecCtxVideo, pDecVideo, NULL);
			if (nRC < 0)
				nIdxVideo = -1;
			else
				pFrmVideo = avcodec_alloc_frame ();
		}
	}

	while (m_nReadStatus != YY_PLAY_Stop)
	{
		if (m_nReadStatus == YY_PLAY_Pause)
		{
			yySleep (10000);
			continue;
		}

		nRC = av_read_frame (m_pFmtCtx, &avPacket);
		if (nRC < 0)
			break;

		nIdx = avPacket.stream_index;
		pStream = m_pFmtCtx->streams[nIdx];
		if (avPacket.stream_index < 0 ||  avPacket.stream_index >= nStreams)
		{
			av_free_packet (&avPacket);
			continue;
		}
		if (bVideodec && avPacket.stream_index == nIdxVideo)
		{
			pPktInfo = new PacketInfo ();
			pPktInfo->flag		= avPacket.flags;
			pPktInfo->size		= avPacket.size;
			pPktInfo->dts		= avPacket.dts;
			pPktInfo->dts_step	= (int)(BaseToTime (avPacket.dts, pStream) - lllPrevDTS[nIdx]);
			pPktInfo->pts		= avPacket.pts;
			pPktInfo->pts_step	= (int)(BaseToTime (avPacket.pts, pStream) - lllPrevPTS[nIdx]);
			pPktInfo->dur		= avPacket.duration;//(int)(BaseToTime (avPacket.duration, pStream));
			pPktInfo->pos		= avPacket.pos;
			lstPkt.AddTail (pPktInfo);

			if (pDecCtxVideo->codec_id == AV_CODEC_ID_H264)
				pPktInfo->picType = IsRefFrame ((char *)avPacket.data + 4, avPacket.size - 4);
			nRC = avcodec_decode_video2(pDecCtxVideo, pFrmVideo, &nGotFrame, &avPacket);
			if (nGotFrame > 0)
			{
				pos = lstPkt.GetHeadPosition ();
				while (pos != NULL)
				{
					pPktInfo = lstPkt.GetNext (pos);
					if (pPktInfo->pts == pFrmVideo->pkt_pts)
					{
						memset (szType, 0, sizeof (szType));
						szType[0] = av_get_picture_type_char (pFrmVideo->pict_type);
						if (pFrmVideo->pict_type == AV_PICTURE_TYPE_B)
						{
							if (pPktInfo->picType)
								_tcscat (szType, _T("_R"));
							else
								_tcscat (szType, _T("BB"));
						}
						else
						{
							_tcscat (szType, _T("  "));
						}
						
						if (pPktInfo->pts == YY_64_INVALID)
							pPktInfo->pts = llLastVpts + pPktInfo->dur;
						_stprintf (szLine, _T("% 6d   %s   %s  % 8d  % 10lld  % 10d  % 10lld  % 10d  % 4d % 10lld\r\n"),	
											nnIndex[nIdx], 
											pPktInfo->flag ? _T("key") : _T(" 0 "), 
											szType,
											pPktInfo->size, 
											BaseToTime (pPktInfo->dts, pStream),
											pPktInfo->dts_step, 
											BaseToTime (pPktInfo->pts, pStream), 
											(int)BaseToTime (pPktInfo->pts - llLastVpts, pStream), 
											(int)BaseToTime (pPktInfo->dur, pStream), 
											pPktInfo->pos);
						llLastVpts = pPktInfo->pts;
						if (_tcslen (m_ppInfoStream[nIdxVideo]) + nLineLen < nnStreamSize[nIdxVideo])
							_tcscat (m_ppInfoStream[nIdxVideo], szLine);

						lstPkt.Remove (pPktInfo);
						delete pPktInfo;
						break;
					}
				}
			}
		}
		else
		{
			_stprintf (szLine, _T("% 6d   %s        % 8d  % 10lld  % 10d  % 10lld  % 10d  % 4d % 10lld\r\n"),	
								nnIndex[nIdx], 
								avPacket.flags ? _T("key") : _T(" 0 "), 
								avPacket.size, 
								BaseToTime (avPacket.dts, pStream), 
								(int)(BaseToTime (avPacket.dts, pStream) - lllPrevDTS[nIdx]), 
								BaseToTime (avPacket.pts, pStream), 
								(int)(BaseToTime (avPacket.pts, pStream) - lllPrevPTS[nIdx]), 
								(int)(BaseToTime (avPacket.duration, pStream)), 
								avPacket.pos);

			if (_tcslen (m_ppInfoStream[nIdx]) + nLineLen < nnStreamSize[nIdx])
				_tcscat (m_ppInfoStream[nIdx], szLine);
		}
		nnIndex[nIdx]++;
		lllPrevPTS[nIdx] = BaseToTime (avPacket.pts, pStream);
		lllPrevDTS[nIdx] = BaseToTime (avPacket.dts, pStream);
		
		av_free_packet (&avPacket);

		if (nIdx == 0)
		{
			if (nnIndex[nIdx] % 10 == 0)
				SendMessage (GetDlgItem (m_hDlg, IDC_PROGRESS_READ), PBM_SETPOS, (int)lllPrevDTS[nIdx] / 1000, 0);
			else if (nnIndex[nIdx] % 10 == 5)
				SendMessage (GetDlgItem (m_hDlg, IDC_PROGRESS_READ), PBM_SETPOS, (int)(lllPrevDTS[nIdx] / 1000 - 5), 0);
		}
		//if (nnIndex[nIdx] > 1000)
		//	break;
	}
//	_tcscpy (szLine, _T("Index   Flag    Size    Start   Duration    DTS   Pos\r\n"));

	delete []nnStreamSize;
	delete []nnIndex;
	delete []lllPrevPTS;
	delete []lllPrevDTS;

	if (nIdxVideo >= 0)
	{
		if (pDecCtxVideo != NULL)
			avcodec_close (pDecCtxVideo);
		if (pFrmVideo != NULL)
			avcodec_free_frame (&pFrmVideo);
	}

	pPktInfo = lstPkt.RemoveHead ();
	while (pPktInfo != NULL)
	{
		delete pPktInfo;
		pPktInfo = lstPkt.RemoveHead ();
	}

	m_hThreadReadPacket = NULL;
	m_nReadStatus = YY_PLAY_Stop;

	int nCurSel = SendMessage (m_hCmbFile, CB_GETCURSEL, 0, 0);
	if (nCurSel > 0)
		SetWindowText (m_hEdtInfo, m_ppInfoStream[nCurSel - 1]);
	EnableWindow (GetDlgItem (m_hDlg, IDC_BUTTON_READ), FALSE);

	return 0;
}

#define XRAW_IS_ANNEXB(p) ( !(*((p)+0)) && !(*((p)+1)) && (*((p)+2)==1))
#define XRAW_IS_ANNEXB2(p) ( !(*((p)+0)) && !(*((p)+1)) && !(*((p)+2))&& (*((p)+3)==1))

bool CDlgFileProp::IsRefFrame (char * buffer , int size)
{
	int naluType = buffer[0]&0x0f;
	int isRef	 = 1;
	while(naluType!=1&&naluType!=5)//find next NALU
	{
		//buffer = GetNextFrame(buffer,size)
		char* p = buffer;  
		char* endPos = buffer+size;
		for (; p < endPos; p++)
		{
			if (XRAW_IS_ANNEXB(p))
			{
				size  -= p-buffer;
				buffer = p+3;
				naluType = buffer[0]&0x0f;
				break;
			}
			if (XRAW_IS_ANNEXB2(p))
			{
				size  -= p-buffer;
				buffer = p+4;
				naluType = buffer[0]&0x0f;
				break;
			}
		}
		if(p>=endPos)
			return true; 
	}
	
	if(naluType == 5)
		return true;

	//if(naluType==1)
	{
		isRef = (buffer[0]>>5) & 3;

		//VOLOGI("....isRef %d", (int)isRef);
	}
	return (isRef != 0);
}

int CDlgFileProp::GetProfileName (AVCodecContext * pCodec, char * pInfo)
{
	if (pCodec == NULL || pInfo == NULL)
		return 0;

	strcpy (pInfo, GetLineText ("Profile:"));
	if (pCodec->codec_id == AV_CODEC_ID_AAC)
	{
		if (pCodec->profile == FF_PROFILE_AAC_MAIN)
			strcat (pInfo, "  Main");
		else if (pCodec->profile == FF_PROFILE_AAC_LOW)
			strcat (pInfo, "  Low");
		else if (pCodec->profile == FF_PROFILE_AAC_SSR)
			strcat (pInfo, "  SSR");
		else if (pCodec->profile == FF_PROFILE_AAC_LTP)
			strcat (pInfo, "  LTP");
		else if (pCodec->profile == FF_PROFILE_AAC_HE)
			strcat (pInfo, "  HE");
		else if (pCodec->profile == FF_PROFILE_AAC_HE_V2)
			strcat (pInfo, "  HE-V2");
		else if (pCodec->profile == FF_PROFILE_AAC_LD)
			strcat (pInfo, "  LD");
		else if (pCodec->profile == FF_PROFILE_AAC_ELD)
			strcat (pInfo, "  ELD");
	}
	else if (pCodec->codec_id == AV_CODEC_ID_DTS)
	{
		if (pCodec->profile == FF_PROFILE_DTS_ES)
			strcat (pInfo, "  ES");
		else if (pCodec->profile == FF_PROFILE_DTS_96_24)
			strcat (pInfo, "  96-24");
		else if (pCodec->profile == FF_PROFILE_DTS_HD_HRA)
			strcat (pInfo, "  HD-HRA");
		else if (pCodec->profile == FF_PROFILE_DTS_HD_MA)
			strcat (pInfo, "  HD-MA");
	}
	else if (pCodec->codec_id == AV_CODEC_ID_MPEG2VIDEO)
	{
		if (pCodec->profile == FF_PROFILE_MPEG2_HIGH)
			strcat (pInfo, "  HIGH");
		else if (pCodec->profile == FF_PROFILE_MPEG2_SS)
			strcat (pInfo, "  SS");
		else if (pCodec->profile == FF_PROFILE_MPEG2_SNR_SCALABLE)
			strcat (pInfo, "  Scalable");
		else if (pCodec->profile == FF_PROFILE_MPEG2_MAIN)
			strcat (pInfo, "  Main");
		else if (pCodec->profile == FF_PROFILE_MPEG2_SIMPLE)
			strcat (pInfo, "  Simple");
	}
	else if (pCodec->codec_id == AV_CODEC_ID_MPEG2VIDEO)
	{
		if (pCodec->profile == FF_PROFILE_MPEG2_HIGH)
			strcat (pInfo, "  HIGH");
		else if (pCodec->profile == FF_PROFILE_MPEG2_SS)
			strcat (pInfo, "  SS");
		else if (pCodec->profile == FF_PROFILE_MPEG2_SNR_SCALABLE)
			strcat (pInfo, "  Scalable");
		else if (pCodec->profile == FF_PROFILE_MPEG2_MAIN)
			strcat (pInfo, "  Main");
		else if (pCodec->profile == FF_PROFILE_MPEG2_SIMPLE)
			strcat (pInfo, "  Simple");
	}
	else if (pCodec->codec_id == AV_CODEC_ID_H264)
	{
		if (pCodec->profile == FF_PROFILE_H264_CONSTRAINED)
			strcat (pInfo, "  Constrained");
		else if (pCodec->profile == FF_PROFILE_H264_INTRA)
			strcat (pInfo, "  Intra");
		else if (pCodec->profile == FF_PROFILE_H264_BASELINE)
			strcat (pInfo, "  BaseLine");
		else if (pCodec->profile == FF_PROFILE_H264_CONSTRAINED_BASELINE)
			strcat (pInfo, "  Constrained BaseLine");
		else if (pCodec->profile == FF_PROFILE_H264_MAIN)
			strcat (pInfo, "  Main");
		else if (pCodec->profile == FF_PROFILE_H264_EXTENDED)
			strcat (pInfo, "  Extended");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH)
			strcat (pInfo, "  High");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_10)
			strcat (pInfo, "  High 10");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_10_INTRA)
			strcat (pInfo, "  High 10 Intra");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_422)
			strcat (pInfo, "  High 422");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_422_INTRA)
			strcat (pInfo, "  High 422 Intra");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_444)
			strcat (pInfo, "  High 444");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_444_PREDICTIVE)
			strcat (pInfo, "  High 444 Predictive");
		else if (pCodec->profile == FF_PROFILE_H264_HIGH_444_INTRA)
			strcat (pInfo, "  High 444 Intra");
		else if (pCodec->profile == FF_PROFILE_H264_CAVLC_444)
			strcat (pInfo, "  Cavlc 444");
	}
	else if (pCodec->codec_id == AV_CODEC_ID_VC1)
	{
		if (pCodec->profile == FF_PROFILE_VC1_SIMPLE)
			strcat (pInfo, "  Simple");
		else if (pCodec->profile == FF_PROFILE_VC1_MAIN)
			strcat (pInfo, "  Main");
		else if (pCodec->profile == FF_PROFILE_VC1_COMPLEX)
			strcat (pInfo, "  Complex");
		else if (pCodec->profile == FF_PROFILE_VC1_ADVANCED)
			strcat (pInfo, "  Advanced");
	}
	else if (pCodec->codec_id == AV_CODEC_ID_MPEG4)
	{
		if (pCodec->profile == FF_PROFILE_MPEG4_SIMPLE)
			strcat (pInfo, "  Simple");
		else if (pCodec->profile == FF_PROFILE_MPEG4_SIMPLE_SCALABLE)
			strcat (pInfo, "  Simple Scalable");
		else if (pCodec->profile == FF_PROFILE_MPEG4_CORE)
			strcat (pInfo, "  Core");
		else if (pCodec->profile == FF_PROFILE_MPEG4_MAIN)
			strcat (pInfo, "  Main");
		else if (pCodec->profile == FF_PROFILE_MPEG4_N_BIT)
			strcat (pInfo, "  N_BIT");
		else if (pCodec->profile == FF_PROFILE_MPEG4_SCALABLE_TEXTURE)
			strcat (pInfo, "  Scalable  Texture");
		else if (pCodec->profile == FF_PROFILE_MPEG4_SIMPLE_FACE_ANIMATION)
			strcat (pInfo, "  Simple Face Animation");
		else if (pCodec->profile == FF_PROFILE_MPEG4_BASIC_ANIMATED_TEXTURE)
			strcat (pInfo, "  Basic Animated Texture");
		else if (pCodec->profile == FF_PROFILE_MPEG4_ADVANCED_REAL_TIME)
			strcat (pInfo, "  Advanced Real Time");
		else if (pCodec->profile == FF_PROFILE_MPEG4_CORE_SCALABLE)
			strcat (pInfo, "  Core Scalable");
		else if (pCodec->profile == FF_PROFILE_MPEG4_ADVANCED_CODING)
			strcat (pInfo, "  Advanced Coding");
		else if (pCodec->profile == FF_PROFILE_MPEG4_ADVANCED_CORE)
			strcat (pInfo, "  Advanced Core");
		else if (pCodec->profile == FF_PROFILE_MPEG4_ADVANCED_SCALABLE_TEXTURE)
			strcat (pInfo, "  Advanced Scalable Texture");
		else if (pCodec->profile == FF_PROFILE_MPEG4_SIMPLE_STUDIO)
			strcat (pInfo, "  Simple Studio");
		else if (pCodec->profile == FF_PROFILE_MPEG4_ADVANCED_SIMPLE)
			strcat (pInfo, "  Advanced Simple");
	}
	else
	{
		sprintf (pInfo, "%s  %d", pInfo, pCodec->profile);
	}

	strcat (pInfo, "\r\n");
	return strlen (pInfo);
}

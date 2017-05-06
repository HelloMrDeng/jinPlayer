/*******************************************************************************
	File:		CDlgFileInfo.h

	Contains:	file info dialog header file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-01		Fenger			Create file

*******************************************************************************/
#ifndef __CDlgFileInfo_H__
#define __CDlgFileInfo_H__

#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavformat/url.h>

#include "CNodeList.h"

#include "yyType.h"

class CDlgFileInfo
{
public:
struct PacketInfo {
	int			flag;
	int 		size;
	long long	dts;
	int			dts_step;
	long long	pts;
	int			pts_step;
	int			dur;
	long long	pos;
	int			picType;
};

	static INT_PTR CALLBACK FileInfoDlgProc (HWND, UINT, WPARAM, LPARAM);

public:
	CDlgFileInfo(HINSTANCE hInst, HWND hParent);
	virtual ~CDlgFileInfo(void);

	int			OpenDlg (const TCHAR * pFile);

protected:
	int			GetFileName (TCHAR * pFileName);
	int			GetFileInfo (const TCHAR * pFile);
	void		CloseFile (void);

	int			FillFileInfo (void);
	int			FillStreamInfo (int nIndex);

	int			GetProfileName (AVCodecContext * pCodec, char * pInfo);

	char *		GetLineText (char * pLine);
	int			GetHexString (unsigned char * pBuffer, int nBuffSize, int nLineSize, char * pText);
	long long	BaseToTime (long long llBase, AVStream * pStream);
	bool		IsRefFrame (char * buffer , int size);

protected:
	HINSTANCE				m_hInst;
	HWND					m_hParent;
	HWND					m_hDlg;

	HWND					m_hCmbFile;
	HWND					m_hEdtInfo;

	AVFormatContext *		m_pFmtCtx;
	URLProtocol				m_ioFileExt;
	TCHAR					m_szFileName[1024];
	TCHAR *					m_pInfoFile;
	TCHAR **				m_ppInfoStream;

	char					m_szLineText[1024];

protected:
	static	int			ReadPacketProc (void * pParam);
	virtual int			ReadPacketLoop (void);

	HANDLE				m_hThreadReadPacket;
	YYPLAY_STATUS		m_nReadStatus;
};
#endif //__CDlgFileInfo_H__
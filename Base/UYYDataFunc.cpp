/*******************************************************************************
	File:		UYYDataFunc.cpp

	Contains:	The utility for library implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include <libavformat/avformat.h>

#include "UYYDataFunc.h"

#include "CBaseObject.h"
#include "CFFMpegVideoRCC.h"

int	yyDataCloneVideoFormat (YY_VIDEO_FORMAT * pTarget, YY_VIDEO_FORMAT * pSource)
{
	if (pTarget == NULL || pSource == NULL)
		return YY_ERR_ARG;

	if (pTarget->pHeadData != NULL)
		delete []pTarget->pHeadData;

	memcpy (pTarget, pSource, sizeof (YY_VIDEO_FORMAT));
	pTarget->pHeadData = NULL;
	pTarget->nHeadSize = 0;

	if (pSource->pHeadData != NULL && pSource->nHeadSize > 0)
	{
		pTarget->nHeadSize = pSource->nHeadSize;
		pTarget->pHeadData = new unsigned char[pTarget->nHeadSize];
		memcpy (pTarget->pHeadData, pSource->pHeadData, pTarget->nHeadSize);
	}

	return YY_ERR_NONE;
}

int	yyDataCloneAudioFormat (YY_AUDIO_FORMAT * pTarget, YY_AUDIO_FORMAT * pSource)
{
	if (pTarget == NULL || pSource == NULL)
		return YY_ERR_ARG;

	if (pTarget->pHeadData != NULL)
		delete []pTarget->pHeadData;

	memcpy (pTarget, pSource, sizeof (YY_AUDIO_FORMAT));
	pTarget->pHeadData = NULL;
	pTarget->nHeadSize = 0;

	if (pSource->pHeadData != NULL && pSource->nHeadSize > 0)
	{
		pTarget->nHeadSize = pSource->nHeadSize;
		pTarget->pHeadData = new unsigned char[pTarget->nHeadSize];
		memcpy (pTarget->pHeadData, pSource->pHeadData, pTarget->nHeadSize);
	}

	return YY_ERR_NONE;
}

void yyDataDeleteVideoFormat (YY_VIDEO_FORMAT ** ppFmt)
{
	if (ppFmt == NULL || *ppFmt == NULL)
		return;

	if ((*ppFmt)->pHeadData != NULL)
		delete [](*ppFmt)->pHeadData;
	delete *ppFmt;
	*ppFmt = NULL;
}

void yyDataDeleteAudioFormat (YY_AUDIO_FORMAT ** ppFmt)
{
	if (ppFmt == NULL || *ppFmt == NULL)
		return;

	if ((*ppFmt)->pHeadData != NULL)
		delete [](*ppFmt)->pHeadData;
	delete *ppFmt;
	*ppFmt = NULL;
}

bool yyDataCloneBuffer (YY_BUFFER * pTarget, YY_BUFFER * pSource)
{
	AVPacket *		pPktTgt = NULL;
	unsigned char * pPktBuff = NULL;
	int				nPktSize = 0;
	if (pTarget->pBuff != NULL)
	{
		if (pTarget->uBuffSize < pSource->uSize || pTarget->uBuffSize > pSource->uSize * 2)
		{
			if ((pTarget->uFlag & YYBUFF_TYPE_PACKET) ==YYBUFF_TYPE_PACKET)
			{
				pPktTgt = (AVPacket *)pTarget->pBuff;
				if (pPktTgt->data != NULL)
					delete []pPktTgt->data;
			}
			delete []pTarget->pBuff;
			pTarget->pBuff = NULL;
			pTarget->uBuffSize = 0;
		}
	}
	if (pTarget->pBuff == NULL)
	{
		if ((pSource->uFlag & YYBUFF_TYPE_PACKET) ==YYBUFF_TYPE_PACKET)
			pTarget->uBuffSize = pSource->uSize;
		else
			pTarget->uBuffSize = pSource->uSize * 3 / 2;
		pTarget->pBuff = new unsigned char[pTarget->uBuffSize];
	}
	else if ((pTarget->uFlag & YYBUFF_TYPE_PACKET) ==YYBUFF_TYPE_PACKET)
	{
		pPktTgt = (AVPacket *)pTarget->pBuff;
		pPktBuff = pPktTgt->data;
		nPktSize = pPktTgt->size;
	}

	pTarget->nType = pSource->nType;
	pTarget->uFlag = pSource->uFlag;
	pTarget->uSize = pSource->uSize;
	pTarget->llTime = pSource->llTime;
	memcpy (pTarget->pBuff, pSource->pBuff, pSource->uSize);

	if ((pSource->uFlag & YYBUFF_TYPE_PACKET) ==YYBUFF_TYPE_PACKET)
	{
		AVPacket *	pPktSrc = (AVPacket *)pSource->pBuff;
		pPktTgt = (AVPacket *)pTarget->pBuff;
		if (nPktSize < pPktSrc->size)
		{
			if (pPktBuff != NULL)
				delete []pPktBuff;
			pPktTgt->data = new unsigned char[pPktSrc->size];
		}
		else
		{
			pPktTgt->data = pPktBuff;
		}

		memcpy (pPktTgt->data, pPktSrc->data, pPktSrc->size);
		pPktTgt->buf = NULL;
	}

	if (pSource->pFormat != NULL)
	{
		if (pSource->nType == YY_MEDIA_Video)
		{
			YY_VIDEO_FORMAT * pFmtVideo = new YY_VIDEO_FORMAT ();
			memset (pFmtVideo, 0, sizeof (YY_VIDEO_FORMAT));
			yyDataCloneVideoFormat (pFmtVideo, (YY_VIDEO_FORMAT *)pSource->pFormat);
			pTarget->pFormat = pFmtVideo;
		}
		else if (pSource->nType == YY_MEDIA_Audio)
		{
			YY_AUDIO_FORMAT * pFmtAudio = new YY_AUDIO_FORMAT ();
			memset (pFmtAudio, 0, sizeof (YY_AUDIO_FORMAT));
			yyDataCloneAudioFormat (pFmtAudio, (YY_AUDIO_FORMAT *)pSource->pFormat);
			pTarget->pFormat = pFmtAudio;
		}
	}

	return true;
}

bool yyDataResetBuffer (YY_BUFFER * pBuffer, bool bDel)
{
	if (pBuffer == NULL)
		return false;

	if (pBuffer->pFormat != NULL)
	{
		if (pBuffer->nType == YY_MEDIA_Video)
		{
			YY_VIDEO_FORMAT * pFmtVideo = (YY_VIDEO_FORMAT *) pBuffer->pFormat;
			if (pFmtVideo->pHeadData != NULL)
				delete []pFmtVideo->pHeadData;
			delete pFmtVideo;
			pBuffer->pFormat = NULL;
		}
		else if (pBuffer->nType == YY_MEDIA_Audio)
		{
			YY_AUDIO_FORMAT * pFmtAudio = (YY_AUDIO_FORMAT *) pBuffer->pFormat;
			if (pFmtAudio->pHeadData != NULL)
				delete []pFmtAudio->pHeadData;
			delete pFmtAudio;
			pBuffer->pFormat = NULL;
		}
	}
	if (pBuffer->pBuff != NULL)
	{
		if ((pBuffer->uFlag & YYBUFF_TYPE_PACKET) ==YYBUFF_TYPE_PACKET)
		{
			AVPacket *	pPktSrc = (AVPacket *)pBuffer->pBuff;
			if (pPktSrc->data != NULL)
				delete []pPktSrc->data;
		}
		delete []pBuffer->pBuff;
		pBuffer->pBuff = NULL;
	}

	if (bDel)
	{
		delete pBuffer;
	}
	else
	{
		YYMediaType nType = pBuffer->nType;
		memset (pBuffer, 0, sizeof (YY_BUFFER));
		pBuffer->nType = nType;
	}

	return true;
}

bool yyDataCloneVideoBuff (YY_BUFFER * pTarget, YY_BUFFER * pSource)
{
	if (pTarget == NULL || pSource == NULL)
		return false;

	if ((pTarget->uFlag & 0XFFFF0000) != (pSource->uFlag & 0XFFFF0000))
		yyDataResetVideoBuff (pTarget, false);

	int i = 0;
	if ((pSource->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		AVFrame * pFrmSrc = (AVFrame * )pSource->pBuff;
		AVFrame * pFrmTar = NULL;
		if (pTarget->pBuff == NULL)
		{
			pFrmTar = new  AVFrame();
			memset (pFrmTar, 0, sizeof (AVFrame));
			pTarget->pBuff = (unsigned char *)pFrmTar;
		}
		else
		{
			pFrmTar = (AVFrame * )pTarget->pBuff;
		}

		pFrmTar->pts = pFrmSrc->pts;
		pFrmTar->pkt_pts = pFrmSrc->pkt_pts;
		pFrmTar->format = pFrmSrc->format;
		pFrmTar->width = pFrmSrc->width;
		pFrmTar->height = pFrmSrc->height;
		pFrmTar->format = AV_PIX_FMT_YUV420P;

		if (pFrmTar->linesize[0] != pFrmSrc->linesize[0])
		{
			for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
				YY_DEL_A (pFrmTar->data[i]);
			pFrmTar->linesize[0] = pFrmSrc->linesize[0];
			pFrmTar->data[0] = new unsigned char[pFrmSrc->linesize[0] * pFrmTar->height];
			pFrmTar->linesize[1] = pFrmSrc->linesize[0] / 2;
			pFrmTar->data[1] = new unsigned char[pFrmSrc->linesize[0] * pFrmTar->height];
			pFrmTar->linesize[2] = pFrmSrc->linesize[0] / 2;
			pFrmTar->data[2] = new unsigned char[pFrmSrc->linesize[0] * pFrmTar->height];
		}
		if (pFrmSrc->format == AV_PIX_FMT_YUV420P)
		{
			memcpy (pFrmTar->data[0], pFrmSrc->data[0], pFrmTar->linesize[0] * pFrmSrc->height);
			memcpy (pFrmTar->data[1], pFrmSrc->data[1], pFrmTar->linesize[1] * pFrmSrc->height / 2);
			memcpy (pFrmTar->data[2], pFrmSrc->data[2], pFrmTar->linesize[2] * pFrmSrc->height / 2);
		}
		else
		{
			CFFMpegVideoRCC vidRCC (NULL);
			YY_VIDEO_BUFF vidBuff;
			yyDataAVFrameToVideoBuff (pFrmTar, &vidBuff);
			vidBuff.nType = YY_VDT_YUV420_P;
			vidRCC.ConvertBuff (pSource, &vidBuff, NULL);
		}
	}
	else if ((pSource->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
	{
		YY_VIDEO_BUFF * pBufSrc = (YY_VIDEO_BUFF * )pSource->pBuff;
		YY_VIDEO_BUFF * pBufTar = NULL;
		if (pTarget->pBuff == NULL)
		{
			pBufTar = new  YY_VIDEO_BUFF();
			memset (pBufTar, 0, sizeof (YY_VIDEO_BUFF));
			pTarget->pBuff = (unsigned char *)pBufTar;
		}
		else
		{
			pBufTar = (YY_VIDEO_BUFF * )pTarget->pBuff;
		}

		if (pBufTar->nWidth != pBufSrc->nWidth || pBufTar->nHeight != pBufSrc->nHeight)
		{
			for (i = 0; i < 3; i++)
				YY_DEL_A (pBufTar->pBuff[i]);
			pBufTar->nStride[0] = pBufSrc->nStride[0];
			pBufTar->pBuff[0] = new unsigned char[pBufTar->nStride[0] * pBufSrc->nHeight];
			pBufTar->nStride[1] = pBufSrc->nStride[1];
			pBufTar->pBuff[1] = new unsigned char[pBufTar->nStride[1] * pBufSrc->nHeight / 2];
			pBufTar->nStride[2] = pBufSrc->nStride[2];
			pBufTar->pBuff[2] = new unsigned char[pBufTar->nStride[2] * pBufSrc->nHeight / 2];
		}
		pBufTar->nWidth = pBufSrc->nWidth;
		pBufTar->nHeight = pBufSrc->nHeight;
		pBufTar->nType = pBufSrc->nType;
		memcpy (pBufTar->pBuff[0], pBufSrc->pBuff[0], pBufTar->nStride[0] * pBufSrc->nHeight);
		memcpy (pBufTar->pBuff[1], pBufSrc->pBuff[1], pBufTar->nStride[1] * pBufSrc->nHeight / 2);
		memcpy (pBufTar->pBuff[2], pBufSrc->pBuff[2], pBufTar->nStride[2] * pBufSrc->nHeight / 2);
	}
	else
	{
		return false;
	}

	pTarget->nType = pSource->nType;
	pTarget->uFlag = pSource->uFlag;
	pTarget->uSize = pSource->uSize;
	pTarget->llTime = pSource->llTime;

	return true;
}

bool yyDataResetVideoBuff (YY_BUFFER * pBuffer, bool bDel)
{
	if (pBuffer == NULL)
		return true;

	int i = 0;
	if ((pBuffer->uFlag & YYBUFF_TYPE_AVFrame) == YYBUFF_TYPE_AVFrame)
	{
		AVFrame * pFrmSrc = (AVFrame * )pBuffer->pBuff;
		for (i = 0; i < AV_NUM_DATA_POINTERS; i++)
		{
			YY_DEL_A (pFrmSrc->data[i]);
			pFrmSrc->linesize[i] = 0;
		}
	}
	else if ((pBuffer->uFlag & YYBUFF_TYPE_VIDEO) == YYBUFF_TYPE_VIDEO)
	{
		YY_VIDEO_BUFF * pBufSrc = (YY_VIDEO_BUFF * )pBuffer->pBuff;
		for (i = 0; i < 3; i++)
		{
			YY_DEL_A (pBufSrc->pBuff[i]);
			pBufSrc->nStride[i] = 0;
		}
	}
	else
	{
		return false;
	}

	if (bDel)
		delete pBuffer;

	return true;
}

bool yyDataAVFrameToVideoBuff (AVFrame * pAVFrame, YY_VIDEO_BUFF * pVideoBuff)
{
	pVideoBuff->pBuff[0] = pAVFrame->data[0];
	pVideoBuff->pBuff[1] = pAVFrame->data[1];
	pVideoBuff->pBuff[2] = pAVFrame->data[2];
	pVideoBuff->nStride[0] = pAVFrame->linesize[0];
	pVideoBuff->nStride[1] = pAVFrame->linesize[1];
	pVideoBuff->nStride[2] = pAVFrame->linesize[2];
	pVideoBuff->nWidth = pAVFrame->width;
	pVideoBuff->nHeight = pAVFrame->height;
	pVideoBuff->nType = YY_VDT_YUV420_P;

	return true;
}

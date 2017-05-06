/*******************************************************************************
	File:		CBaseVideoDec.cpp

	Contains:	The base Video dec implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#include "CBaseVideoDec.h"

#include "yyConfig.h"
#include "yyLog.h"

CBaseVideoDec::CBaseVideoDec(void * hInst)
	: CBaseObject ()
	, m_hInst (hInst)
	, m_hView (NULL)
	, m_pInBuff (NULL)
	, m_uBuffFlag (0)
	, m_uCodecTag (0)
	, m_bWaitKeyFrame (false)
	, m_bDropFrame (false)
	, m_nDecCount (0)
	, m_nBuffNum (1)
	, m_llLastTime (0)
	, m_llFirstTime (0)
	, m_llFrameTime (0)
	, m_uNalLen (4)
	, m_uFrameSize (0)
	, m_uNalWord (0X01000000)
	, m_pVideoData (NULL)
	, m_uVideoSize (0)
	, m_pHeadData (NULL)
	, m_nHeadSize (0)
{
	SetObjectName ("CBaseVideoDec");

	memset (&m_fmtVideo, 0, sizeof (m_fmtVideo));
	memset (&m_bufVideo, 0, sizeof (m_bufVideo));
	m_nVdoBuffSize = sizeof (YY_VIDEO_BUFF);

	av_init_packet (&m_pktData);
	m_pktData.data = NULL;
	m_pktData.size = 0;
}

CBaseVideoDec::~CBaseVideoDec(void)
{
	ReleaseInfoList ();
	YY_DEL_A (m_fmtVideo.pHeadData);
}

int	CBaseVideoDec::SetDisplay (void * hView, RECT * pRect)
{
	m_hView = hView;
	memset (&m_rcView, 0, sizeof (RECT));
	if (pRect != NULL)
		memcpy (&m_rcView, pRect, sizeof (RECT));
	return YY_ERR_NONE;
}

int CBaseVideoDec::Init (YY_VIDEO_FORMAT * pFmt)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoDec::Uninit (void)
{
	return YY_ERR_IMPLEMENT;
}

int CBaseVideoDec::SetBuff (YY_BUFFER * pBuff)
{
	m_pInBuff = pBuff;

	if ((pBuff->uFlag & YYBUFF_EOS) == YYBUFF_EOS)
		m_uBuffFlag |= YYBUFF_EOS;

	if ((pBuff->uFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		m_uBuffFlag |= YYBUFF_NEW_POS;

	return YY_ERR_NONE;
}

int CBaseVideoDec::GetBuff (YY_BUFFER * pBuff)
{
	if ((m_uBuffFlag & YYBUFF_NEW_POS) == YYBUFF_NEW_POS)
		pBuff->uFlag |= YYBUFF_NEW_POS;
	if ((m_uBuffFlag & YYBUFF_EOS) == YYBUFF_EOS)
		pBuff->uFlag |= YYBUFF_EOS;
	m_uBuffFlag = 0;

	return YY_ERR_NONE;
}

int CBaseVideoDec::RndBuff (YY_BUFFER * pBuff, bool bRend)
{
	return YY_ERR_FAILED;
}

int CBaseVideoDec::RndRest (void)
{
	return YY_ERR_NONE;
}

int	CBaseVideoDec::Start (void)
{
	return YY_ERR_NONE;
}

int CBaseVideoDec::Pause (void)
{
	return YY_ERR_NONE;
}

int	CBaseVideoDec::Stop (void)
{
	return YY_ERR_NONE;
}

int CBaseVideoDec::Flush (void)
{
	return YY_ERR_NONE;
}

int CBaseVideoDec::ReleaseInfoList (void)
{
	CAutoLock lock (&m_mtInfo);
	YYVDEC_INFO * pInfo = NULL;
	
	pInfo = m_lstFull.RemoveHead ();
	while (pInfo != NULL)
	{
		delete pInfo;
		pInfo = m_lstFull.RemoveHead ();
	}

	pInfo = m_lstFree.RemoveHead ();
	while (pInfo != NULL)
	{
		delete pInfo;
		pInfo = m_lstFree.RemoveHead ();
	}

	return YY_ERR_NONE;
}

int CBaseVideoDec::ResetInfoList (void)
{
	CAutoLock lock (&m_mtInfo);
	YYVDEC_INFO * pInfo = NULL;
	
	pInfo = m_lstFull.RemoveHead ();
	while (pInfo != NULL)
	{
		m_lstFree.AddTail (pInfo);
		pInfo = m_lstFull.RemoveHead ();
	}

	return YY_ERR_NONE;
}

int CBaseVideoDec::FillInfo (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtInfo);
	YYVDEC_INFO * pInfo = NULL;

	NODEPOS pos = m_lstFull.GetHeadPosition ();
	while (pos != NULL)
	{
		pInfo = m_lstFull.GetNext (pos);
		if (pInfo->llTime == pBuff->llTime)
			return YY_ERR_NONE;
	}

	pInfo = m_lstFree.RemoveHead ();
	if (pInfo == NULL)
		pInfo = new YYVDEC_INFO ();
	if (pInfo == NULL)
		return YY_ERR_MEMORY;

	pInfo->llTime	= pBuff->llTime;
	pInfo->llDelay	= pBuff->llDelay;
	pInfo->nFlag	= pBuff->uFlag;
	pInfo->nValue	= pBuff->nValue;
	pInfo->pBuff	= pBuff->pBuff;
	pInfo->uSize	= pBuff->uSize;

	m_lstFull.AddTail (pInfo);

	return YY_ERR_NONE;
}

int CBaseVideoDec::RestoreInfo (YY_BUFFER * pBuff)
{
	CAutoLock lock (&m_mtInfo);
	YYVDEC_INFO * pInfo = NULL;

	pInfo = m_lstFull.GetHead ();
	while (pInfo != NULL)
	{
		if (pInfo->llTime + 1000 > pBuff->llTime)
			break;

		pInfo = m_lstFull.RemoveHead ();
		m_lstFree.AddTail (pInfo);
		pInfo = m_lstFull.GetHead ();
	}

	pInfo = NULL;
	NODEPOS pos = m_lstFull.GetHeadPosition ();
	while (pos != NULL)
	{
		pInfo = m_lstFull.GetNext (pos);
		if (pInfo->llTime == pBuff->llTime)
			break;
	}
	if (pInfo == NULL)
		return YY_ERR_FAILED;

	if ((pInfo->nFlag & YYBUFF_DROP_FRAME) == YYBUFF_DROP_FRAME)
	{
		pBuff->uFlag |= YYBUFF_DROP_FRAME;
		pBuff->nValue = pInfo->nValue;
	}

	m_lstFull.Remove (pInfo);
	m_lstFree.AddTail (pInfo);

	return YY_ERR_NONE;
}

bool CBaseVideoDec::ConvertH264HeadData(unsigned char * pHeadData, int nHeadSize)
{
	if (nHeadSize < 12)
		return true;
	if (m_pHeadData != NULL)
		return true;
	if (!memcmp (pHeadData, &m_uNalWord, 4))
	{
		m_nHeadSize = nHeadSize;
		m_pHeadData = new unsigned char[m_nHeadSize];
		memcpy (m_pHeadData, pHeadData, m_nHeadSize);
		return true;
	}	
	m_uFrameSize = (m_uFrameSize > nHeadSize) ? m_uFrameSize : nHeadSize;
		
	char* 			pData = (char *)pHeadData;
	unsigned int 	numOfPictureParameterSets;
	int 			configurationVersion = pData[0];
	int 			AVCProfileIndication = pData[1];
	int 			profile_compatibility = pData[2];
	int 			AVCLevelIndication  = pData[3];

	m_uNalLen =  (pData[4]&0x03)+1;
	int nNalLen = m_uNalLen;
	if (m_uNalLen == 3)
		m_uNalWord = 0X010000;
	if (m_uNalLen < 3)
	{
		m_pVideoData = new unsigned char[512 + m_uFrameSize];
		nNalLen = 4;
	}

	m_pHeadData = new unsigned char[512 + nHeadSize];
	m_nHeadSize = 0;

	unsigned int 	i = 0;
	unsigned int 	numOfSequenceParameterSets = pData[5]&0x1f;
	unsigned char * pBuffer = (unsigned char*)pData+6;
	for (i=0; i< numOfSequenceParameterSets; i++)
	{
		unsigned int sequenceParameterSetLength = (pBuffer[0]<<8)|pBuffer[1];
		pBuffer+=2;

		memcpy (m_pHeadData + m_nHeadSize, &m_uNalWord, nNalLen);
		m_nHeadSize += nNalLen;

		if(sequenceParameterSetLength > (nHeadSize - (unsigned int)(pBuffer-pHeadData)))
		{
			delete []m_pHeadData;
			m_pHeadData = NULL;
			m_nHeadSize = 0;
			return false;
		}

		memcpy (m_pHeadData + m_nHeadSize, pBuffer, sequenceParameterSetLength);
		m_nHeadSize += sequenceParameterSetLength;

		pBuffer += sequenceParameterSetLength;
	}

	numOfPictureParameterSets = *pBuffer++;
	for (i=0; i< numOfPictureParameterSets; i++)
	{
		unsigned int pictureParameterSetLength = (pBuffer[0]<<8)|pBuffer[1];
		pBuffer+=2;

		memcpy (m_pHeadData + m_nHeadSize, &m_uNalWord, nNalLen);
		m_nHeadSize += nNalLen;
		
		if(pictureParameterSetLength > (nHeadSize - (unsigned int)(pBuffer - pHeadData)))
		{
			delete []m_pHeadData;
			m_pHeadData = NULL;			
			m_nHeadSize = 0;
			return false;
		}

		memcpy (m_pHeadData + m_nHeadSize, pBuffer, pictureParameterSetLength);
		m_nHeadSize += pictureParameterSetLength;

		pBuffer += pictureParameterSetLength;
	}
	
	return true;
}

bool CBaseVideoDec::ConvertHEVCHeadData(unsigned char * pHeadData, int nHeadSize)
{
	if (nHeadSize < 12)
		return true;

	if (m_pHeadData != NULL)
		return true;

	if (!memcmp (pHeadData, &m_uNalWord, 4))
	{
		m_nHeadSize = nHeadSize;
		m_pHeadData = new unsigned char[m_nHeadSize];
		memcpy (m_pHeadData, pHeadData, m_nHeadSize);
		return true;
	}

	unsigned char * pData = pHeadData;
	m_uNalLen =  (pData[21]&0x03)+1;
	unsigned int nNalLen = m_uNalLen;
	m_uFrameSize = (m_uFrameSize > nHeadSize) ? m_uFrameSize : nHeadSize;

	if (m_uNalLen == 3)
		m_uNalWord = 0X010000;
	if (m_uNalLen < 3)
	{
		YY_DEL_A (m_pVideoData);
		m_pVideoData = new unsigned char[512 + m_uFrameSize];
		nNalLen = 4;
	}

	YY_DEL_A (m_pHeadData);
	m_pHeadData = new unsigned char[512 + m_uFrameSize];
	m_nHeadSize = 0;

	unsigned char numOfArrays = pData[22];

	pData += 23;
	if(numOfArrays)
	{
		for(int arrNum = 0; arrNum < numOfArrays; arrNum++)
		{
			unsigned char nal_type = 0;
			nal_type = pData[0]&0x3F;
			pData += 1;
			switch(nal_type)
			{
			case 33://sps
			{
				unsigned short numOfSequenceParameterSets = 0;
				numOfSequenceParameterSets = ((numOfSequenceParameterSets|pData[0]) << 8)|pData[1];
				pData += 2;
				for(int i = 0; i < numOfSequenceParameterSets; i++)
				{
					memcpy (m_pHeadData + m_nHeadSize, &m_uNalWord, nNalLen);
					m_nHeadSize += nNalLen;
					unsigned short sequenceParameterSetLength = pData[0];
					sequenceParameterSetLength = (sequenceParameterSetLength << 8)|pData[1];
					pData += 2;
					memcpy (m_pHeadData + m_nHeadSize, pData, sequenceParameterSetLength);
					m_nHeadSize += sequenceParameterSetLength;

					pData += sequenceParameterSetLength;
				}
			}
			break;

			case 34://pps
			{
				unsigned short numofPictureParameterSets = pData[0];
				numofPictureParameterSets = (numofPictureParameterSets << 8)|pData[1];
				pData += 2;

				for(int i = 0; i < numofPictureParameterSets; i++)
				{
					memcpy (m_pHeadData + m_nHeadSize, &m_uNalWord, nNalLen);
					m_nHeadSize += nNalLen;
					unsigned short pictureParameterSetLength = pData[0];
					pictureParameterSetLength = (pictureParameterSetLength << 8)|pData[1];
					pData += 2;
					memcpy (m_pHeadData + m_nHeadSize, pData, pictureParameterSetLength);
					m_nHeadSize += pictureParameterSetLength;
					pData += pictureParameterSetLength;
				}
			}
			break;

			case 32: //aps
			{
				unsigned short numofAdaptationParameterSets = pData[0];
				numofAdaptationParameterSets = (numofAdaptationParameterSets << 8)|pData[1];
				pData += 2;

				for(int i = 0; i < numofAdaptationParameterSets; i++)
				{
					memcpy (m_pHeadData + m_nHeadSize, &m_uNalWord, nNalLen);
					m_nHeadSize += nNalLen;
					unsigned short adaptationParameterSetLength = pData[0];
					adaptationParameterSetLength = (adaptationParameterSetLength << 8)|pData[1];
					pData += 2;
					memcpy (m_pHeadData + m_nHeadSize, pData, adaptationParameterSetLength);
					m_nHeadSize += adaptationParameterSetLength;
					pData += adaptationParameterSetLength;
				}
			}
			break;

			default://just skip the data block
			{
				unsigned short numofskippingParameter = pData[0];
				numofskippingParameter = (numofskippingParameter << 8)|pData[1];
				pData += 2;
				for(int i = 0; i < numofskippingParameter; i++)
				{
					unsigned short adaptationParameterSetLength = pData[0];
					adaptationParameterSetLength = (adaptationParameterSetLength << 8)|pData[1];
					pData += 2;
					pData += adaptationParameterSetLength;
				}
			}
			break;

			}
		}
	}

	return true;
}

bool CBaseVideoDec::ConvertVideoData (unsigned char * pData, int nSize)
{
	if (pData == NULL || nSize <= 4)
		return false;
		
	if (!memcmp (pData, &m_uNalWord, 4))
		return true;

	unsigned char *	pBuffer = pData;
	int				nFrameLen = 0;
	int				i = 0;

	m_uVideoSize = 0;
	while (pBuffer - pData + m_uNalLen < nSize)
	{
		nFrameLen = *pBuffer++;
		for (i = 0; i < (int)m_uNalLen - 1; i++)
		{
			nFrameLen = nFrameLen << 8;
			nFrameLen += *pBuffer++;
		}

		if(nFrameLen > nSize)
			return false;

		if (m_uNalLen == 3 || m_uNalLen == 4)
		{
			memcpy ((pBuffer - m_uNalLen), &m_uNalWord, m_uNalLen);
		}
		else
		{
			memcpy (m_pVideoData + m_uVideoSize, &m_uNalWord, 4);
			m_uVideoSize += 4;
			memcpy (m_pVideoData + m_uVideoSize, pBuffer, nFrameLen);
			m_uVideoSize += nFrameLen;
		}

		pBuffer += nFrameLen;
	}

	return true;
}


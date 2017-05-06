/*******************************************************************************
	File:		UAudioFunc.cpp

	Contains:	The utility for audio operation implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-05-01		Fenger			Create file

*******************************************************************************/
#include <string.h>
#include "voADPCM.h"
#include "UAudioFunc.h"

//  AAC header parser functions 
typedef struct{
	unsigned char *bytePtr;
	unsigned int iCache;
	int cachedBits;
	int nBytes;
	int noBytes;
} lBitStream;

typedef struct
{
	unsigned char *pBitBufBase;          
	unsigned char *pBitBufEnd;           
	unsigned char *pWriteNext;           
	VO_U32  cache;
	VO_S16  wBitPos;              
	VO_S16  cntBits;             
	VO_S16  size;               
	VO_S16  isValid;    
}WBitStream; // size Word16: 8 

static void lBitStreamInit(lBitStream *bsi,
			               unsigned int		len,
			               void *buf )
{
	bsi->bytePtr = (unsigned char*)buf;
	bsi->iCache = 0;		
	bsi->cachedBits = 0;	
	bsi->nBytes = len;
	bsi->noBytes = 0;
}

static __inline void lRefillBitstreamCache(lBitStream *bsi)
{
	int nBytes = bsi->nBytes;

	// optimize for common case, independent of machine endian-ness 
	if (nBytes >= 4) {
		bsi->iCache  = (*bsi->bytePtr++) << 24;
		bsi->iCache |= (*bsi->bytePtr++) << 16;
		bsi->iCache |= (*bsi->bytePtr++) <<  8;
		bsi->iCache |= (*bsi->bytePtr++);
		bsi->cachedBits = 32;
		bsi->nBytes -= 4;
	} else {
		bsi->iCache = 0;
		if(nBytes<=0)
		{	
			bsi->cachedBits = 32;
			bsi->noBytes+=4;
			return;
		}

		while (nBytes--) {
			bsi->iCache |= (*bsi->bytePtr++);
			bsi->iCache <<= 8;
		}
		bsi->iCache <<= ((3 - bsi->nBytes)*8);
		bsi->cachedBits = 8*bsi->nBytes;
		bsi->nBytes = 0;
	}
}

static VO_U32 lBitStreamGetBits(lBitStream * const bsi,
		const unsigned int nBits)
{
	unsigned int data, lowBits;

	data = bsi->iCache >> (32 - nBits);		
	bsi->iCache <<= nBits;					
	bsi->cachedBits -= nBits;				

	if (bsi->cachedBits < 0) {
		lowBits = -bsi->cachedBits;
		lRefillBitstreamCache(bsi);
		data |= bsi->iCache >> (32 - lowBits);		

		bsi->cachedBits -= lowBits;			
		bsi->iCache <<= lowBits;			
	}

	return data;
}

int program_config_element(lBitStream *bs,YYADIF_CONFIG *pce)
{
	int i,channels;

	channels = 0;
	pce->element_instance_tag = (unsigned char)lBitStreamGetBits(bs, 4);

	pce->object_type = (unsigned char)lBitStreamGetBits(bs, 2);
	pce->sampling_frequency_index = (unsigned char)lBitStreamGetBits(bs, 4);
	pce->num_front_channel_elements = (unsigned char)lBitStreamGetBits(bs, 4);
	pce->num_side_channel_elements = (unsigned char)lBitStreamGetBits(bs, 4);
	pce->num_back_channel_elements = (unsigned char)lBitStreamGetBits(bs, 4);
	pce->num_lfe_channel_elements = (unsigned char)lBitStreamGetBits(bs, 2);
	pce->num_assoc_data_elements = (unsigned char)lBitStreamGetBits(bs, 3);
	pce->num_valid_cc_elements = (unsigned char)lBitStreamGetBits(bs, 4);

	pce->mono_mixdown_present = (unsigned char)lBitStreamGetBits(bs, 1);
	if (pce->mono_mixdown_present == 1)
	{
		pce->mono_mixdown_element_number = (unsigned char)lBitStreamGetBits(bs, 4);
	}

	pce->stereo_mixdown_present = (unsigned char)lBitStreamGetBits(bs, 1);
	if (pce->stereo_mixdown_present == 1)
	{
		pce->stereo_mixdown_element_number = (unsigned char)lBitStreamGetBits(bs, 4);
	}

	pce->matrix_mixdown_idx_present = (unsigned char)lBitStreamGetBits(bs,1);
	if (pce->matrix_mixdown_idx_present == 1)
	{
		pce->matrix_mixdown_idx = (unsigned char)lBitStreamGetBits(bs, 2);
		pce->pseudo_surround_enable = (unsigned char)lBitStreamGetBits(bs, 1);
	}

	pce->num_front_channels = 0;
	for (i = 0; i < pce->num_front_channel_elements; i++)
	{
		pce->front_element_is_cpe[i] = (unsigned char)lBitStreamGetBits(bs, 1);
		pce->front_element_tag_select[i] = (unsigned char)lBitStreamGetBits(bs, 4);

		if (pce->front_element_is_cpe[i] & 1)
		{            
			pce->num_front_channels += 2;
			channels += 2;
		} 
		else 
		{          
			pce->num_front_channels += 1;
			channels++;
		}
	}

	pce->num_side_channels = 0;
	for (i = 0; i < pce->num_side_channel_elements; i++)
	{
		pce->side_element_is_cpe[i] = (unsigned char)lBitStreamGetBits(bs, 1);
		pce->side_element_tag_select[i] = (unsigned char)lBitStreamGetBits(bs, 4);

		if (pce->side_element_is_cpe[i] & 1)
		{            
			pce->num_side_channels += 2;
			channels += 2;
		} 
		else 
		{			
			pce->num_side_channels += 1;
			channels++;
		}
	}

	pce->num_back_channels = 0;
	for (i = 0; i < pce->num_back_channel_elements; i++)
	{
		pce->back_element_is_cpe[i] = (unsigned char)lBitStreamGetBits(bs,1);
		pce->back_element_tag_select[i] = (unsigned char)lBitStreamGetBits(bs, 4);
		if (pce->back_element_is_cpe[i] & 1)
		{            
			pce->num_back_channels += 2;
			channels += 2;
		} 
		else 
		{			
			pce->num_back_channels += 1;
			channels++;
		}
	}

	pce->num_lfe_channels = 0;
	for (i = 0; i < pce->num_lfe_channel_elements; i++)
	{
		pce->lfe_element_tag_select[i] = (unsigned char)lBitStreamGetBits(bs, 4);
		pce->num_lfe_channels += 1;
		channels++;
	}

	for (i = 0; i < pce->num_assoc_data_elements; i++)
		pce->assoc_data_element_tag_select[i] = (unsigned char)lBitStreamGetBits(bs, 4);

	for (i = 0; i < pce->num_valid_cc_elements; i++)
	{
		pce->cc_element_is_ind_sw[i] = (unsigned char)lBitStreamGetBits(bs,1);
		pce->valid_cc_element_tag_select[i] = (unsigned char)lBitStreamGetBits(bs, 4);
	}

	pce->channels = channels;
	//BitStreamByteAlign(bs);

	////comment_field_bytes
	//i = BitStreamGetBits(bs, 8);
	//while (i--)
	//	BitStreamGetBits(bs, 8);

	return 0 ;
}

static int lParseADIFHeader(lBitStream*	bs, YY_AUDIO_HEADDATAINFO *pAudioHeadDataInfo)
{
	int i;
	int channels,sampleRateIdx,profile;
	YYADIF_INFO adif;
	YYADIF_CONFIG *pce;

	adif.adif_id = (unsigned char)lBitStreamGetBits(bs,32);
	adif.copyright_id_present = (unsigned char)lBitStreamGetBits(bs, 1); 

	if(adif.copyright_id_present)
	{
		for (i = 0; i < 9; i++)
		{
			adif.copyright_id[i] = (unsigned char)lBitStreamGetBits(bs, 8);
		}
		//MCW_subscript_overflow(???): adif.copyright_id[i] = 0; 
	}
	adif.original_copy  = (unsigned char)lBitStreamGetBits(bs, 1);
	adif.home = (unsigned char)lBitStreamGetBits(bs, 1);
	adif.bitstream_type = (unsigned char)lBitStreamGetBits(bs, 1);
	adif.bitrate = (unsigned char)lBitStreamGetBits(bs, 23);
	adif.num_program_config_elements = (unsigned char)lBitStreamGetBits(bs, 4);

	for (i = 0; i < adif.num_program_config_elements + 1; i++)
	{
		if(adif.bitstream_type == 0)
		{
			adif.adif_buffer_fullness = lBitStreamGetBits(bs, 20);
		} else {
			adif.adif_buffer_fullness = 0;
		}

		program_config_element(bs,&adif.pce[i]);
	}

	pce = &adif.pce[0];
	profile = pce->object_type + 1;
	sampleRateIdx = pce->sampling_frequency_index;
	channels = pce->channels;

	if(sampleRateIdx >= MAXNUM_SAMPLE_RATES)
		return VO_ERR_AUDIO_UNSSAMPLERATE;

	pAudioHeadDataInfo->nSampleRate = AACSampRateTab[sampleRateIdx];
	pAudioHeadDataInfo->nChannels = channels;
	pAudioHeadDataInfo->nFrameType = VOAAC_ADIF;
	pAudioHeadDataInfo->nProfiles = profile;
	pAudioHeadDataInfo->nSampleBits = 16;

	return 0;
}

static int lParseADTSHeader(lBitStream*	bs, YY_AUDIO_HEADDATAINFO *pAudioHeadDataInfo)
{
	int nSampRate = 0;
	int nSampIndex = 0;
	int HeadLens = 0;
	YYATDS_INFO adts;

	adts.syncword = (unsigned short)lBitStreamGetBits(bs, 12);	
	adts.ID = (unsigned char)lBitStreamGetBits(bs, 1);
	adts.layer = (unsigned char)lBitStreamGetBits(bs, 2);
	adts.protection_absent = (unsigned char)lBitStreamGetBits(bs,1);
	adts.profile = (unsigned char)lBitStreamGetBits(bs, 2) + 1;
	adts.sampling_frequency_index = (unsigned char)lBitStreamGetBits(bs, 4);
	adts.private_bit = (unsigned char)lBitStreamGetBits(bs, 1);
	adts.channel_configuration = (unsigned char)lBitStreamGetBits(bs, 3);
	adts.original = (unsigned char)lBitStreamGetBits(bs,1);
	adts.home = (unsigned char)lBitStreamGetBits(bs,1);

	adts.copyright_identification_bit = (unsigned char)lBitStreamGetBits(bs, 1);
	adts.copyright_identification_start = (unsigned char)lBitStreamGetBits(bs, 1);
	adts.frame_length = (unsigned short)lBitStreamGetBits(bs, 13);
	adts.adts_buffer_fullness = (unsigned short)lBitStreamGetBits(bs, 11);
	adts.number_of_raw_data_blocks_in_frame = (unsigned char)lBitStreamGetBits(bs, 2);

	HeadLens = 7;
	if(adts.protection_absent == 0)
	{
		adts.crc_check = (unsigned short)lBitStreamGetBits(bs, 16);
        HeadLens = 9;
	}

	nSampIndex = adts.sampling_frequency_index;

	if (nSampIndex < MAXNUM_SAMPLE_RATES )
	{
		nSampRate = AACSampRateTab[nSampIndex];
	}

	pAudioHeadDataInfo->nChannels = adts.channel_configuration;
	pAudioHeadDataInfo->nFrameType = VOAAC_ADTS;
	pAudioHeadDataInfo->nProfiles = adts.profile;
	pAudioHeadDataInfo->nSampleBits = 16;
	pAudioHeadDataInfo->nSampleRate = nSampRate;

	//BitStreamByteAlign(bs);
	return HeadLens;

}

#define ADIFHEADER_CHECK(h)	 ((h)[0] == 'A' && (h)[1] == 'D' && (h)[2] == 'I' && (h)[3] == 'F')
#define ADTSHEADER_CHECK(h)  (((h)[0] & 0xFF) == 0xFF && ((h)[1] & 0xF0) == 0xF0) 

VO_U32 yyAudioGetAACHeadInfo(unsigned char * pBuff, int nSize, YY_AUDIO_HEADDATAINFO *pAudioHeadDataInfo)
{
	int nRc = 0;
	int profile,sampIdx,chanNum,sampFreq=0; //MCW_MAY_UNINIT <sampFreq>
	int nLen = nSize;
	lBitStream pBs;

	if (nLen < 2)
		return VO_ERR_INVALID_ARG;

	lBitStreamInit(&pBs, nLen, pBuff);

	//ADIF
	if (nLen > 4 && ADIFHEADER_CHECK(pBuff))
	{
		if(nRc == lParseADIFHeader(&pBs, pAudioHeadDataInfo))
		{
			return nRc;
		}
		else
		{
			return VO_ERR_INVALID_ARG;
		}
	}

	//ADTS length 7 or 9 Bytes
	if (nLen >= 7 && ADTSHEADER_CHECK(pBuff))
	{
		if (lParseADTSHeader(&pBs, pAudioHeadDataInfo))
		{
			return nRc;
		}
		else
		{
			return VO_ERR_INVALID_ARG;
		}
	}

	//RAW DATA 2bytes
	profile = lBitStreamGetBits(&pBs,5);
	if(profile==31)
	{
		profile = lBitStreamGetBits(&pBs,6);
		profile +=32;
	}

	sampIdx = lBitStreamGetBits(&pBs,4);
	if(sampIdx==0x0f)
	{
		sampFreq = lBitStreamGetBits(&pBs,24);
	}
	else
	{
		if(sampIdx < MAXNUM_SAMPLE_RATES) {
			sampFreq = AACSampRateTab[sampIdx];
		}
	}

	chanNum = lBitStreamGetBits(&pBs,4);

	pAudioHeadDataInfo->nChannels = chanNum;
	pAudioHeadDataInfo->nProfiles = profile;
	pAudioHeadDataInfo->nFrameType = VOAAC_RAWDATA;
	pAudioHeadDataInfo->nSampleBits = 16;
	pAudioHeadDataInfo->nSampleRate = sampFreq;

	return VO_ERR_NONE;
}

static int SearchSampleRateIndex(int nSampleRate)
{
	int i;
	for (i = 0; i < MAXNUM_SAMPLE_RATES; i++)
	{
		if (nSampleRate == AACSampRateTab[i])
		{
			return i;
		}	
	}
	return -1; //MCW C4715
}

VO_S16 WriteBits(WBitStream *hBitBuf,
				           VO_U32 writeValue,
				           VO_S16 noBitsToWrite)
{
	VO_S16 wBitPos;

	hBitBuf->cntBits += noBitsToWrite;   

	wBitPos = hBitBuf->wBitPos;
	wBitPos += noBitsToWrite;
	writeValue <<= 32 - wBitPos;	
	writeValue |= hBitBuf->cache;

	while (wBitPos >= 8) 
	{
		VO_U8 tmp;
		tmp = (VO_U8)((writeValue >> 24) & 0xFF);

		*hBitBuf->pWriteNext++ = tmp;		
		writeValue <<= 8;
		wBitPos -= 8;
	}

	hBitBuf->wBitPos = wBitPos;
	hBitBuf->cache = writeValue;

	return noBitsToWrite;
}

VO_U32 yyAudioAddAACADTSHeadPack (unsigned char * pInBuff, int nInSize, 
							      unsigned char * pOutBuff, int & nOutSize, YY_AUDIO_HEADDATAINFO *pAudioHeadDataInfo)
{
	//ACW_UNUSED <nRc>: int nRc = 0;
	int profile,sampIdx,chanNum,sampFreq;
	int nLen;
	//ACW_UNUSED <pBs>: lBitStream pBs;
	unsigned char *pData;
	//ACW_UNUSED <params>: VO_CODECBUFFER* params = (VO_CODECBUFFER*)pIn;

	nLen = nInSize;
	pData = (unsigned char*)pOutBuff;

	if (nLen < 2)
	{
		return VO_ERR_INVALID_ARG;
	}

	if(ADTSHEADER_CHECK(pInBuff))
	{
		memcpy(pData, pInBuff, nInSize);
		nOutSize = nInSize;
		return VO_ERR_NONE;
	}

	profile = pAudioHeadDataInfo->nProfiles;
	sampFreq = pAudioHeadDataInfo->nSampleRate;
	chanNum = pAudioHeadDataInfo->nChannels;

	sampIdx = SearchSampleRateIndex(sampFreq);

	// Pack the basic information to ADTS frame header 
	//add adts header
	pData[0] = 0xFF;
	pData[1] = 0xF9;

	pData[2] = 0x40 | (unsigned char)(sampIdx << 2) |((unsigned char)(chanNum >> 2) & 0x01);
	pData[3] = (unsigned char)((chanNum << 6) & 0xc0) | (0x01 << 3) | (unsigned char)(((nLen + 7) >> 11) & 0x03); //MCW_suggest_parentheses
	pData[4] = (unsigned char)((unsigned short)((nLen + 7) >> 3) & 0x00ff);
	pData[5] = (unsigned char)((unsigned char)((nLen + 7) & 0x07) << 5) | 0x1f;
	pData[6] = 0xF8;

	memcpy(pData + 7, pInBuff, nInSize);

	nOutSize = nInSize + 7;

	return VO_ERR_NONE;
}

VO_U32 yyAudioAACHeadData7to2Bytes(unsigned char * pInBuff, int nInSize, unsigned char * pOutBuff, int & nOutSize)
{
	VO_U32  nLen;
	VO_U32  nIdx;
    WBitStream pWbuf;
	lBitStream pBs;
	YY_AUDIO_HEADDATAINFO pAudioHeadDataInfo;

	nLen = nOutSize;
	if (nInSize < 7)
	{
		return VO_ERR_INVALID_ARG;
	}
	if (nLen < 2)
	{
		return VO_ERR_INVALID_ARG;
	}

	lBitStreamInit(&pBs, nInSize , pInBuff);
	pWbuf.pBitBufBase = pOutBuff;                                                    
	pWbuf.pBitBufEnd  = pOutBuff + nLen - 1;                                  
	pWbuf.pWriteNext  = pOutBuff;                                                    
	pWbuf.cache       = 0;
	pWbuf.wBitPos     = 0;                                                              
	pWbuf.cntBits     = 0;   
	pWbuf.size        = (nLen << 3);                                             
	pWbuf.isValid     = 1;  

	//ADTS length 7 or 9 Bytes
	if (nInSize >= 7 && ADTSHEADER_CHECK(pInBuff))
	{
		if (lParseADTSHeader(&pBs, &pAudioHeadDataInfo))
		{

			WriteBits(&pWbuf, pAudioHeadDataInfo.nProfiles, 5);  //profile
			// Write SampleRate
			for (nIdx = 0; nIdx < MAXNUM_SAMPLE_RATES; nIdx++)
			{
				if (pAudioHeadDataInfo.nSampleRate == AACSampRateTab[nIdx])
				{
					break;
				}
			}
			WriteBits(&pWbuf, nIdx, 4);   //samplerate 

			// Write Channels 
			WriteBits(&pWbuf, pAudioHeadDataInfo.nChannels, 4);

			// Write padding 
			WriteBits(&pWbuf, 0, 3);

			nOutSize = 2;

			return 0;
		}
		else
		{
			return VO_ERR_INVALID_ARG;
		}
	}
}

VO_U32 yyAudioAACRAWHeadPack (unsigned char * pInBuff, int nInSize, YY_AUDIO_HEADDATAINFO *pAudioHeadDataInfo)
{
	VO_U32  nLen;
	VO_U32  nIdx;
	WBitStream pWbuf;

	nLen = nInSize;
	if (nLen < 2)
	{
		return VO_ERR_INVALID_ARG;
	}

	// Init 
	pWbuf.pBitBufBase = pInBuff;                                                    
	pWbuf.pBitBufEnd  = pInBuff + nLen - 1;                                  
	pWbuf.pWriteNext  = pInBuff;                                                    
	pWbuf.cache       = 0;
	pWbuf.wBitPos     = 0;                                                              
	pWbuf.cntBits     = 0;   
	pWbuf.size        = (nLen << 3);                                             
	pWbuf.isValid     = 1;    
	
	// Write profile 
	//WriteBits(&pWbuf, pAudioHeadDataInfo->nProfiles, 5);
	WriteBits(&pWbuf, 2, 5);    //default aac lc profile

	// Write SampleRate 
    for (nIdx = 0; nIdx < MAXNUM_SAMPLE_RATES; nIdx++)
    {
		if (pAudioHeadDataInfo->nSampleRate == AACSampRateTab[nIdx])
		{
			break;
		}
    }
	WriteBits(&pWbuf, nIdx, 4);

	// Write Channels
	WriteBits(&pWbuf, pAudioHeadDataInfo->nChannels, 4);

	// Write padding
	WriteBits(&pWbuf, 0, 3);

	return 0;

}

VO_U32 yyAudioAACRAWDataOffset(unsigned char * pInBuff, int nInSize)
{
	VO_S32 nRc;
	VO_U32 nLen;
	lBitStream pBs;
	YY_AUDIO_HEADDATAINFO pAudioHeadDataInfo;

	nLen = nInSize;
	if (nLen < 9)
	{
		return VO_ERR_INVALID_ARG;
	}
	
	lBitStreamInit(&pBs, nLen, pInBuff);

	if(ADTSHEADER_CHECK(pInBuff))
	{
		nRc = lParseADTSHeader(&pBs, &pAudioHeadDataInfo);
		return nRc;
	}else
	{
		return 0;
	}
}


// MP3 Header Info 
#define  VOBUFGUARD    8
static int yyAudioSeekSyncWords(unsigned char *buf, unsigned int nbytes)
{
	unsigned int i;

	if(nbytes < VOBUFGUARD)
		return -1;

	for (i = 0; i < nbytes - VOBUFGUARD; i++) {
		if ( buf[i+0] == 0xff && (buf[i+1] & 0xe0) == 0xe0)
			return (int)i;
	}

	return -1;
}


VO_U32 yyAudioGetMP3HeadInfo (unsigned char * pBuff, int nSize, YY_AUDIO_HEADDATAINFO *pAudioHeadDataInfo)
{
	//ACW_UNUSED <nRc>: int nRc = 0;
	unsigned char *pData;
	//ACW_UNUSED <nSampleIndex>: int nSampleIndex;    //ACW_UNUSED <nSampleFrq>: int nSampleFrq, nSampleIndex;  //ACW_UNUSED <nChs>: int nChs, nSampleFrq, nSampleIndex;
	int nLen, nTempLen;
	int nVer, nBitR, nSampR;
	int nSlots;
	YYMP3HEADINFO sHeadInfo;

	nLen  = nSize;
	pData = pBuff;

	nTempLen = yyAudioSeekSyncWords (pData, nLen);
	if (nTempLen < 0)
	{
		return VO_ERR_INVALID_ARG;
	}

	//seek to Sync 
	pData = pBuff + nTempLen;
	nLen = nSize - nTempLen;

	nVer			=    (pData[1] >> 3) & 0x03;
	sHeadInfo.version =  (YYMP3VERS)( nVer == 0 ? VOMPEG25 : ((nVer & 0x01) ? VOMPEG1 : VOMPEG2) );
	sHeadInfo.layer	=    (YYMP3LAYERS)(4 - ((pData[1] >> 1) & 0x03));     
	sHeadInfo.crc		=    1 - ((pData[1] >> 0) & 0x01);
	nBitR			=    (pData[2] >> 4) & 0x0f;
	nSampR			=    (pData[2] >> 2) & 0x03;
	sHeadInfo.paddingBit = (pData[2] >> 1) & 0x01;
	sHeadInfo.mode	=    (YYCHMODES)(3 - ((pData[3] >> 6) & 0x03));        
	sHeadInfo.modeext =    (pData[3] >> 4) & 0x03;

	if (nSampR == 3 || nBitR == 15 || nVer == 1)
		return VO_ERR_INVALID_ARG;

	sHeadInfo.channels = (sHeadInfo.mode == YYMP3MONO ? 1 : 2);
	sHeadInfo.samplerate = YYMP3SampRateTab[sHeadInfo.version][nSampR];
	sHeadInfo.subIndex = sHeadInfo.version*3 + nSampR;
	sHeadInfo.bitrate = ((int)YYMP3BitrateTab[sHeadInfo.version][sHeadInfo.layer - 1][nBitR]) * 1000;

	if(sHeadInfo.bitrate)
	{	
		if (sHeadInfo.layer == VOLAYER1)
			sHeadInfo.framelen = ((12 * sHeadInfo.bitrate / sHeadInfo.samplerate) + sHeadInfo.paddingBit) * 4;
		else {
			nSlots = (sHeadInfo.version > VOMPEG1 && sHeadInfo.layer == VOLAYER2) ? 72 : 144;
			sHeadInfo.framelen = nSlots * sHeadInfo.bitrate/sHeadInfo.samplerate + sHeadInfo.paddingBit;					
		}
	}	

	pAudioHeadDataInfo->nChannels = sHeadInfo.channels;
	pAudioHeadDataInfo->nSampleRate = sHeadInfo.samplerate;
	pAudioHeadDataInfo->nSampleBits = 16;

	return VO_ERR_NONE;
}


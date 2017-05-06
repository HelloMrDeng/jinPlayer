/*******************************************************************************
	File:		neonAudioDec.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "yyneonFunc.h"

#include "neonAudioDec.h"

#include "libavcodec\fmtconvert.h"

void YY_format_convert_init (void * pCtx, void * pCtx2)
{
	AVCodecContext *	pavCtx = NULL;
	AACContext *		pAACCtx = NULL;
	FmtConvertContext * fmtConvert = (FmtConvertContext *)pCtx;
	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pFormatCvtFunc == NULL)
			g_neonFunc->pFormatCvtFunc = malloc (sizeof (FmtConvertContext));
		memcpy (g_neonFunc->pFormatCvtFunc, pCtx, sizeof (FmtConvertContext));
	}

	pavCtx = (AVCodecContext *)pCtx2;
	if (pavCtx->codec_id == AV_CODEC_ID_AAC)
	{
		pAACCtx = (AACContext *)pavCtx->priv_data;
//		pAACCtx->force_dmono_mode = 1;
//		pAACCtx->dmono_mode = 1;
#ifdef _OS_WINCE
		pAACCtx->oc[1].m4ac.sbr = 0;
#endif // WINCE
	}
}
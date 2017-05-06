/*******************************************************************************
	File:		neonMpvDec.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "yyneonFunc.h"

#include "config.h"
#include "neonMpvDec.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

MpegEncContext * pMpvCmn = NULL;

void yy_dct_unquantize_h263_inter(MpegEncContext *s, int16_t *block, int n, int qscale)
{
#ifdef _CPU_NEON
	ff_dct_unquantize_h263_intra_neon (s, block, n, qscale);
#else
	pMpvCmn->dct_unquantize_h263_intra (s, block, n, qscale);
#endif // _CPU_NEON
}

void yy_dct_unquantize_h263_intra(MpegEncContext *s, int16_t *block, int n, int qscale)
{
#ifdef _CPU_NEON
	ff_dct_unquantize_h263_inter_neon (s, block, n, qscale);
#else
	pMpvCmn->dct_unquantize_h263_inter (s, block, n, qscale);
#endif // _CPU_NEON
}

void YY_mpv_common_init (void * pCtx)
{
	MpegEncContext * pMpegCtx = (MpegEncContext *)pCtx;
	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pMPVCmnFunc == NULL)
			g_neonFunc->pMPVCmnFunc = malloc (sizeof (MpegEncContext));
		memcpy (g_neonFunc->pMPVCmnFunc, pCtx, sizeof (MpegEncContext));

		pMpvCmn = (MpegEncContext *)g_neonFunc->pMPVCmnFunc;
	}

#ifdef _CPU_NEON
	pMpegCtx->dct_unquantize_h263_intra = yy_dct_unquantize_h263_intra;
	pMpegCtx->dct_unquantize_h263_inter = yy_dct_unquantize_h263_inter;
#endif // _CPU_NEON
}

/*******************************************************************************
	File:		neonDspUtil.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "yyneonFunc.h"

#include "neonDspUtil.h"

#include "libavcodec\avcodec.h"
#include "libavcodec/dsputil.h"

DSPContext * gDspCtx = NULL;

void yy_simple_idct_c (int16_t *data)
{
#ifdef _CPU_NEON
	ff_simple_idct_neon (data);
#else
	gDspCtx->idct (data);
#endif // _CPU_NEON
}

void yy_simple_idct_put_c (uint8_t *dest, int line_size, int16_t *data)
{
#ifdef _CPU_NEON
	ff_simple_idct_put_neon (dest, line_size, data);
#else
	gDspCtx->idct_put (dest, line_size, data);
#endif // _CPU_NEON
}

void yy_simple_idct_add_c (uint8_t *dest, int line_size, int16_t *data)
{
#ifdef _CPU_NEON
	ff_simple_idct_add_neon (dest, line_size, data);
#else
	gDspCtx->idct_add (dest, line_size, data);
#endif // _CPU_NEON
}

void yy_clear_block_c (int16_t *block)
{
#ifdef _CPU_NEON
	ff_clear_block_neon (block);
#else
	gDspCtx->clear_block (block);
#endif // _CPU_NEON

}

void yy_clear_blocks_c(int16_t *blocks)
{
#ifdef _CPU_NEON
	ff_clear_blocks_neon (blocks);
#else
	gDspCtx->clear_blocks (blocks);
#endif // _CPU_NEON
}

void YY_dsp_util_init (void * pCtx, void * pCtx2)
{
	DSPContext *		pDspCtx = (DSPContext *)pCtx;
	AVCodecContext *	pAVCtx = (AVCodecContext *)pCtx2;
	const int			high_bit_depth = pAVCtx->bits_per_raw_sample > 8;
	AVCodecContext *	pVideoCtx = NULL;

	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pDspUtilFunc == NULL)
			g_neonFunc->pDspUtilFunc = malloc (sizeof (DSPContext));
		memcpy (g_neonFunc->pDspUtilFunc, pCtx, sizeof (DSPContext));

		gDspCtx = (DSPContext *)g_neonFunc->pDspUtilFunc;

		pVideoCtx = (AVCodecContext *)g_neonFunc->pVideoDecCtx;
	}

#ifdef _CPU_NEON

	if (!pAVCtx->lowres && pAVCtx->bits_per_raw_sample <= 8)
	{
		if (pAVCtx->idct_algo == FF_IDCT_AUTO ||
			pAVCtx->idct_algo == FF_IDCT_SIMPLENEON) 
		{
		if (pVideoCtx != NULL && pVideoCtx->codec_id == AV_CODEC_ID_RV30 ||
			pVideoCtx != NULL && pVideoCtx->codec_id == AV_CODEC_ID_RV40 ||
			pVideoCtx != NULL && pVideoCtx->codec_id == AV_CODEC_ID_H264)
			{
//				pDspCtx->idct_put              = yy_simple_idct_put_c;
//				pDspCtx->idct_add              = yy_simple_idct_add_c;
//				pDspCtx->idct                  = yy_simple_idct_c;
//				pDspCtx->idct_permutation_type = FF_PARTTRANS_IDCT_PERM;
			}
		}
	}

	if (!high_bit_depth) 
	{
		if (pVideoCtx != NULL && pVideoCtx->codec_id == AV_CODEC_ID_RV30 ||
			pVideoCtx != NULL && pVideoCtx->codec_id == AV_CODEC_ID_RV40 ||
			pVideoCtx != NULL && pVideoCtx->codec_id == AV_CODEC_ID_H264)
		{
			pDspCtx->clear_block  = yy_clear_block_c;
			pDspCtx->clear_blocks = yy_clear_blocks_c;
		}
	}

	pDspCtx->add_pixels_clamped			= ff_add_pixels_clamped_neon;
	pDspCtx->put_pixels_clamped			= ff_put_pixels_clamped_neon;
	pDspCtx->put_signed_pixels_clamped	= ff_put_signed_pixels_clamped_neon;

//	pDspCtx->vector_clipf               = ff_vector_clipf_neon;
	pDspCtx->vector_clip_int32          = ff_vector_clip_int32_neon;

	pDspCtx->scalarproduct_int16			= ff_scalarproduct_int16_neon;
	pDspCtx->scalarproduct_and_madd_int16	= ff_scalarproduct_and_madd_int16_neon;

	pDspCtx->apply_window_int16 = ff_apply_window_int16_neon;
#endif // _CPU_NEON
}
/*******************************************************************************
	File:		neonRV34Dec.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "yyneonFunc.h"

#include "neonRV34Dec.h"
#include "neonH264Dec.h"

#include "libavcodec\avcodec.h"
#include "libavcodec/rv34dsp.h"

RV34DSPContext * pRV34Dsp = NULL;
RV34DSPContext * pRV40Dsp = NULL;


void YY_rv34_dsp_init (void * pCtx)
{
	RV34DSPContext * pRV34Dsp = (RV34DSPContext *)pCtx;
	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pRV34DspFunc == NULL)
			g_neonFunc->pRV34DspFunc = malloc (sizeof (RV34DSPContext));
		memcpy (g_neonFunc->pRV34DspFunc, pCtx, sizeof (RV34DSPContext));

		pRV34Dsp = (RV34DSPContext *)g_neonFunc->pRV34DspFunc;
	}

#ifdef _CPU_NEON
    pRV34Dsp->rv34_inv_transform    = ff_rv34_inv_transform_noround_neon;
    pRV34Dsp->rv34_inv_transform_dc = ff_rv34_inv_transform_noround_dc_neon;
    pRV34Dsp->rv34_idct_add			= ff_rv34_idct_add_neon;
    pRV34Dsp->rv34_idct_dc_add		= ff_rv34_idct_dc_add_neon;
#endif // YY_NEON
}

void ff_put_rv40_qpel16_mc33_c(uint8_t *dst, uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
   ff_put_pixels16_xy2_neon (dst, src, stride, 16);
#endif // YY_NEON
}

void ff_avg_rv40_qpel16_mc33_c(uint8_t *dst, uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
    ff_avg_pixels16_xy2_neon (dst, src, stride, 16);
#endif // YY_NEON
}

void ff_put_rv40_qpel8_mc33_c(uint8_t *dst, uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
    ff_put_pixels8_xy2_neon (dst, src, stride, 8);
#endif // YY_NEON
}

void ff_avg_rv40_qpel8_mc33_c(uint8_t *dst, uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
    ff_avg_pixels8_xy2_neon (dst, src, stride, 8);
#endif // YY_NEON
}

void YY_rv40_dsp_init (void * pCtx)
{
	RV34DSPContext * pDsp = (RV34DSPContext *)pCtx;
	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pRV40DspFunc == NULL)
			g_neonFunc->pRV40DspFunc = malloc (sizeof (RV34DSPContext));
		memcpy (g_neonFunc->pRV40DspFunc, pCtx, sizeof (RV34DSPContext));

		pRV40Dsp = (RV34DSPContext *)g_neonFunc->pRV40DspFunc;
	}

//	return;
#ifdef _CPU_NEON
	pDsp->put_pixels_tab[0][ 1] = ff_put_rv40_qpel16_mc10_neon;
	pDsp->put_pixels_tab[0][ 3] = ff_put_rv40_qpel16_mc30_neon;

	pDsp->put_pixels_tab[0][ 4] = ff_put_rv40_qpel16_mc01_neon;
	pDsp->put_pixels_tab[0][ 5] = ff_put_rv40_qpel16_mc11_neon;
	pDsp->put_pixels_tab[0][ 6] = ff_put_rv40_qpel16_mc21_neon;
	pDsp->put_pixels_tab[0][ 7] = ff_put_rv40_qpel16_mc31_neon;
	pDsp->put_pixels_tab[0][ 9] = ff_put_rv40_qpel16_mc12_neon;
	pDsp->put_pixels_tab[0][10] = ff_put_rv40_qpel16_mc22_neon;
	pDsp->put_pixels_tab[0][11] = ff_put_rv40_qpel16_mc32_neon;
	pDsp->put_pixels_tab[0][12] = ff_put_rv40_qpel16_mc03_neon;
	pDsp->put_pixels_tab[0][13] = ff_put_rv40_qpel16_mc13_neon;
	pDsp->put_pixels_tab[0][14] = ff_put_rv40_qpel16_mc23_neon;
	pDsp->put_pixels_tab[0][15] = ff_put_rv40_qpel16_mc33_c;//ff_put_rv40_qpel16_mc33_neon;

	pDsp->avg_pixels_tab[0][ 1] = ff_avg_rv40_qpel16_mc10_neon;
	pDsp->avg_pixels_tab[0][ 3] = ff_avg_rv40_qpel16_mc30_neon;
	pDsp->avg_pixels_tab[0][ 4] = ff_avg_rv40_qpel16_mc01_neon;
	pDsp->avg_pixels_tab[0][ 5] = ff_avg_rv40_qpel16_mc11_neon;
	pDsp->avg_pixels_tab[0][ 6] = ff_avg_rv40_qpel16_mc21_neon;
	pDsp->avg_pixels_tab[0][ 7] = ff_avg_rv40_qpel16_mc31_neon;
	pDsp->avg_pixels_tab[0][ 9] = ff_avg_rv40_qpel16_mc12_neon;
	pDsp->avg_pixels_tab[0][10] = ff_avg_rv40_qpel16_mc22_neon;
	pDsp->avg_pixels_tab[0][11] = ff_avg_rv40_qpel16_mc32_neon;
	pDsp->avg_pixels_tab[0][12] = ff_avg_rv40_qpel16_mc03_neon;
	pDsp->avg_pixels_tab[0][13] = ff_avg_rv40_qpel16_mc13_neon;
	pDsp->avg_pixels_tab[0][14] = ff_avg_rv40_qpel16_mc23_neon;
	pDsp->avg_pixels_tab[0][15] = ff_avg_rv40_qpel16_mc33_c; //ff_avg_rv40_qpel16_mc33_neon;

	pDsp->put_pixels_tab[1][ 1] = ff_put_rv40_qpel8_mc10_neon;
	pDsp->put_pixels_tab[1][ 3] = ff_put_rv40_qpel8_mc30_neon;
	pDsp->put_pixels_tab[1][ 4] = ff_put_rv40_qpel8_mc01_neon;
	pDsp->put_pixels_tab[1][ 5] = ff_put_rv40_qpel8_mc11_neon;
	pDsp->put_pixels_tab[1][ 6] = ff_put_rv40_qpel8_mc21_neon;
	pDsp->put_pixels_tab[1][ 7] = ff_put_rv40_qpel8_mc31_neon;
	pDsp->put_pixels_tab[1][ 9] = ff_put_rv40_qpel8_mc12_neon;
	pDsp->put_pixels_tab[1][10] = ff_put_rv40_qpel8_mc22_neon;
	pDsp->put_pixels_tab[1][11] = ff_put_rv40_qpel8_mc32_neon;
	pDsp->put_pixels_tab[1][12] = ff_put_rv40_qpel8_mc03_neon;
	pDsp->put_pixels_tab[1][13] = ff_put_rv40_qpel8_mc13_neon;
	pDsp->put_pixels_tab[1][14] = ff_put_rv40_qpel8_mc23_neon;
	pDsp->put_pixels_tab[1][15] = ff_put_rv40_qpel8_mc33_c; //ff_put_rv40_qpel8_mc33_neon;

	pDsp->avg_pixels_tab[1][ 1] = ff_avg_rv40_qpel8_mc10_neon;
	pDsp->avg_pixels_tab[1][ 3] = ff_avg_rv40_qpel8_mc30_neon;
	pDsp->avg_pixels_tab[1][ 4] = ff_avg_rv40_qpel8_mc01_neon;
	pDsp->avg_pixels_tab[1][ 5] = ff_avg_rv40_qpel8_mc11_neon;
	pDsp->avg_pixels_tab[1][ 6] = ff_avg_rv40_qpel8_mc21_neon;
	pDsp->avg_pixels_tab[1][ 7] = ff_avg_rv40_qpel8_mc31_neon;
	pDsp->avg_pixels_tab[1][ 9] = ff_avg_rv40_qpel8_mc12_neon;
	pDsp->avg_pixels_tab[1][10] = ff_avg_rv40_qpel8_mc22_neon;
	pDsp->avg_pixels_tab[1][11] = ff_avg_rv40_qpel8_mc32_neon;
	pDsp->avg_pixels_tab[1][12] = ff_avg_rv40_qpel8_mc03_neon;
	pDsp->avg_pixels_tab[1][13] = ff_avg_rv40_qpel8_mc13_neon;
	pDsp->avg_pixels_tab[1][14] = ff_avg_rv40_qpel8_mc23_neon;
	pDsp->avg_pixels_tab[1][15] = ff_avg_rv40_qpel8_mc33_c; //ff_avg_rv40_qpel8_mc33_neon;

	pDsp->put_chroma_pixels_tab[0] = yy_put_h264_chroma_mc8_func; //ff_put_rv40_chroma_mc8_neon;
	pDsp->put_chroma_pixels_tab[1] = yy_put_h264_chroma_mc4_func; //ff_put_rv40_chroma_mc4_neon;
	pDsp->avg_chroma_pixels_tab[0] = yy_avg_h264_chroma_mc8_func; //ff_avg_rv40_chroma_mc8_neon;
	pDsp->avg_chroma_pixels_tab[1] = yy_avg_h264_chroma_mc4_func; //ff_avg_rv40_chroma_mc4_neon;

	pDsp->rv40_weight_pixels_tab[0][0] = ff_rv40_weight_func_16_neon;
	pDsp->rv40_weight_pixels_tab[0][1] = ff_rv40_weight_func_8_neon;

	pDsp->rv40_loop_filter_strength[0] = ff_rv40_h_loop_filter_strength_neon;
	pDsp->rv40_loop_filter_strength[1] = ff_rv40_v_loop_filter_strength_neon;
	pDsp->rv40_weak_loop_filter[0]     = ff_rv40_h_weak_loop_filter_neon;
	pDsp->rv40_weak_loop_filter[1]     = ff_rv40_v_weak_loop_filter_neon;
#endif // _CPU_NEON
}

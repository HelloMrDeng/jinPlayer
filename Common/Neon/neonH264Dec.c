/*******************************************************************************
	File:		neonH264Dec.cpp

	Contains:	neon H264 decoder wrap code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "yyneonFunc.h"

#include "neonH264Dec.h"

#include "libavcodec\avcodec.h"
#include "libavcodec\h264chroma.h"
#include "libavcodec\h264dsp.h"
#include "libavcodec\h264pred.h"
#include "libavcodec\h264qpel.h"

const uint8_t scan8[16 * 3 + 3] = {
    4 +  1 * 8, 5 +  1 * 8, 4 +  2 * 8, 5 +  2 * 8,
    6 +  1 * 8, 7 +  1 * 8, 6 +  2 * 8, 7 +  2 * 8,
    4 +  3 * 8, 5 +  3 * 8, 4 +  4 * 8, 5 +  4 * 8,
    6 +  3 * 8, 7 +  3 * 8, 6 +  4 * 8, 7 +  4 * 8,
    4 +  6 * 8, 5 +  6 * 8, 4 +  7 * 8, 5 +  7 * 8,
    6 +  6 * 8, 7 +  6 * 8, 6 +  7 * 8, 7 +  7 * 8,
    4 +  8 * 8, 5 +  8 * 8, 4 +  9 * 8, 5 +  9 * 8,
    6 +  8 * 8, 7 +  8 * 8, 6 +  9 * 8, 7 +  9 * 8,
    4 + 11 * 8, 5 + 11 * 8, 4 + 12 * 8, 5 + 12 * 8,
    6 + 11 * 8, 7 + 11 * 8, 6 + 12 * 8, 7 + 12 * 8,
    4 + 13 * 8, 5 + 13 * 8, 4 + 14 * 8, 5 + 14 * 8,
    6 + 13 * 8, 7 + 13 * 8, 6 + 14 * 8, 7 + 14 * 8,
    0 +  0 * 8, 0 +  5 * 8, 0 + 10 * 8
};

H264ChromaContext * pH264Chm	= NULL;
H264DSPContext *	pH264Dsp	= NULL;
H264QpelContext *	pH264QPEL	= NULL;
H264PredContext *	pH264Pred	= NULL;


void yy_put_h264_chroma_mc2_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y)
{
#ifdef _CPU_NEON
	ff_put_h264_chroma_mc2_neon (dst, src, srcStride, h, x, y);
#else
	pH264Chm->put_h264_chroma_pixels_tab[2] (dst, src, srcStride, h, x, y);
#endif // _CPU_NEON
}

void yy_put_h264_chroma_mc4_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y)
{
#ifdef _CPU_NEON
	ff_put_h264_chroma_mc4_neon (dst, src, srcStride, h, x, y);
#else
	pH264Chm->put_h264_chroma_pixels_tab[1] (dst, src, srcStride, h, x, y);
#endif // _CPU_NEON
}

void yy_put_h264_chroma_mc8_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y)
{
#ifdef _CPU_NEON
	ff_put_h264_chroma_mc8_neon (dst, src, srcStride, h, x, y);
#else
	pH264Chm->put_h264_chroma_pixels_tab[0] (dst, src, srcStride, h, x, y);
#endif // _CPU_NEON
}

void yy_avg_h264_chroma_mc2_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y)
{
#ifdef _CPU_NEON
	ff_avg_h264_chroma_mc2_neon (dst, src, srcStride, h, x, y);
#else
	pH264Chm->avg_h264_chroma_pixels_tab[2] (dst, src, srcStride, h, x, y);
#endif // _CPU_NEON
}

void yy_avg_h264_chroma_mc4_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y)
{
#ifdef _CPU_NEON
	ff_avg_h264_chroma_mc4_neon (dst, src, srcStride, h, x, y);
#else
	pH264Chm->avg_h264_chroma_pixels_tab[1]  (dst, src, srcStride, h, x, y);
#endif // _CPU_NEON
}

void yy_avg_h264_chroma_mc8_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y)
{
#ifdef _CPU_NEON
	ff_avg_h264_chroma_mc8_neon (dst, src, srcStride, h, x, y);
#else
	pH264Chm->avg_h264_chroma_pixels_tab[0] (dst, src, srcStride, h, x, y);
#endif // _CPU_NEON
}


void YY_h264chroma_init (void * pCtx, int bit_depth)
{
	H264ChromaContext * pChroma = (H264ChromaContext *)pCtx;
	if (bit_depth)
		return;

	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pH264ChromaFunc == NULL)
			g_neonFunc->pH264ChromaFunc = malloc (sizeof (H264ChromaContext));
		memcpy (g_neonFunc->pH264ChromaFunc, pCtx, sizeof (H264ChromaContext));

		pH264Chm = (H264ChromaContext *)g_neonFunc->pH264ChromaFunc;
	}

#ifdef _CPU_NEON
    pChroma->put_h264_chroma_pixels_tab[0] = yy_put_h264_chroma_mc8_func;
    pChroma->put_h264_chroma_pixels_tab[1] = yy_put_h264_chroma_mc4_func;
    pChroma->put_h264_chroma_pixels_tab[2] = yy_put_h264_chroma_mc2_func;

    pChroma->avg_h264_chroma_pixels_tab[0] = yy_avg_h264_chroma_mc8_func;
    pChroma->avg_h264_chroma_pixels_tab[1] = yy_avg_h264_chroma_mc4_func;
    pChroma->avg_h264_chroma_pixels_tab[2] = yy_avg_h264_chroma_mc2_func;
#endif // _CPU_NEON
}


void  yy_h264_v_loop_filter_luma (uint8_t *pix , //align 16 
									int stride,
									int alpha, int beta, int8_t *tc0)
{
#ifdef _CPU_NEON
	ff_h264_v_loop_filter_luma_neon (pix, stride, alpha, beta, tc0);
#else
	pH264Dsp->h264_v_loop_filter_luma (pix, stride, alpha, beta, tc0);
#endif // YY_NEON
}

void  yy_h264_h_loop_filter_luma (uint8_t *pix , //align 4 
									int stride,
									int alpha, int beta, int8_t *tc0)
{
#ifdef _CPU_NEON
	ff_h264_h_loop_filter_luma_neon (pix, stride, alpha, beta, tc0);
#else
	pH264Dsp->h264_h_loop_filter_luma (pix, stride, alpha, beta, tc0);
#endif // YY_NEON
}

void  yy_h264_h_loop_filter_luma_mbaff (uint8_t *pix , //align 16 
										int stride,
										int alpha, int beta, int8_t *tc0)
{
}

// v/h_loop_filter_luma_intra: align 16 
void  yy_h264_v_loop_filter_luma_intra (uint8_t *pix, int stride,
                                      int alpha, int beta)
{
}

void  yy_h264_h_loop_filter_luma_intra (uint8_t *pix, int stride,
                                      int alpha, int beta)
{
}

void  yy_h264_h_loop_filter_luma_mbaff_intra (uint8_t *pix , //align 16
                                            int stride, int alpha, int beta)
{
}

void  yy_h264_v_loop_filter_chroma (uint8_t *pix , // align 8, 
									int stride,
									int alpha, int beta, int8_t *tc0)
{
#ifdef _CPU_NEON
	ff_h264_v_loop_filter_chroma_neon (pix, stride, alpha, beta, tc0);
#else
	pH264Dsp->h264_v_loop_filter_chroma (pix, stride, alpha, beta, tc0);
#endif // YY_NEON
}

void  yy_h264_h_loop_filter_chroma (uint8_t *pix, // align 4 
									int stride,
									int alpha, int beta, int8_t *tc0)
{
#ifdef _CPU_NEON
	ff_h264_h_loop_filter_chroma_neon (pix, stride, alpha, beta, tc0);
#else
	pH264Dsp->h264_h_loop_filter_chroma (pix, stride, alpha, beta, tc0);
#endif // YY_NEON
}

void  yy_h264_h_loop_filter_chroma_mbaff (uint8_t *pix, // align 8
                                        int stride, int alpha, int beta,
                                        int8_t *tc0)
{
}

void  yy_h264_v_loop_filter_chroma_intra (uint8_t *pix, //align 8
                                        int stride, int alpha, int beta)
{
}

void  yy_h264_h_loop_filter_chroma_intra (uint8_t *pix, //align 8
                                        int stride, int alpha, int beta)
{
}

void  yy_h264_h_loop_filter_chroma_mbaff_intra (uint8_t *pix, //align 8
                                              int stride, int alpha, int beta)
{
}

// h264_loop_filter_strength: simd only. the C version is inlined in h264.c
void  yy_h264_loop_filter_strength (int16_t bS[2][4][4], uint8_t nnz[40],
                                  int8_t ref[2][40], int16_t mv[2][40][2],
                                  int bidir, int edges, int step,
                                  int mask_mv0, int mask_mv1, int field)
{
}

// use the c function to call neon functions.
void yy_h264_idct_add16_c(uint8_t *dst, const int *block_offset, int16_t *block, int stride, const uint8_t nnzc[15*8])
{
#ifdef _CPU_NEON
	int i;
	for(i=0; i<16; i++)
	{
		int nnz = nnzc [scan8[i]];
		if(nnz)
		{
			if(nnz==1 && ((dctcoef*)block)[i*16])
				ff_h264_idct_dc_add_neon (dst + block_offset[i], block + i*16*sizeof(pixel), stride);
			else                                  
				ff_h264_idct_add_neon (dst + block_offset[i], block + i*16*sizeof(pixel), stride);
		}
	}
#else
	pH264Dsp->h264_idct_add16 (dst, block_offset, block, stride, nnzc);
#endif // __CPU_NEON
}

void yy_h264_idct_add16intra_c (uint8_t *dst, const int *block_offset, int16_t *block, int stride, const uint8_t nnzc[15*8])
{
#ifdef _CPU_NEON
	int i;
	for(i=0; i<16; i++)
	{
		if(nnzc[ scan8[i] ])             
			ff_h264_idct_add_neon (dst + block_offset[i], block + i*16*sizeof(pixel), stride);
		else if(((dctcoef*)block)[i*16]) 
			ff_h264_idct_dc_add_neon (dst + block_offset[i], block + i*16*sizeof(pixel), stride);
	}
#else
	pH264Dsp->h264_idct_add16intra (dst, block_offset, block, stride, nnzc);
#endif // __CPU_NEON
}

void yy_h264_idct_add8_c (uint8_t **dest, const int *block_offset, int16_t *block, int stride, const uint8_t nnzc[15*8])
{
#ifdef _CPU_NEON
	int i, j;
	for(j=1; j<3; j++)
	{
		for(i=j*16; i<j*16+4; i++)
		{
			if(nnzc[ scan8[i] ])
				ff_h264_idct_add_neon (dest[j-1] + block_offset[i], block + i*16*sizeof(pixel), stride);
			else if(((dctcoef*)block)[i*16])
				ff_h264_idct_dc_add_neon (dest[j-1] + block_offset[i], block + i*16*sizeof(pixel), stride);
		}
	}
#else
	pH264Dsp->h264_idct_add8 (dest, block_offset, block, stride, nnzc);
#endif // __CPU_NEON
}

void yy_h264_idct8_add4_c (uint8_t *dst, const int *block_offset, int16_t *block, int stride, const uint8_t nnzc[15*8])
{
#ifdef _CPU_NEON
	int i;
	for(i=0; i<16; i+=4)
	{
		int nnz = nnzc[ scan8[i] ];
		if(nnz)
		{
			if(nnz==1 && ((dctcoef*)block)[i*16]) 
				ff_h264_idct8_dc_add_neon (dst + block_offset[i], block + i*16*sizeof(pixel), stride);
			else                                  
				ff_h264_idct8_add_neon (dst + block_offset[i], block + i*16*sizeof(pixel), stride);
		}
	}
#else
	pH264Dsp->h264_idct8_add4 (dst, block_offset, block, stride, nnzc);
#endif // __CPU_NEON
}

// IDCT 
void  yy_h264_idct_add (uint8_t *dst, // align 4,
                      int16_t *block , //align 16 
					  int stride)
{
#ifdef _CPU_NEON
	ff_h264_idct_add_neon (dst, block, stride);
#else
	pH264Dsp->h264_idct_add (dst, block, stride);
#endif // YY_NEON
}

void  yy_h264_idct8_add (uint8_t *dst, //align 8,
                       int16_t *block , //align 16 
					   int stride)
{
#ifdef _CPU_NEON
	ff_h264_idct8_add_neon (dst, block, stride);
#else
	pH264Dsp->h264_idct8_add (dst, block, stride);
#endif // YY_NEON
}

void  yy_h264_idct_dc_add (uint8_t *dst, // align 4
                         int16_t *block , //align 16 
						 int stride)
{
#ifdef _CPU_NEON
	ff_h264_idct_dc_add_neon (dst, block, stride);
#else
	pH264Dsp->h264_idct_dc_add (dst, block, stride);
#endif // YY_NEON
}

void  yy_h264_idct8_dc_add (uint8_t *dst, // align 8
                          int16_t *block , //align 16 
						  int stride)
{
#ifdef _CPU_NEON
	ff_h264_idct8_dc_add_neon (dst, block, stride);
#else
	pH264Dsp->h264_idct8_dc_add (dst, block, stride);
#endif // YY_NEON
}

void  yy_h264_idct_add16 (uint8_t *dst , //align 16 
						  const int *blockoffset,
                        int16_t *block , //align 16 
						int stride,
                        const uint8_t nnzc[15 * 8])
{
#ifdef _CPU_NEON
	ff_h264_idct_add16_neon (dst, blockoffset, block, stride, nnzc);
#else
	pH264Dsp->h264_idct_add16 (dst, blockoffset, block, stride, nnzc);
#endif // YY_NEON
}

void  yy_h264_idct8_add4 (uint8_t *dst, //align 16, 
						const int *blockoffset,
                        int16_t *block, //align 16
						int stride,
                        const uint8_t nnzc[15 * 8])
{
#ifdef _CPU_NEON
	ff_h264_idct8_add4_neon (dst, blockoffset, block, stride, nnzc);
#else
	pH264Dsp->h264_idct8_add4 (dst, blockoffset, block, stride, nnzc);
#endif // YY_NEON
}

void  yy_h264_idct_add8 (uint8_t **dst, //align 16 
							const int *blockoffset,
							int16_t *block, //align 16
							int stride,
							const uint8_t nnzc[15 * 8])
{
#ifdef _CPU_NEON
	ff_h264_idct_add8_neon (dst, blockoffset, block, stride, nnzc);
#else
	pH264Dsp->h264_idct_add8 (dst, blockoffset, block, stride, nnzc);
#endif // YY_NEON
}

void  yy_h264_idct_add16intra (uint8_t *dst, // align 16
								const int *blockoffset,
								int16_t *block, //align 16
								int stride, const uint8_t nnzc[15 * 8])
{
#ifdef _CPU_NEON
	ff_h264_idct_add16intra_neon (dst, blockoffset, block, stride, nnzc);
#else
	pH264Dsp->h264_idct_add16intra (dst, blockoffset, block, stride, nnzc);
#endif // YY_NEON
}

void  yy_h264_luma_dc_dequant_idct (int16_t *output,
                                  int16_t *input, //align 16
								  int qmul)
{
}

void  yy_h264_chroma_dc_dequant_idct (int16_t *block, int qmul)
{
}

void  yy_h264_add_pixels8_clear (uint8_t *dst, int16_t *block, int stride)
{
}

void  yy_h264_add_pixels4_clear (uint8_t *dst, int16_t *block, int stride)
{
}


void YY_h264dsp_init (void * pCtx, const int bit_depth, const int chroma_format_idc)
{
	H264DSPContext * pDspCtx = (H264DSPContext *)pCtx;

	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pH264DspFunc == NULL)
			g_neonFunc->pH264DspFunc = malloc (sizeof (H264DSPContext));
		memcpy (g_neonFunc->pH264DspFunc, pCtx, sizeof (H264DSPContext));

		pH264Dsp = (H264DSPContext *)g_neonFunc->pH264DspFunc;
	}

	if (bit_depth == 8) 
	{
#ifdef _CPU_NEON
		pDspCtx->h264_v_loop_filter_luma   = yy_h264_v_loop_filter_luma;
		pDspCtx->h264_h_loop_filter_luma   = yy_h264_h_loop_filter_luma;
		if(chroma_format_idc == 1)
		{
			pDspCtx->h264_v_loop_filter_chroma = yy_h264_v_loop_filter_chroma;
			pDspCtx->h264_h_loop_filter_chroma = yy_h264_h_loop_filter_chroma;
		}
		pDspCtx->weight_h264_pixels_tab[0] = ff_weight_h264_pixels_16_neon;
		pDspCtx->weight_h264_pixels_tab[1] = ff_weight_h264_pixels_8_neon;
		pDspCtx->weight_h264_pixels_tab[2] = ff_weight_h264_pixels_4_neon;

		pDspCtx->biweight_h264_pixels_tab[0] = ff_biweight_h264_pixels_16_neon;
		pDspCtx->biweight_h264_pixels_tab[1] = ff_biweight_h264_pixels_8_neon;
		pDspCtx->biweight_h264_pixels_tab[2] = ff_biweight_h264_pixels_4_neon;

		pDspCtx->h264_idct_add        = yy_h264_idct_add;
		pDspCtx->h264_idct_dc_add     = yy_h264_idct_dc_add;
		pDspCtx->h264_idct_add16      = yy_h264_idct_add16_c; //yy_h264_idct_add16;
		pDspCtx->h264_idct_add16intra = yy_h264_idct_add16intra_c; //yy_h264_idct_add16intra;
		if (chroma_format_idc == 1)
			pDspCtx->h264_idct_add8   = yy_h264_idct_add8_c; //yy_h264_idct_add8;
		pDspCtx->h264_idct8_add       = yy_h264_idct8_add;
		pDspCtx->h264_idct8_dc_add    = yy_h264_idct8_dc_add;
		pDspCtx->h264_idct8_add4      = yy_h264_idct8_add4_c; //yy_h264_idct8_add4;
#endif // _CPU_NEON
	}
}

void YY_h264qpel_init (void * pCtx, int bit_depth)
{
    const int high_bit_depth = bit_depth > 8;
	H264QpelContext * pQPelCtx = (H264QpelContext *)pCtx;

	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pH264QpelFunc == NULL)
			g_neonFunc->pH264QpelFunc = malloc (sizeof (H264QpelContext));
		memcpy (g_neonFunc->pH264QpelFunc, pCtx, sizeof (H264QpelContext));

		pH264QPEL = (H264QpelContext *)g_neonFunc->pH264QpelFunc;
	}

    if (!high_bit_depth) 
	{
#ifdef _CPU_NEON
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 0] = ff_put_h264_qpel16_mc00_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 1] = ff_put_h264_qpel16_mc10_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 2] = ff_put_h264_qpel16_mc20_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 3] = ff_put_h264_qpel16_mc30_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 4] = ff_put_h264_qpel16_mc01_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 5] = ff_put_h264_qpel16_mc11_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 6] = ff_put_h264_qpel16_mc21_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 7] = ff_put_h264_qpel16_mc31_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 8] = ff_put_h264_qpel16_mc02_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][ 9] = ff_put_h264_qpel16_mc12_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][10] = ff_put_h264_qpel16_mc22_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][11] = ff_put_h264_qpel16_mc32_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][12] = ff_put_h264_qpel16_mc03_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][13] = ff_put_h264_qpel16_mc13_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][14] = ff_put_h264_qpel16_mc23_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[0][15] = ff_put_h264_qpel16_mc33_neon;

        pQPelCtx->put_h264_qpel_pixels_tab[1][ 0] = ff_put_h264_qpel8_mc00_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 1] = ff_put_h264_qpel8_mc10_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 2] = ff_put_h264_qpel8_mc20_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 3] = ff_put_h264_qpel8_mc30_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 4] = ff_put_h264_qpel8_mc01_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 5] = ff_put_h264_qpel8_mc11_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 6] = ff_put_h264_qpel8_mc21_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 7] = ff_put_h264_qpel8_mc31_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 8] = ff_put_h264_qpel8_mc02_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][ 9] = ff_put_h264_qpel8_mc12_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][10] = ff_put_h264_qpel8_mc22_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][11] = ff_put_h264_qpel8_mc32_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][12] = ff_put_h264_qpel8_mc03_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][13] = ff_put_h264_qpel8_mc13_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][14] = ff_put_h264_qpel8_mc23_neon;
        pQPelCtx->put_h264_qpel_pixels_tab[1][15] = ff_put_h264_qpel8_mc33_neon;

        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 0] = ff_avg_h264_qpel16_mc00_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 1] = ff_avg_h264_qpel16_mc10_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 2] = ff_avg_h264_qpel16_mc20_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 3] = ff_avg_h264_qpel16_mc30_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 4] = ff_avg_h264_qpel16_mc01_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 5] = ff_avg_h264_qpel16_mc11_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 6] = ff_avg_h264_qpel16_mc21_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 7] = ff_avg_h264_qpel16_mc31_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 8] = ff_avg_h264_qpel16_mc02_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][ 9] = ff_avg_h264_qpel16_mc12_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][10] = ff_avg_h264_qpel16_mc22_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][11] = ff_avg_h264_qpel16_mc32_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][12] = ff_avg_h264_qpel16_mc03_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][13] = ff_avg_h264_qpel16_mc13_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][14] = ff_avg_h264_qpel16_mc23_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[0][15] = ff_avg_h264_qpel16_mc33_neon;

        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 0] = ff_avg_h264_qpel8_mc00_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 1] = ff_avg_h264_qpel8_mc10_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 2] = ff_avg_h264_qpel8_mc20_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 3] = ff_avg_h264_qpel8_mc30_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 4] = ff_avg_h264_qpel8_mc01_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 5] = ff_avg_h264_qpel8_mc11_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 6] = ff_avg_h264_qpel8_mc21_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 7] = ff_avg_h264_qpel8_mc31_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 8] = ff_avg_h264_qpel8_mc02_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][ 9] = ff_avg_h264_qpel8_mc12_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][10] = ff_avg_h264_qpel8_mc22_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][11] = ff_avg_h264_qpel8_mc32_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][12] = ff_avg_h264_qpel8_mc03_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][13] = ff_avg_h264_qpel8_mc13_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][14] = ff_avg_h264_qpel8_mc23_neon;
        pQPelCtx->avg_h264_qpel_pixels_tab[1][15] = ff_avg_h264_qpel8_mc33_neon;
#endif // _CPU_NEON
    }
}

void yy_pred16x16_vert_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_vert_neon (src, stride);
#else
	pH264Pred->pred16x16[VERT_PRED8x8   ](src, stride);
#endif // YY_NEON
}

void yy_pred16x16_hor_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_hor_neon (src, stride);
#else
	pH264Pred->pred16x16[HOR_PRED8x8    ](src, stride);
#endif // YY_NEON
}

void yy_pred16x16_plane_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_plane_neon (src, stride);
#else
	pH264Pred->pred16x16[PLANE_PRED8x8  ](src, stride);
#endif // YY_NEON
}

void yy_pred16x16_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_dc_neon (src, stride);
#else
	pH264Pred->pred16x16[DC_PRED8x8     ] (src, stride);
#endif // YY_NEON
}

void yy_pred16x16_128_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_128_dc_neon (src, stride);
#else
	pH264Pred->pred16x16[DC_128_PRED8x8 ] (src, stride);
#endif // YY_NEON
}

void yy_pred16x16_left_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_left_dc_neon (src, stride);
#else
	pH264Pred->pred16x16[LEFT_DC_PRED8x8] (src, stride);
#endif // YY_NEON
}

void yy_pred16x16_top_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred16x16_top_dc_neon (src, stride);
#else
	pH264Pred->pred16x16[TOP_DC_PRED8x8 ] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_vert_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_vert_neon (src, stride);
#else
	pH264Pred->pred8x8[VERT_PRED8x8     ] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_hor_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_hor_neon (src, stride);
#else
	pH264Pred->pred8x8[HOR_PRED8x8      ] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_plane_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_plane_neon (src, stride);
#else
	pH264Pred->pred8x8[PLANE_PRED8x8] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[DC_PRED8x8     ] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_128_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_128_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[DC_128_PRED8x8   ] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_left_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_left_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[LEFT_DC_PRED8x8] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_top_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_top_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[TOP_DC_PRED8x8 ] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_l0t_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_l0t_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[ALZHEIMER_DC_L0T_PRED8x8] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_0lt_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_0lt_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[ALZHEIMER_DC_0LT_PRED8x8] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_l00_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_l00_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[ALZHEIMER_DC_L00_PRED8x8] (src, stride);
#endif // YY_NEON
}

void yy_pred8x8_0l0_dc_neon(uint8_t *src, ptrdiff_t stride)
{
#ifdef _CPU_NEON
	ff_pred8x8_0l0_dc_neon (src, stride);
#else
	pH264Pred->pred8x8[ALZHEIMER_DC_0L0_PRED8x8] (src, stride);
#endif // YY_NEON
}

void YY_h264pred_init (void * pCtx, int codec_id, int bit_depth, const int chroma_format_idc)
{
	H264PredContext * pPredCtx = (H264PredContext *)pCtx;
	const int high_depth = bit_depth > 8;
	if (high_depth)
		return;

	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pH264predFunc == NULL)
			g_neonFunc->pH264predFunc = malloc (sizeof (H264PredContext));
		memcpy (g_neonFunc->pH264predFunc, pCtx, sizeof (H264PredContext));

		pH264Pred = (H264PredContext *)g_neonFunc->pH264predFunc;
	}

#ifdef _CPU_NEON
	if(chroma_format_idc == 1)
	{
		pPredCtx->pred8x8[VERT_PRED8x8     ] = yy_pred8x8_vert_neon;
		pPredCtx->pred8x8[HOR_PRED8x8      ] = yy_pred8x8_hor_neon;
//		if (codec_id != AV_CODEC_ID_VP8)
//			pPredCtx->pred8x8[PLANE_PRED8x8] = yy_pred8x8_plane_neon;
		pPredCtx->pred8x8[DC_128_PRED8x8   ] = yy_pred8x8_128_dc_neon;
		if (codec_id != AV_CODEC_ID_RV40 && codec_id != AV_CODEC_ID_VP8) 
		{
			pPredCtx->pred8x8[DC_PRED8x8     ] = yy_pred8x8_dc_neon;
			pPredCtx->pred8x8[LEFT_DC_PRED8x8] = yy_pred8x8_left_dc_neon;
			pPredCtx->pred8x8[TOP_DC_PRED8x8 ] = yy_pred8x8_top_dc_neon;
			pPredCtx->pred8x8[ALZHEIMER_DC_L0T_PRED8x8] = yy_pred8x8_l0t_dc_neon;
			pPredCtx->pred8x8[ALZHEIMER_DC_0LT_PRED8x8] = yy_pred8x8_0lt_dc_neon;
			pPredCtx->pred8x8[ALZHEIMER_DC_L00_PRED8x8] = yy_pred8x8_l00_dc_neon;
			pPredCtx->pred8x8[ALZHEIMER_DC_0L0_PRED8x8] = yy_pred8x8_0l0_dc_neon;
		}
	}

	pPredCtx->pred16x16[DC_PRED8x8     ] = yy_pred16x16_dc_neon;
	pPredCtx->pred16x16[VERT_PRED8x8   ] = yy_pred16x16_vert_neon;
	pPredCtx->pred16x16[HOR_PRED8x8    ] = yy_pred16x16_hor_neon;
	pPredCtx->pred16x16[LEFT_DC_PRED8x8] = yy_pred16x16_left_dc_neon;
	pPredCtx->pred16x16[TOP_DC_PRED8x8 ] = yy_pred16x16_top_dc_neon;
	pPredCtx->pred16x16[DC_128_PRED8x8 ] = yy_pred16x16_128_dc_neon;
//	if (codec_id != AV_CODEC_ID_SVQ3 && codec_id != AV_CODEC_ID_RV40 && codec_id != AV_CODEC_ID_VP8)
//		pPredCtx->pred16x16[PLANE_PRED8x8  ] = yy_pred16x16_plane_neon;
#endif // _CPU_NEON
}


/*******************************************************************************
	File:		neonH264Dec.h

	Contains:	the neon function head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-29		Fenger			Create file

*******************************************************************************/
#ifndef __neonH264Dec_H__
#define __neonH264Dec_H__

#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

extern  const uint8_t scan8[16 * 3 + 3];

#ifndef BIT_DEPTH
#define BIT_DEPTH 8
#endif

#if BIT_DEPTH > 8
#   define pixel  uint16_t
#   define pixel2 uint32_t
#   define pixel4 uint64_t
#   define dctcoef int32_t
#else
#   define pixel  uint8_t
#   define pixel2 uint16_t
#   define pixel4 uint32_t
#   define dctcoef int16_t
#endif

void yy_put_h264_chroma_mc2_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y);
void yy_put_h264_chroma_mc4_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y);
void yy_put_h264_chroma_mc8_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y);
void yy_avg_h264_chroma_mc2_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y);
void yy_avg_h264_chroma_mc4_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y);
void yy_avg_h264_chroma_mc8_func (unsigned char *dst, unsigned char *src, int srcStride, int h, int x, int y);

// define the chroma neon functions
void ff_put_h264_chroma_mc8_neon(unsigned char *, unsigned char *, int, int, int, int);
void ff_put_h264_chroma_mc4_neon(unsigned char *, unsigned char *, int, int, int, int);
void ff_put_h264_chroma_mc2_neon(unsigned char *, unsigned char *, int, int, int, int);
void ff_avg_h264_chroma_mc8_neon(unsigned char *, unsigned char *, int, int, int, int);
void ff_avg_h264_chroma_mc4_neon(unsigned char *, unsigned char *, int, int, int, int);
void ff_avg_h264_chroma_mc2_neon(unsigned char *, unsigned char *, int, int, int, int);

// define the dsp and idct neon functions
void ff_h264_v_loop_filter_luma_neon(uint8_t *pix, int stride, int alpha,
                                     int beta, int8_t *tc0);
void ff_h264_h_loop_filter_luma_neon(uint8_t *pix, int stride, int alpha,
                                     int beta, int8_t *tc0);
void ff_h264_v_loop_filter_chroma_neon(uint8_t *pix, int stride, int alpha,
                                       int beta, int8_t *tc0);
void ff_h264_h_loop_filter_chroma_neon(uint8_t *pix, int stride, int alpha,
                                       int beta, int8_t *tc0);

void ff_weight_h264_pixels_16_neon(uint8_t *dst, int stride, int height,
                                   int log2_den, int weight, int offset);
void ff_weight_h264_pixels_8_neon(uint8_t *dst, int stride, int height,
                                  int log2_den, int weight, int offset);
void ff_weight_h264_pixels_4_neon(uint8_t *dst, int stride, int height,
                                  int log2_den, int weight, int offset);

void ff_biweight_h264_pixels_16_neon(uint8_t *dst, uint8_t *src, int stride,
                                     int height, int log2_den, int weightd,
                                     int weights, int offset);
void ff_biweight_h264_pixels_8_neon(uint8_t *dst, uint8_t *src, int stride,
                                    int height, int log2_den, int weightd,
                                    int weights, int offset);
void ff_biweight_h264_pixels_4_neon(uint8_t *dst, uint8_t *src, int stride,
                                    int height, int log2_den, int weightd,
                                    int weights, int offset);

void ff_h264_idct_add_neon(uint8_t *dst, int16_t *block, int stride);
void ff_h264_idct_dc_add_neon(uint8_t *dst, int16_t *block, int stride);
void ff_h264_idct_add16_neon(uint8_t *dst, const int *block_offset,
                             int16_t *block, int stride,
                             const uint8_t nnzc[6*8]);
void ff_h264_idct_add16intra_neon(uint8_t *dst, const int *block_offset,
                                  int16_t *block, int stride,
                                  const uint8_t nnzc[6*8]);
void ff_h264_idct_add8_neon(uint8_t **dest, const int *block_offset,
                            int16_t *block, int stride,
                            const uint8_t nnzc[6*8]);

void ff_h264_idct8_add_neon(uint8_t *dst, int16_t *block, int stride);
void ff_h264_idct8_dc_add_neon(uint8_t *dst, int16_t *block, int stride);
void ff_h264_idct8_add4_neon(uint8_t *dst, const int *block_offset,
                             int16_t *block, int stride,
                             const uint8_t nnzc[6*8]);

// define pred neon functions
void ff_pred16x16_vert_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_hor_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_plane_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_128_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_left_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_top_dc_neon(uint8_t *src, ptrdiff_t stride);

void ff_pred8x8_vert_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_hor_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_plane_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_128_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_left_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_top_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_l0t_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_0lt_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_l00_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_0l0_dc_neon(uint8_t *src, ptrdiff_t stride);

// define the qpel neon functions
void ff_put_h264_qpel16_mc00_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc10_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc20_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc30_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc01_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc11_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc21_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc31_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc02_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc12_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc22_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc32_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc03_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc13_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc23_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel16_mc33_neon(uint8_t *, uint8_t *, ptrdiff_t);

void ff_put_h264_qpel8_mc00_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc10_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc20_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc30_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc01_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc11_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc21_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc31_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc02_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc12_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc22_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc32_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc03_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc13_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc23_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_put_h264_qpel8_mc33_neon(uint8_t *, uint8_t *, ptrdiff_t);

void ff_avg_h264_qpel16_mc00_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc10_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc20_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc30_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc01_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc11_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc21_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc31_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc02_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc12_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc22_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc32_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc03_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc13_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc23_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel16_mc33_neon(uint8_t *, uint8_t *, ptrdiff_t);

void ff_avg_h264_qpel8_mc00_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc10_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc20_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc30_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc01_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc11_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc21_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc31_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc02_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc12_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc22_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc32_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc03_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc13_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc23_neon(uint8_t *, uint8_t *, ptrdiff_t);
void ff_avg_h264_qpel8_mc33_neon(uint8_t *, uint8_t *, ptrdiff_t);

// the define the pred neon functions
void ff_pred16x16_vert_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_hor_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_plane_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_128_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_left_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred16x16_top_dc_neon(uint8_t *src, ptrdiff_t stride);

void ff_pred8x8_vert_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_hor_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_plane_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_128_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_left_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_top_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_l0t_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_0lt_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_l00_dc_neon(uint8_t *src, ptrdiff_t stride);
void ff_pred8x8_0l0_dc_neon(uint8_t *src, ptrdiff_t stride);

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __neonH264Dec_H__

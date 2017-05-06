/*******************************************************************************
	File:		neonDspUtil.h

	Contains:	the neon function head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-29		Fenger			Create file

*******************************************************************************/
#ifndef __neonDspUtil_H__
#define __neonDspUtil_H__

#include "stdint.h"
#include "stddef.h"

#include "config.h"
#include "libavcodec/mpegvideo.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

void ff_simple_idct_neon(int16_t *data);
void ff_simple_idct_put_neon(uint8_t *dest, int line_size, int16_t *data);
void ff_simple_idct_add_neon(uint8_t *dest, int line_size, int16_t *data);

void ff_clear_block_neon(int16_t *block);
void ff_clear_blocks_neon(int16_t *blocks);

void ff_add_pixels_clamped_neon(const int16_t *, uint8_t *, int);
void ff_put_pixels_clamped_neon(const int16_t *, uint8_t *, int);
void ff_put_signed_pixels_clamped_neon(const int16_t *, uint8_t *, int);

void ff_vector_clipf_neon(float *dst, const float *src, float min, float max,
                          int len);
void ff_vector_clip_int32_neon(int32_t *dst, const int32_t *src, int32_t min,
                               int32_t max, unsigned int len);

int32_t ff_scalarproduct_int16_neon(const int16_t *v1, const int16_t *v2, int len);
int32_t ff_scalarproduct_and_madd_int16_neon(int16_t *v1, const int16_t *v2,
                                             const int16_t *v3, int len, int mul);

void ff_apply_window_int16_neon(int16_t *dst, const int16_t *src,
                                const int16_t *window, unsigned n);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __neonDspUtil_H__

/*******************************************************************************
	File:		neonRV34Dec.h

	Contains:	the neon function head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-29		Fenger			Create file

*******************************************************************************/
#ifndef __neonRV34Dec_H__
#define __neonRV34Dec_H__

#include "stdint.h"
#include "stddef.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

// define RV40 decode neon functions
#define DECL_QPEL3(type, w, pos) \
    void ff_##type##_rv40_qpel##w##_mc##pos##_neon(uint8_t *dst, uint8_t *src,\
                                                   ptrdiff_t stride)
#define DECL_QPEL2(w, pos)                      \
    DECL_QPEL3(put, w, pos);                    \
    DECL_QPEL3(avg, w, pos)

#define DECL_QPEL_XY(x, y)                      \
    DECL_QPEL2(16, x ## y);                     \
    DECL_QPEL2(8,  x ## y)

#define DECL_QPEL_Y(y)                          \
    DECL_QPEL_XY(0, y);                         \
    DECL_QPEL_XY(1, y);                         \
    DECL_QPEL_XY(2, y);                         \
    DECL_QPEL_XY(3, y);                         \

DECL_QPEL_Y(0);
DECL_QPEL_Y(1);
DECL_QPEL_Y(2);
DECL_QPEL_Y(3);

void ff_put_rv40_chroma_mc8_neon(uint8_t *, uint8_t *, int, int, int, int);
void ff_put_rv40_chroma_mc4_neon(uint8_t *, uint8_t *, int, int, int, int);

void ff_avg_rv40_chroma_mc8_neon(uint8_t *, uint8_t *, int, int, int, int);
void ff_avg_rv40_chroma_mc4_neon(uint8_t *, uint8_t *, int, int, int, int);

void ff_rv40_weight_func_16_neon(uint8_t *, uint8_t *, uint8_t *, int, int, ptrdiff_t);
void ff_rv40_weight_func_8_neon(uint8_t *, uint8_t *, uint8_t *, int, int, ptrdiff_t);

int ff_rv40_h_loop_filter_strength_neon(uint8_t *src, ptrdiff_t stride,
                                        int beta, int beta2, int edge,
                                        int *p1, int *q1);
int ff_rv40_v_loop_filter_strength_neon(uint8_t *src, ptrdiff_t stride,
                                        int beta, int beta2, int edge,
                                        int *p1, int *q1);

void ff_rv40_h_weak_loop_filter_neon(uint8_t *src, ptrdiff_t stride, int filter_p1,
                                     int filter_q1, int alpha, int beta,
                                     int lim_p0q0, int lim_q1, int lim_p1);
void ff_rv40_v_weak_loop_filter_neon(uint8_t *src, ptrdiff_t stride, int filter_p1,
                                     int filter_q1, int alpha, int beta,
                                     int lim_p0q0, int lim_q1, int lim_p1);

void ff_put_pixels16_xy2_neon(uint8_t *, const uint8_t *, ptrdiff_t, int);
void ff_avg_pixels16_xy2_neon(uint8_t *, const uint8_t *, ptrdiff_t, int);
void ff_put_pixels8_xy2_neon(uint8_t *, const uint8_t *, ptrdiff_t, int);
void ff_avg_pixels8_xy2_neon(uint8_t *, const uint8_t *, ptrdiff_t, int);

// define the rv34 decode neon functions
void ff_rv34_inv_transform_noround_neon(int16_t *block);
void ff_rv34_inv_transform_noround_dc_neon(int16_t *block);
void ff_rv34_idct_add_neon(uint8_t *dst, ptrdiff_t stride, int16_t *block);
void ff_rv34_idct_dc_add_neon(uint8_t *dst, ptrdiff_t stride, int dc);


#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __neonRV34Dec_H__

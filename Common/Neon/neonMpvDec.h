/*******************************************************************************
	File:		neonMpvDec.h

	Contains:	the neon function head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-29		Fenger			Create file

*******************************************************************************/
#ifndef __neonMpvDec_H__
#define __neonMpvDec_H__

#include "stdint.h"
#include "stddef.h"

#include "libavcodec/mpegvideo.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 


#if 0
CHK_OFFS(MpegEncContext, y_dc_scale,       Y_DC_SCALE);
CHK_OFFS(MpegEncContext, c_dc_scale,       C_DC_SCALE);
CHK_OFFS(MpegEncContext, ac_pred,          AC_PRED);
CHK_OFFS(MpegEncContext, block_last_index, BLOCK_LAST_INDEX);
CHK_OFFS(MpegEncContext, inter_scantable.raster_end, INTER_SCANTAB_RASTER_END);
CHK_OFFS(MpegEncContext, h263_aic,         H263_AIC);
#endif // 

void ff_dct_unquantize_h263_inter_neon(MpegEncContext *s, int16_t *block,
                                       int n, int qscale);
void ff_dct_unquantize_h263_intra_neon(MpegEncContext *s, int16_t *block,
                                       int n, int qscale);
#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __neonMpvDec_H__

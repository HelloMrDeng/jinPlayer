/*******************************************************************************
	File:		yyneonFunc.h

	Contains:	the neon function head file

	Written by:	Fenger King

	Change History (most recent first):
	2013-04-29		Fenger			Create file

*******************************************************************************/
#ifndef __yyneonFunc_H__
#define __yyneonFunc_H__

#include "libavcodec/yyExtNeon.h"

#define	YY_MEDIA_OPEN_PLAY		0
#define	YY_MEDIA_OPEN_THUMB		1

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus 

#if 0
// float dsp neon functions.
void	YY_float_dsp_init (void * pCtx, int strict);
typedef void (* yy_float_dsp_init)(void * pCtx, int strict);

void	YY_format_convert_init (void * pCtx, void * pCtx2);
typedef void (* yy_format_convert_init) (void * pCtx, void * pCtx2);

void	YY_dsp_util_init (void * pCtx, void * pCtx2);
typedef void (* yy_dsp_util_init) (void * pCtx, void * pCtx2);

void	YY_h264chroma_init (void * pCtx, int bit_depth);
typedef void (*	yy_h264chroma_init) (void * pCtx, int bit_depth);

void	YY_h264dsp_init (void * pCtx, const int bit_depth, const int chroma_format_idc);
typedef void (*	yy_h264dsp_init) (void * pCtx, const int bit_depth, const int chroma_format_idc);

void	YY_h264qpel_init (void * pCtx, int bit_depth);
typedef void (*	yy_h264qpel_init) (void * pCtx, int bit_depth);

void	YY_h264pred_init (void * pCtx, int codec_id, int bit_depth, const int chroma_format_idc);
typedef void (*	yy_h264pred_init) (void * pCtx, int codec_id, int bit_depth, const int chroma_format_idc);

void	YY_rv34_dsp_init (void * pCtx);
typedef void (*	yy_rv34_dsp_init) (void * pCtx);

void	YY_rv40_dsp_init (void * pCtx);
typedef void (*	yy_rv40_dsp_init) (void * pCtx);

void	YY_AC3_Dsp_init (void * pCtx, int flag);
typedef void (*	yy_AC3_Dsp_init) (void * pCtx, int flag);

void	YY_mpa_dsp_init (void * pCtx);
typedef void (*	yy_mpa_dsp_init) (void * pCtx);

void	YY_vorbis_dsp_init (void * pCtx);
typedef void (*	yy_vorbis_dsp_init) (void * pCtx);

void	YY_mpv_common_init (void * pCtx);
typedef void (*	yy_mpv_common_init) (void * pCtx);

typedef void * (* YY_MEM_ALLOC) (size_t nSize);
typedef int    (* YY_MMEM_FREE) (void * ptr);

typedef struct {
	void *					pAVContext;
	void *					pVideoDecCtx;
	void *					pAudioDecCtx;

	yy_float_dsp_init 		pFloatDspInit;
	void *					pFloatDspFunc;

	yy_format_convert_init	pFormatCvtInit;
	void *					pFormatCvtFunc;

	yy_dsp_util_init		pDspUtilInit;
	void *					pDspUtilFunc;

	yy_h264chroma_init		pH264ChromaInit;
	void *					pH264ChromaFunc;

	yy_h264dsp_init			pH264DspInit;
	void *					pH264DspFunc;

	yy_h264qpel_init		pH264QpelInit;
	void *					pH264QpelFunc;

	yy_h264pred_init		pH264predInit;
	void *					pH264predFunc;

	yy_rv34_dsp_init		pRV34DspInit;
	void *					pRV34DspFunc;

	yy_rv40_dsp_init		pRV40DspInit;
	void *					pRV40DspFunc;

	yy_AC3_Dsp_init			pAC3DspInit;
	void *					pAC3DspFunc;

	yy_mpa_dsp_init			pMpaDspInit;
	void *					pMpaDspFunc;

	yy_vorbis_dsp_init		pVorbisDspInit;
	void *					pVorbisDspFunc;

	yy_mpv_common_init		pMPVCmnInit;
	void *					pMPVCmnFunc;

	YY_MEM_ALLOC			pMemAlloc;
	YY_MMEM_FREE			pMemFree;

} yyNeonFunc;

typedef struct {
	int		nOpenFlag;
	int		nSourceType;
} yyAVContext;
#endif // new

int		InitNeonFunc (yyNeonFunc * pFunc);
int		UninitNeonFunc (yyNeonFunc * pFunc);

extern  yyNeonFunc *	g_neonFunc;
extern  yyAVContext *	g_yyavCtx;

#ifdef __cplusplus
}
#endif // __cplusplus 

#endif // __yyneonFunc_H__

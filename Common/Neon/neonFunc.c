/*******************************************************************************
	File:		neonFunc.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"

#include "yyneonFunc.h"

yyNeonFunc *	g_neonFunc = NULL;
yyAVContext *	g_yyavCtx = NULL;

int InitNeonFunc (yyNeonFunc * pFunc)
{
	memset (pFunc, 0, sizeof (yyNeonFunc));

#ifdef _CPU_NEON
//	pFunc->pFloatDspInit	= YY_float_dsp_init;
	pFunc->pH264ChromaInit	= YY_h264chroma_init;
	pFunc->pH264DspInit		= YY_h264dsp_init;
	pFunc->pH264QpelInit	= YY_h264qpel_init;
	pFunc->pH264predInit	= YY_h264pred_init;
	pFunc->pRV34DspInit		= YY_rv34_dsp_init;
	pFunc->pRV40DspInit		= YY_rv40_dsp_init;
//	pFunc->pMPVCmnInit		= YY_mpv_common_init;
	pFunc->pDspUtilInit		= YY_dsp_util_init;

//	pFunc->pFormatCvtInit	= YY_format_convert_init;
#endif // _CPU_NEON

	g_neonFunc = pFunc;

	return 0;
}

int UninitNeonFunc (yyNeonFunc * pFunc)
{
	if (pFunc->pFloatDspFunc != NULL)
		free (pFunc->pFloatDspFunc);

	if (pFunc->pFormatCvtFunc != NULL)
		free (pFunc->pFormatCvtFunc);

	if (pFunc->pDspUtilFunc != NULL)
		free (pFunc->pDspUtilFunc);

	if (pFunc->pH264ChromaFunc != NULL)
		free (pFunc->pH264ChromaFunc);

	if (pFunc->pH264DspFunc != NULL)
		free (pFunc->pH264DspFunc);

	if (pFunc->pH264QpelFunc != NULL)
		free (pFunc->pH264QpelFunc);

	if (pFunc->pH264predFunc != NULL)
		free (pFunc->pH264predFunc);

	if (pFunc->pRV34DspFunc != NULL)
		free (pFunc->pRV34DspFunc);

	if (pFunc->pRV40DspFunc != NULL)
		free (pFunc->pRV40DspFunc);

	if (pFunc->pAC3DspFunc != NULL)
		free (pFunc->pAC3DspFunc);

	if (pFunc->pMpaDspFunc != NULL)
		free (pFunc->pMpaDspFunc);

	if (pFunc->pVorbisDspFunc != NULL)
		free (pFunc->pVorbisDspFunc);

	if (pFunc->pMPVCmnFunc != NULL)
		free (pFunc->pMPVCmnFunc);

	memset (pFunc, 0, sizeof (yyNeonFunc));

	return 0;
}

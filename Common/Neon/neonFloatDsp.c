/*******************************************************************************
	File:		neonFunc.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#include "stddef.h"

#include "yyneonFunc.h"

#include "libavutil\float_dsp.h"

void ff_vector_fmul_neon(float *dst, const float *src0, const float *src1, int len);
void ff_vector_fmac_scalar_neon(float *dst, const float *src, float mul, int len);
void ff_vector_fmul_scalar_neon(float *dst, const float *src, float mul, int len);
void ff_vector_fmul_window_neon(float *dst, const float *src0, const float *src1, const float *win, int len);
void ff_vector_fmul_add_neon(float *dst, const float *src0, const float *src1, const float *src2, int len);
void ff_vector_fmul_reverse_neon(float *dst, const float *src0, const float *src1, int len);
void ff_butterflies_float_neon(float *v1, float *v2, int len);
float ff_scalarproduct_float_neon(const float *v1, const float *v2, int len);

#ifdef _CPU_NEON
static void vector_fmul_c(float *dst, const float *src0, const float *src1,
                          int len)
{
#ifdef WINCE
	ff_vector_fmul_neon (dst, src0, src1, len);
#else
    int i;
    for (i = 0; i < len; i++)
        dst[i] = src0[i] * src1[i];
#endif //WINCE
}

static void vector_fmac_scalar_c(float *dst, const float *src, float mul,
                                 int len)
{
#ifdef WINCE
	ff_vector_fmac_scalar_neon (dst, src, mul, len);
#else
    int i;
    for (i = 0; i < len; i++)
        dst[i] += src[i] * mul;
#endif // WINCE
}

static void vector_fmul_scalar_c(float *dst, const float *src, float mul,
                                 int len)
{
#ifdef WINCE
	ff_vector_fmul_scalar_neon (dst, src, mul, len);
#else
   int i;
    for (i = 0; i < len; i++)
        dst[i] = src[i] * mul;
#endif // WINCE
}

static void vector_dmul_scalar_c(double *dst, const double *src, double mul,
                                 int len)
{
#ifdef WINCE
#else
#endif // WINCE
    int i;
    for (i = 0; i < len; i++)
        dst[i] = src[i] * mul;
}

static void vector_fmul_window_c(float *dst, const float *src0,
                                 const float *src1, const float *win, int len)
{
#ifdef WINCE
	ff_vector_fmul_window_neon (dst, src0, src1, win, len);
#else
   int i, j;

    dst  += len;
    win  += len;
    src0 += len;

    for (i = -len, j = len - 1; i < 0; i++, j--) {
        float s0 = src0[i];
        float s1 = src1[j];
        float wi = win[i];
        float wj = win[j];
        dst[i] = s0 * wj - s1 * wi;
        dst[j] = s0 * wi + s1 * wj;
    }
#endif // WINCE
}

static void vector_fmul_add_c(float *dst, const float *src0, const float *src1,
                              const float *src2, int len)
{
#ifdef WINCE
	ff_vector_fmul_add_neon (dst, src0, src1, src2, len);
#else
    int i;

    for (i = 0; i < len; i++)
        dst[i] = src0[i] * src1[i] + src2[i];
#endif // WINCE
}

static void vector_fmul_reverse_c(float *dst, const float *src0,
                                  const float *src1, int len)
{
#ifdef WINCE
	ff_vector_fmul_reverse_neon (dst, src0, src1, len);
#else
    int i;

    src1 += len-1;
    for (i = 0; i < len; i++)
        dst[i] = src0[i] * src1[-i];
#endif // WINCE
}

static void butterflies_float_c(float *av_restrict v1, float *av_restrict v2,
                                int len)
{
#ifdef WINCE
	ff_butterflies_float_neon (v1, v2, len);
#else
    int i;

    for (i = 0; i < len; i++) {
        float t = v1[i] - v2[i];
        v1[i] += v2[i];
        v2[i] = t;
    }
#endif // WINCE
}

float avpriv_scalarproduct_float_c(const float *v1, const float *v2, int len)
{
#ifdef WINCE
	return ff_scalarproduct_float_neon (v1, v2, len);
#else
    float p = 0.0;
    int i;

    for (i = 0; i < len; i++)
        p += v1[i] * v2[i];

    return p;
#endif // WINCE
}
#endif // _CPU_NEON

void YY_float_dsp_init (void * pCtx, int strict)
{
	AVFloatDSPContext * fdsp = (AVFloatDSPContext *)pCtx;
	if (g_neonFunc != NULL)
	{
		if (g_neonFunc->pFloatDspFunc == NULL)
			g_neonFunc->pFloatDspFunc = malloc (sizeof (AVFloatDSPContext));
		memcpy (g_neonFunc->pFloatDspFunc, pCtx, sizeof (AVFloatDSPContext));
	}

#ifdef _CPU_NEON
    fdsp->vector_fmul = vector_fmul_c;
    fdsp->vector_fmac_scalar = vector_fmac_scalar_c;
    fdsp->vector_fmul_scalar = vector_fmul_scalar_c;
    fdsp->vector_dmul_scalar = vector_dmul_scalar_c;
    fdsp->vector_fmul_window = vector_fmul_window_c;
    fdsp->vector_fmul_add = vector_fmul_add_c;
    fdsp->vector_fmul_reverse = vector_fmul_reverse_c;
    fdsp->butterflies_float = butterflies_float_c;
    fdsp->scalarproduct_float = avpriv_scalarproduct_float_c;
#endif // _CPU_NEON
}


/***************************************************************************
 *                                                                         *
 *                   SiRF Technology, Inc. Platform Software               *
 *                                                                         *
 *    Copyright (c) 2008 by SiRF Technology, Inc.  All rights reserved.    *
 *                                                                         *
 *    This Software is protected by United States copyright laws and       *
 *    international treaties.  You may not reverse engineer, decompile     *
 *    or disassemble this Software.                                        *
 *                                                                         *
 *    WARNING:                                                             *
 *    This Software contains SiRF Technology, Inc.'s confidential and      *
 *    proprietary information. UNAUTHORIZED COPYING, USE, DISTRIBUTION,    *
 *    PUBLICATION, TRANSFER, SALE, RENTAL OR DISCLOSURE IS PROHIBITED      *
 *    AND MAY RESULT IN SERIOUS LEGAL CONSEQUENCES.  Do not copy this      *
 *    Software without SiRF Technology, Inc.'s  express written            *
 *    permission.   Use of any portion of the contents of this Software    *
 *    is subject to and restricted by your written agreement with          *
 *    SiRF Technology, Inc.                                                *
 *                                                                         *
 ***************************************************************************/

#define YUV2R(Y, U, V) (clp[384+((tab_76309[Y] + crv_tab[V])>>16)])
#define YUV2G(Y, U, V) (clp[384+((tab_76309[Y] - cgu_tab[U] - cgv_tab[V])>>16)])
#define YUV2B(Y, U, V) (clp[384+((tab_76309[Y] + cbu_tab[U])>>16)])

#define YUV2RGB565(Y, U, V) (unsigned short)(((YUV2R(Y,U,V)>>3)<<11) | ((YUV2G(Y,U,V)>>2)<<5) | ((YUV2B(Y,U,V)>>3)<<0))

//comes from mpg_plus
static long crv_tab[256];
static long cbu_tab[256];
static long cgu_tab[256];
static long cgv_tab[256];
static long tab_76309[256];
static unsigned char clp[1024];



void InitConvtTbl(void)
{
	long int crv,cbu,cgu,cgv;
	int i,ind;

	crv = 104597;  //  1.596 << 16 
	cbu = 132201;  //  2.017 << 16
	cgu = 25675;   //  0.392 << 16
	cgv = 53279;   //  0.813 << 16

	for (i = 0; i < 256; i++) 
	{
		crv_tab[i] = (i-128) * crv;
		cbu_tab[i] = (i-128) * cbu;
		cgu_tab[i] = (i-128) * cgu;
		cgv_tab[i] = (i-128) * cgv;
		tab_76309[i] = 76309*(i-16);  //  1.164 << 16
	}

	for (i=0; i<384; i++)
		clp[i] =0;

	ind=384;
	for (i=0;i<256; i++)
		clp[ind++]=i;

	ind=640;
	for (i=0;i<384;i++)
		clp[ind++]=255;
}

void FrameNV12Rgb565(	unsigned char *pY, unsigned char *pC,
						int stepY,
						unsigned char *pRGB, 
						int stepRGB,
						int width, int height)
{
	long y0, y1, y2, y3, u, v;
	unsigned char *pY0 = pY, *pY1 = pY0 + stepY;
	unsigned short *pRGB1, *pRGB0;

	if (height > 0)
	{
		pRGB0 = (unsigned short *)(pRGB + stepRGB * (height - 1));
		stepRGB = -stepRGB;
	}
	else
	{
		height = -height;
		pRGB0 = (unsigned short *)(pRGB);
	}
	pRGB1 = pRGB0 + (stepRGB >> 1);

	for (int j = 0; j < (height >> 1); j++)
	{
		for(int i = 0; i < (width >> 1); i++)
		{
			u  = pC[2*i];
			v  = pC[2*i+1];
			y0 = pY0[2*i];
			y1 = pY0[2*i+1];
			y2 = pY1[2*i];
			y3 = pY1[2*i+1];

			pRGB0[2*i]   = YUV2RGB565(y0, u, v);
			pRGB0[2*i+1] = YUV2RGB565(y1, u, v);
			pRGB1[2*i]   = YUV2RGB565(y2, u, v);
			pRGB1[2*i+1] = YUV2RGB565(y3, u, v);
		}

		pY0 += (stepY << 1);
		pY1 += (stepY << 1);
		pC += stepY;
		pRGB0 += stepRGB;
		pRGB1 += stepRGB;
	}
}

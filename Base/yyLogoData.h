/*******************************************************************************
	File:		yyLogoData.h

	Contains:	The logo data header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-10-08		Fenger			Create file

*******************************************************************************/
#ifndef __yyLogoData_H__
#define __yyLogoData_H__

#define	YYLOGO_WIDTH	108
#define	YYLOGO_HEIGHT	32

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern const unsigned char	* yyLogoBuffY;
extern const unsigned char	* yyLogoBuffU;
extern const unsigned char	* yyLogoBuffV;

extern const unsigned char	* yyLogoBuffRGB24;
extern const unsigned char	* yyLogoBuffRGBA;
extern const unsigned char	* yyLogoBuffARGB;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__yyLogoData_H__

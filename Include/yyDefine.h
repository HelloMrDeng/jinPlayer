/*******************************************************************************
	File:		yyDefine.h

	Contains:	yy player type define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-12		Fenger			Create file

*******************************************************************************/
#ifndef __yyDefine_H__
#define __yyDefine_H__

#define YYMAX(a,b) ((a) > (b) ? (a) : (b))
#define YYMAX3(a,b,c) YYMAX(YYMAX(a,b),c)
#define YYMIN(a,b) ((a) > (b) ? (b) : (a))
#define YYMIN3(a,b,c) YYMIN(YYMIN(a,b),c)

#define YYMKTAG(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))
#define YYMKBETAG(a,b,c,d) ((d) | ((c) << 8) | ((b) << 16) | ((unsigned)(a) << 24))

#endif // __yyDefine_H__

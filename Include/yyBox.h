/*******************************************************************************
	File:		yyBox.h

	Contains:	yy player type define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-03-23		Fenger			Create file

*******************************************************************************/
#ifndef __yyBox_H__
#define __yyBox_H__

#include "yyType.h"

#define YY_BOX_BASE					0X01400000

// Get the box interface.
// The parameter should be int.
// Return the box pointer.
#define	YYPLAY_PID_BOX				YY_BOX_BASE + 1

#endif // __yyBox_H__

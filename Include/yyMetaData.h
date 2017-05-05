/*******************************************************************************
	File:		yyMetaData.h

	Contains:	yy subtitle info define header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-08-03		Fenger			Create file

*******************************************************************************/
#ifndef __yyMetaData_h__
#define __yyMetaData_h__

#include "yyType.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define YY_PLAY_BASE_META		0X01200001

// The meta data value with key
struct YYMETA_Value {
	char			szKey[64];
	TCHAR			szValue[256];
	int				nSize;
	unsigned char *	pValue;
};

/*
 It supports the following key name:
 album        
 album_artist        
 artist       
 comment      
 composer     
 copyright    
 creation_time
 date         
 disc         
 encoder      
 encoded_by   
 filename     
 genre        
 language                      
 performer                
 publisher    
 service_name 
 title 
 track
*/

#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif // __yyMetaData_h__

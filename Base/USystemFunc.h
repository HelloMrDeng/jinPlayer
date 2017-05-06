/*******************************************************************************
	File:		USystemFunc.h

	Contains:	The base utility for system header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-09-02		Fenger			Create file

*******************************************************************************/
#ifndef __USystemFunc_H__
#define __USystemFunc_H__

#include "yyType.h"

typedef enum  {  
  YYMonitro_PowerOn		= -1,		//Turn on 
  YYMonitor_GoLowPower	= 1,		//LOw power  
  YYMonitor_PowerOff	= 2,		//Turn off  
} MonitorPowerCmd;

int		yyGetSysTime (void);
void	yySleep (int nTime);
int		yyGetThreadTime (void * hThread);
int		yyGetCPUNum (void);

int		yyGetAppPath (void * hInst, TCHAR * pPath, int nSize);
int		yyGetDataPath (void * hInst, TCHAR * pPath, int nSize);
bool	yyDeleteFolder (TCHAR * pFolder);

extern	TCHAR g_szWorkPath[1024];

#endif // __USystemFunc_H__

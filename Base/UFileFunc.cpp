/*******************************************************************************
	File:		UFileFunc.cpp

	Contains:	The utility for file operation implement file.

	Written by:	Fenger King

	Change History (most recent first):
	2014-04-17		Fenger			Create file

*******************************************************************************/
#include "stdlib.h"
#ifdef _OS_LINUX
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif // _OS_LINUX

#include "UFileFunc.h"
#include "UStringFunc.h"

#ifdef _OS_WIN32
#pragma warning (disable : 4996)
#endif // _OS_WIN32

yyFile yyFileOpen (TCHAR * pFile, int nFlag)
{
	yyFile	hFile = NULL;
#ifdef _OS_WIN32
	if (nFlag & YYFILE_READ)
		hFile = CreateFile(pFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
	else if (nFlag & YYFILE_WRITE) 
		hFile = CreateFile(pFile, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, (DWORD) 0, NULL);
	else
		hFile = CreateFile(pFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = NULL;
#elif defined _OS_LINUX
	int nOpenFlag = O_RDONLY;
	int nMode = 0640;
	if (nFlag & YYFILE_READ)
		nOpenFlag = O_RDONLY;
	else if (nFlag & YYFILE_WRITE) 
		nOpenFlag = O_RDWR | O_CREAT;
	else
		nOpenFlag = O_RDWR | O_CREAT;
	hFile = open (pFile, nOpenFlag, nMode);
#endif// _OS_WIN32
	return hFile;
}

int yyFileRead (yyFile hFile, unsigned char * pBuff, int nSize)
{
	int nRead = 0;
#ifdef _OS_WIN32
	ReadFile (hFile, pBuff, nSize, (DWORD *)&nRead, NULL);
#elif defined _OS_LINUX
	nRead = read (hFile, pBuff, nSize);
#endif// _OS_WIN32
	return nRead;
}

int yyFileWrite (yyFile hFile, unsigned char * pBuff, int nSize)
{
	int nWrite = 0;
#ifdef _OS_WIN32
	WriteFile (hFile, pBuff, nSize, (DWORD *)&nWrite, NULL);
#elif defined _OS_LINUX
	nWrite = write (hFile, pBuff, nSize);
#endif// _OS_WIN32
	return nWrite;
}

long long yyFileSeek (yyFile hFile, long long llPos, int nFlag)
{
	long long llSeek = 0;
#ifdef _OS_WIN32
	long	lPos = (long)llPos;
	long	lHigh = (long)(llPos >> 32);
	int		sMove = FILE_BEGIN;
	if (nFlag == YYFILE_BEGIN)
		sMove = FILE_BEGIN;
	else if (nFlag == YYFILE_CUR)
		sMove = FILE_CURRENT;
	else if (nFlag == YYFILE_END)
		sMove = FILE_END;
	DWORD dwRC = SetFilePointer (hFile, lPos, &lHigh, sMove);
	if(dwRC == INVALID_SET_FILE_POINTER)
		llSeek = -1;
#elif defined _OS_LINUX
	int sMove = SEEK_SET;
	if (nFlag == YYFILE_BEGIN)
		sMove = SEEK_SET;
	else if (nFlag == YYFILE_CUR)
		sMove = SEEK_CUR;
	else if (nFlag == YYFILE_END)
		sMove = SEEK_END;
	llSeek = lseek64 (hFile, llPos, sMove);
#endif// _OS_WIN32
	return llSeek;
}

long long yyFileSize (yyFile hFile)
{
	long long llSize = 0;
#ifdef _OS_WIN32
	DWORD dwHigh = 0;
	DWORD dwSize = GetFileSize (hFile, &dwHigh);
	llSize = dwHigh;
	llSize = llSize << 32;
	llSize += dwSize;
#elif defined _OS_LINUX
	struct stat st;
	memset(&st, 0, sizeof(struct stat));		
    fstat(hFile, &st); 
	llSize = st.st_size;
#endif// _OS_WIN32
	return llSize;
}

int yyFileClose (yyFile hFile)
{
	if (hFile == NULL)
		return -1;

#ifdef _OS_WIN32
	CloseHandle (hFile);
#elif defined _OS_LINUX
	close (hFile);
#endif// _OS_WIN32
	return 0;
}


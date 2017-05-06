/*******************************************************************************
	File:		CBaseFile.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#ifdef _OS_LINUX
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif // _OS_LINUX

#include "CBaseFile.h"
#include "CBaseKey.H"

#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "yyLog.h"

bool	  CBaseFile::g_bFileError = false;
long long CBaseFile::g_llFileSize = 0;
long long CBaseFile::g_llFileSeek = 0;
char	  CBaseFile::g_szKey[32] = {""};

CBaseFile::CBaseFile(void)
	: CBaseIO ()
	, m_hFile (NULL)
	, m_nFD (-1)	
	, m_bKeyFile (false)
	, m_nKeyLen (0)
	, m_llFileSize (0)
	, m_llFilePos (0)
	, m_pBuffer (NULL)
	, m_nBuffSize (0)
	, m_nReadTimes (0)
	, m_nBitrate (0)
	, m_nStartTime (0)
{
	SetObjectName ("CBaseFile");
}

CBaseFile::~CBaseFile(void)
{
	close (NULL);

	YY_DEL_A (m_pBuffer);
}

int CBaseFile::open (URLContext *h, const TCHAR * filename, int flags)
{
//	YYLOGI ("Open File %s", filename);
	
#ifdef _OS_WIN32
	if (flags & AVIO_FLAG_READ)
		m_hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);
	else if (flags & AVIO_FLAG_WRITE) 
		m_hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, (DWORD) 0, NULL);
	else
		m_hFile = CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, (DWORD) 0, NULL);

	if (m_hFile == INVALID_HANDLE_VALUE)
	{
		m_hFile = NULL;
		//YYLOGE ("Open file %s failed!", filename);
		return -1;
	}
	if (flags & AVIO_FLAG_READ)
	{
		DWORD dwHigh = 0;
		DWORD dwSize = GetFileSize (m_hFile, &dwHigh);
		m_llFileSize = dwHigh;
		m_llFileSize = m_llFileSize << 32;
		m_llFileSize += dwSize;
	}
#elif defined _OS_LINUX	
	int nFlag = O_RDONLY;

	if (flags & AVIO_FLAG_READ)
		nFlag = O_RDONLY;
	else if (flags & AVIO_FLAG_WRITE) 
		nFlag = O_RDWR | O_CREAT;
	else
		nFlag = O_RDWR | O_CREAT;

	int nMode = 0640;
#ifdef _OS_LINUX_X86
	m_nFD = open64 (filename, nFlag, nMode);
#else
	m_nFD = ::open (filename, nFlag, nMode);
#endif // _OS_LINUX_X86

	if (m_nFD > 0 && (flags & AVIO_FLAG_READ))
	{
		struct stat st;
		
		memset(&st, 0, sizeof(struct stat));		
	    fstat(m_nFD, &st); 

		m_llFileSize = st.st_size;
	}
	else
	{
		if (flags & AVIO_FLAG_READ)
			m_hFile = fopen (filename, "rb");
		else if (flags & AVIO_FLAG_WRITE) 
			m_hFile = fopen (filename, "wb");
		else
			m_hFile = fopen (filename, "a+b");
			
		if (m_hFile != NULL && (flags & AVIO_FLAG_READ))
		{
			fseeko (m_hFile, 0LL, SEEK_END);
			m_llFileSize = ftello (m_hFile);
			fseeko (m_hFile, 0, SEEK_SET);		
		}
	}

	if (m_hFile == NULL && m_nFD <= 0)
	{
		if (h != NULL)
			YYLOGE ("Open file %s failed!", filename);
		return -1;
	}
#endif // _OS_WIN32

	m_bKeyFile = false;
	m_nKeyLen = 0;
	if (CBaseKey::g_Key != NULL)
	{
		unsigned char szKeyData[64];
		read (NULL, szKeyData, sizeof (szKeyData));
		m_bKeyFile = CBaseKey::g_Key->IsKeyFile (szKeyData, sizeof (szKeyData));
		if (m_bKeyFile)
			m_nKeyLen = YYKEY_LEN;
		seek (NULL, m_nKeyLen, SEEK_SET);
		m_llFileSize = m_llFileSize - m_nKeyLen;
	}

	m_nReadTimes = 0;
	m_llFilePos = 0;
	g_bFileError = false;
	g_llFileSize = m_llFileSize;

//	m_nBitrate = 58982;
//	if (m_nBitrate > 0)
//		m_nStartTime = yyGetSysTime ();
	
	return 0;
}

int CBaseFile::read (URLContext *h, unsigned char *buf, int size)
{	
	if (m_nFD <= 0 && m_hFile == NULL)
		return -1;

	if (g_llFileSeek > 0)
	{
		seek (h, g_llFileSeek, SEEK_SET);
		g_llFileSeek = 0;
	}

	m_nReadTimes++;
	int nRead = -1;
#ifdef _OS_WIN32
	DWORD	dwRead = 0;
	ReadFile (m_hFile, buf, size, &dwRead, NULL);
	if (dwRead == 0)
	{
		if (m_llFilePos < m_llFileSize)
			g_bFileError = true;
		return -1;
	}
	m_llFilePos += dwRead;
	nRead = (int)dwRead;
//	YYLOGI ("Read Time: % 8d ,  size % 8d", m_nReadTimes, dwRead);
#elif defined _OS_LINUX
	if (m_nFD > 0)
		nRead = ::read (m_nFD, buf, size);
	else	
		nRead = fread (buf, 1, size, m_hFile);
	if(nRead == -1)
	{
		YYLOGE ("It was error when Read file!");
		if (m_llFilePos < m_llFileSize)
			g_bFileError = true;
		return -1;
	}

	m_llFilePos += nRead;	
	
	if (nRead < size)
	{
		if (m_hFile != NULL)
		{
			if (feof(m_hFile) == 0)
			{
				YYLOGE ("It can't Read data from file enough!");			
				return -1;
			}
		}
	}
#endif // _OS_WIN32

	if (m_bKeyFile && CBaseKey::g_Key != NULL)
		CBaseKey::g_Key->DecryptData (buf, nRead);

/*
	if (m_nBitrate > 0)
	{
		while ((yyGetSysTime () - m_nStartTime) * m_nBitrate < m_llFilePos * 1000)
			yySleep (5000);
	}
*/
	return nRead;
}

int CBaseFile::write(URLContext *h, unsigned char *buf, int size)
{	
	unsigned int uWrite = 0;

#ifdef _OS_WIN32
	WriteFile (m_hFile, buf, size, (DWORD *)&uWrite, NULL);
#else
	if (m_nFD > 0)
		uWrite = ::write (m_nFD, buf, size);
	else
		uWrite = fwrite(buf,1, size, m_hFile);
#endif //_OS_WIN32

	return uWrite;
}

int64_t CBaseFile::seek (URLContext *h, int64_t pos, int whence)
{	
	if (m_nFD <= 0 && m_hFile == NULL)
		return -1;

#ifdef _OS_WIN32
	long		lPos = (long)pos;
	long		lHigh = (long)(pos >> 32);
	
	int sMove = FILE_BEGIN;
	if (whence == AVSEEK_SIZE)
		return m_llFileSize;
	else if (whence == SEEK_SET)
	{
		sMove = FILE_BEGIN;
		m_llFilePos = pos;

		pos = pos + m_nKeyLen;
		lPos = (long)pos;
		lHigh = (long)(pos >> 32);
	}
	else if (whence == SEEK_CUR)
	{
		sMove = FILE_CURRENT;
		m_llFilePos = m_llFilePos + pos;
	}
	else if (whence == SEEK_END)
	{
		sMove = FILE_END;
		m_llFilePos = m_llFileSize - pos;
		pos = pos - m_nKeyLen;
		lPos = (long)pos;
		lHigh = (long)(pos >> 32);
	}

	if (m_llFilePos > m_llFileSize)
	{
		g_bFileError = true;
		YYLOGW ("The file is wrong. Pos %lld > size %lld", m_llFilePos, m_llFileSize);
		return -1;
	}

	DWORD dwRC = SetFilePointer (m_hFile, lPos, &lHigh, sMove);
	//modefied by Aiven,return the currect file pointer if finish the seek.
	if(dwRC == INVALID_SET_FILE_POINTER)
		return -1;

	return pos;

#elif defined _OS_LINUX
	
	if (whence == AVSEEK_SIZE)
		return m_llFileSize;
	else if (whence == SEEK_SET)
	{
		m_llFilePos = pos;
		pos = pos + m_nKeyLen;
	}
	else if (whence == SEEK_CUR)
	{
		m_llFilePos = m_llFilePos + pos;
	}
	else if (whence == SEEK_END)
	{
		m_llFilePos = m_llFileSize - pos;
		pos = pos - m_nKeyLen;
	}

	if (m_llFilePos > m_llFileSize)
	{
		g_bFileError = true;
		YYLOGW ("The file is wrong. Pos %lld > size %lld", m_llFilePos, m_llFileSize);
		return -1;
	}
	
	long long llPos = 0;
	if (m_nFD > 0)
	{
		if((llPos = lseek64(m_nFD, pos, whence)) < 0)
		{
			return -1;
		}
	}
	else
	{	
		if (fseeko (m_hFile, pos, whence) < 0)
		{
			YYLOGE("fseeko to  : %lld failed", (long long) pos);
			return -1;
		}	
		
		llPos = ftello (m_hFile);
		if (llPos < 0)
		{
			YYLOGE("ftello the position failed");
			return -1;
		}	
	}
	
	return llPos;
#endif // _OS_WIN32
}

int CBaseFile::close (URLContext *h)
{	
	if (m_nFD > 0)
	{
#ifdef _OS_LINUX		
		::close (m_nFD);
#endif // _OS_LINUX

		m_nFD = -1;
	}
	
	if (m_hFile != NULL)
#ifdef _OS_WIN32
		CloseHandle (m_hFile);
#elif defined _OS_LINUX
		fclose(m_hFile);
#endif // _OS_WIN32

	m_hFile = NULL;

	return 0;
}

int CBaseFile::get_handle (URLContext *h)
{	
	if (m_nFD > 0)
		return m_nFD;
		
	return (int)m_hFile;
}

int CBaseFile::check (URLContext *h, int mask)
{	
	if (m_nFD > 0 || m_hFile != NULL)
		return 0;
	else
		return -1;
}



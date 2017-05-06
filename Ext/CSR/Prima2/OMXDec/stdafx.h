/***************************************************************************
*                                                                         *
*                   SiRF Technology, Inc. Platform Software               *
*                                                                         *
*    Copyright (c) 2010 by SiRF Technology, Inc.  All rights reserved.    *
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

#pragma once

#define WINVER	_WIN32_WCE
#define _WIN32_WCE_AYGSHELL 1

#define _CSTRING_NS

#include <afxres.h>
#include <atlbase.h>
#include <altcecrt.h>
#include <atlconv.h>
#include <atlcoll.h>
#include <atlfile.h>
#include <atlsync.h>

#ifdef _DEBUG
#define TRACE	ATLTRACE
#else
#define TRACE	_tprintf
#endif

/*******************************************************************************
	File:		CBaseTCP.cpp

	Contains:	base object implement code

	Written by:	Fenger King

	Change History (most recent first):
	2013-06-14		Fenger			Create file

*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#ifdef _OS_WIN32
#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#else
#include "unistd.h"
#include "netdb.h"
#include "fcntl.h"
#endif // _WIN32

#include "CBaseTCP.h"

#include "UThreadFunc.h"
#include "USystemFunc.h"
#include "USocketFunc.h"
#include "UStringFunc.h"

#include "yyLog.h"

#ifdef _OS_WIN32
#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

#ifndef S_IRUSR
#define S_IRUSR S_IREAD
#endif
#ifndef S_IWUSR
#define S_IWUSR S_IWRITE
#endif
#endif

typedef struct TCPContext {
    int fd;
    int listen;
    int rw_timeout;
    int listen_timeout;
} TCPContext;

CBaseTCP::CBaseTCP(void)
	: CBaseIO ()
{
	SetObjectName (__FILE__);
}

CBaseTCP::~CBaseTCP(void)
{
}

int CBaseTCP::open (URLContext *h, const TCHAR * filename, int flags)
{
    struct		addrinfo hints = { 0 }, *ai, *cur_ai;
    TCPContext *s = (TCPContext *)h->priv_data;
	int			port = 80, fd = -1;
	const char *p;
	char		buf[256];
	int			ret;
	int			optlen;
	char		hostname[1024],proto[1024],path[1024];
	char		portstr[10];
	char		szURL[2048];

#ifdef UNICODE
	memset (szURL, 0, sizeof (szURL));
	WideCharToMultiByte (CP_ACP, 0, filename, -1, szURL, sizeof (szURL), NULL, NULL);
#else
	strcpy (szURL, filename);
#endif // UICODE

	memset (hostname, 0, sizeof (hostname));
    yyURLSplit (proto, sizeof(proto), NULL, 0, hostname, sizeof(hostname), &port, path, sizeof(path), szURL);
	if (port <= 0 || port >= 65536) 
	{
		YYLOGE("Port missing in uri");
		return -1;
	}

	p = strchr(szURL, '?');
	if (p) 
	{
		if (yyFindInfoTag (buf, sizeof(buf), "listen", p))
			s->listen = 1;
		if (yyFindInfoTag (buf, sizeof(buf), "timeout", p)) 
		{
			s->rw_timeout = strtol(buf, NULL, 10);
		}
		if (yyFindInfoTag (buf, sizeof(buf), "listen_timeout", p)) 
		{
			s->listen_timeout = strtol(buf, NULL, 10);
		}
	}

	h->rw_timeout = s->rw_timeout;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	snprintf(portstr, sizeof(portstr), "%d", port);
	if (s->listen)
		hints.ai_flags |= AI_PASSIVE;
	if (!hostname[0])
		ret = getaddrinfo(NULL, portstr, &hints, &ai);
	else
		ret = getaddrinfo(hostname, portstr, &hints, &ai);
	if (ret) 
	{
		YYLOGE("Failed to resolve hostname %s: %s\n", hostname, gai_strerror(ret));
		return -1;
	}

	cur_ai = ai;

 restart:
    ret = -1;
    fd = socket(cur_ai->ai_family, cur_ai->ai_socktype, cur_ai->ai_protocol);
    if (fd < 0)
        goto fail;

//	u_long param = 1;
//	ioctlsocket(fd, FIONBIO, &param);
    ret = connect(fd, cur_ai->ai_addr, cur_ai->ai_addrlen);
    
    if (ret < 0) 
	{
		return ret;
/*
        struct pollfd p = {fd, POLLOUT, 0};
        int64_t wait_started;
        ret = -1;
        if (ret == AVERROR(EINTR)) 
		{
            if (ff_check_interrupt(&h->interrupt_callback)) 
			{
                ret = -1;
                goto fail1;
            }
            goto redo;
        }
        if (ret != AVERROR(EINPROGRESS) && ret != AVERROR(EAGAIN))
            goto fail;

        // wait until we are connected or until abort 
        wait_started = av_gettime();
        do {
            if (ff_check_interrupt(&h->interrupt_callback)) {
                ret = AVERROR_EXIT;
                goto fail1;
            }
            ret = poll(&p, 1, 100);
            if (ret > 0)
                break;
        } while (!h->rw_timeout || (av_gettime() - wait_started < h->rw_timeout));
        if (ret <= 0) {
            ret = AVERROR(ETIMEDOUT);
            goto fail;
        }
        // test error 
        optlen = sizeof(ret);
        if (getsockopt (fd, SOL_SOCKET, SO_ERROR, &ret, &optlen))
            ret = AVUNERROR(ff_neterrno());
        if (ret != 0) {
            char errbuf[100];
            ret = AVERROR(ret);
            av_strerror(ret, errbuf, sizeof(errbuf));
            av_log(h, AV_LOG_ERROR,
                   "TCP connection to %s:%d failed: %s\n",
                   hostname, port, errbuf);
            goto fail;
        }
*/
    }
    h->is_streamed = 1;
    s->fd = fd;
    freeaddrinfo(ai);
    return 0;

 fail:
    if (cur_ai->ai_next) 
	{
        // Retry with the next sockaddr 
        cur_ai = cur_ai->ai_next;
        if (fd >= 0)
            closesocket(fd);
        goto restart;
    }
 fail1:
    if (fd >= 0)
        closesocket(fd);
    freeaddrinfo(ai);

    return ret;
}

int CBaseTCP::read (URLContext *h, unsigned char *buf, int size)
{	
    TCPContext *s = (TCPContext *)h->priv_data;
    int ret;
/*
    if (!(h->flags & AVIO_FLAG_NONBLOCK)) 
	{
        ret = ff_network_wait_fd_timeout(s->fd, 0, h->rw_timeout, &h->interrupt_callback);
        if (ret)
            return ret;
    }
*/
    ret = recv(s->fd, (char *)buf, size, 0);
    return ret < 0 ? -1 : ret;
}

int CBaseTCP::write(URLContext *h, const unsigned char *buf, int size)
{
    TCPContext *s = (TCPContext *)h->priv_data;
    int ret;
/*
    if (!(h->flags & AVIO_FLAG_NONBLOCK)) {
        ret = ff_network_wait_fd_timeout(s->fd, 1, h->rw_timeout, &h->interrupt_callback);
        if (ret)
            return ret;
    }
*/
    ret = send(s->fd, (char *)buf, size, 0);
    return ret < 0 ? -1 : ret;
}

int CBaseTCP::close (URLContext *h)
{	
    TCPContext *s = (TCPContext *)h->priv_data;
    closesocket(s->fd);
    return 0;
}

int CBaseTCP::get_handle (URLContext *h)
{	
	return (int)this;
}

int CBaseTCP::shutdown (URLContext *h, int flags)
{
    TCPContext *s = (TCPContext *)h->priv_data;
    int how;

    if (flags & AVIO_FLAG_WRITE && flags & AVIO_FLAG_READ) 
	{
        how = SHUT_RDWR;
    } 
	else if (flags & AVIO_FLAG_WRITE) 
	{
        how = SHUT_WR;
    } 
	else 
	{
        how = SHUT_RD;
    }

	return ::shutdown(s->fd, how);
}

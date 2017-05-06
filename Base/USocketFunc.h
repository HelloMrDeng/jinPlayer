/*******************************************************************************
	File:		USocketFunc.h

	Contains:	The base utility for socket header file.

	Written by:	Fenger King

	Change History (most recent first):
	2013-11-28		Fenger			Create file

*******************************************************************************/
#ifndef __USocketFunc_H__
#define __USocketFunc_H__

#include "yyType.h"

bool	yySocketInit (void);
void	yySocketUninit (void);

/*

#define DEFAULTVOSOCKETRECVTIMEOUT	2
#define DEFAULTVOSOCKETSENDTIMEOUT	2

VO_S32 vo_socket_send( VO_S32 s , VO_PBYTE buffer , VO_S32 size );
VO_S32 vo_socket_recv( VO_S32 s , VO_PBYTE buffer , VO_S32 size );
VO_S32 vo_socket_send_safe( VO_S32 s , VO_PBYTE buffer , VO_S32 size , VO_BOOL * bexit = NULL );
VO_S32 vo_socket_close( VO_S32 s );
VO_BOOL vo_socket_connect( VO_S32 * ptr_socket , VO_CHAR * str_host , VO_CHAR * str_port , VO_BOOL * bexit = NULL );

#ifdef ENABLE_ASYNCDNS
VO_BOOL vo_socket_connect_asyncdns( VO_S32 * ptr_socket , VO_CHAR * str_host , VO_CHAR * str_port , VO_BOOL * bexit , VO_BOOL * bDNSFailed , VO_CHAR *ptr_addr );
#endif

//set socket time out of recv & send 
VO_BOOL vo_socket_setTimeout(VO_S32 * ptr_socket, VO_S32 recv = DEFAULTVOSOCKETRECVTIMEOUT , VO_S32 send = DEFAULTVOSOCKETSENDTIMEOUT ); 

//set receive time out value acoording http server response field "X-SocketTimeout"
VO_S32 vo_socket_setReceiveTimeout(VO_S32 * ptr_socket, VO_S32 seconds); 
        
*/

#endif // __USocketFunc_H__
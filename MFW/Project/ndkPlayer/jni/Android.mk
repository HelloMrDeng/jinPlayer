#*******************************************************************************
#	File:		Android.mk file
#
#	Contains:	yy NDK player ndk make file
#
#	Written by:	Fenger King
#
#	Change History (most recent first):
#	2013-09-28		Fenger			Create file
#
#*******************************************************************************
LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
LOCAL_MODULE    := yyMediaJNI

LOCAL_SRC_FILES :=	jniPlayer.cpp \
					CNDKPlayer.cpp \
					CNativeWndRender.cpp \
					../../../../Base/yyLog.c \
					../../../../Base/USystemFunc.cpp \
					../../../../Common/CBaseObject.cpp \
					../../../../Common/CMutexLock.cpp \
					../../../../Common/CNodeList.cpp \
										
LOCAL_C_INCLUDES := ../../../../Include \
					../../../../Base \
					../../../../Common \
					../../../OMBox \
					../../../Common \

LOCAL_CFLAGS := -D_OS_LINUX -D_OS_NDK -D_YYLOG_INFO -D_YYLOG_ERROR 

	
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

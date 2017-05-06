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
					CMediaCodecRnd.cpp \
					../../../../Common/CBaseObject.cpp \
					../../../../Base/UJNIFunc.cpp \
					../../../../Base/yyLog.c \
					../../../../Base/USystemFunc.cpp \

										
LOCAL_C_INCLUDES := ../../../../Include \
					../../../../Base \
					../../../../Common \
					../../../../Ext/ffmpeg21/trunk \
					
LOCAL_CFLAGS := -DNDEBUG -D_OS_LINUX -D_OS_NDK -D_YY_MEDIA -D_YY_PLAYER \
				-D_YYLOG_ERROR -D_YYLOG_WARNING -D_YYLOG_INFO -D_YYLOG_TEXT \
				-mfloat-abi=soft -march=armv6j -mtune=arm1136jf-s -fsigned-char
	
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

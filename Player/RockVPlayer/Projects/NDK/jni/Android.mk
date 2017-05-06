#*******************************************************************************
#	File:		Android.mk file
#
#	Contains:	RockV player ndk make file
#
#	Written by:	Fenger King
#
#	Change History (most recent first):
#	2014-10-28		Fenger			Create file
#
#*******************************************************************************
LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
LOCAL_MODULE    := yyRockVJNI

LOCAL_SRC_FILES :=	jniPlayer.cpp \
					CNDKPlayer.cpp \
					../../../Source/CLessonInfo.cpp \
					../../../Source/CMultiPlayer.cpp \
					../../../../../Player/Common/CMediaEngine.cpp \
					../../../../../Common/CBaseObject.cpp \
					../../../../../Common/CThreadWork.cpp \
					../../../../../Common/CBaseConfig.cpp \
					../../../../../Base/UJNIFunc.cpp \
					../../../../../Base/yyLog.c \
					../../../../../Base/ULibFunc.cpp \
					../../../../../Base/USystemFunc.cpp \
					../../../../../Base/UThreadFunc.cpp \
					../../../../../Base/UFileFunc.cpp \
										
LOCAL_C_INCLUDES := ../../../../../Include \
					../../../../../Base \
					../../../../../Common \
					../../../../../Player/Common \
					../../../Source \
					../../../../../Ext/ffmpeg21/trunk \
					
LOCAL_CFLAGS := -DNDEBUG -D_OS_LINUX -D_OS_NDK -D_YY_MEDIA -D_YY_PLAYER \
				-D_YYLOG_ERROR -D_YYLOG_WARNING -D_YYLOG_INFO -D_YYLOG_TEXT \
				-mfloat-abi=soft -march=armv6j -mtune=arm1136jf-s -fsigned-char
	
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

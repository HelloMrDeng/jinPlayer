#*******************************************************************************
#	File:		Android.mk file
#
#	Contains:	yy ext data ndk make file
#
#	Written by:	Fenger King
#
#	Change History (most recent first):
#	2013-12-24		Fenger			Create file
#
#*******************************************************************************
LOCAL_PATH := $(call my-dir)
 
include $(CLEAR_VARS)
 
LOCAL_MODULE    := yyExtDataJNI

LOCAL_SRC_FILES :=	jniExtData.cpp \
					CExtData.cpp \
								
LOCAL_LDLIBS := -llog

include $(BUILD_SHARED_LIBRARY)

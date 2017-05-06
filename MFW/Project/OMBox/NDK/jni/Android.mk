
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := yyBaseEngn      # name it whatever
LOCAL_SRC_FILES := ../../../../../ext/ffmpeg21/trunk/library/NDK/libyyBaseEngn.so
#LOCAL_SRC_FILES := ../../../../../ext/ffmpeg21/trunk/libs/armeabi-v7a/libyyBaseEngn.so
include $(PREBUILT_SHARED_LIBRARY)  


include $(CLEAR_VARS)

LOCAL_ARM_MODE  := arm

LOCAL_MODULE    := yyMediaEng

LOCAL_SRC_FILES := \
		../../../../../Common/CBaseObject.cpp \
		../../../../../Common/CBaseConfig.cpp \
		../../../../../Common/CBaseIO.cpp \
		../../../../../Common/CBaseFile.cpp \
		../../../../../Common/CBasePDP.cpp \
		../../../../../Common/CBaseIOHook.cpp \
		../../../../../Common/CBaseExtData.cpp \
		../../../../../Common/CBaseUtils.cpp \
		../../../../../Common/CMutexLock.cpp \
		../../../../../Common/CBaseKey.cpp \
		../../../../../Common/CThreadSem.cpp \
		../../../../../Common/CNodeList.cpp \
		../../../../../Common/CLicenseCheck.cpp \
		../../../../../Base/USystemFunc.cpp \
		../../../../../Base/UThreadFunc.cpp \
		../../../../../Base/UStringFunc.cpp \
		../../../../../Base/UFFMpegFunc.cpp \
		../../../../../Base/ULibFunc.cpp \
		../../../../../Base/UFileFunc.cpp \
		../../../../../Base/UJNIFunc.cpp \
		../../../../../Base/UFileFormat.cpp \
		../../../../../Base/UVV2FF.cpp \
		../../../../../Base/UYYDataFunc.cpp \
		../../../../../Base/yyLog.c \
		../../../../../Base/yyLogoData.c \
		../../../../Common/CBaseClock.cpp \
		../../../../Common/CBaseSource.cpp \
		../../../../Common/CImageSource.cpp \
		../../../../Common/CFFMpegSource.cpp \
		../../../../Common/CFFMpegSourceTool.cpp \
		../../../../Common/CStreamSource.cpp \
		../../../../Common/CBaseAudioDec.cpp \
		../../../../Common/CFFMpegAudioDec.cpp \
		../../../../Common/CFFMpegAudioRSM.cpp \
		../../../../Common/CBaseVideoDec.cpp \
		../../../../Common/CFFMpegVideoDec.cpp \
		../../../../Common/CFFMpegSubTTDec.cpp \
		../../../../Common/CVVVideoDec.cpp \
		../../../../Common/CVVVideoH265Dec.cpp \
		../../../../Common/CVVVideoOMXDec.cpp \
		../../../../Common/CFFMpegVideoRCC.cpp \
		../../../../Common/CVideoRCCThread.cpp \
		../../../../Common/CDataConvert.cpp \
		../../../../Common/CBaseAudioRnd.cpp \
		../../../../Common/CBaseVideoRnd.cpp \
		../../../../Common/CMediaThumb.cpp \
		../../../../../Ext/VV3.9.3/trunk/Common/cmnMemory.c \
		../../../../OMBox/CBoxBase.cpp \
		../../../../OMBox/CBoxSource.cpp \
		../../../../OMBox/CBoxExtData.cpp \
		../../../../OMBox/CBoxAudioDec.cpp \
		../../../../OMBox/CBoxVideoDec.cpp \
		../../../../OMBox/CBoxVDBASmt.cpp \
		../../../../OMBox/CBoxRender.cpp \
		../../../../OMBox/CBoxAudioRnd.cpp \
		../../../../OMBox/CBoxVideoRnd.cpp \
		../../../../OMBox/CBoxAudioRndExt.cpp \
		../../../../OMBox/CBoxVideoRndExt.cpp \
		../../../../OMBox/CBoxExtRnd.cpp \
		../../../../OMBox/CBoxMonitor.cpp \
		../../../../OMBox/COMBoxMng.cpp \
		../../../../../Source/AdptStream/COutBuffer.cpp \
		../../../../../Source/AdptStream/COutSample.cpp \
		../../../../../Source/AdptStream/COutSampleV2.cpp \
		../../../../../Source/AdptStream/CProgInfo.cpp \
		../../../../../Source/AdptStream/CSourceIO.cpp \
		../../../../../Source/AdptStream/CStreamBA.cpp \
		../../../../../Source/AdptStream/CStreamDRM.cpp \
		../../../../../Source/AdptStream/CStreamDemux.cpp \
		../../../../../Source/AdptStream/CStreamDemuxTS.cpp \
		../../../../../Source/AdptStream/CStreamDownLoad.cpp \
		../../../../../Source/AdptStream/CStreamParser.cpp \
		../../../../../Subtitle/CSubTitleItem.cpp \
		../../../../../Subtitle/CSubTitleBase.cpp \
		../../../../../Subtitle/CSubTitleAss.cpp \
		../../../../../Subtitle/CSubTitleSrt.cpp \
		../../../../../Subtitle/CSubTitleSmi.cpp \
		../../../../../Subtitle/CSubTitleFFMpeg.cpp \
		../../../../../Subtitle/CSubTitleEngine.cpp \
		../../CMediaManager.cpp \
		../../yyMediaEng.cpp

LOCAL_C_INCLUDES := \
	../../../../../Include \
	../../../../../Base \
	../../../../../Common \
	../../../../../Source/AdptStream \
	../../../../../Subtitle \
	../../../../Common \
	../../../../OMBox \
	../../../../../Ext/ffmpeg21/trunk \
	../../../../../Ext/VV3.9.3/trunk/Include \
	../../../../../Ext/VV3.9.3/trunk/Include/vome \
	../../../../../Ext/VV3.9.3/trunk/Common \
	../../../../../Ext/VV3.9.3/trunk/Source/Include \
	../../../../../ext/ndk/r9/jni/include \
	
LOCAL_CFLAGS := -DNDEBUG -D_OS_LINUX -D_OS_NDK -D_YY_MEDIA -D_YY_PLAYER -D_EXT_VO \
				-D_YYLOG_ERROR -D_YYLOG_WARNING -D_YYLOG_INFO -D_YYLOG_TEXT -D_new_programinfo \
				-mfloat-abi=soft -march=armv6j -mtune=arm1136jf-s -fsigned-char

		
LOCAL_LDLIBS  := -llog -ldl -lstdc++ \
				../../../../../Ext/VV3.9.3/trunk/Bin/NDK/libyyClrCvtLib.a \
				../../../../../Ext/VV3.9.3/trunk/Bin/NDK/libyyHEVCDecLib.a \
				../../../../../Ext/VV3.9.3/trunk/Bin/NDK/libyyDataIOLib.a \
				../../../../../Ext/VV3.9.3/trunk/Bin/NDK/libyyHLSParserLib.a \
				../../../../../Ext/VV3.9.3/trunk/Bin/NDK/libyyTSDemuxLib.a \
				../../../../../Ext/VV3.9.3/trunk/Bin/NDK/libyyVideoParser.a \
				../../../../../ext/ndk/r9/lib/libandroid_support.a \
											
LOCAL_SHARED_LIBRARIES := yyBaseEngn

include $(BUILD_SHARED_LIBRARY)

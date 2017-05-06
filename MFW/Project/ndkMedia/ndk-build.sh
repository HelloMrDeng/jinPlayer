cd /Volumes/Data/Works/yyMedia/trunk/MFW/Project/ndkMedia/jni

ndk-build 2>&1

cp ../libs/armeabi/libyyMediaJNI.so /Volumes/Data/Works/yyMedia/trunk/Player/Sample/ndk2/libs/armeabi
cp ../libs/armeabi/libyyMediaJNI.so /Volumes/Data/Works/yyMedia/trunk/Player/Sample/BASmooth/libs/armeabi
cp ../libs/armeabi/libyyMediaJNI.so /Volumes/Data/Works/yyMedia/trunk/Player/RVLesson/libs/armeabi
#cp ../libs/armeabi/libyyMediaJNI.so /Volumes/Data/Works/yyMedia/trunk/Player/ndk2/libs/armeabi

#adb push ../libs/armeabi/libyyMediaJNI.so /data/data/com.cansure.samplePlayer/lib/
#adb push ../libs/armeabi/libyyMediaJNI.so /data/data/com.cansure.yysampleplayer/lib/

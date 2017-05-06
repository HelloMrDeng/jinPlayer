cd /Volumes/Data/Works/yyMedia/trunk/Player/RockVPlayer/Projects/NDK/jni

ndk-build 2>&1

cp ../libs/armeabi/yyRockVJNI.so /Volumes/Data/Works/yyMedia/trunk/Player/RockVPlayer/Projects/Android/libs/armeabi
#cp ../libs/armeabi/libyyMediaJNI.so /Volumes/Data/Works/yyMedia/trunk/Player/ndk2/libs/armeabi

#adb push ../libs/armeabi/libyyMediaJNI.so /data/data/com.cansure.samplePlayer/lib/
#adb push ../libs/armeabi/libyyMediaJNI.so /data/data/com.cansure.yysampleplayer/lib/
Install        : libyyRockVJNI.so => libs/armeabi/libyyRockVJNI.so
cp: ../libs/armeabi/yyRockVJNI.so: No such file or directory

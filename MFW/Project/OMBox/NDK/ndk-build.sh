
cd /Volumes/Data/Works/yyMedia/trunk/MFW/Project/OMBox/NDK/jni

ndk-build 2>&1

adb push ../libs/armeabi/libyyMediaEng.so /data/local/tmp/yylib/

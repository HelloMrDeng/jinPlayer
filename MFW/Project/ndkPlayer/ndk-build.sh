
cd /Volumes/Data/Works/yyMedia/trunk/MFW/Project/ndkPlayer/jni

ndk-build 2>&1

#adb push ../libs/armeabi/libyyNDKPlayer.so /data/local/tmp/yylib/
cp ../libs/armeabi/libyyNDKPlayer.so /Volumes/Data/Works/yyMedia/trunk/Player/Demo/Android/libs/armeabi
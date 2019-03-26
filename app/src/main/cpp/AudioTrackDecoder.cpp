//
// Created by liuqikang on 2019/3/26.
//
#include <jni.h>
#include "AudioTrackDecoder.h"


extern "C"
JNIEXPORT jint JNICALL
Java_lqk_video_audiotrack_MusicDecoder_getMusicMeta(JNIEnv *env, jobject instance,
                                                    jstring musicPath_, jintArray metaArray_) {
    const char *musicPath = env->GetStringUTFChars(musicPath_, 0);
    jint *metaArray = env->GetIntArrayElements(metaArray_, NULL);

    // TODO

    env->ReleaseStringUTFChars(musicPath_, musicPath);
    env->ReleaseIntArrayElements(metaArray_, metaArray, 0);
}

extern "C"
JNIEXPORT jint JNICALL
Java_lqk_video_audiotrack_MusicDecoder_openFile(JNIEnv *env, jobject instance,
                                                jstring accompanyPath_,
                                                jfloat packetBufferTimePercent) {
    const char *accompanyPath = env->GetStringUTFChars(accompanyPath_, 0);

    // TODO

    env->ReleaseStringUTFChars(accompanyPath_, accompanyPath);
}

extern "C"
JNIEXPORT jint JNICALL
Java_lqk_video_audiotrack_MusicDecoder_readSamples__Lshort_3_093_2ILint_3_093_2(JNIEnv *env,
                                                                                jobject instance,
                                                                                jshortArray samples_,
                                                                                jint size,
                                                                                jintArray slientSizeArr_) {
    jshort *samples = env->GetShortArrayElements(samples_, NULL);
    jint *slientSizeArr = env->GetIntArrayElements(slientSizeArr_, NULL);

    // TODO

    env->ReleaseShortArrayElements(samples_, samples, 0);
    env->ReleaseIntArrayElements(slientSizeArr_, slientSizeArr, 0);
}

extern "C"
JNIEXPORT void JNICALL
Java_lqk_video_audiotrack_MusicDecoder_closeFile(JNIEnv *env, jobject instance) {

    // TODO

}
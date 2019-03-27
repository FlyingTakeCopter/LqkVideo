//
// Created by liuqikang on 2019/3/26.
//
#include <jni.h>
#include "AudioTrackDecoder.h"
#include "CommonTools.h"

#define LQKFUNC(name) Java_lqk_video_audiotrack_MusicDecoder_##name
#define LOG_TAG "AudioTrackDecoder"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/jni.h>
#include <libavutil/avutil.h>

JNIEXPORT jint JNICALL LQKFUNC(getMusicMeta)
    (JNIEnv *env, jobject instance, jstring musicPath_, jintArray metaArray_) {
    const char *musicPath = env->GetStringUTFChars(musicPath_, 0);
    jint *metaArray = env->GetIntArrayElements(metaArray_, NULL);

    // TODO 注册
    av_register_all();// 封装格式注册
    avformat_network_init();// 网络注册

    // TODO 解封装
    AVFormatContext* avFormatContext = avformat_alloc_context();
    int res = avformat_open_input(&avFormatContext, musicPath, 0, 0);
    if (res != 0){
        LOGE("avformat_open_input failed : %s", av_err2str(res));
        return 0;
    }
    // 探测流信息
    res = avformat_find_stream_info(avFormatContext, 0);
    if (res < 0){
        LOGE("avformat_find_stream_info failed : %s", av_err2str(res));
        return 0;
    }
    // 探测 音频流
    AVStream* audioStream = nullptr;
    for (int i = 0; i < avFormatContext->nb_streams; ++i) {
        if (avFormatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            audioStream = avFormatContext->streams[i];
            break;
        }
    }
    if (audioStream == nullptr){
        LOGE("search audioStreamIdx failed");
        return 0;
    }

//     TODO 解码
    // 查找解码器 根据ID
    AVCodec* pCodec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if (pCodec == NULL){
        LOGE("avcodec_find_decoder failed");
        return 0;
    }
    // 创建一个解码器上下文
    AVCodecContext* avCodecContext = avcodec_alloc_context3(pCodec);
    // 拷贝参数
    res = avcodec_parameters_to_context(avCodecContext, audioStream->codecpar);
    if (res < 0){
        LOGE("avcodec_parameters_to_context failed : %s", av_err2str(res));
        return 0;
    }
    // 修改解码器参数
    avCodecContext->thread_count = 2;// 解码线程数量
    // 打开解码器
    res = avcodec_open2(avCodecContext, pCodec, 0);
    if (res != 0){
        LOGE("avcodec_open2 failed : %s", av_err2str(res));
        return 0;
    }

    // TODO 参数赋值
    metaArray[0] = avCodecContext->sample_rate;// 采样率
    metaArray[1] = avCodecContext->bit_rate;// 比特率

    // TODO 销毁释放
    avformat_free_context(avFormatContext);
    avcodec_free_context(&avCodecContext);

    env->ReleaseStringUTFChars(musicPath_, musicPath);
    env->ReleaseIntArrayElements(metaArray_, metaArray, 0);

    LOGI("getMusicMeta success");
    return 0;
}

JNIEXPORT jint JNICALL LQKFUNC(openFile)
    (JNIEnv *env, jobject instance, jstring accompanyPath_, jfloat packetBufferTimePercent) {
    const char *accompanyPath = env->GetStringUTFChars(accompanyPath_, 0);

    // TODO

    env->ReleaseStringUTFChars(accompanyPath_, accompanyPath);
}

JNIEXPORT void JNICALL LQKFUNC(closeFile)
    (JNIEnv *env, jobject instance) {

    // TODO

}

JNIEXPORT jint JNICALL LQKFUNC(readSamples)
    (JNIEnv *env, jobject instance, jshortArray samples_, jint size, jintArray slientSizeArr_) {
    jshort *samples = env->GetShortArrayElements(samples_, NULL);
    jint *slientSizeArr = env->GetIntArrayElements(slientSizeArr_, NULL);

    // TODO

    env->ReleaseShortArrayElements(samples_, samples, 0);
    env->ReleaseIntArrayElements(slientSizeArr_, slientSizeArr, 0);
}

}


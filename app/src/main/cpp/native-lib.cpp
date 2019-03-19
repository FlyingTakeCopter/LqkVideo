#include <jni.h>
#include <string>
#include <sstream>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libavcodec/jni.h>
}

#ifdef ANDROID
#include "android/log.h"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "XPlay", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "XPlay", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "XPlay", __VA_ARGS__)
#else
#define LOGI(...) pringf("XPlay", __VA_ARGS__)
#define LOGD(...) printf("XPlay", __VA_ARGS__)
#define LOGE(...) printf("XPlay", __VA_ARGS__)
#endif

// 全局变量 用作输出
//std::ostringstream oss;
//// 清空
//#define OSS_CLEAR (oss).str("");
//// 换行拼接
//#define LOGE(info, data) oss << (info) << (data) << "\n";
//#define LOGE_AVRATIONAL(info, r) oss << (info) << (r.num) << " / " << (r.den) << "\n";
//#define LOGE_ENUM(info, em) oss << (info) << (em) << "\n";
//// oss结果
//#define OSS_STR (oss.str())

// AVRational 转换公式
static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

extern "C"
JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved){
    LOGE("native jni_onload");
    av_jni_set_java_vm(vm, 0);
    return JNI_VERSION_1_4;
}


// 视频源获取 https://blog.csdn.net/m0_37677536/article/details/83304674
extern "C"
JNIEXPORT jstring
JNICALL
Java_lqk_video_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from \n C++";
//    hello += avcodec_configuration();

    // 解封装流程
    // 1.注册解封装
    av_register_all();
    // 2.注册网络
    avformat_network_init();
    // 3.打开文件
    AVFormatContext *ps = NULL;
//    av_find_input_format()
//    AVInputFormat
//    char p[] = "http://ksy.fffffive.com/mda-hinp1ik37b0rt1mj/mda-hinp1ik37b0rt1mj.mp4";
    char p[] = "sdcard/1080.mp4";
    int re = avformat_open_input(&ps, p, 0, 0);
    if (re == 0){
        LOGE("avformat_open_input success : %s", p);
    } else{
        LOGE("avformat_open_input failed : %s", av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    }

    // 4.探测stream流信息 手动探测 媒体信息 比如flv h264等不包含头的数据
    if (avformat_find_stream_info(ps, 0) < 0){
        LOGE("avformat_find_stream_info failed : %s", av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    }

    // 5.获取流的标号
    int video_stream = -1;
    int audio_stream = -1;
    // AVStream
    // AVRational time_base; 代表duration的单位 含义是(num/den)秒
    // int64_t duration;  以time_base为单位的一个数
    // 总时长(秒)= duration * ((double)time_base.num / (double)time_base.den)// 注意精度损失
    // 小心分母为0
    // AVRational avg_frame_rate 帧率 例如 25 / 1
    // AVCodecParameters *codecpar 音视频参数
    for (int i = 0; i < ps->nb_streams; ++i) {
        AVStream* stream = ps->streams[i];
        LOGE("AVMediaType: %d", stream->codecpar->codec_type);
        LOGE("AVCodecID: %d", stream->codecpar->codec_id);
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream = i;
            AVPixelFormat fmt;
            LOGE("AVPixelFormat: %d", stream->codecpar->format);
            LOGE("width: %d", stream->codecpar->width);
            LOGE("height: %d", stream->codecpar->height);
            LOGE("fps: %f", r2d(stream->avg_frame_rate));
        } else{
            audio_stream = i;
            AVSampleFormat fmt;
            LOGE("AVSampleFormat: %d", stream->codecpar->format);
            LOGE("channels: %d", stream->codecpar->channels);
            LOGE("sample_rate: %d", stream->codecpar->sample_rate);
        }
        LOGE("time_base: %d / %d", stream->time_base.num , stream->time_base.den);
        LOGE("duration: %lld", stream->duration);
        LOGE("总时长: %f", stream->duration * r2d(stream->time_base));
        LOGE("bit_rate: %lld", stream->codecpar->bit_rate);
    }

    // 6.解码器初始化
    avcodec_register_all();
    // 视频
    // 查找解码器 软解码
    AVCodec*videoCodec = avcodec_find_decoder(ps->streams[video_stream]->codecpar->codec_id);
    // 硬解码
    videoCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    LOGE("videocodec name : %s", videoCodec->name);
    // 创建解码器上下文
    AVCodecContext* videoCodecContext = avcodec_alloc_context3(videoCodec);
    // 复制参数
    avcodec_parameters_to_context(videoCodecContext, ps->streams[video_stream]->codecpar);
    // 修改线程数量
    videoCodecContext->thread_count = 8;
    // 打开解码器
    re = avcodec_open2(videoCodecContext, videoCodec, 0);
    if (re != 0){
        LOGE("avcodec_open2 video codec failed ");
        return env->NewStringUTF(hello.c_str());
    }

    // 音频
    AVCodec*audioCodec = avcodec_find_decoder(ps->streams[audio_stream]->codecpar->codec_id);
    if (audioCodec == NULL){
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("audioCodec name : %s", audioCodec->name);

    AVCodecContext* audioCodecContext = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(audioCodecContext, ps->streams[audio_stream]->codecpar);
    // 打开解码器
    re = avcodec_open2(audioCodecContext, audioCodec, 0);
    if (re != 0){
        LOGE("avcodec_open2 audio codec failed ");
        return env->NewStringUTF(hello.c_str());
    }

    //AVPacket容易造成内存泄漏
    // AVBufferRef:引用计数
    // pts: 显示时间(单位AVRational)
    // dts: 解码时间
    //    av_packet_alloc();    库内部初始化
    //    av_packet_clone();    拷贝
    //    av_packet_ref();av_packet_unref(); 手动计数
    //    av_packet_free();     库内部销毁
    //    av_packet_from_data() 手动创建packet
    //    av_seek_frame(AVFormatContext *s, int stream_index, int64_t timestamp,int flags)
    // 6.按帧读取
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    int replay = 0;
    while (replay < 5){
        int re = av_read_frame(ps, pkt);
        if (re != 0){
            // av_seek_frame
            int pos = 5 * r2d(ps->streams[video_stream]->time_base);
            av_seek_frame(ps, video_stream, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            replay++;
            continue;
        }

        // AVCodecContext解码器
//        avcodec_register_all();
//        avcodec_find_decoder();
//        avcodec_find_decoder_by_name();
//        // 手动指定解码器 arm硬解
//        avcodec_find_decoder_by_name("h264_mediacodec");

        // 解码环境 AVCodecContext内存注册方式
        // AVCodecContext *avcodec_alloc_context3(const AVCodec *codec); 申请
        // void avcodec_free_context(AVCodecContext **avctx);释放
        // int avcodec_open2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options); 打开解码器
        // AVDictionary **options /libavcodec/options_table.h 设置多线程解码int thread_count
        // opencv源码 解码会获取CPU数量 在根据数量开线程解码
        // time_base 可设置和AVStream 的时间基数一致 编码 同步 单位统一：毫秒
        // avcodec_parameters_to_context() AVStream 复制到 codec中  可修改time_base 或者 多线程

//        AVFrame;
//        av_frame_alloc();
//        av_frame_free();
//        av_frame_ref();引用计数+1     av_frame_unref -1
//        av_frame_clone();复制 计数+1
//        linesize // 视频 一行大小    音频 一个通道大小  用来对齐 拷贝使用
//        nb_samples单通道样本数量
//        pts     搜到这一帧对应的pts   时间基数可能和pkt不一样
//        pkt_dts
//        format AVPixelFormat  AVSampleFormat

//        解码/编码流程：
//        1.发送
//        对于解码 请调用avcodec_send_packet()以在AVPacket中给出解码器原始的压缩数据
//        对于编码 请调用avcodec_send_frame()为编码器提供包含未压缩音频或视频的AVFrame
//        2.循环接受
//        在循环中接受输出，定期调用avcodec_receive_xxxxxx()函数并处理输出
//        对于解码 请调用avcodec_receive_frame()。成功后，它将返回一个包含未压缩音频或者视频数据的AVFrame
//        对于编码 请调用avcodec_receive_packet()。成功后，将返回带有压缩帧的AVPacket
//        重复此呼叫，知道返回AVERROR(EAGAIN)或错误。AVERROR(EAGAIN)意味着需要重新输入数据才能返回新的输出

//        在解码或编码开始时，编码器可能会接受多个输入帧/数据包而不返回帧，知道其内部缓冲区被填满为止
//        3.结束流
//        需要"刷新(排水)"编解码器，因为编解码器内部可能会缓冲多个帧数据包以实现性能或不必要性(考虑B帧)
//        处理方式：
//        发送NULL到avcodec_send_packet或avcodec_send_frame,知道返回AVERROR_EOF
//        除非忘记进入排水模式，否则这些功能将不会返回AVERROR

//        在再次解码之前，必须使用avcodec_flush_buffer()重新编码

        AVCodecContext*curCodecCtx = NULL;
        if (pkt->stream_index == audio_stream){
            curCodecCtx = audioCodecContext;
        } else if (pkt->stream_index == video_stream){
            curCodecCtx = videoCodecContext;
        } else{
            LOGE("unknown stream %d", pkt->stream_index);
        }

        int ret = avcodec_send_packet(curCodecCtx, pkt);
        if (ret == AVERROR(EAGAIN))
        {
//            input is not accepted in the current state - user
//            must read output with avcodec_receive_frame() (once
//            all output is read, the packet should be resent, and
//            the call will not fail with EAGAIN).

            while (avcodec_receive_frame(curCodecCtx, frame) == 0)
            {
                if (pkt->stream_index == audio_stream){
                    LOGE("audio pts: %f nb_samples: %d linesize: %d",
                         frame->pts * r2d(ps->streams[audio_stream]->time_base),
                        frame->nb_samples,
                        frame->linesize[0]);

                } else if (pkt->stream_index == video_stream){
                    LOGE("video pts: %f l1: %d l2: %d, l3: %d",
                         frame->pts * r2d(ps->streams[video_stream]->time_base),
                         frame->linesize[0], frame->linesize[1], frame->linesize[2]);

                }

            }
        }



        // 清理pkt
//        LOGI("av_read_frame: pts: %lld  dts: %lld streamindex: %d duration: %lld", pkt->pts, pkt->dts, pkt->stream_index, pkt->duration);
        av_packet_unref(pkt);
        av_frame_unref(frame);
    }

    // 释放packet防止内存泄露
    av_packet_free(&pkt);
    av_frame_free(&frame);

    avcodec_free_context(&audioCodecContext);
    avcodec_free_context(&videoCodecContext);

    // 关闭AVFormatContext
    avformat_close_input(&ps);
    if (ps == NULL){
        LOGE("close success");
    } else{
        LOGE("close failed");
    }

    return env->NewStringUTF(hello.c_str());
}

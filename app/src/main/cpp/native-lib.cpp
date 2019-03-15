#include <jni.h>
#include <string>
#include <sstream>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
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
std::ostringstream oss;
// 清空
#define OSS_CLEAR (oss).str("");
// 换行拼接
#define OSS_FORMAT(info, data) oss << (info) << (data) << "\n";
#define OSS_FORMAT_AVRATIONAL(info, r) oss << (info) << (r.num) << " / " << (r.den) << "\n";
#define OSS_FORMAT_ENUM(info, em) oss << (info) << (em) << "\n";
// oss结果
#define OSS_STR (oss.str())

// AVRational 转换公式
static double r2d(AVRational r)
{
    return r.num == 0 || r.den == 0 ? 0 : (double)r.num / (double)r.den;
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
    char p[] = "sdcard/v1080.mp4";
    int re = avformat_open_input(&ps, p, 0, 0);
    if (re == 0){
        OSS_FORMAT("avformat_open_input success : ", p)
        hello = OSS_STR;
    } else{
        OSS_FORMAT("avformat_open_input failed : ", av_err2str(re))
        hello = OSS_STR;
        return env->NewStringUTF(hello.c_str());
    }

    // 4.探测stream流信息 手动探测 媒体信息 比如flv h264等不包含头的数据
    if (avformat_find_stream_info(ps, 0) < 0){
        OSS_FORMAT("avformat_find_stream_info failed : ", av_err2str(re))
        hello = OSS_STR;
        return env->NewStringUTF(hello.c_str());
    }

    OSS_CLEAR // 清空 (oss.clear() 是清除错误位 不能清空)
    OSS_FORMAT(" ", " ")

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
        OSS_FORMAT_ENUM("AVMediaType: ", stream->codecpar->codec_type)
        OSS_FORMAT_ENUM("AVCodecID: ", stream->codecpar->codec_id)
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream = i;
            AVPixelFormat fmt;
            OSS_FORMAT("AVPixelFormat: ", stream->codecpar->format)
            OSS_FORMAT("width: ", stream->codecpar->width)
            OSS_FORMAT("height: ", stream->codecpar->height)
            OSS_FORMAT("fps: ", r2d(stream->avg_frame_rate))
        } else{
            audio_stream = i;
            AVSampleFormat fmt;
            OSS_FORMAT("AVSampleFormat: ", stream->codecpar->format)
            OSS_FORMAT("channels: ", stream->codecpar->channels)
            OSS_FORMAT("sample_rate: ", stream->codecpar->sample_rate)
        }
        OSS_FORMAT_AVRATIONAL("time_base: ", stream->time_base)
        OSS_FORMAT("duration: ", stream->duration)
        OSS_FORMAT("总时长: ", stream->duration * r2d(stream->time_base))
        OSS_FORMAT("bit_rate: ", stream->codecpar->bit_rate)

        OSS_FORMAT(" ", " ")
    }

    hello += OSS_STR;

    // 6.解码器初始化
    avcodec_register_all();
    // 视频
    // 查找解码器 软解码
    AVCodec*videoCodec = avcodec_find_decoder(ps->streams[video_stream]->codecpar->codec_id);
    // 硬解码
//    codec = avcodec_find_decoder_by_name("h264_mediacodec");
    if (videoCodec == NULL){
        goto end;
    }
    // 创建解码器上下文
    AVCodecContext* videoCodecContext = avcodec_alloc_context3(videoCodec);
    // 复制参数
    avcodec_parameters_to_context(videoCodecContext, ps->streams[video_stream]->codecpar);
    // 修改线程数量
    videoCodecContext->thread_count = 1;

    // 音频
    AVCodec*audioCodec = avcodec_find_decoder(ps->streams[audio_stream]->codecpar->codec_id);
    if (audioCodec == NULL){
        goto end;
    }
    AVCodecContext* audioCodecContext = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(audioCodecContext, ps->streams[audio_stream]->codecpar);


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
    while (1){
        int re = av_read_frame(ps, pkt);
        if (re != 0){
            // av_seek_frame
//            int pos = 5 * r2d(ps->streams[video_stream]->time_base);
//            av_seek_frame(ps, video_stream, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            break;
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

        // 解码流程
        // 打开解码器
        re = avcodec_open2(videoCodecContext, videoCodec, 0);
        if (re != 0){
            LOGE("avcodec_open2 failed ");
            goto end;
        }

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

        avcodec_send_packet();
        avcodec_receive_packet();

        // 清理pkt
        LOGI("av_read_frame: pts: %lld  dts: %lld streamindex: %d duration: %lld", pkt->pts, pkt->dts, pkt->stream_index, pkt->duration);
        av_packet_unref(pkt);
    }

    // 释放packet防止内存泄露
    av_packet_free(&pkt);

end:
    // 关闭AVFormatContext
    avformat_close_input(&ps);
    if (ps == NULL){
        hello += "close success";
    } else{
        hello += "close filed";
    }

    return env->NewStringUTF(hello.c_str());
}

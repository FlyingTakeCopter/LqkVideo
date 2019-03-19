#include <jni.h>
#include <string>
#include <sstream>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
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
        hello = OSS_STR;
    } else{
        hello = OSS_STR;
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: avformat_open_input ");

    // 4.探测stream流信息 手动探测 媒体信息 比如flv h264等不包含头的数据
    if (avformat_find_stream_info(ps, 0) < 0){
        hello = OSS_STR;
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: avformat_find_stream_info ");

    OSS_CLEAR // 清空 (oss.clear() 是清除错误位 不能清空)
    OSS_FORMAT(" ", " ")

    // 5.获取流的标号
    int video_stream = -1;
    int audio_stream = -1;
    for (int i = 0; i < ps->nb_streams; ++i) {
        AVStream* stream = ps->streams[i];

        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            video_stream = i;
        } else{
            audio_stream = i;
        }
    }

    hello += OSS_STR;

    // 6.解码器初始化
    avcodec_register_all();
    // 视频
    // 查找解码器 软解码
    AVCodec*videoCodec = avcodec_find_decoder(ps->streams[video_stream]->codecpar->codec_id);
    // 硬解码
//    AVCodec* videoCodec = avcodec_find_decoder_by_name("h264_mediacodec");
    if (videoCodec == NULL){
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: avcodec_find_decoder_by_name(\"h264_mediacodec\") ");

    // 创建解码器上下文
    AVCodecContext* videoCodecContext = avcodec_alloc_context3(videoCodec);
    // 复制参数
    avcodec_parameters_to_context(videoCodecContext, ps->streams[video_stream]->codecpar);
    // 修改线程数量
    videoCodecContext->thread_count = 8;
    // 打开解码器
    re = avcodec_open2(videoCodecContext, videoCodec, 0);
    if (re != 0){
        LOGE("avcodec_open2 video codec failed : %s", av_err2str(re));
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: avcodec_open2 ");

    // 音频
    AVCodec*audioCodec = avcodec_find_decoder(ps->streams[audio_stream]->codecpar->codec_id);
    if (audioCodec == NULL){
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: avcodec_find_decoder audio ");

    AVCodecContext* audioCodecContext = avcodec_alloc_context3(audioCodec);
    avcodec_parameters_to_context(audioCodecContext, ps->streams[audio_stream]->codecpar);
    // 打开解码器
    re = avcodec_open2(audioCodecContext, audioCodec, 0);
    if (re != 0){
        LOGE("avcodec_open2 audio codec failed ");
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: avcodec_open2 audio ");

    // 初始化解码后数据的结构体
    SwrContext* swrContext = swr_alloc();

    swr_alloc_set_opts(swrContext,
                       av_get_channel_layout_nb_channels(2), // out_ch_layout
                       AV_SAMPLE_FMT_S16, // AVSampleFormat out_sample_fmt
                       audioCodecContext->sample_rate,//int out_sample_rate
                       av_get_channel_layout_nb_channels(audioCodecContext->channel_layout), // in_ch_layout
                       audioCodecContext->sample_fmt, // enum AVSampleFormat in_sample_fmt
                       audioCodecContext->sample_rate,// in_sample_rate
                       0, 0);
    if (!swrContext || swr_init(swrContext)){
        if (swrContext){
            LOGE("failed: swr_init error");
            swr_free(&swrContext);
        }
        LOGE("failed: swr_alloc_set_opts error");
        return env->NewStringUTF(hello.c_str());
    }
    LOGE("success: swrContext init ");

//    r 以只读方式打开文件，该文件必须存在。
//    　　r+ 以可读写方式打开文件，该文件必须存在。
//    　　rb+ 读写打开一个二进制文件，只允许读写数据。
//    　　rt+ 读写打开一个文本文件，允许读和写。
//    　　w 打开只写文件，若文件存在则文件长度清为0，即该文件内容会消失。若文件不存在则建立该文件。
//    　　w+ 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立该文件。
//    　　a 以附加的方式打开只写文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾，即文件原先的内容会被保留。（EOF符保留）
//    　　a+ 以附加方式打开可读写的文件。若文件不存在，则会建立该文件，如果文件存在，写入的数据会被加到文件尾后，即文件原先的内容会被保留。 （原来的EOF符不保留）
//    　　wb 只写打开或新建一个二进制文件；只允许写数据。
//    　　wb+ 读写打开或建立一个二进制文件，允许读和写。
//    　　wt+ 读写打开或着建立一个文本文件；允许读写。
//    　　at+ 读写打开一个文本文件，允许读或在文本末追加数据。
//    　　ab+ 读写打开一个二进制文件，允许读或在文件末追加数据。

    SwsContext* swsContext = NULL;

    int outWidth = 320;
    int outHeight = 180;
    char *rgb = new char[1920*1080*3];

    // 创建临时buf
    char *pcm = new char[44100 * 2 * 2];// 1s采样数量 * 占用2字节 * 双通道

    swsContext = sws_getCachedContext(swsContext,
                                      videoCodecContext->width, videoCodecContext->height, videoCodecContext->pix_fmt,
                                      outWidth, outHeight, AV_PIX_FMT_RGBA,
                                      SWS_BILINEAR,
                                      0, 0, 0);

    //打开输出视频的文件
    FILE* fileV = fopen("/sdcard/testV.rgb", "wb+");
    FILE* file = fopen("/sdcard/test.pcm", "wb+");
    // 6.按帧读取
    AVPacket* pkt = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();

    int read_frame_count = 0;
    while (read_frame_count <= 200){
        int re = av_read_frame(ps, pkt);
        if (re != 0){
            // end of file
            break;
        }
        // 根据stream id获取解码器上下文
        AVCodecContext*cc = pkt->stream_index == audio_stream ? audioCodecContext : videoCodecContext;
        // 发送到线程中解码
        re = avcodec_send_packet(cc, pkt);

        if (re != 0){
            LOGE("avcodec_send_packet failed!");
            continue;
        }

        while (re >= 0)
        {
            re = avcodec_receive_frame(cc, frame);
            if (re == AVERROR(EAGAIN) || re == AVERROR_EOF)
            {
//                LOGE("avcodec_receive_frame AVERROR(EAGAIN) ||  AVERROR_EOF");
                break;
            } else if (re < 0)
            {
                LOGE("failed: avcodec_receive_frame error");
                return env->NewStringUTF(hello.c_str());
            }
            // 视频帧
            if (pkt->stream_index == video_stream)
            {
                // 开始像素格式转换
                uint8_t  *data[AV_NUM_DATA_POINTERS] = {0};
                data[0] = (uint8_t *)rgb;
                int lines[AV_NUM_DATA_POINTERS] = {0};
                lines[0] = outWidth * 4;
                int h = sws_scale(swsContext,
                                  (const uint8_t* const*)frame->data,
                                  frame->linesize,
                                  0,
                                  videoCodecContext->height,
                                  data, lines);
                LOGE("video pts: %f h: %d", frame->pts * r2d(ps->streams[audio_stream]->time_base), h);

                // rgb播放指令
                // ./ffplay -f rawvideo -pixel_format rgba -video_size 320*180 ./testV.rgb
                fwrite(rgb, 1, (size_t) (outWidth * outHeight * 4), fileV);

                read_frame_count++;
            } else if (pkt->stream_index == audio_stream)
            {
                continue;
                uint8_t  *out[2] = {0};
                out[0] = (uint8_t*)pcm;

//                // 获取对应参数的采样需要占用的内存大小 每一个单独的声道需要的大小
//                int out_buffer_size = av_samples_get_buffer_size(NULL,
//                                                                 av_get_channel_layout_nb_channels(2),
//                                                                 frame->nb_samples,
//                                                                 AV_SAMPLE_FMT_S16, 1);
//                // 存储pcm数据
//                uint8_t *out_buffer = (uint8_t*)av_malloc((size_t) out_buffer_size);
                // 重采样
                int len = swr_convert(swrContext, out, frame->nb_samples,
                            (const uint8_t **) frame->data, frame->nb_samples);

                // 获取对应参数的采样需要占用的内存大小
                int out_buffer_size = av_samples_get_buffer_size(NULL,
                                                                 av_get_channel_layout_nb_channels(2),
                                                                 frame->nb_samples,
                                                                 AV_SAMPLE_FMT_S16, 1);
                // 写入文件
                fwrite(pcm, 1, (size_t) out_buffer_size, file);

                LOGI("len: %d  frame->nb_samples：%d out_buffer_size: %d pts: %f",
                     len,
                     frame->nb_samples,
                     out_buffer_size,
                     frame->pts * r2d(ps->streams[audio_stream]->time_base));

//                av_free(out_buffer);
//                out_buffer = NULL;
            }
        }
        // 清理pkt
//        LOGI("av_read_frame: pts: %lld  dts: %lld streamindex: %d duration: %lld", pkt->pts, pkt->dts, pkt->stream_index, pkt->duration);
        av_packet_unref(pkt);
//        av_frame_unref(frame);
    }
    delete pcm;
    delete rgb;

    swr_free(&swrContext);
    sws_freeContext(swsContext);

    av_packet_free(&pkt);
    av_frame_free(&frame);
    pkt = NULL;
    frame = NULL;
    // 关闭写入文件
    fclose(file);
    fclose(fileV);
    // 关闭AVFormatContext
    avformat_close_input(&ps);
    if (ps == NULL){
        hello += "close success";
    } else{
        hello += "close filed";
    }

    LOGE("end");

    return env->NewStringUTF(hello.c_str());
}

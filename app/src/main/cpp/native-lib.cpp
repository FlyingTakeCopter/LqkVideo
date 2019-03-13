#include <jni.h>
#include <string>
#include <sstream>

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/error.h>
}

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

    // 注册解封装
    av_register_all();
    // 注册网络
    avformat_network_init();
    // 打开文件
    AVFormatContext *ps = NULL;
    char p[] = "http://ksy.fffffive.com/mda-hinp1ik37b0rt1mj/mda-hinp1ik37b0rt1mj.mp4";
    int re = avformat_open_input(&ps, p, 0, 0);
    if (re == 0){
        OSS_FORMAT("avformat_open_input success : ", p)
        hello = OSS_STR;
    } else{
        OSS_FORMAT("avformat_open_input failed : ", av_err2str(re))
        hello = OSS_STR;
        return env->NewStringUTF(hello.c_str());
    }

    // 手动探测 媒体信息 比如flv h264等不包含头的数据
    if (avformat_find_stream_info(ps, 0) < 0){
        OSS_FORMAT("avformat_find_stream_info failed : ", av_err2str(re))
        hello = OSS_STR;
        return env->NewStringUTF(hello.c_str());
    }

    OSS_CLEAR // 清空 (oss.clear() 是清除错误位 不能清空)
    OSS_FORMAT(" ", " ")

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
    AVPacket* pkt = av_packet_alloc();
    for (;;) {
        int re = av_read_frame(ps, pkt);
        if (re != 0){
            int pos = 5 * r2d(ps->streams[video_stream]->time_base);
            av_seek_frame(ps, video_stream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            continue;
        }

    }


    avformat_close_input(&ps);
    if (ps == NULL){
        hello += "close success";
    } else{
        hello += "close filed";
    }

    return env->NewStringUTF(hello.c_str());
}

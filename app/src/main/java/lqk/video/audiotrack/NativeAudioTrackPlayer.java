package lqk.video.audiotrack;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;

import java.io.File;

/**
 * Created by liuqikang on 2019/3/26.
 * 音频播放器AudioTrackPlayer
 */

public class NativeAudioTrackPlayer {

    private static final int BITS_PER_BYTE = 8;
    private static final int BITS_PER_CHANNEL = 16;//S16
    private static final int CHANNEL_PER_FRAME = 2;

    private MusicDecoder musicDecoder;

    private AudioTrack audioTrack;
    // 样本率
    int sampleRateInHz;
    //比特率
    int bitRate;
    // 声道类型 立体声
    int channelConfig = AudioFormat.CHANNEL_OUT_STEREO;
    // 音频类型 PCM 16 bit per sample
    int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
    // 每秒的字节数 根据bitRate计算
    int mp3CapacityPerSec = -1;
    // 总字节数
    long totalCapacity = -1;
    // 总时长
    int duration = -1;

    private void createAudioTrack(){
        // 计算最小buffer
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz,channelConfig,audioFormat);
        // 根据文件信息初始化
        audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                sampleRateInHz,
                channelConfig,
                audioFormat,
                bufferSizeInBytes,
                AudioTrack.MODE_STREAM);
    }

    private void createDecoder(){
        musicDecoder = new MusicDecoder();
        musicDecoder.init();
    }

    public boolean setAudioDataSource(String path){
        createDecoder();
        int[] metaArray = new int[]{0, 0};
        musicDecoder.getMusicMetaByPath(path, metaArray);
        this.sampleRateInHz = metaArray[0];
        this.bitRate = metaArray[1];
        if (sampleRateInHz <= 0 || bitRate <= 0){
            return false;
        }
        // 计算时长
        totalCapacity = (new File(path)).length();
        mp3CapacityPerSec = bitRate / BITS_PER_BYTE;
        if (mp3CapacityPerSec == 0){
            return false;
        }
        duration = (int) (totalCapacity / mp3CapacityPerSec);



        return true;
    }

    public void start(){

    }

    public void stop(){

    }
}

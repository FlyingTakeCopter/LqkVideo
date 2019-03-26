package lqk.video.audiotrack;

import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.AudioTrack;

/**
 * Created by liuqikang on 2019/3/26.
 * 音频播放器AudioTrackPlayer
 */

public class NativeAudioTrackPlayer {

    private MusicDecoder musicDecoder;

    private AudioTrack audioTrack;
    int sampleRateInHz;
    int channelConfig;
    int audioFormat;
    int bufferSizeInBytes;

    private void createAudioTrack(){
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

        return true;
    }

    public void start(){

    }

    public void stop(){

    }
}

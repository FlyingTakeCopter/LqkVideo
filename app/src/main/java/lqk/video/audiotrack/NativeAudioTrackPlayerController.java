package lqk.video.audiotrack;


import android.os.Handler;

/**
 * Created by liuqikang on 2019/3/26.
 * AudioTrackPlayer 的控制器
 */

public class NativeAudioTrackPlayerController {

    private NativeAudioTrackPlayer player;

//    private Timer mTimer;
//    private TimerTask mTimerTask = null;
    private Handler mHandler;

    NativeAudioTrackPlayerController(){
        if (player == null){
            player = new NativeAudioTrackPlayer();
        }
    }

    public void setHandler(Handler handler){
        this.mHandler = handler;
    }

    public boolean setAudioDataSource(String path){
        boolean res = player.setAudioDataSource(path);
        if (res){
            player.prepare();
        }
        return res;
    }

    public void start(){
        if (player != null){
            player.start();
        }
    }

    public void stop(){
        if (player != null){
            player.stop();
        }
    }

//    private void timerStop() {
//        if (mTimer != null) {
//            mTimer.cancel();
//            mTimer = null;
//        }
//        if (mTimerTask != null) {
//            mTimerTask.cancel();
//            mTimerTask = null;
//        }
//    }
//
//    private void timerStart() {
//        if (mTimer == null) {
//            mTimer = new Timer();
//            mTimerTask = new MusicTimerTask();
//            mTimer.schedule(mTimerTask, 0, 50);
//        }
//    }
//
//    class MusicTimerTask extends TimerTask {
//        @Override
//        public void run() {
//            int time = getPlayerCurrentTime();
//            if (time != 0) {
//                mHandler.sendMessage(mHandler.obtainMessage(
//                        UPDATE_PLAY_VOICE_PROGRESS, time, isPlaying ? 1 : 0));
//            }
//        }
//    }
}

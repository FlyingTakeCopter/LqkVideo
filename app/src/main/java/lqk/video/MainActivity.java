package lqk.video;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.Button;
import android.widget.TextView;
import android.view.View.OnClickListener;
import android.util.Log;
import android.view.View;
import android.os.Handler;
import android.os.Message;

public class MainActivity extends AppCompatActivity {
    private static String TAG = "MainActivity";

    private Button audioTrackPlayBtn;
    private Button audioTrackStopBtn;
    private Button openSLESPlayBtn;
    private Button openSLESStopBtn;
    /** 要播放的文件路径 **/
    private static String playFilePath = "/mnt/sdcard/v1080.mp4";

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        tv.setText(stringFromJNI());

        findView();
        bindListener();
    }

    private void findView() {
        audioTrackPlayBtn = (Button) findViewById(R.id.play_audiotrack_btn);
        audioTrackStopBtn = (Button) findViewById(R.id.stop_audiotrack_btn);
        openSLESPlayBtn = (Button) findViewById(R.id.play_opensl_es_btn);
        openSLESStopBtn = (Button) findViewById(R.id.stop_opensl_es_btn);
    }

    private void bindListener() {
        audioTrackPlayBtn.setOnClickListener(audioTrackPlayBtnListener);
        audioTrackStopBtn.setOnClickListener(audioTrackStopBtnListener);
        openSLESPlayBtn.setOnClickListener(openSLESPlayBtnListener);
        openSLESStopBtn.setOnClickListener(openSLESStopBtnListener);
    }

    private NativeMp3PlayerController audioTrackPlayerController;
    OnClickListener audioTrackPlayBtnListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Log.i(TAG, "Click AudioTrack Play Btn");
            audioTrackPlayerController = new NativeMp3PlayerController();
            audioTrackPlayerController.setHandler(handler);
            audioTrackPlayerController.setAudioDataSource(playFilePath);
            audioTrackPlayerController.start();
        }
    };

    OnClickListener audioTrackStopBtnListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Log.i(TAG, "Click AudioTrack Stop Btn");
            // 普通AudioTrack的停止播放
            if (null != audioTrackPlayerController) {
                audioTrackPlayerController.stop();
                audioTrackPlayerController = null;
            }
        }
    };

    private SoundTrackController openSLPlayerController;
    OnClickListener openSLESPlayBtnListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Log.i(TAG, "Click OpenSL ES Play Btn");
            // OpenSL EL初始化播放器
            openSLPlayerController = new SoundTrackController();
            openSLPlayerController.setAudioDataSource(playFilePath, 0.2f);
            // OpenSL EL进行播放
            openSLPlayerController.play();
        }
    };

    OnClickListener openSLESStopBtnListener = new OnClickListener() {

        @Override
        public void onClick(View v) {
            Log.i(TAG, "Click OpenSL ES Stop Btn");
            if (null != openSLPlayerController) {
                openSLPlayerController.stop();
                openSLPlayerController = null;
            }
        }
    };

    private Handler handler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            // 计算当前时间
            int _time = Math.max(msg.arg1, 0) / 1000;
            int total_time = Math.max(msg.arg2, 0) / 1000;
            float ratio = (float) _time / (float) total_time;
            Log.i(TAG, "Play Progress : " + ratio);
        }
    };

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String stringFromJNI();
}

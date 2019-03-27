package lqk.video.audiotrack;

/**
 * Created by liuqikang on 2019/3/26.
 */

public class MusicDecoder {

    // 初始化
    public void init(){

    }

    // 销毁native
    public void destory(){

    }

    // 读取样本
    public int readSamples(){

        return -1;
    }

    // 读取参数
    public int getMusicMetaByPath(String musicPath, int[] metaArray){
        return getMusicMeta(musicPath, metaArray);
    }

    /** 第一步进行获取伴奏文件的meta信息，一个是采样率一个是比特率 **/
    private native int getMusicMeta(String musicPath, int[] metaArray);
    /** 最开始init的时候进行打开伴奏文件 **/
    private native int openFile(String accompanyPath, float packetBufferTimePercent);
    /** 打开文件之后进行读取samples处理 **/
    private native int readSamples(short[] samples, int size, int[] slientSizeArr);
    /** 最终当歌曲结束的时候 关闭文件  **/
    private native void closeFile();


}

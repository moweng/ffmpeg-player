#include "ffmpegpalyer.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include "demuxthread.h"
#include "decodethread.h"
#include "audiooutput.h"
#include "avframequeue.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavdevice/avdevice.h>
#include <libavformat/version.h>
#include <libavutil/time.h>
#include <libavutil/mathematics.h>
}

int main(int argc, char *argv[])
{
    // 集成 SDL 测试 https://juejin.cn/post/7215796935298531384
  /*  SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("SDL Test", 640, 480, SDL_WINDOW_FULLSCREEN
    );
    SDL_Delay(13000);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;*/
    int ret = 0;
    //QApplication a(argc, argv);
 /*   ffmpegpalyer w;
    w.show();
    const char* pstrFFmpegConfig = av_version_info();
    std::cout << "config111111:=====================" << pstrFFmpegConfig << std::endl;*/

    // 初始化解复用线程
    AVPacketQueue audio_packet_queue;
    AVPacketQueue video_packet_queue;
    DemuxThread *demux_thread = new DemuxThread(&audio_packet_queue, &video_packet_queue);
    ret = demux_thread->Init(argv[1]);
    av_log_set_level(AV_LOG_DEBUG);
    if (ret < 0) {
        printf("demux_thread.Init failed \n");
        return -1;
    }
    
    ret = demux_thread->Start();

    if (ret < 0) {
        printf("demux_thread.Start failed \n");
        return -1;
    }
    
    // 初始化音解码线程
    AVFrameQueue audio_frame_queue;
    DecodeThread *audio_decode_thread = new DecodeThread(&audio_packet_queue, &audio_frame_queue);
    AVCodecParameters *audioCodecs = demux_thread->AudioCodecParameters();

    ret = audio_decode_thread->Init(audioCodecs);
    if (ret < 0) {
        printf("audio_decode_thread.Init failed \n");
        return -1;
    }
    ret = audio_decode_thread->Start();

    if (ret < 0) {
        printf("audio_decode_thread.Start failed \n");
        return -1;
    }

    //初始化视频解码线程
    //AVFrameQueue video_frame_queue;
    //DecodeThread* video_decode_thread = new DecodeThread(&video_packet_queue, &video_frame_queue);
    //AVCodecParameters* videoCodecs = demux_thread->VideoCodecParameters();

    //ret = video_decode_thread->Init(videoCodecs);
  /*  if (ret < 0) {
        printf("video_decode_thread.Init failed \n");
        return -1;
    }
    ret = video_decode_thread->Start();*/

    if (ret < 0) {
        printf("video_decode_thread.Start failed \n");
        return -1;
    }

    // 初始化 audio 输出
    AudioParams audio_params = { 0 };
    memset(&audio_params, 0, sizeof(AudioParams));
    audio_params.channels = audioCodecs->ch_layout.nb_channels;
    // 将 int channel_layout 数值转化为AVChannelLayout
    audio_params.channel_layout = audioCodecs->ch_layout;

    audio_params.fmt = (enum AVSampleFormat)audioCodecs->format;
    audio_params.freq = audioCodecs->sample_rate;
    audio_params.frame_size = audioCodecs->frame_size;
    AudioOutput* audio_output = new AudioOutput(audio_params, &audio_frame_queue);
    ret = audio_output->Init();

    if (ret < 0) {
        printf("audio_output.Init failed \n");
        return -1;
    }

    // 休眠2s
    std::this_thread::sleep_for(std::chrono::milliseconds(125000));
    demux_thread->Stop();
    audio_decode_thread->Stop();
    //video_decode_thread->Stop();
    delete demux_thread;
    delete audio_decode_thread;
    //delete video_decode_thread;
    
    //return a.exec();
}

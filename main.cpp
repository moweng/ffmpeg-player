#include "ffmpegpalyer.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include "demuxthread.h"
#include "decodethread.h"
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
    ret = audio_decode_thread->Init(demux_thread->AudioCodecParameters());
    if (ret < 0) {
        printf("audio_decode_thread.Init failed \n");
        return -1;
    }
    ret = audio_decode_thread->Start();

    if (ret < 0) {
        printf("audio_decode_thread.Start failed \n");
        return -1;
    }


    // 休眠2s
    std::this_thread::sleep_for(std::chrono::milliseconds(11000));
    demux_thread->Stop();
 
    //return a.exec();
}

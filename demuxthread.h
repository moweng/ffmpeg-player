#pragma once

#include "thread.h"
#include "log.h"
#include "avpacketqueue.h"

extern "C" {
	#include <libavformat/avformat.h>
	#include <libavutil/avutil.h>
	#include <libavcodec/avcodec.h>
}

class DemuxThread: public Thread
{
public:
	DemuxThread(AVPacketQueue *audio_queue, AVPacketQueue *video_queue);
	~DemuxThread();
	int Init(const char* url);
	int Start();
	int Stop();
	void Run();
	AVCodecParameters *AudioCodecParameters();
	AVCodecParameters *VideoCodecParameters();
protected:
	std::string url_; // 文件名
	char err2str[256] = { 0 }; // 错误码
	 AVPacketQueue *audio_queue_ = NULL;
	 AVPacketQueue *video_queue_ = NULL;
	AVFormatContext *ifmt_ctx_ = NULL;
	Log *log_ = new Log();
	int audio_index_ = -1; // 音频的索引
	int video_index_ = -1; // 视频的索引
};


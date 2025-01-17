#pragma once
#include "thread.h"
#include "avpacketqueue.h"
#include "avframequeue.h"
#include "log.h"

class DecodeThread: public Thread
{
public:
	DecodeThread(AVPacketQueue *packet_queue, AVFrameQueue *frame_queue);
	~DecodeThread();
	int Init(AVCodecParameters* par);
	int Start();
	int Stop();
	void Run();
protected:
	Log *log_ = new Log();
	AVCodecContext* codec_ctx_ = NULL; // 解码上下文
	AVPacketQueue* packet_queue_ = NULL;
	AVFrameQueue* frame_queue_ = NULL;
};


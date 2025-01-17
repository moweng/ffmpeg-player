#pragma once
#include "queue.h"
extern "C" {
#include <libavcodec/avcodec.h>
}


class AVFrameQueue
{
public:
	AVFrameQueue();
	~AVFrameQueue();
	void Abort();
	int Push(AVFrame *frame);
	AVFrame* Pop(const int timeout);
	int Size();

	AVFrame* Front();
private:
	void release();
	Queue<AVFrame *> queue_;
};


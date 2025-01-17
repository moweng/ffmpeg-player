#pragma once
#include "queue.h"
extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

class AVPacketQueue
{
public:
	AVPacketQueue();
	~AVPacketQueue();
	void Abort();
	void Release(); // �ͷ���Դ
	int Size();
	int Push(AVPacket *val);
	AVPacket* Pop(const int timeout);
private:
	Queue<AVPacket*>queue_;

};


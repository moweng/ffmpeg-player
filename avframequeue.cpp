#include "avframequeue.h"

AVFrameQueue::AVFrameQueue()
{
}

AVFrameQueue::~AVFrameQueue()
{
	AVFrameQueue::release();
}

void AVFrameQueue::Abort()
{
	release();
	queue_.Abort();
}

int AVFrameQueue::Push(AVFrame* frame)
{
	AVFrame* tmp_frame = av_frame_alloc();
	av_frame_move_ref(tmp_frame, frame);
	return queue_.Push(tmp_frame);
}

AVFrame* AVFrameQueue::Pop(const int timeout)
{
	AVFrame* tmp_frame = NULL;
	int ret = queue_.Pop(tmp_frame, timeout);
	if (ret < 0) {
		if (ret == -1) {
			printf("fte AVFrame::Pop \n");
		}
	}
	return tmp_frame;
}

int AVFrameQueue::Size()
{
	return queue_.Size();
}

AVFrame* AVFrameQueue::Front()
{
	AVFrame* tmp_frame = NULL;
	int ret = queue_.Pop(tmp_frame);
	if (ret < 0) {
		if (ret == -1) {
			printf("fte AVFrame::Front Failed \n");
		}
	}
	return tmp_frame;
}

void AVFrameQueue::release()
{
	AVFrame* frame = NULL;
	int ret = 0;
	while (true) {
		ret = queue_.Pop(frame, 1);
		if (ret >= 0) {
			av_frame_free(&frame);
			continue;
		}
		else {
			break;
		}
	}
}

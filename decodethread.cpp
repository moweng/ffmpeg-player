#include "decodethread.h"
extern "C" {
	#include <libavcodec/avcodec.h>
}

DecodeThread::DecodeThread(AVPacketQueue* packet_queue, AVFrameQueue* frame_queue): packet_queue_(packet_queue),frame_queue_(frame_queue)
{
}

DecodeThread::~DecodeThread()
{
	if (thread_) {
		Stop();
	}
	// 释放掉解码上下文
	if (codec_ctx_) {
		//avcodec_close(codec_ctx_);
	}
}

int DecodeThread::Init(AVCodecParameters* par)
{
	if (!par) {
		printf("fte DecodeThread::Init par is null \n");
		return -1;
	}
	codec_ctx_ = avcodec_alloc_context3(NULL);
	int ret = avcodec_parameters_to_context(codec_ctx_, par);
	log_->logFailedRetToString(ret, "avcodec_parameters_to_context");
	if (ret != 0) {
		return -1;
	}
	// 查找解码器
	const AVCodec* codec = avcodec_find_decoder(codec_ctx_->codec_id);
	if (!codec) {
		printf("fte avcodec_find_decoder find no decoder \n");
		return -1;
	}

	// 打开解码器
	ret = avcodec_open2(codec_ctx_, codec, NULL);
	log_->logFailedRetToString(ret, "avcodec_open2");
	if (ret != 0) {
		return -1;
	}
	printf("fte DecodeThread::Init finish");

	return ret;
}

int DecodeThread::Start()
{
	// 主要是创建解码线程
	thread_ = new std::thread(&DecodeThread::Run, this);
	if (!thread_) {
		printf("fte DecodeThread new std::thread failed \n");
		return -1;
	}

	return 0;
}

int DecodeThread::Stop()
{
	Thread::Stop();
	return 0;
}

void DecodeThread::Run()
{
	printf("DecodeThread::Run into \n");
	AVFrame* frame = av_frame_alloc();
	
	while (abort_ != 1) {
		AVPacket* pkt = packet_queue_->Pop(10);
		if (pkt) {
			// 将数据包发送到解码器
			int ret = avcodec_send_packet(codec_ctx_, pkt);
			if (ret < 0) {
				log_->logFailedRetToString(ret, "avcodec_send_packet");
				break;
			}
			// 读取帧
			while (true) {
				ret = avcodec_receive_frame(codec_ctx_, frame);
				if (ret == 0) {
					frame_queue_->Push(frame);
					continue;
				}
				else if(ret == AVERROR(EAGAIN)){
					break;
				}
				else {
					abort_ = 1;
					log_->logFailedRetToString(ret, "avcodec_receive_frame");
					break;
				}
			}

		}
		else {
			printf("fte not packet");
		}

	}
	printf("DecodeThread::Run finish \n");
	

}

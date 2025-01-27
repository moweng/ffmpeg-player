#include "decodethread.h"
extern "C" {
	#include <libavcodec/avcodec.h>
#include<libavutil/avutil.h>
#include<libavformat/avformat.h>
}

void print_decoder_context_info(AVCodecContext* dec_ctx) {
	if (!dec_ctx) {
		fprintf(stderr, "Decoder context is NULL.\n");
		return;
	}

	// 通道数
	//printf("Channels: %d\n", dec_ctx->channels);

	// 采样率
	printf("Sample Rate: %d Hz\n", dec_ctx->sample_rate);

	// 数据格式（如 AV_SAMPLE_FMT_FLTP, AV_SAMPLE_FMT_S16 等）
	printf("Sample Format: %s\n", av_get_sample_fmt_name(dec_ctx->sample_fmt));

	 //通道布局
	//printf("Channel Layout: 0x%" PRIx64 "\n", dec_ctx->ch_layout.nb_channels);
	//	dec_ctx->channel_layout,
	//	av_get_channel_layout_string((char[128]) { 0 }, 128, dec_ctx->channels, dec_ctx->channel_layout));
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
	print_decoder_context_info(codec_ctx_);
	

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
			if (pkt->pts == AV_NOPTS_VALUE || pkt->dts == AV_NOPTS_VALUE) {
				printf("Invalid timestamps\n");
			}
			printf("Packet size: %d, pts: %lld\n", pkt->size, pkt->pts);
			int ret = avcodec_send_packet(codec_ctx_, pkt);
			if (ret < 0) {
				log_->logFailedRetToString(ret, "avcodec_send_packet");
				break;
			}
			// 读取帧
			while (true) {
				ret = avcodec_receive_frame(codec_ctx_, frame);
				if (ret == 0) {
					/*printf("Decoded frame: sample_rate=%d, channels=%d, format=%d\n",
						frame->sample_rate, frame->format);*/
					frame_queue_->Push(frame);
					printf("Decoded frame size:%s %d \n", codec_ctx_->codec->name,frame_queue_->Size());
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
			av_packet_free(&pkt);

		}
		else {
			//printf("fte not packet \n");
		}

	}
	printf("DecodeThread::Run finish \n");
	

}

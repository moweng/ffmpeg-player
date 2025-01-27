#include "demuxthread.h"


DemuxThread::DemuxThread(AVPacketQueue* audio_queue, AVPacketQueue* video_queue): audio_queue_(audio_queue), video_queue_(video_queue)
{
}

DemuxThread::~DemuxThread()
{
}

int DemuxThread::Init(const char* url)
{
	printf("fte DemuxThread::Init %s \n", url);
	int ret = 0;
	url_ = url;
	ifmt_ctx_ = avformat_alloc_context();
	ret = avformat_open_input(&ifmt_ctx_, url_.c_str(), NULL, NULL);
	log_->logFailedRetToString(ret, "avformat_open_input");
	

	ret = avformat_find_stream_info(ifmt_ctx_, NULL);
	log_->logFailedRetToString(ret, "avformat_find_stream_inf");
	av_dump_format(ifmt_ctx_, 0, url_.c_str(), 0);

	// 查找音频流河视频流的index
	audio_index_ = av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	video_index_ = av_find_best_stream(ifmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (audio_index_ < 0 || video_index_ < 0) {
		printf("fte av_find_best_stream failed \n");
		return -1;
	}
	printf("fte av_find_best_stream audioIndex: %d, videoIndex: %d \n", audio_index_, video_index_);
	
	return 0;
}

int DemuxThread::Start()
{
	thread_ = new std::thread(&Thread::Run, this);
	if (!thread_) {
		printf("new std::thread, failed");
		return -1;
	}
	return 0;
}

int DemuxThread::Stop()
{
	Thread::Stop();
	avformat_close_input(&ifmt_ctx_);
	return 0;
}

void DemuxThread::Run()
{
	printf("fte DemuxThread Run into \n");
	int ret = 0;
	AVPacket pkt;
	while (abort_ != 1) {
		if (audio_queue_->Size() > 50 || video_queue_->Size() > 50) {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			continue;
		}
		ret = av_read_frame(ifmt_ctx_, &pkt);
		if (ret < 0) {
			log_->logFailedRetToString(ret, "av_read_pkt_frame");
			break;
		}
		if (pkt.stream_index == audio_index_) {
			audio_queue_->Push(&pkt);
			printf("fte audio_pkt_queue_ size %d \n", audio_queue_->Size());

		}
	/*	else if (pkt.stream_index == video_index_) {
			video_queue_->Push(&pkt);
			printf("fte video_pkt_queue_ size %d \n", video_queue_->Size());
		}*/
		av_packet_unref(&pkt);
	}
	printf("fte Run finish \n");
}

AVCodecParameters* DemuxThread::AudioCodecParameters()
{
	if (audio_index_ == -1) {
		return NULL;
	}
	else {
		return ifmt_ctx_->streams[audio_index_]->codecpar;
	}
}

AVCodecParameters* DemuxThread::VideoCodecParameters()
{
	if (video_index_ == -1) {
		return NULL;
	}
	else {
		return ifmt_ctx_->streams[video_index_]->codecpar;
	}
}

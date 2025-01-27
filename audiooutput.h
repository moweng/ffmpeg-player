#pragma once
#include "avframequeue.h"


extern "C" {
#include "libswresample/swresample.h" // 重采样
}

typedef struct AudioParams {
	int freq; // 采样率
	int channels;
	AVChannelLayout *channel_layout; // 通道布局
	enum  AVSampleFormat fmt;
	int frame_size;

} AudioParams;
class AudioOutput

{
public:
	AudioOutput(const AudioParams &audio_params, AVFrameQueue* frame_queue);
	~AudioOutput();
	int Init();
	int DeInit();
	AudioParams src_tgt_; // 解码后的参数；
	AudioParams dst_tgt_; // SDL 实际输出的格式
	AVFrameQueue *frame_queue_ = NULL;
	struct SwrContext *swr_ctx_ = NULL;
	uint8_t* audio_buf_ = NULL;
	uint8_t* audio_buf1_ = NULL;
	uint32_t audio_buf_size = 0; // buf 大小
	uint32_t audio_buf1_size = 0;
	uint32_t audio_buf_index = 0; // buffer 索引
};


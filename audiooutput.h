#pragma once
#include "avframequeue.h"


extern "C" {
#include "libswresample/swresample.h" // �ز���
}

typedef struct AudioParams {
	int freq; // ������
	int channels;
	AVChannelLayout *channel_layout; // ͨ������
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
	AudioParams src_tgt_; // �����Ĳ�����
	AudioParams dst_tgt_; // SDL ʵ������ĸ�ʽ
	AVFrameQueue *frame_queue_ = NULL;
	struct SwrContext *swr_ctx_ = NULL;
	uint8_t* audio_buf_ = NULL;
	uint8_t* audio_buf1_ = NULL;
	uint32_t audio_buf_size = 0; // buf ��С
	uint32_t audio_buf1_size = 0;
	uint32_t audio_buf_index = 0; // buffer ����
};


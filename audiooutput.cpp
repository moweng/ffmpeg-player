#include "audiooutput.h"
#include <SDL.h>

AudioOutput::AudioOutput(const AudioParams& audio_params, AVFrameQueue *frame_queue): src_tgt_(audio_params), frame_queue_(frame_queue)
{
}

AudioOutput::~AudioOutput()
{
}


// 从 buffer 中读取数据到stream, len 为 stream 字节流的长度
void fill_audio_pcm(void* data, Uint8* stream, int len) {
	// 1. 从frame_queue 读取解码后的pcm数据，填充到stream;
	// 2. len =4000字节，一个frame有6000字节，一次读取4000，frame 还剩2000， 需要额外的buffer保存
	AudioOutput *is = (AudioOutput *)data;
	int len1 = 0;
	int audio_size = 0;
	while (len > 0) {
		// 说明 bufer 中的数据已经读取完，需要从 frame_queue 中重新读取数据到buf
		if (is->audio_buf_index == is->audio_buf_size) {
			is->audio_buf_index = 0;
			AVFrame* frame = is->frame_queue_->Pop(10);
			if (frame) {
				// 读取到解码后的数据
				// 判断要不要做重采样：采样格式发生变化
				if (
					(
						(frame->format != is->dst_tgt_.fmt) 
						|| (frame->sample_rate != is->dst_tgt_.freq)
						|| av_channel_layout_compare(&frame->ch_layout, &is->dst_tgt_.channel_layout)
						
					) // channel_layout 可判断，ffmpeg 的版本暂时没有找到 channel_layout
					&& (!is->swr_ctx_) // 采样器没有初始) 
				){
					// 创建采样器
					int ret = swr_alloc_set_opts2(&is->swr_ctx_,&is->dst_tgt_.channel_layout, (enum AVSampleFormat)is->dst_tgt_.fmt,
						is->dst_tgt_.freq, &frame->ch_layout, (enum AVSampleFormat)frame->format,frame->sample_rate,
						0, NULL);
					if (!is->swr_ctx_ || swr_init(is->swr_ctx_) < 0){
						printf("fte swr_ctw 初始化异常");
						swr_free((SwrContext**)is->swr_ctx_);
						return;
					}
				}
				if (is->swr_ctx_) {
					const uint8_t** in = (const uint8_t**)frame->extended_data;
					uint8_t** out = &is->audio_buf1_;
					// 输出的样本数
					int out_samples = frame->nb_samples * is->dst_tgt_.freq / frame->sample_rate + 256;
					int out_bytes = av_samples_get_buffer_size(NULL, is->dst_tgt_.channels, out_samples, is->dst_tgt_.fmt, 0);
					if (out_bytes < 0) {
						printf("fte av_samples_get_buffer_size failed");
						return;
					}
					av_fast_malloc(&is->audio_buf1_, &is->audio_buf1_size, out_bytes);
					int len2 = swr_convert(is->swr_ctx_, out, out_samples, in, frame->nb_samples);
					if (len2 < 0) {
						printf("fte swr_convert failed");
						return;
					}
					is->audio_buf_ = is->audio_buf1_;
					is->audio_buf_size = av_samples_get_buffer_size(NULL, is->dst_tgt_.channels, len2, is->dst_tgt_.fmt, 0);

				}
				else {
					// 没有重采样
					// 计算音频缓冲区的大小
					audio_size = av_samples_get_buffer_size(NULL, frame->ch_layout.nb_channels, frame->nb_samples, (enum AVSampleFormat)frame->format, 1);
					av_fast_malloc(&is->audio_buf1_, &is->audio_buf1_size, audio_size);
					is->audio_buf_ = is->audio_buf1_;
					is->audio_buf_size = audio_size;
					memcpy(is->audio_buf_, frame->data[0], audio_size);
				}
				av_frame_free(&frame);
			}else {
				// 读取不到解码后的数据
				is->audio_buf_ = NULL;
				is->audio_buf_size = 512; // 为什么需要这样设置
			}
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if (len1 > len) {
			len1 = len;
		}
		if (!is->audio_buf_) {
			// 将0复制到stream 前n的位置
			memset(stream, 0, len1);
		}else {
			// 真正拷贝数据
			memcpy(stream, is->audio_buf_ + is->audio_buf_index, len1);

		}
		len -= len1;
		stream += len1;
		is->audio_buf_index += len1;
	}
}

int AudioOutput::Init()
{
	if (SDL_Init(SDL_INIT_AUDIO) != 0) {
		printf("fte SDL_Init failed");
		return -1;
	}

	SDL_AudioSpec wanted_spec, spec;
	wanted_spec.channels = src_tgt_.channels;
	wanted_spec.freq = src_tgt_.freq;
	wanted_spec.format = AUDIO_S16SYS; // TODO, 查这个flag的含义
	wanted_spec.silence = 0;
	wanted_spec.callback = fill_audio_pcm;
	wanted_spec.userdata = this;
	wanted_spec.samples = src_tgt_.frame_size; // 采样数量
	int ret = SDL_OpenAudio(&wanted_spec, &spec);
	if (ret < 0) {
		printf("fte SDL_OpenAudio failed \n");
		return -1;
	}
	dst_tgt_.channels = spec.channels;
	dst_tgt_.fmt = AV_SAMPLE_FMT_S16;
	dst_tgt_.freq = spec.freq;
	//dst_tgt_.channel_layout = av_channel_layout_default(src_tgt_.channels);
	dst_tgt_.frame_size = src_tgt_.frame_size;
	/*av_channel_layout_default(dst_tgt_.channel_layout, src_tgt_.channels);*/
	av_channel_layout_default(&dst_tgt_.channel_layout, src_tgt_.channel_layout.nb_channels);
	SDL_PauseAudio(0);
	printf("fte AudioOutput::Init leave");
	return 0;
}

int AudioOutput::DeInit()
{
	SDL_PauseAudio(1);
	SDL_CloseAudio();
	return 0;
}

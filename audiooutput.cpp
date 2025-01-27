#include "audiooutput.h"
#include <SDL.h>

AudioOutput::AudioOutput(const AudioParams& audio_params, AVFrameQueue *frame_queue): src_tgt_(audio_params), frame_queue_(frame_queue)
{
}

AudioOutput::~AudioOutput()
{
}


// �� buffer �ж�ȡ���ݵ�stream, len Ϊ stream �ֽ����ĳ���
void fill_audio_pcm(void* data, Uint8* stream, int len) {
	// 1. ��frame_queue ��ȡ������pcm���ݣ���䵽stream;
	// 2. len =4000�ֽڣ�һ��frame��6000�ֽڣ�һ�ζ�ȡ4000��frame ��ʣ2000�� ��Ҫ�����buffer����
	AudioOutput *is = (AudioOutput *)data;
	int len1 = 0;
	int audio_size = 0;
	while (len > 0) {
		// ˵�� bufer �е������Ѿ���ȡ�꣬��Ҫ�� frame_queue �����¶�ȡ���ݵ�buf
		if (is->audio_buf_index == is->audio_buf_size) {
			is->audio_buf_index = 0;
			AVFrame* frame = is->frame_queue_->Pop(10);
			if (frame) {
				// ��ȡ������������
				// �ж�Ҫ��Ҫ���ز�����������ʽ�����仯
				if (
					(
						(frame->format != is->dst_tgt_.fmt) 
						|| (frame->sample_rate != is->dst_tgt_.freq)
						|| av_channel_layout_compare(&frame->ch_layout, &is->dst_tgt_.channel_layout)
						
					) // channel_layout ���жϣ�ffmpeg �İ汾��ʱû���ҵ� channel_layout
					&& (!is->swr_ctx_) // ������û�г�ʼ) 
				){
					// ����������
					int ret = swr_alloc_set_opts2(&is->swr_ctx_,&is->dst_tgt_.channel_layout, (enum AVSampleFormat)is->dst_tgt_.fmt,
						is->dst_tgt_.freq, &frame->ch_layout, (enum AVSampleFormat)frame->format,frame->sample_rate,
						0, NULL);
					if (!is->swr_ctx_ || swr_init(is->swr_ctx_) < 0){
						printf("fte swr_ctw ��ʼ���쳣");
						swr_free((SwrContext**)is->swr_ctx_);
						return;
					}
				}
				if (is->swr_ctx_) {
					const uint8_t** in = (const uint8_t**)frame->extended_data;
					uint8_t** out = &is->audio_buf1_;
					// �����������
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
					// û���ز���
					// ������Ƶ�������Ĵ�С
					audio_size = av_samples_get_buffer_size(NULL, frame->ch_layout.nb_channels, frame->nb_samples, (enum AVSampleFormat)frame->format, 1);
					av_fast_malloc(&is->audio_buf1_, &is->audio_buf1_size, audio_size);
					is->audio_buf_ = is->audio_buf1_;
					is->audio_buf_size = audio_size;
					memcpy(is->audio_buf_, frame->data[0], audio_size);
				}
				av_frame_free(&frame);
			}else {
				// ��ȡ��������������
				is->audio_buf_ = NULL;
				is->audio_buf_size = 512; // Ϊʲô��Ҫ��������
			}
		}
		len1 = is->audio_buf_size - is->audio_buf_index;
		if (len1 > len) {
			len1 = len;
		}
		if (!is->audio_buf_) {
			// ��0���Ƶ�stream ǰn��λ��
			memset(stream, 0, len1);
		}else {
			// ������������
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
	wanted_spec.format = AUDIO_S16SYS; // TODO, �����flag�ĺ���
	wanted_spec.silence = 0;
	wanted_spec.callback = fill_audio_pcm;
	wanted_spec.userdata = this;
	wanted_spec.samples = src_tgt_.frame_size; // ��������
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

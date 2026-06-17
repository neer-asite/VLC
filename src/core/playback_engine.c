#include "playback_engine.h"

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

/* Audio buffer for decoded samples */
#define MAX_AUDIOQ_SIZE (32 * 1024)
#define AUDIO_BUFFER_SIZE 384000

/* Forward declaration for audio_packet */
struct audio_packet;
typedef struct audio_packet audio_packet_t;

/* Audio packet structure */
struct audio_packet {
    AVPacket *pkt;
    struct audio_packet *next;
};

/* Decode thread function */
static void *decode_thread(void *arg);

/* Queue audio packet */
static int queue_audio_packet(vlc_playback_t *player, AVPacket *pkt) {
    audio_packet_t *pkt1 = malloc(sizeof(audio_packet_t));
    if (!pkt1) return -1;
    
    pkt1->pkt = av_packet_alloc();
    if (!pkt1->pkt) {
        free(pkt1);
        return -1;
    }
    av_packet_ref(pkt1->pkt, pkt);
    pkt1->next = NULL;
    
    pthread_mutex_lock(&player->mutex);
    
    if (!player->audioq_last) {
        player->audioq_first = player->audioq_last = pkt1;
    } else {
        player->audioq_last->next = pkt1;
        player->audioq_last = pkt1;
    }
    player->audioq_size += pkt1->pkt->size;
    
    pthread_mutex_unlock(&player->mutex);
    
    return 0;
}

/* Get next audio packet */
static AVPacket *get_audio_packet(vlc_playback_t *player) {
    pthread_mutex_lock(&player->mutex);
    
    audio_packet_t *pkt1 = player->audioq_first;
    if (pkt1) {
        player->audioq_first = pkt1->next;
        if (!player->audioq_first) {
            player->audioq_last = NULL;
        }
        player->audioq_size -= pkt1->pkt->size;
    }
    
    pthread_mutex_unlock(&player->mutex);
    
    if (pkt1) {
        AVPacket *pkt = pkt1->pkt;
        free(pkt1);
        return pkt;
    }
    return NULL;
}

/* SDL Audio callback - fetches decoded audio */
static void audio_callback(void *userdata, Uint8 *stream, int len) {
    vlc_playback_t *player = (vlc_playback_t *)userdata;
    if (!player) return;
    
    int16_t *out = (int16_t *)stream;
    
    while (len > 0) {
        if (player->audio_buf_size <= 0) {
            /* Get more audio data */
            AVPacket *pkt = get_audio_packet(player);
            if (!pkt) {
                /* No packet, generate silence or get from decode thread */
                memset(out, 0, len);
                return;
            }
            
            /* Decode audio */
            AVFrame *frame = av_frame_alloc();
            int ret = avcodec_send_packet(player->audio_ctx, pkt);
            av_packet_free(&pkt);
            
            if (ret < 0) {
                av_frame_free(&frame);
                memset(out, 0, len);
                return;
            }
            
            ret = avcodec_receive_frame(player->audio_ctx, frame);
            if (ret < 0) {
                av_frame_free(&frame);
                player->audio_buf_size = 0;
                continue;
            }
            
            /* Convert to interleaved S16 */
            uint8_t *conv_buf = player->audio_buf;
            int conv_buf_size = AUDIO_BUFFER_SIZE;
            
            if (player->swr_ctx) {
                int out_samples = av_rescale_rnd(
                    swr_get_delay(player->swr_ctx, player->audio_ctx->sample_rate) + frame->nb_samples,
                    44100,
                    player->audio_ctx->sample_rate,
                    AV_ROUND_UP);
                
                if (out_samples > conv_buf_size / 4) {
                    out_samples = conv_buf_size / 4;
                }
                
                ret = swr_convert(player->swr_ctx, &conv_buf, out_samples,
                                  (const uint8_t **)frame->data, frame->nb_samples);
                
                if (ret > 0) {
                    player->audio_buf_size = ret * 2 * 2; /* stereo * 2 bytes */
                    player->audio_buf_ptr = player->audio_buf;
                }
            } else {
                /* Direct copy if no resampling needed */
                memcpy(conv_buf, frame->data[0], frame->linesize[0]);
                player->audio_buf_size = frame->linesize[0];
                player->audio_buf_ptr = player->audio_buf;
            }
            
            av_frame_free(&frame);
        }
        
        if (player->audio_buf_size > 0) {
            int copy_size = player->audio_buf_size;
            if (copy_size > len) copy_size = len;
            
            int16_t *src = (int16_t *)(player->audio_buf_ptr);
            int16_t *dst = out;
            int samples = copy_size / 2;
            
            /* Apply volume */
            for (int i = 0; i < samples; i++) {
                float sample = src[i] * player->volume_factor;
                if (sample > 32767) sample = 32767;
                if (sample < -32768) sample = -32768;
                dst[i] = (int16_t)sample;
            }
            
            player->audio_buf_ptr += copy_size;
            player->audio_buf_size -= copy_size;
            out += copy_size / 2;
            len -= copy_size;
        } else {
            break;
        }
    }
    
    if (len > 0) {
        memset(out, 0, len);
    }
}

/* Decode thread */
static void *decode_thread(void *arg) {
    vlc_playback_t *player = (vlc_playback_t *)arg;
    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    
    while (player->running) {
        /* Check if we need more data */
        pthread_mutex_lock(&player->mutex);
        if (player->audioq_size > MAX_AUDIOQ_SIZE) {
            pthread_mutex_unlock(&player->mutex);
            usleep(10000);
            continue;
        }
        pthread_mutex_unlock(&player->mutex);
        
        /* Read packet */
        if (av_read_frame(player->format_ctx, pkt) < 0) {
            /* End of file */
            break;
        }
        
        if (pkt->stream_index == player->audio_stream) {
            queue_audio_packet(player, pkt);
        } else {
            av_packet_free(&pkt);
        }
    }
    
    av_frame_free(&frame);
    av_packet_free(&pkt);
    
    return NULL;
}

vlc_playback_t *vlc_playback_create(void) {
    vlc_playback_t *player = calloc(1, sizeof(vlc_playback_t));
    if (!player) return NULL;
    
    player->state = VLC_STATE_IDLE;
    player->volume = 80;
    player->volume_factor = 0.8f;
    player->speed = 1.0f;
    
    pthread_mutex_init(&player->mutex, NULL);
    pthread_cond_init(&player->cond, NULL);
    
    /* Initialize SDL */
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL init failed: %s\n", SDL_GetError());
    }
    
    return player;
}

void vlc_playback_destroy(vlc_playback_t *player) {
    if (!player) return;
    
    pthread_mutex_lock(&player->mutex);
    player->running = false;
    pthread_cond_broadcast(&player->cond);
    pthread_mutex_unlock(&player->mutex);
    
    /* Wait for decode thread */
    if (player->decode_thread) {
        pthread_join(player->decode_thread, NULL);
    }
    
    /* Close SDL audio */
    if (player->audio_device) {
        SDL_CloseAudioDevice(player->audio_device);
    }
    
    /* Close FFmpeg */
    if (player->audio_ctx) {
        avcodec_free_context(&player->audio_ctx);
    }
    if (player->format_ctx) {
        avformat_close_input(&player->format_ctx);
    }
    
    SDL_Quit();
    
    free(player->current_uri);
    pthread_mutex_destroy(&player->mutex);
    pthread_cond_destroy(&player->cond);
    free(player);
}

int vlc_playback_open_uri(vlc_playback_t *player, const char *uri) {
    if (!player || !uri) return VLC_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&player->mutex);
    
    /* Stop any running playback */
    player->running = false;
    if (player->decode_thread) {
        pthread_join(player->decode_thread, NULL);
        player->decode_thread = 0;
    }
    
    /* Close previous file */
    if (player->audio_ctx) {
        avcodec_free_context(&player->audio_ctx);
        player->audio_ctx = NULL;
    }
    if (player->format_ctx) {
        avformat_close_input(&player->format_ctx);
        player->format_ctx = NULL;
    }
    if (player->swr_ctx) {
        swr_free(&player->swr_ctx);
        player->swr_ctx = NULL;
    }
    
    /* Clear audio queue */
    while (player->audioq_first) {
        AVPacket *pkt = get_audio_packet(player);
        if (pkt) av_packet_free(&pkt);
    }
    player->audio_buf_size = 0;
    
    free(player->current_uri);
    player->current_uri = strdup(uri);
    player->position = 0;
    player->duration = 0;
    player->audio_stream = -1;
    
    /* Open input file */
    AVFormatContext *fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, uri, NULL, NULL) < 0) {
        fprintf(stderr, "Cannot open file: %s\n", uri);
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_FILE_NOT_FOUND;
    }
    
    player->format_ctx = fmt_ctx;
    
    /* Find stream info */
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Cannot find stream info\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_DEMUXER;
    }
    
    /* Find audio stream */
    for (unsigned int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            player->audio_stream = i;
            break;
        }
    }
    
    if (player->audio_stream < 0) {
        fprintf(stderr, "No audio stream found\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_UNSUPPORTED_FORMAT;
    }
    
    /* Get codec */
    AVStream *stream = fmt_ctx->streams[player->audio_stream];
    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_DECODER;
    }
    
    /* Create codec context */
    player->audio_ctx = avcodec_alloc_context3(codec);
    if (!player->audio_ctx) {
        fprintf(stderr, "Cannot allocate codec context\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_NO_MEMORY;
    }
    
    if (avcodec_parameters_to_context(player->audio_ctx, stream->codecpar) < 0) {
        fprintf(stderr, "Cannot copy codec params\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_DECODER;
    }
    
    if (avcodec_open2(player->audio_ctx, codec, NULL) < 0) {
        fprintf(stderr, "Cannot open codec\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_DECODER;
    }
    
    /* Set up resampler for uniform output format (44100 Hz, stereo, S16) */
    player->swr_ctx = swr_alloc();
    if (!player->swr_ctx) {
        fprintf(stderr, "Cannot allocate resampler\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_NO_MEMORY;
    }
    
    av_opt_set_int(player->swr_ctx, "in_channel_layout", 
        av_get_default_channel_layout(player->audio_ctx->channels), 0);
    av_opt_set_int(player->swr_ctx, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(player->swr_ctx, "in_sample_rate", player->audio_ctx->sample_rate, 0);
    av_opt_set_int(player->swr_ctx, "out_sample_rate", 44100, 0);
    av_opt_set_sample_fmt(player->swr_ctx, "in_sample_fmt", player->audio_ctx->sample_fmt, 0);
    av_opt_set_sample_fmt(player->swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);
    
    if (swr_init(player->swr_ctx) < 0) {
        fprintf(stderr, "Cannot initialize resampler\n");
        swr_free(&player->swr_ctx);
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_DECODER;
    }
    
    /* Get duration */
    if (fmt_ctx->duration != AV_NOPTS_VALUE) {
        player->duration = fmt_ctx->duration / 1000; /* Convert to ms */
    } else {
        player->duration = 0;
    }
    
    player->state = VLC_STATE_LOADED;
    pthread_mutex_unlock(&player->mutex);
    
    printf("Opened: %s\n", uri);
    printf("Duration: %lld ms\n", (long long)player->duration);
    printf("Codec: %s (%d Hz, %d channels)\n", 
           codec->name, player->audio_ctx->sample_rate, 
           player->audio_ctx->channels);
    
    return VLC_SUCCESS;
}

int vlc_playback_start(vlc_playback_t *player) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&player->mutex);
    
    if (player->state == VLC_STATE_IDLE) {
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_NOT_INITIALIZED;
    }
    
    /* Open SDL audio device if not already open */
    if (!player->audio_device) {
        SDL_AudioSpec want, have;
        memset(&want, 0, sizeof(want));
        want.freq = 44100;
        want.format = AUDIO_S16SYS;
        want.channels = 2;
        want.samples = 4096;
        want.callback = audio_callback;
        want.userdata = player;
        
        player->audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
        if (!player->audio_device) {
            fprintf(stderr, "SDL audio open failed: %s\n", SDL_GetError());
        } else {
            memcpy(&player->audio_spec, &have, sizeof(SDL_AudioSpec));
            printf("Audio device: %d Hz, %d channels\n", have.freq, have.channels);
        }
    }
    
    /* Start decode thread */
    player->running = true;
    if (!player->decode_thread) {
        pthread_create(&player->decode_thread, NULL, decode_thread, player);
    }
    
    player->state = VLC_STATE_PLAYING;
    
    if (player->audio_device) {
        SDL_PauseAudioDevice(player->audio_device, 0);
    }
    
    pthread_mutex_unlock(&player->mutex);
    
    printf("Playing...\n");
    return VLC_SUCCESS;
}

int vlc_playback_pause_playback(vlc_playback_t *player) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&player->mutex);
    player->state = VLC_STATE_PAUSED;
    if (player->audio_device) {
        SDL_PauseAudioDevice(player->audio_device, 1);
    }
    pthread_mutex_unlock(&player->mutex);
    
    printf("Paused\n");
    return VLC_SUCCESS;
}

int vlc_playback_stop_playback(vlc_playback_t *player) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&player->mutex);
    player->running = false;
    player->position = 0;
    player->state = VLC_STATE_STOPPED;
    
    if (player->audio_device) {
        SDL_PauseAudioDevice(player->audio_device, 1);
    }
    
    pthread_cond_broadcast(&player->cond);
    pthread_mutex_unlock(&player->mutex);
    
    printf("Stopped\n");
    return VLC_SUCCESS;
}

int vlc_playback_seek_to(vlc_playback_t *player, int64_t timestamp) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    
    pthread_mutex_lock(&player->mutex);
    
    if (!player->format_ctx) {
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_NOT_INITIALIZED;
    }
    
    if (timestamp < 0) timestamp = 0;
    if (timestamp > player->duration) timestamp = player->duration;
    
    int64_t seek_pos = timestamp * AV_TIME_BASE / 1000;
    if (av_seek_frame(player->format_ctx, player->audio_stream, seek_pos, AVSEEK_FLAG_BACKWARD) < 0) {
        fprintf(stderr, "Seek failed\n");
        pthread_mutex_unlock(&player->mutex);
        return VLC_ERROR_GENERIC;
    }
    
    player->position = timestamp;
    player->state = VLC_STATE_SEEKING;
    pthread_mutex_unlock(&player->mutex);
    
    printf("Seeked to %lld ms\n", (long long)timestamp);
    return VLC_SUCCESS;
}

int64_t vlc_playback_get_pos(vlc_playback_t *player) {
    if (!player) return 0;
    int64_t pos;
    pthread_mutex_lock(&player->mutex);
    pos = player->position;
    pthread_mutex_unlock(&player->mutex);
    return pos;
}

int64_t vlc_playback_get_dur(vlc_playback_t *player) {
    if (!player) return 0;
    return player->duration;
}

vlc_state_t vlc_playback_get_state(vlc_playback_t *player) {
    if (!player) return VLC_STATE_IDLE;
    vlc_state_t state;
    pthread_mutex_lock(&player->mutex);
    state = player->state;
    pthread_mutex_unlock(&player->mutex);
    return state;
}

int vlc_playback_set_vol(vlc_playback_t *player, int volume) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    
    pthread_mutex_lock(&player->mutex);
    player->volume = volume;
    player->volume_factor = player->mute ? 0.0f : (float)volume / 100.0f;
    pthread_mutex_unlock(&player->mutex);
    
    return VLC_SUCCESS;
}

int vlc_playback_get_vol(vlc_playback_t *player) {
    if (!player) return 0;
    int vol;
    pthread_mutex_lock(&player->mutex);
    vol = player->volume;
    pthread_mutex_unlock(&player->mutex);
    return vol;
}
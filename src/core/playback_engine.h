#ifndef PLAYBACK_ENGINE_H
#define PLAYBACK_ENGINE_H

#include "../../include/vlc.h"
#include <pthread.h>

/* Forward declarations for FFmpeg types */
struct AVFormatContext;
struct AVCodecContext;
struct SwrContext;

struct vlc_playback {
    vlc_state_t state;
    char *current_uri;
    int64_t position;
    int64_t duration;
    int volume;
    bool mute;
    float speed;
    bool running;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    
    /* FFmpeg context */
    struct AVFormatContext *format_ctx;
    struct AVCodecContext *audio_ctx;
    int audio_stream;
    struct SwrContext *swr_ctx;
    
    /* Audio queue */
    struct audio_packet *audioq_first;
    struct audio_packet *audioq_last;
    int audioq_size;
    
    /* Audio buffer */
    uint8_t audio_buf[384000];
    uint8_t *audio_buf_ptr;
    int audio_buf_size;
    
    /* SDL audio */
    void *audio_device;
    void *audio_spec;
    
    /* Decode thread */
    pthread_t decode_thread;
    
    /* Volume control */
    float volume_factor;
};

vlc_playback_t *vlc_playback_create(void);
void vlc_playback_destroy(vlc_playback_t *player);
int vlc_playback_open_uri(vlc_playback_t *player, const char *uri);
int vlc_playback_start(vlc_playback_t *player);
int vlc_playback_pause_playback(vlc_playback_t *player);
int vlc_playback_stop_playback(vlc_playback_t *player);
int vlc_playback_seek_to(vlc_playback_t *player, int64_t timestamp);
int64_t vlc_playback_get_pos(vlc_playback_t *player);
int64_t vlc_playback_get_dur(vlc_playback_t *player);
vlc_state_t vlc_playback_get_state(vlc_playback_t *player);
int vlc_playback_set_vol(vlc_playback_t *player, int volume);
int vlc_playback_get_vol(vlc_playback_t *player);

#endif
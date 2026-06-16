/**
 * VLC - VLC-Style Media Player for Linux
 * Public API Header
 */

#ifndef VLC_H
#define VLC_H

#include <stdint.h>
#include <stdbool.h>

#define VLC_VERSION_MAJOR 1
#define VLC_VERSION_MINOR 0
#define VLC_VERSION_PATCH 0

typedef enum {
    VLC_SUCCESS = 0,
    VLC_ERROR_GENERIC = -1,
    VLC_ERROR_INVALID_PARAM = -2,
    VLC_ERROR_NO_MEMORY = -3,
    VLC_ERROR_FILE_NOT_FOUND = -4,
    VLC_ERROR_UNSUPPORTED_FORMAT = -5,
    VLC_ERROR_DECODER = -6,
    VLC_ERROR_DEMUXER = -7,
    VLC_ERROR_PLAYBACK = -8,
    VLC_ERROR_NOT_INITIALIZED = -9
} vlc_error_t;

typedef enum {
    VLC_EVENT_PLAY, VLC_EVENT_PAUSE, VLC_EVENT_STOP, VLC_EVENT_SEEK,
    VLC_EVENT_POSITION_CHANGED, VLC_EVENT_STATE_CHANGED, VLC_EVENT_MEDIA_LOADED,
    VLC_EVENT_MEDIA_ENDED, VLC_EVENT_ERROR, VLC_EVENT_VOLUME_CHANGED
} vlc_event_type_t;

typedef enum {
    VLC_STATE_IDLE, VLC_STATE_LOADED, VLC_STATE_PLAYING, VLC_STATE_PAUSED,
    VLC_STATE_STOPPED, VLC_STATE_SEEKING, VLC_STATE_BUFFERING, VLC_STATE_ENDED
} vlc_state_t;

typedef struct vlc_playback vlc_playback_t;
typedef struct vlc_playlist vlc_playlist_t;
typedef struct vlc_config vlc_config_t;
typedef void (*vlc_event_callback_t)(vlc_event_type_t event, void *data, void *user_data);

int vlc_init(void);
void vlc_shutdown(void);

vlc_playback_t *vlc_playback_create(void);
void vlc_playback_destroy(vlc_playback_t *player);
int vlc_playback_open(vlc_playback_t *player, const char *uri);
int vlc_playback_play(vlc_playback_t *player);
int vlc_playback_pause(vlc_playback_t *player);
int vlc_playback_stop(vlc_playback_t *player);
int vlc_playback_seek(vlc_playback_t *player, int64_t timestamp);
int64_t vlc_playback_get_position(vlc_playback_t *player);
int64_t vlc_playback_get_duration(vlc_playback_t *player);
vlc_state_t vlc_playback_get_state(vlc_playback_t *player);
int vlc_playback_set_volume(vlc_playback_t *player, int volume);
int vlc_playback_get_volume(vlc_playback_t *player);

vlc_playlist_t *vlc_playlist_create(void);
void vlc_playlist_destroy(vlc_playlist_t *playlist);
int vlc_playlist_add(vlc_playlist_t *playlist, const char *uri);
int vlc_playlist_remove(vlc_playlist_t *playlist, int index);
int vlc_playlist_clear(vlc_playlist_t *playlist);
int vlc_playlist_get_count(vlc_playlist_t *playlist);
int vlc_playlist_next(vlc_playlist_t *playlist);
int vlc_playlist_prev(vlc_playlist_t *playlist);

vlc_config_t *vlc_config_create(void);
void vlc_config_destroy(vlc_config_t *config);
int vlc_config_load(vlc_config_t *config, const char *path);
int vlc_config_save(vlc_config_t *config, const char *path);
int vlc_config_get_int(vlc_config_t *config, const char *key, int default_val);
int vlc_config_set_int(vlc_config_t *config, const char *key, int value);

int vlc_event_register(vlc_playback_t *player, vlc_event_type_t type,
                       vlc_event_callback_t callback, void *user_data);

#endif /* VLC_H */
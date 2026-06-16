#include "playback_engine.h"
#include <stdlib.h>
#include <string.h>

vlc_playback_t *vlc_playback_create(void) {
    vlc_playback_t *player = calloc(1, sizeof(vlc_playback_t));
    if (!player) return NULL;
    player->state = VLC_STATE_IDLE;
    player->volume = 80;
    player->speed = 1.0f;
    pthread_mutex_init(&player->mutex, NULL);
    pthread_cond_init(&player->cond, NULL);
    return player;
}

void vlc_playback_destroy(vlc_playback_t *player) {
    if (!player) return;
    if (player->running) {
        player->running = false;
        pthread_cond_broadcast(&player->cond);
    }
    free(player->current_uri);
    pthread_mutex_destroy(&player->mutex);
    pthread_cond_destroy(&player->cond);
    free(player);
}

int vlc_playback_open_uri(vlc_playback_t *player, const char *uri) {
    if (!player || !uri) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&player->mutex);
    free(player->current_uri);
    player->current_uri = strdup(uri);
    player->position = 0;
    player->duration = 0;  /* Would be set by demuxer */
    player->state = VLC_STATE_LOADED;
    pthread_mutex_unlock(&player->mutex);
    return VLC_SUCCESS;
}

int vlc_playback_start(vlc_playback_t *player) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&player->mutex);
    player->running = true;
    player->state = VLC_STATE_PLAYING;
    pthread_mutex_unlock(&player->mutex);
    return VLC_SUCCESS;
}

int vlc_playback_pause_playback(vlc_playback_t *player) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&player->mutex);
    player->state = VLC_STATE_PAUSED;
    pthread_mutex_unlock(&player->mutex);
    return VLC_SUCCESS;
}

int vlc_playback_stop_playback(vlc_playback_t *player) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&player->mutex);
    player->running = false;
    player->position = 0;
    player->state = VLC_STATE_STOPPED;
    pthread_cond_broadcast(&player->cond);
    pthread_mutex_unlock(&player->mutex);
    return VLC_SUCCESS;
}

int vlc_playback_seek_to(vlc_playback_t *player, int64_t timestamp) {
    if (!player) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&player->mutex);
    if (timestamp < 0) timestamp = 0;
    if (timestamp > player->duration) timestamp = player->duration;
    player->position = timestamp;
    player->state = VLC_STATE_SEEKING;
    pthread_mutex_unlock(&player->mutex);
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
#include "playlist.h"
#include "../../include/vlc.h"
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

static playlist_item_t *item_create(const char *uri) {
    if (!uri) return NULL;
    playlist_item_t *item = calloc(1, sizeof(playlist_item_t));
    if (!item) return NULL;
    item->uri = strdup(uri);
    char *copy = strdup(uri);
    char *file = basename(copy);
    if (file) {
        char *dot = strrchr(file, '.');
        if (dot) *dot = '\0';
        item->title = strdup(file);
    }
    free(copy);
    return item;
}

vlc_playlist_t *vlc_playlist_create(void) {
    vlc_playlist_t *pl = calloc(1, sizeof(vlc_playlist_t));
    if (pl) pthread_mutex_init(&pl->mutex, NULL);
    return pl;
}

void vlc_playlist_destroy(vlc_playlist_t *pl) {
    if (!pl) return;
    vlc_playlist_clear(pl);
    pthread_mutex_destroy(&pl->mutex);
    free(pl);
}

int vlc_playlist_add(vlc_playlist_t *pl, const char *uri) {
    if (!pl || !uri) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&pl->mutex);
    playlist_item_t *item = item_create(uri);
    if (!item) { pthread_mutex_unlock(&pl->mutex); return VLC_ERROR_NO_MEMORY; }
    if (pl->tail) { item->prev = pl->tail; pl->tail->next = item; pl->tail = item; }
    else pl->head = pl->tail = item;
    pl->count++;
    pthread_mutex_unlock(&pl->mutex);
    return VLC_SUCCESS;
}

int vlc_playlist_remove(vlc_playlist_t *pl, int index) {
    if (!pl || index < 0 || index >= pl->count) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&pl->mutex);
    playlist_item_t *item = pl->head;
    for (int i = 0; i < index && item; i++) item = item->next;
    if (!item) { pthread_mutex_unlock(&pl->mutex); return VLC_ERROR_INVALID_PARAM; }
    if (item->prev) item->prev->next = item->next;
    else pl->head = item->next;
    if (item->next) item->next->prev = item->prev;
    else pl->tail = item->prev;
    if (pl->current == item) pl->current = item->next ? item->next : item->prev;
    pl->count--;
    free(item->uri); free(item->title); free(item);
    pthread_mutex_unlock(&pl->mutex);
    return VLC_SUCCESS;
}

int vlc_playlist_clear(vlc_playlist_t *pl) {
    if (!pl) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&pl->mutex);
    playlist_item_t *item = pl->head;
    while (item) {
        playlist_item_t *next = item->next;
        free(item->uri); free(item->title); free(item);
        item = next;
    }
    pl->head = pl->tail = pl->current = NULL;
    pl->count = pl->current_index = 0;
    pthread_mutex_unlock(&pl->mutex);
    return VLC_SUCCESS;
}

int vlc_playlist_get_count(vlc_playlist_t *pl) {
    if (!pl) return 0;
    int count;
    pthread_mutex_lock(&pl->mutex);
    count = pl->count;
    pthread_mutex_unlock(&pl->mutex);
    return count;
}

playlist_item_t *vlc_playlist_get_item(vlc_playlist_t *pl, int index) {
    if (!pl || index < 0 || index >= pl->count) return NULL;
    playlist_item_t *item = pl->head;
    for (int i = 0; i < index && item; i++) item = item->next;
    return item;
}

playlist_item_t *vlc_playlist_get_current(vlc_playlist_t *pl) {
    if (!pl) return NULL;
    pthread_mutex_lock(&pl->mutex);
    playlist_item_t *cur = pl->current;
    pthread_mutex_unlock(&pl->mutex);
    return cur;
}

int vlc_playlist_next(vlc_playlist_t *pl) {
    if (!pl || pl->count == 0) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&pl->mutex);
    if (pl->current_index >= pl->count - 1) {
        if (pl->loop) pl->current_index = 0;
        else { pthread_mutex_unlock(&pl->mutex); return VLC_ERROR_GENERIC; }
    } else pl->current_index++;
    pl->current = vlc_playlist_get_item(pl, pl->current_index);
    pthread_mutex_unlock(&pl->mutex);
    return VLC_SUCCESS;
}

int vlc_playlist_prev(vlc_playlist_t *pl) {
    if (!pl || pl->count == 0) return VLC_ERROR_INVALID_PARAM;
    pthread_mutex_lock(&pl->mutex);
    if (pl->current_index <= 0) pl->current_index = pl->loop ? pl->count - 1 : 0;
    else pl->current_index--;
    pl->current = vlc_playlist_get_item(pl, pl->current_index);
    pthread_mutex_unlock(&pl->mutex);
    return VLC_SUCCESS;
}
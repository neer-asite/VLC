#ifndef PLAYLIST_H
#define PLAYLIST_H

#include <pthread.h>
#include <stdbool.h>

typedef struct playlist_item {
    char *uri;
    char *title;
    struct playlist_item *next;
    struct playlist_item *prev;
} playlist_item_t;

typedef struct vlc_playlist {
    playlist_item_t *head;
    playlist_item_t *tail;
    playlist_item_t *current;
    int count;
    int current_index;
    bool loop;
    bool shuffle;
    pthread_mutex_t mutex;
} vlc_playlist_t;

vlc_playlist_t *vlc_playlist_create(void);
void vlc_playlist_destroy(vlc_playlist_t *playlist);
int vlc_playlist_add(vlc_playlist_t *playlist, const char *uri);
int vlc_playlist_remove(vlc_playlist_t *playlist, int index);
int vlc_playlist_clear(vlc_playlist_t *playlist);
int vlc_playlist_get_count(vlc_playlist_t *playlist);
playlist_item_t *vlc_playlist_get_item(vlc_playlist_t *playlist, int index);
playlist_item_t *vlc_playlist_get_current(vlc_playlist_t *playlist);
int vlc_playlist_next(vlc_playlist_t *playlist);
int vlc_playlist_prev(vlc_playlist_t *playlist);

#endif
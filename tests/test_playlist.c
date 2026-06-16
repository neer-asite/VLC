#include <stdio.h>
#include <assert.h>
#include "../src/playlist/playlist.h"

int test_count = 0, test_passed = 0;

#define ASSERT(cond) do { test_count++; if (cond) test_passed++; else printf("FAIL: %s\n", #cond); } while(0)

void test_playlist_create(void) {
    vlc_playlist_t *pl = vlc_playlist_create();
    ASSERT(pl != NULL);
    ASSERT(vlc_playlist_get_count(pl) == 0);
    vlc_playlist_destroy(pl);
}

void test_playlist_add(void) {
    vlc_playlist_t *pl = vlc_playlist_create();
    vlc_playlist_add(pl, "/path/to/video.mp4");
    ASSERT(vlc_playlist_get_count(pl) == 1);
    vlc_playlist_add(pl, "/path/to/audio.mp3");
    ASSERT(vlc_playlist_get_count(pl) == 2);
    vlc_playlist_destroy(pl);
}

void test_playlist_navigation(void) {
    vlc_playlist_t *pl = vlc_playlist_create();
    vlc_playlist_add(pl, "/path/to/file1.mp4");
    vlc_playlist_add(pl, "/path/to/file2.mp4");
    vlc_playlist_add(pl, "/path/to/file3.mp4");
    
    playlist_item_t *item = vlc_playlist_get_item(pl, 0);
    ASSERT(item != NULL);
    ASSERT(strcmp(item->uri, "/path/to/file1.mp4") == 0);
    
    vlc_playlist_next(pl);
    vlc_playlist_destroy(pl);
}

int main(void) {
    printf("=== VLC Playlist Tests ===\n");
    test_playlist_create();
    test_playlist_add();
    test_playlist_navigation();
    printf("\nTests: %d/%d passed\n", test_passed, test_count);
    return test_passed == test_count ? 0 : 1;
}
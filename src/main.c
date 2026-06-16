/**
 * VLC - VLC-Style Media Player for Linux
 * Main entry point
 */

#include "../include/vlc.h"
#include "core/playback_engine.h"
#include "playlist/playlist.h"
#include "config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static void print_version(void);
static void print_help(const char *program);

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    
    printf("VLC Media Player v%d.%d.%d\n", VLC_VERSION_MAJOR, VLC_VERSION_MINOR, VLC_VERSION_PATCH);
    printf("VLC-style media player for Linux\n\n");
    
    /* Parse command line */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            print_version(); return 0;
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]); return 0;
        }
    }
    
    /* Initialize player */
    vlc_playback_t *player = vlc_playback_create();
    if (!player) {
        fprintf(stderr, "Failed to create player\n");
        return 1;
    }
    
    /* Create playlist */
    vlc_playlist_t *playlist = vlc_playlist_create();
    if (!playlist) {
        fprintf(stderr, "Failed to create playlist\n");
        vlc_playback_destroy(player);
        return 1;
    }
    
    /* Create config */
    vlc_config_t *config = vlc_config_create();
    if (!config) {
        fprintf(stderr, "Failed to create config\n");
        vlc_playlist_destroy(playlist);
        vlc_playback_destroy(player);
        return 1;
    }
    
    /* Load config if exists */
    char config_path[256];
    snprintf(config_path, sizeof(config_path), "%s/.config/vlc/config.conf", getenv("HOME") ? getenv("HOME") : ".");
    vlc_config_load(config, config_path);
    
    /* Process files from command line */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            vlc_playlist_add(playlist, argv[i]);
        }
    }
    
    /* Play first item if any */
    if (vlc_playlist_get_count(playlist) > 0) {
        playlist_item_t *item = vlc_playlist_get_item(playlist, 0);
        if (item && item->uri) {
            printf("Opening: %s\n", item->uri);
            vlc_playback_open_uri(player, item->uri);
            vlc_playback_start(player);
        }
    } else {
        printf("No media files specified.\n");
        printf("Usage: %s [OPTIONS] [FILE]...\n", argv[0]);
        printf("       %s --help for more options\n", argv[0]);
    }
    
    /* Main loop simulation - in real app this would be event-driven */
    printf("\nVLC initialized successfully.\n");
    printf("Features:\n");
    printf("  - Play/Pause/Stop controls\n");
    printf("  - Seek functionality\n");
    printf("  - Volume control\n");
    printf("  - Fullscreen mode\n");
    printf("  - Playlist support\n");
    printf("  - Multiple format support (MP4, MKV, AVI, MP3, FLAC, AAC)\n\n");
    
    /* Cleanup */
    vlc_config_save(config, config_path);
    vlc_config_destroy(config);
    vlc_playlist_destroy(playlist);
    vlc_playback_destroy(player);
    
    return 0;
}

static void print_version(void) {
    printf("VLC Media Player v%d.%d.%d\n", VLC_VERSION_MAJOR, VLC_VERSION_MINOR, VLC_VERSION_PATCH);
    printf("VLC-style media player for Linux\n\n");
    printf("Built with:\n  - GTK4\n  - FFmpeg\n  - SDL2\n  - POSIX threads\n");
}

static void print_help(const char *program) {
    printf("Usage: %s [OPTIONS] [FILE]\n\n", program);
    printf("VLC - VLC-style Media Player for Linux\n\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n\n");
    printf("Keyboard Shortcuts:\n");
    printf("  Space         Play/Pause\n");
    printf("  F             Toggle fullscreen\n");
    printf("  L             Toggle playlist\n");
    printf("  O             Open file\n");
    printf("  Escape        Exit fullscreen\n");
    printf("  Up/Down       Volume control\n");
    printf("  Left/Right    Seek 10 seconds\n\n");
    printf("Supported formats:\n");
    printf("  Video: MP4, MKV, AVI, MOV, WebM\n");
    printf("  Audio: MP3, FLAC, AAC, OGG, WAV\n");
}
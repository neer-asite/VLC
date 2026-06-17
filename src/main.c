/**
 * VLC - VLC-Style Media Player for Linux
 * Main entry point with interactive CLI
 */

#include "../include/vlc.h"
#include "core/playback_engine.h"
#include "playlist/playlist.h"
#include "config/config_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <unistd.h>
#include <termios.h>
#include <sys/select.h>

static void print_version(void);
static void print_help(const char *program);
static char get_key(void);
static int kbhit(void);

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");
    
    printf("\n");
    printf("╔══════════════════════════════════════════╗\n");
    printf("║     VLC Media Player v%d.%d.%d              ║\n", VLC_VERSION_MAJOR, VLC_VERSION_MINOR, VLC_VERSION_PATCH);
    printf("║     VLC-style player for Linux           ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    
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
            vlc_playback_open_uri(player, item->uri);
            vlc_playback_start(player);
        }
    }
    
    /* Interactive control loop */
    printf("\n┌─────────────────────────────────────────┐\n");
    printf("│  Controls:                              │\n");
    printf("│    Space  - Play/Pause                  │\n");
    printf("│    S      - Stop                        │\n");
    printf("│    ←/→    - Seek ±10 seconds            │\n");
    printf("│    ↑/↓    - Volume ±10                  │\n");
    printf("│    M      - Mute toggle                 │\n");
    printf("│    N      - Next track                  │\n");
    printf("│    P      - Previous track              │\n");
    printf("│    Q      - Quit                        │\n");
    printf("│    H      - Show help                   │\n");
    printf("└─────────────────────────────────────────┘\n\n");
    
    struct termios old_tty, new_tty;
    tcgetattr(STDIN_FILENO, &old_tty);
    new_tty = old_tty;
    new_tty.c_lflag &= ~(ICANON | ECHO);
    new_tty.c_cc[VMIN] = 0;
    new_tty.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tty);
    
    int running = 1;
    while (running) {
        if (kbhit()) {
            char ch = get_key();
            vlc_state_t state = vlc_playback_get_state(player);
            
            switch (ch) {
                case ' ':
                case 'p':
                case 'P':
                    if (state == VLC_STATE_PLAYING) {
                        vlc_playback_pause_playback(player);
                    } else if (state == VLC_STATE_PAUSED || state == VLC_STATE_LOADED) {
                        vlc_playback_start(player);
                    }
                    break;
                    
                case 's':
                case 'S':
                    vlc_playback_stop_playback(player);
                    break;
                    
                case 'm':
                case 'M':
                    {
                        int vol = vlc_playback_get_vol(player);
                        if (vol > 0) {
                            vlc_playback_set_vol(player, 0);
                            printf("[Muted]\n");
                        } else {
                            vlc_playback_set_vol(player, 80);
                            printf("[Unmuted - Volume: 80%%]\n");
                        }
                    }
                    break;
                    
                case 27: /* Arrow keys start with ESC */
                    {
                        char ch2 = get_key();
                        if (ch2 == '[') {
                            char ch3 = get_key();
                            if (ch3 == 'A') { /* Up */
                                int vol = vlc_playback_get_vol(player);
                                vlc_playback_set_vol(player, vol + 10);
                                printf("[Volume: %d%%]\n", vol + 10);
                            } else if (ch3 == 'B') { /* Down */
                                int vol = vlc_playback_get_vol(player);
                                vlc_playback_set_vol(player, vol - 10);
                                printf("[Volume: %d%%]\n", vol - 10);
                            } else if (ch3 == 'C') { /* Right */
                                int64_t pos = vlc_playback_get_pos(player);
                                int64_t dur = vlc_playback_get_dur(player);
                                vlc_playback_seek_to(player, pos + 10000);
                                printf("[Position: %lld / %lld ms]\n", (long long)(pos + 10000), (long long)dur);
                            } else if (ch3 == 'D') { /* Left */
                                int64_t pos = vlc_playback_get_pos(player);
                                int64_t dur = vlc_playback_get_dur(player);
                                vlc_playback_seek_to(player, pos - 10000);
                                printf("[Position: %lld / %lld ms]\n", (long long)(pos - 10000), (long long)dur);
                            }
                        }
                    }
                    break;
                    
                case 'n':
                case 'N':
                    if (vlc_playlist_next(playlist) == VLC_SUCCESS) {
                        playlist_item_t *item = vlc_playlist_get_current(playlist);
                        if (item) {
                            vlc_playback_open_uri(player, item->uri);
                            vlc_playback_start(player);
                        }
                    }
                    break;
                    
                case 'q':
                case 'Q':
                    running = 0;
                    break;
                    
                case 'h':
                case 'H':
                    print_help(argv[0]);
                    break;
            }
        }
        
        /* Update position display */
        static int64_t last_pos = 0;
        int64_t pos = vlc_playback_get_pos(player);
        int64_t dur = vlc_playback_get_dur(player);
        
        if (pos != last_pos && pos > 0) {
            printf("\r  Playing: %lld / %lld ms    ", (long long)pos, (long long)dur);
            fflush(stdout);
            last_pos = pos;
        }
        
        usleep(100000); /* 100ms */
    }
    
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tty);
    
    printf("\n\nExiting...\n");
    
    /* Cleanup */
    vlc_config_save(config, config_path);
    vlc_config_destroy(config);
    vlc_playlist_destroy(playlist);
    vlc_playback_destroy(player);
    
    return 0;
}

static int kbhit(void) {
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
}

static char get_key(void) {
    char buf[1];
    if (read(STDIN_FILENO, buf, 1) > 0) {
        return buf[0];
    }
    return 0;
}

static void print_version(void) {
    printf("VLC Media Player v%d.%d.%d\n", VLC_VERSION_MAJOR, VLC_VERSION_MINOR, VLC_VERSION_PATCH);
    printf("VLC-style media player for Linux\n\n");
    printf("Built with:\n  - FFmpeg (libavformat, libavcodec)\n  - SDL2\n  - POSIX threads\n");
}

static void print_help(const char *program) {
    printf("\n╔══════════════════════════════════════════╗\n");
    printf("║           MiniVLC Help                  ║\n");
    printf("╚══════════════════════════════════════════╝\n\n");
    printf("Usage: %s [OPTIONS] [FILE]\n\n", program);
    printf("VLC - VLC-style Media Player for Linux\n\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message\n");
    printf("  -v, --version  Show version information\n\n");
    printf("Interactive Controls:\n");
    printf("  Space/p       Play / Pause\n");
    printf("  S             Stop playback\n");
    printf("  M             Toggle mute\n");
    printf("  ← / →         Seek backward / forward 10s\n");
    printf("  ↑ / ↓         Volume up / down 10%%\n");
    printf("  N             Next track in playlist\n");
    printf("  P             Previous track\n");
    printf("  Q             Quit\n");
    printf("  H             Show this help\n\n");
    printf("Supported formats:\n");
    printf("  Video: MP4, MKV, AVI, MOV, WebM\n");
    printf("  Audio: MP3, FLAC, AAC, OGG, WAV\n\n");
}
# VLC Architecture Documentation

## Overview

VLC is a VLC-style media player for Linux written in C17, following a modular architecture with clear separation of concerns.

## System Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                           UI Layer                               │
│   (GTK4 + SDL2 for video)                                        │
└───────────────────────────────┬─────────────────────────────────┘
                                │
┌───────────────────────────────┴─────────────────────────────────┐
│                       Playback Engine                            │
│   (State machine, thread management)                            │
└───────┬─────────────────────┬─────────────────────┬─────────────┘
        │                     │                     │
┌───────▼───────┐    ┌───────▼───────┐    ┌───────▼───────┐
│    Demuxer    │    │Audio Decoder  │    │Video Decoder  │
│   (FFmpeg)    │    │   (FFmpeg)    │    │   (FFmpeg)    │
└───────────────┘    └───────┬───────┘    └───────┬───────┘
                            │                     │
                    ┌───────▼───────┐             │
                    │Audio Output  │             │
                    │   (SDL2)     │             │
                    └───────────────┘             │
                                                  │
                    ┌─────────────────────────────┘
                    │
            ┌───────▼───────┐
            │   Subtitle    │
            │   Engine      │
            └───────────────┘
```

## Module Descriptions

### Core Module (`src/core/`)
- **playback_engine.c/h** - Main playback orchestration
- **playback_state.c/h** - State machine management
- **media_types.h** - Core data type definitions

### Demuxer Module (`src/demuxer/`)
- **demuxer.c/h** - Media format detection and demuxing
- **stream_info.c/h** - Stream metadata handling

### Audio Module (`src/audio/`)
- **audio_decoder.c/h** - FFmpeg audio decoding
- **audio_output.c/h** - SDL2 audio output

### Video Decoder Module (`src/decoders/`)
- **video_decoder.c/h** - FFmpeg video decoding
- **frame_queue.c/h** - Decoded frame buffering

### Subtitle Module (`src/subtitle/`)
- **subtitle_parser.c/h** - Subtitle format parsing (SRT, ASS)
- **subtitle_renderer.c/h** - Subtitle overlay rendering

### Playlist Module (`src/playlist/`)
- **playlist.c/h** - Playlist management
- **playlist_item.c/h** - Individual playlist items

### Config Module (`src/config/`)
- **config_manager.c/h** - Configuration persistence
- **recent_files.c/h** - Recent files tracking

### Logging Module (`src/logging/`)
- **logger.c/h** - Thread-safe logging system

### Utils Module (`src/utils/`)
- **thread_utils.c/h** - Thread management
- **ring_buffer.c/h** - Lock-free ring buffer
- **file_utils.c/h** - File operations
- **time_utils.c/h** - Time formatting

### UI Module (`src/ui/`)
- **main_window.c/h** - Main application window
- **video_widget.c/h** - SDL2 video display widget
- **controls.c/h** - Playback control widgets
- **menu_bar.c/h** - Application menu bar
- **playlist_panel.c/h** - Playlist sidebar
- **app.c/h** - GTK Application

## Directory Structure

```
vlc/
├── CMakeLists.txt
├── include/
│   ├── vlc.h              # Public API
│   └── vlc_version.h      # Version info
├── src/
│   ├── core/              # Playback engine
│   ├── demuxer/           # FFmpeg demuxing
│   ├── decoders/          # Audio/Video decoders
│   ├── audio/             # Audio output
│   ├── subtitle/          # Subtitle processing
│   ├── playlist/          # Playlist management
│   ├── config/            # Configuration
│   ├── logging/           # Logging system
│   ├── utils/             # Utilities
│   ├── ui/                # GTK4 UI components
│   └── main.c             # Entry point
├── tests/
│   ├── CMakeLists.txt
│   └── test_*.c           # Unit tests
└── docs/
    ├── ARCHITECTURE.md     # This file
    ├── API.md             # API documentation
    ├── BUILD.md           # Build instructions
    ├── CLEANUP.md         # Code quality report
    └── DEPENDENCIES.md    # Dependency info
```

## Build System

Uses CMake with the following structure:
- Core library (`vlc_core`) containing all modules
- Optional tests enabled with `-DBUILD_TESTS=ON`
- Public headers installed to `include/vlc/`

## Threading Model

- Main thread: GTK event loop
- Decoder threads: Separate threads for audio/video decoding
- Synchronization: POSIX mutexes and condition variables
- A/V Sync: Audio master clock with video frame dropping
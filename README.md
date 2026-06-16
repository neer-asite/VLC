# VLC - VLC-Style Media Player for Linux

A modular, VLC-style media player for Linux written in C17, featuring a clean architecture with separated concerns for UI, playback engine, and media processing.

## Features

- Play/Pause/Stop controls
- Seek functionality
- Volume control
- Fullscreen mode
- Keyboard shortcuts
- Playlist support
- Multiple format support

### Supported Formats
- **Video**: MP4, MKV, AVI, MOV, WebM
- **Audio**: MP3, FLAC, AAC, OGG, WAV
- **Subtitles**: SRT, ASS/SSA, VTT

## Architecture

VLC follows a modular architecture:
- **UI Layer**: GTK4 interface
- **Playback Engine**: State machine, thread management
- **Demuxer**: FFmpeg-based media format detection
- **Decoders**: Audio/Video decoding with FFmpeg

## Dependencies

- GCC/Clang with C17 support
- CMake ≥ 3.16
- FFmpeg libraries (optional)
- SDL2 (optional)
- POSIX threads

## Building

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

## Running

```bash
./vlc /path/to/video.mp4
./vlc --help
```

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| Space | Play/Pause |
| F | Toggle fullscreen |
| Up/Down | Volume control |
| Left/Right | Seek 10 seconds |

## License

GPL-3.0

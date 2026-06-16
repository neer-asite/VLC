# VLC Dependencies

## Required Dependencies

### Build Tools
- **GCC/Clang** with C17 support (GCC 9+, Clang 10+)
- **CMake** ≥ 3.16

### Core Libraries
- **libc** (standard C library)
- **POSIX threads** (libpthread)

## Optional Dependencies

### UI (GTK4)
- **GTK4** ≥ 4.0 - Main UI toolkit
- **libglib-2.0** - GLib utilities
- **libgio-3.0** - GIO file operations

### Multimedia (FFmpeg)
- **libavcodec** - Audio/Video codecs
- **libavformat** - Container format parsing
- **libavutil** - Utility functions
- **libswscale** - Image scaling
- **libswresample** - Audio resampling
- **libavfilter** - Audio/Video filters

### Audio/Video Output
- **SDL2** ≥ 2.0 - Cross-platform multimedia
- **SDL2_ttf** - TrueType font rendering
- **libasound2** - ALSA audio (Linux)
- **libpulse** - PulseAudio (optional)

### Graphics
- **libgl** - OpenGL rendering
- **libx11** - X11 windowing system
- **libxcursor** - Mouse cursor
- **libxinerama** - Multi-monitor

## Installation

### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake \
    libgtk-4-dev \
    libavcodec-dev libavformat-dev libavutil-dev \
    libswscale-dev libswresample-dev \
    libsdl2-dev libsdl2-ttf-dev \
    libasound2-dev libpulse-dev \
    libgl-dev libx11-dev
```

### Fedora/RHEL
```bash
sudo dnf install -y \
    gcc cmake \
    gtk4-devel \
    ffmpeg-devel \
    SDL2-devel SDL2_ttf-devel \
    alsa-lib-devel pulseaudio-libs-devel \
    mesa-libGL-devel libX11-devel
```

### Arch Linux
```bash
sudo pacman -S --needed \
    base-devel cmake \
    gtk4 \
    ffmpeg \
    sdl2 sdl2_ttf \
    alsa-lib \
    mesa libx11
```

## CMake Configuration

The build system auto-detects available dependencies. Set paths manually if needed:

```bash
cmake -DCMAKE_PREFIX_PATH=/custom/path ..
```
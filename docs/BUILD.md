# VLC Build Instructions

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Ubuntu/Debian](#ubuntudebian)
3. [Fedora/RHEL](#fedorarhel)
4. [Arch Linux](#arch-linux)
5. [Building](#building)
6. [Troubleshooting](#troubleshooting)

## Prerequisites

### System Requirements
- Linux (x86_64)
- 2GB RAM minimum
- GCC 9+ or Clang 10+ with C17 support
- CMake 3.16+

### Required Dependencies

| Package | Version | Description |
|---------|---------|-------------|
| CMake | ≥3.16 | Build system |
| GTK4 | ≥4.0 | UI toolkit (optional) |
| FFmpeg libs | ≥5.0 | Media decoding (optional) |
| SDL2 | ≥2.0 | Audio/Video output (optional) |

## Ubuntu/Debian

```bash
# Update package index
sudo apt-get update

# Install build tools
sudo apt-get install -y \
    build-essential \
    cmake \
    pkg-config

# Install GTK4
sudo apt-get install -y libgtk-4-dev

# Install FFmpeg libraries (optional)
sudo apt-get install -y \
    libavcodec-dev \
    libavformat-dev \
    libavutil-dev \
    libswscale-dev \
    libswresample-dev

# Install SDL2 (optional)
sudo apt-get install -y \
    libSDL2-dev \
    libsdl2-ttf-dev

# Install additional dependencies
sudo apt-get install -y \
    libgl-dev \
    libgio-3.0-dev
```

## Fedora/RHEL

```bash
# Install build tools
sudo dnf install -y \
    gcc \
    cmake \
    pkg-config \
    make

# Install GTK4
sudo dnf install -y gtk4-devel

# Install FFmpeg
sudo dnf install -y ffmpeg-devel

# Install SDL2
sudo dnf install -y \
    SDL2-devel \
    SDL2_ttf-devel

# Install OpenGL
sudo dnf install -y mesa-libGL-devel
```

## Arch Linux

```bash
# Update package database
sudo pacman -Sy

# Install build tools
sudo pacman -S --needed \
    base-devel \
    cmake \
    pkg-config

# Install GTK4
sudo pacman -S gtk4

# Install FFmpeg
sudo pacman -S ffmpeg

# Install SDL2
sudo pacman -S sdl2 sdl2_ttf
```

## Building

### Step 1: Clone or Extract Source

```bash
cd /path/to/VLC
```

### Step 2: Create Build Directory

```bash
mkdir -p build
cd build
```

### Step 3: Configure

```bash
cmake ..
```

#### CMake Options

| Option | Default | Description |
|--------|---------|-------------|
| `-DCMAKE_BUILD_TYPE` | Release | Build type (Debug/Release) |
| `-DCMAKE_INSTALL_PREFIX` | /usr/local | Install prefix |
| `-DBUILD_TESTS` | ON | Build unit tests |

### Step 4: Build

```bash
# Multi-threaded (faster)
cmake --build . -j$(nproc)
```

### Step 5: Install

```bash
sudo cmake --install .
```

### Step 6: Run Tests (Optional)

```bash
ctest --output-on-failure
```

## Running

### From Build Directory
```bash
./vlc
```

### With Arguments
```bash
./vlc /path/to/media/file.mp4
./vlc --help
```

## Troubleshooting

### "Could not find GTK4"
Install the GTK4 development package:
- Ubuntu/Debian: `sudo apt-get install libgtk-4-dev`
- Fedora/RHEL: `sudo dnf install gtk4-devel`

### "Could not find FFmpeg"
Install FFmpeg development packages:
- Ubuntu/Debian: `sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev`

### Build Fails with C17 Errors
Ensure you have a recent compiler:
```bash
gcc --version  # Should be 9+
```

## Cleaning Build

```bash
rm -rf build
mkdir build && cd build
cmake ..
cmake --build .
```
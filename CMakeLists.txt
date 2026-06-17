cmake_minimum_required(VERSION 3.16)
project(vlc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build unit tests" ON)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Check for FFmpeg
pkg_check_modules(AVCODEC libavcodec)
pkg_check_modules(AVFORMAT libavformat)
pkg_check_modules(AVUTIL libavutil)
pkg_check_modules(SWRESAMPLE libswresample)

# Check for SDL2
pkg_check_modules(SDL2 SDL2)

set(AVAILABLE_LIBS "")
set(NEEDED_LIBS "")
set(NEEDED_INCLUDES "")

# Build list of libraries
if(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND AND SWRESAMPLE_FOUND)
    list(APPEND AVAILABLE_LIBS "FFmpeg")
    list(APPEND NEEDED_LIBS 
        ${AVCODEC_LIBRARIES} 
        ${AVFORMAT_LIBRARIES} 
        ${AVUTIL_LIBRARIES} 
        ${SWRESAMPLE_LIBRARIES}
    )
    list(APPEND NEEDED_INCLUDES 
        ${AVCODEC_INCLUDE_DIRS} 
        ${AVFORMAT_INCLUDE_DIRS} 
        ${AVUTIL_INCLUDE_DIRS} 
        ${SWRESAMPLE_INCLUDE_DIRS}
    )
endif()

if(SDL2_FOUND)
    list(APPEND AVAILABLE_LIBS "SDL2")
    list(APPEND NEEDED_LIBS ${SDL2_LIBRARIES})
    list(APPEND NEEDED_INCLUDES ${SDL2_INCLUDE_DIRS})
endif()

# Source files
set(SOURCES
    src/main.c
    src/core/playback_engine.c
    src/playlist/playlist.c
    src/config/config_manager.c
)

set(HEADERS
    include/vlc.h
    src/core/playback_engine.h
    src/playlist/playlist.h
    src/config/config_manager.h
)

# Create executable
add_executable(vlc ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(vlc PRIVATE 
    ${CMAKE_SOURCE_DIR}/include
    ${NEEDED_INCLUDES}
)

# Compiler warnings
target_compile_options(vlc PRIVATE -Wall -Wextra)

# Link libraries (order matters: dependent libs first)
target_link_libraries(vlc pthread ${NEEDED_LIBS} m)

# Installation
install(TARGETS vlc RUNTIME DESTINATION bin)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Summary
message(STATUS "VLC Media Player v${PROJECT_VERSION}")
message(STATUS "C Standard: C17")
message(STATUS "Available: ${AVAILABLE_LIBS}")
message(STATUS "NEEDED_LIBS: ${NEEDED_LIBS}")
message(STATUS "SDL2_FOUND: ${SDL2_FOUND}")
message(STATUS "SDL2_LIBRARIES: ${SDL2_LIBRARIES}")
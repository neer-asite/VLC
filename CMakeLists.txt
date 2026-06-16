cmake_minimum_required(VERSION 3.16)
project(vlc VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build unit tests" ON)

# Find dependencies (optional for this simplified version)
find_package(PkgConfig)

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
target_include_directories(vlc PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Compiler warnings
target_compile_options(vlc PRIVATE -Wall -Wextra)

# Link libraries
target_link_libraries(vlc m pthread)

# Installation
install(TARGETS vlc RUNTIME DESTINATION bin)

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Documentation
message(STATUS "VLC Media Player v${PROJECT_VERSION}")
message(STATUS "C Standard: C17")
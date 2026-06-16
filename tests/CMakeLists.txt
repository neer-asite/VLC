enable_testing()
add_executable(vlc_test test_playlist.c)
target_link_libraries(vlc_test m pthread)
add_test(NAME vlc_test COMMAND vlc_test)
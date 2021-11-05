add_executable(sdl2_test demo.c)
target_link_libraries(sdl2_test PUBLIC hagl_hal hagl)

set_target_properties(sdl2_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

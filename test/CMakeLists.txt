add_executable(demo demo.c)
target_link_libraries(demo mapmini fileio_posix hagl hagl_hal) #agg agg_hal)

set_target_properties(demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(demo PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")

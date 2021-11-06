add_executable(demo demo.c)
target_link_libraries(demo hagl hagl_hal mapmini)

set_target_properties(demo PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(hagl_hal INTERFACE)

target_sources(hagl_hal INTERFACE
  ${CMAKE_CURRENT_LIST_DIR}/src/hagl_hal.c
)

target_include_directories(hagl_hal INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

target_link_libraries(hagl_hal INTERFACE hagl SDL2)

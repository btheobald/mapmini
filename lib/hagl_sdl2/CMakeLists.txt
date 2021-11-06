add_library(hagl_hal STATIC)

target_sources(hagl_hal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hagl_hal.c)

target_include_directories(hagl_hal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.)

target_link_libraries(hagl_hal PUBLIC hagl SDL2)

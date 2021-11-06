add_library(mapmini STATIC)

target_include_directories(mapmini PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
set_target_properties(mapmini PROPERTIES PUBLIC_HEADER include/map.h)

add_subdirectory(src)

target_link_libraries(mapmini PRIVATE m fileio_posix)

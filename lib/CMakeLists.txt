# Bypassing git CMakeLists to statically link hagl
    
add_library(hagl STATIC)
target_sources(hagl PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/bitmap.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/clip.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/fontx.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/hagl.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/hsl.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/rgb565.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/rgb888.c
    ${CMAKE_CURRENT_LIST_DIR}/hagl/src/tjpgd.c)
target_include_directories(hagl PUBLIC ${CMAKE_CURRENT_LIST_DIR}/hagl/include)
set_property(TARGET hagl PROPERTY C_STANDARD 99)
set_target_properties(hagl PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
target_link_libraries(hagl PUBLIC hagl_hal)

add_subdirectory(hagl_sdl2)
add_subdirectory(fileio_posix)

add_library(agg STATIC)
target_sources(agg PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/agg-2.4/src/agg_vcgen_stroke.cpp
    ${CMAKE_CURRENT_LIST_DIR}/agg-2.4/src/agg_vcgen_dash.cpp
    ${CMAKE_CURRENT_LIST_DIR}/agg-2.4/src/agg_curves.cpp
    ${CMAKE_CURRENT_LIST_DIR}/agg-2.4/src/agg_rounded_rect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/agg-2.4/src/agg_arc.cpp)
target_include_directories(agg PUBLIC ${CMAKE_CURRENT_LIST_DIR}/agg-2.4/include)
set_target_properties(agg PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")
target_link_libraries(agg)

add_subdirectory(agg_sdl2)


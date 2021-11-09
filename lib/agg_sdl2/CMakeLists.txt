add_library(agg_hal STATIC)

target_sources(agg_hal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/agg_hal.cpp)

target_include_directories(agg_hal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.)

set_target_properties(agg_hal PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32")

target_link_libraries(agg_hal PUBLIC agg SDL2main SDL2 )

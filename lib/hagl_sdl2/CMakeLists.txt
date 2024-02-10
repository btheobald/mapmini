add_library(hagl_hal STATIC)

target_sources(hagl_hal PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hagl_hal.c ${CMAKE_CURRENT_LIST_DIR}/thick.c ${CMAKE_CURRENT_LIST_DIR}/aa.c)

target_include_directories(hagl_hal PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.)

#set_target_properties(hagl_hal PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32 -flto")

target_link_libraries(hagl_hal PUBLIC m hagl)

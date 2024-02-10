add_library(fileio_posix STATIC)

target_sources(fileio_posix PRIVATE ${CMAKE_CURRENT_LIST_DIR}/io_posix.c)

target_include_directories(fileio_posix PUBLIC ${CMAKE_CURRENT_LIST_DIR}/.)

#set_target_properties(fileio_posix PROPERTIES COMPILE_OPTIONS "-m32" LINK_FLAGS "-m32 -flto")

target_link_libraries(fileio_posix)

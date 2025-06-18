add_library(core STATIC core.c)

target_include_directories(core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(core PUBLIC logger::logger global state raylib)

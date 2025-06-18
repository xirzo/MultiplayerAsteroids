add_library(render STATIC render.c)

target_include_directories(render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(render PUBLIC global state raylib)

add_library(state STATIC state.c)

target_include_directories(state PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(state PUBLIC network)

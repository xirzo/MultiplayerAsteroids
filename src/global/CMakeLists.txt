add_library(global INTERFACE)

target_include_directories(global INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(global INTERFACE math)

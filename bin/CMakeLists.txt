add_executable(${PROJECT_NAME} client.c)

target_link_libraries(${PROJECT_NAME} core render)

add_executable(asteroids_server server.c)

target_link_libraries(asteroids_server logger::logger global state network)

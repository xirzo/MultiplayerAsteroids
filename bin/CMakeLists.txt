add_executable(${PROJECT_NAME} client.c)

target_link_libraries(${PROJECT_NAME} logger::logger global state)

add_executable(asteroids_server server.c)

target_link_libraries(asteroids_server logger::logger global state)

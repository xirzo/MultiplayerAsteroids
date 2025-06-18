#ifndef SERVER_H
#define SERVER_H

#include <logger.h>
#include "vec2.h"
#include <netinet/in.h>
#include <pthread.h>
#include <stddef.h>
#include <unistd.h>
#include "settings.h"

typedef enum {
    SERVER_MSG_PLAYER_ID_SET,
    SERVER_MSG_PLAYER_ENTERED,
    SERVER_MSG_ACTIVE_PLAYER_IDS,
    SERVER_MSG_NOT_ENOUGH_PLAYERS,
    SERVER_MSG_GAME_START,
    SERVER_MSG_PLAYER_POSITION,
    SERVER_MSG_PLAYER_DISCONNECT,
} server_message_type;

typedef struct player_posititon_message {
    int player_id;
    vec2 pos;
} player_posititon_message_t;

typedef struct server_message {
    server_message_type type;
    int client_id;
    union {
        player_posititon_message_t player_pos_msg;
        int active_players[MAX_PLAYERS];
        vec2 pos;
        int player_id;
        int needed_players;
    } data;
} server_message_t;

typedef struct client_connection {
    int socket_fd;
    struct sockaddr_in addr;
    int active;
    pthread_t thread_id;
    int client_id;
} client_connection_t;

typedef struct server {
    int fd;
    struct sockaddr_in servaddr;
    socklen_t addrlen;

    client_connection_t clients[MAX_PLAYERS];
    size_t client_count;
    pthread_mutex_t clients_mutex;
    pthread_t server_listen_thread;

    void *(*handle_client_callback)(void *);
} server_t;

typedef struct thread_args {
    server_t *server;
    int client_index;
} thread_args_t;

server_t *sr_create_server(unsigned short port, void *(*handle_client_callback)(void *));
void sr_destroy_server(server_t *server);
void sr_start_listen(server_t *server);
int sr_add_client(server_t *server, int socket_fd, struct sockaddr_in addr);
void sr_send_message_to_all(server_t *server, const server_message_t *message);
void sr_send_message_to_all_except(
    server_t *server,
    int except_client_id,
    const server_message_t *message
);
void sr_send_message_to_client(server_t *server, int client_id, const server_message_t *message);

#endif  // !SERVER_H

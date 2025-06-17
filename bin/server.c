#include "server.h"
#include <arpa/inet.h>
#include <logger.h>
#include <stdlib.h>
#include "client.h"
#include "settings.h"
#include "state.h"

// TODO: remove global state
server_state_t g_State = {};

static void *handle_client(void *arg);

int main(void) {
    FILE *log_file = fopen("server_log", "w");
    // logger_set_output_file(log_file);

    server_t *server = sr_create_server(SERVER_PORT, handle_client);

    if (!server) {
        LOG_ERROR("Failed to start server");
        fclose(log_file);
        return 1;
    }

    sr_start_listen(server);

    while (1);

    fclose(log_file);
    return 0;
}

void *handle_client(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    server_t *server = args->server;
    client_connection_t *con = &server->clients[args->client_index];

    LOG_INFO("Client %d connected from %s", con->client_id, inet_ntoa(con->addr.sin_addr));

    server_message_t srv_msg = {
        .type = SERVER_MSG_PLAYER_ENTERED,
        .data.player_id = con->client_id,
    };

    LOG_INFO("Sending player entered message");
    sr_send_message_to_all_except(server, con->client_id, &srv_msg);

    if (server->client_count < PLAYERS_TO_START) {
        LOG_INFO("Clients count: %d", server->client_count);
        LOG_INFO("Sending not enough players message");

        srv_msg = (server_message_t){
            .type = SERVER_MSG_NOT_ENOUGH_PLAYERS,
            .data.needed_players = PLAYERS_TO_START - server->client_count,
        };

        sr_send_message_to_all(server, &srv_msg);
    } else {
        LOG_INFO("Sending game start message");

        srv_msg = (server_message_t){
            .type = SERVER_MSG_GAME_START,
        };

        sr_send_message_to_all(server, &srv_msg);
    }

    while (con->active) {
        client_message_t cl_msg;
        ssize_t bytes_read = read(con->socket_fd, &cl_msg, sizeof(client_message_t));

        if (bytes_read <= 0) {
            LOG_INFO("Client %d disconnected\n", con->client_id);
            break;
        }

        if (bytes_read == sizeof(client_message_t)) {
            switch (cl_msg.type) {
                /*

                Fill in

                */
                default: {
                    LOG_ERROR(
                        "Unknown message type %d from client %d\n", cl_msg.type, con->client_id
                    );
                    break;
                }
            }
        }
    }

    close(con->socket_fd);
    con->active = 0;

    pthread_mutex_lock(&server->clients_mutex);
    server->client_count--;
    pthread_mutex_unlock(&server->clients_mutex);

    free(arg);
    return NULL;
}

// TODO: add player to server_state_t
// when player enters his position should be replicated too

// TODO: spawn asteroids on server

#include "server.h"
#include <arpa/inet.h>
#include <logger.h>
#include <stdlib.h>
#include "client.h"
#include "settings.h"
#include "state.h"
#include "types.h"

// TODO: remove global state
server_state_t g_State = {
    .is_game_running = 0,
};

static void *handle_client(void *arg);

int main(void) {
    FILE *log_file = fopen("server_log", "w");
    // logger_set_output_file(log_file);

    server_t *server = sr_create_server(SERVER_PORT, handle_client);

    for (int i = 0; i < MAX_PLAYERS; i++) { g_State.players[i].client_id = -1; }

    if (!server) {
        LOG_ERROR("Failed to start server");
        fclose(log_file);
        return 1;
    }

    sr_start_listen(server);

    while (1) {
        if (!g_State.is_game_running) {
            continue;
        }
    }

    fclose(log_file);
    return 0;
}

void *handle_client(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    server_t *server = args->server;
    client_connection_t *con = &server->clients[args->client_index];

    LOG_INFO("Client %d connected from %s", con->client_id, inet_ntoa(con->addr.sin_addr));

    LOG_INFO("Sending player set player id message");

    server_message_t srv_msg = (server_message_t){
        .type = SERVER_MSG_PLAYER_ID_SET,
        .data.player_id = con->client_id,
    };

    sr_send_message_to_client(server, con->client_id, &srv_msg);

    srv_msg = (server_message_t){
        .type = SERVER_MSG_PLAYER_ENTERED,
        .data.player_id = con->client_id,
    };

    LOG_INFO("Sending player entered message");
    sr_send_message_to_all_except(server, con->client_id, &srv_msg);

    g_State.players[con->client_id].pos = SCREEN_CENTER;

    srv_msg = (server_message_t){
        .type = SERVER_MSG_PLAYER_POSITION,
        .data.player_pos_msg.player_id = con->client_id,
        .data.player_pos_msg.pos = g_State.players[con->client_id].pos,
    };

    LOG_INFO("Sending player initial position");
    sr_send_message_to_client(server, con->client_id, &srv_msg);

    srv_msg = (server_message_t){
        .type = SERVER_MSG_ACTIVE_PLAYER_IDS,
    };

    for (int i = 0; i < MAX_PLAYERS; i++) {
        srv_msg.data.active_players[i] = g_State.players[i].client_id;
    }

    LOG_INFO("Sending active player ids");
    sr_send_message_to_client(server, con->client_id, &srv_msg);

    if (server->client_count < PLAYERS_TO_START) {
        LOG_INFO("Clients count: %d", server->client_count);
        LOG_INFO("Sending not enough players message");

        srv_msg = (server_message_t){
            .type = SERVER_MSG_NOT_ENOUGH_PLAYERS,
            .data.needed_players = PLAYERS_TO_START - server->client_count,
        };

        sr_send_message_to_all(server, &srv_msg);
    } else {
        srv_msg = (server_message_t){
            .type = SERVER_MSG_GAME_START,
        };

        LOG_INFO("Sending game start message");
        sr_send_message_to_all(server, &srv_msg);

        g_State.is_game_running = 1;
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
                case CLIENT_MSG_PLAYER_POSITION: {
                    g_State.players[con->client_id].pos = cl_msg.data.position;

                    srv_msg = (server_message_t){
                        .type = SERVER_MSG_PLAYER_POSITION,
                        .data.player_pos_msg.player_id = con->client_id,
                        .data.player_pos_msg.pos = g_State.players[con->client_id].pos,
                    };

                    LOG_INFO("Sending player: %d position to clients", con->client_id);
                    sr_send_message_to_all_except(server, con->client_id, &srv_msg);
                    break;
                }

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

// FIX: still a small bug that other players inital position is not replciated on start

// FIX: by some reason on second player connect there is a copy of second player in top right corner

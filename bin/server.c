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

    if (!server) {
        LOG_ERROR("Failed to start server");
        fclose(log_file);
        return 1;
    }

    sr_start_listen(server);

    server_message_t srv_msg;

    while (1) {
        if (!g_State.is_game_running) {
            continue;
        }

        for (int i = 0; i < MAX_PLAYERS; i++) {
            server_player_t *player = &g_State.players[i];

            if (i + 1 > (int)server->client_count) {
                continue;
            }

            srv_msg = (server_message_t){
                .type = SERVER_MSG_PLAYER_POSITION,
                .data.player_pos_msg.player_id = i,
                .data.player_pos_msg.pos = player->pos,
            };

            LOG_INFO("Sending player: %d position to clients", i);
            sr_send_message_to_all_except(server, i, &srv_msg);
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

    srv_msg = (server_message_t){
        .type = SERVER_MSG_PLAYER_ENTERED,
        .data.player_id = con->client_id,
    };

    LOG_INFO("Sending player entered message");
    sr_send_message_to_client(server, con->client_id, &srv_msg);

    g_State.players[con->client_id].pos = SCREEN_CENTER;

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
                    // TODO: receive player position from client and set
                    // g_State.players[con->id].position = pos it will automaticly get replicated
                    // (that logic is in main while loop inside main function)

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

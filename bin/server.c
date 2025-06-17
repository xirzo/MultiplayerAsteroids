#include "server.h"
#include <arpa/inet.h>
#include <logger.h>
#include <stdlib.h>
#include "client.h"
#include "state.h"

// TODO: remove global state
server_state_t g_State = {
    .players_count = 0,
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

    while (1);

    fclose(log_file);
    return 0;
}

void *handle_client(void *arg) {
    thread_args_t *args = (thread_args_t *)arg;
    server_t *server = args->server;
    client_connection_t *con = &server->clients[args->client_index];

    g_State.players_count++;

    LOG_INFO("Client %d connected from %s", con->client_id, inet_ntoa(con->addr.sin_addr));

    server_message_t connection_msg = {
        .type = SERVER_MSG_PLAYER_ENTERED,
        .data.player_id = con->client_id,
    };

    LOG_INFO("Sending player entered message");
    sr_send_message_to_all_except(server, con->client_id, &connection_msg);

    while (con->active) {
        client_message_t msg;
        ssize_t bytes_read = read(con->socket_fd, &msg, sizeof(client_message_t));

        if (bytes_read <= 0) {
            LOG_INFO("Client %d disconnected\n", con->client_id);
            break;
        }

        if (bytes_read == sizeof(client_message_t)) {
            switch (msg.type) {
                /*

                Fill in

                */
                default: {
                    LOG_ERROR("Unknown message type %d from client %d\n", msg.type, con->client_id);
                    break;
                }
            }
        }
    }

    close(con->socket_fd);
    con->active = 0;
    g_State.players_count--;

    pthread_mutex_lock(&server->clients_mutex);
    server->client_count--;
    pthread_mutex_unlock(&server->clients_mutex);

    free(arg);
    return NULL;
}

// TODO: when connecting clients wait for lobby to fill

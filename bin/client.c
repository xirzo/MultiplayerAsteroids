#include "client.h"
#include <logger.h>
#include <stdio.h>
#include "server.h"
#include "settings.h"
#include "state.h"

static void receive_server_message(client_state_t *state);

int main(void) {
    FILE *log_file = fopen("client_log", "w");
    // logger_set_output_file(log_file);

    client_state_t state = {
        .is_running = 1,
        .is_connected = 0,
        .is_game_running = 0,
        .player_id = -1,
    };

    LOG_INFO("Connecting to server");

    if ((sr_client_connect(&state.client, SERVER_IP, SERVER_PORT)) != 0) {
        LOG_ERROR("Failed connecting to server");
        fclose(log_file);
        return 1;
    }

    state.is_connected = 1;

    while (state.is_running) {
        receive_server_message(&state);

        if (!state.is_game_running) {
            continue;
        }
    }

    fclose(log_file);
    return 0;
}

void receive_server_message(client_state_t *state) {
    server_message_t msg;

    int res = sr_receive_server_message(&state->client, &msg);

    if (res == -3) {
        usleep(10000);
        return;
    } else if (res < 0) {
        LOG_INFO("Disconnecting...");
        state->is_connected = 0;
        state->is_running = 0;
        state->is_game_running = 0;
        return;
    }

    switch (msg.type) {
        case SERVER_MSG_PLAYER_ID_SET: {
            LOG_INFO("Setting player id: %d", msg.data.player_id);
            state->player_id = msg.data.player_id;
            break;
        }
        case SERVER_MSG_PLAYER_ENTERED: {
            LOG_INFO("Another player entered with player id: %d", msg.data.player_id);
            break;
        }
        case SERVER_MSG_NOT_ENOUGH_PLAYERS: {
            LOG_INFO(
                "Not enough players to start, still need %d more players", msg.data.needed_players
            );
            break;
        }
        case SERVER_MSG_GAME_START: {
            LOG_INFO("Game start");
            state->is_game_running = 1;
            break;
        }
        case SERVER_MSG_PLAYER_POSITION: {
            player_posititon_message_t *pos_msg = &msg.data.player_pos_msg;

            if (pos_msg->player_id == state->player_id) {
                // TODO: set player initial positon

                LOG_INFO(
                    "TODO: IMPLEMENT | Set initial player position: x: %.1f, y: %.1f",
                    pos_msg->pos.x,
                    pos_msg->pos.y
                );
                break;
            }

            LOG_INFO(
                "TODO: IMPLEMENT | Received player: %d position: x: %.1f, y: %.1f",
                pos_msg->player_id,
                pos_msg->pos.x,
                pos_msg->pos.y
            );
            break;
        }
        default: {
            LOG_ERROR("Unknown server message type");
            break;
        }
    }
}

// TODO: ? console with events

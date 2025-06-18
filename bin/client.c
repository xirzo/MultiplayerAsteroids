#include "client.h"
#include <pthread.h>
#include <logger.h>
#include <raylib.h>
#include <unistd.h>
#include "render.h"
#include "settings.h"
#include "state.h"
#include "types.h"
#include "vec2.h"
#include "core.h"

static void receive_server_message(client_state_t *state);
static void send_server_message(client_state_t *state);

int main(void) {
    client_state_t state = { 0 };
    client_state_init(&state);

    LOG_INFO("Connecting to server");

    if ((sr_client_connect(&state.client, SERVER_IP, SERVER_PORT)) != 0) {
        LOG_ERROR("Failed connecting to server");
        return 1;
    }

    state.is_connected = 1;

    LOG_INFO("Starting raylib window");

    init_window();

    LOG_INFO("Starting game loop");

    while (!WindowShouldClose()) {
        receive_server_message(&state);

        if (state.is_game_running) {
            process_input_and_move_player(&state);
            send_server_message(&state);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        render_frame(&state);
        EndDrawing();
    }

    CloseWindow();
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
            state->players[msg.data.player_id].is_active = 1;
            break;
        }
        case SERVER_MSG_ACTIVE_PLAYER_IDS: {
            LOG_INFO("Got active players list", msg.data.needed_players);

            for (int i = 0; i < MAX_PLAYERS; i++) {
                state->players[i].is_active = msg.data.active_players[i];
            }
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
                LOG_INFO(
                    "Set initial player position: x: %.1f, y: %.1f", pos_msg->pos.x, pos_msg->pos.y
                );
                state->players[state->player_id].pos = pos_msg->pos;
                break;
            }

            LOG_INFO(
                "Received player: %d position: x: %.1f, y: %.1f",
                pos_msg->player_id,
                pos_msg->pos.x,
                pos_msg->pos.y
            );
            state->players[pos_msg->player_id].pos = pos_msg->pos;
            break;
        }

        case SERVER_MSG_PLAYER_DISCONNECT: {
            LOG_INFO("Player %d disconnected", msg.data.player_id);
            state->players[msg.data.player_id].is_active = 0;
            break;
        }
        default: {
            LOG_ERROR("Unknown server message type");
            break;
        }
    }
}

void send_server_message(client_state_t *state) {
    client_message_t msg = { .type = CLIENT_MSG_PLAYER_POSITION,
                             .data.position = state->players[state->player_id].pos };

    LOG_INFO("Sending own position to server");

    if ((sr_send_message_to_server(&state->client, &msg)) != 0) {
        LOG_ERROR("Failed to send position to server");
        return;
    }
}

// TODO: ? console with events

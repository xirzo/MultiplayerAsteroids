#include "core.h"
#include <raylib.h>
#include <logger.h>
#include <unistd.h>

void client_state_init(client_state_t *state) {
    state->is_connected = 0;
    state->is_game_running = 0;
    state->player_id = -1;

    for (int i = 0; i < MAX_PLAYERS; i++) {
        client_player_t *player = &state->players[i];

        player->is_active = 0;
        player->size = (vec2){ 80, 80 };
        player->color = (color_t
        ){ GetRandomValue(0, 255), GetRandomValue(0, 255), GetRandomValue(0, 255), 255 };
        player->velocity = VEC_2_ZERO;
        player->speed = PLAYER_SPEED;
    }

    SetTraceLogLevel(LOG_NONE);
}

void process_input_and_move_player(client_state_t *state) {
    client_player_t *player = &state->players[state->player_id];

    player->velocity = VEC_2_ZERO;

    if (IsKeyDown(KEY_A)) {
        player->velocity.x = -1;
    }
    if (IsKeyDown(KEY_D)) {
        player->velocity.x = 1;
    }
    if (IsKeyDown(KEY_W)) {
        player->velocity.y = -1;
    }
    if (IsKeyDown(KEY_S)) {
        player->velocity.y = 1;
    }

    vec2_normalize(&player->velocity);
    vec2_multiply_scalar(&player->velocity, player->speed * GetFrameTime());
    vec2_add(&player->pos, &player->velocity, &player->pos);
}

int connect_to_server(client_state_t *state) {
    LOG_INFO("Connecting to server");
    if ((sr_client_connect(&state->client, SERVER_IP, SERVER_PORT)) != 0) {
        LOG_ERROR("Failed connecting to server");
        return -1;
    }

    state->is_connected = 1;
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
